/*
    Copyright (c) 2013 Boldizsár Lipka <lipkab@zoho.com>

    This file is part of the Apertium Visual Rule Editor.

    Apertium Visual Rule Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Apertium Visual Rule Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Apertium Visual Rule Editor.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "node.h"
#include "config.h"
#include <QDebug>
#include <QVector>

Property::Property(const QString &fullName, const QString &value)
    : name_()
    , prefix_()
    , value_(value)
{
    int spos = fullName.lastIndexOf("/");
    if (spos != -1) {
        name_ = fullName.right(fullName.size() - spos - 1);
        prefix_ = fullName.left(spos);
    }
}

Property::Property()
    : name_()
    , prefix_()
    , value_()
{}

void Property::setFullName(const QString &str)
{
    if (str == (prefix_ + "/" + name_)) {
        return;
    }

    int spos = str.lastIndexOf("/");
    if (spos != -1) {
        name_ = str.right(str.size() - spos - 1);
        prefix_ = str.left(spos);
    }

    emit fullNameChanged(str);
}

void Property::setName(const QString &str)
{
    if (str == name_) {
        return;
    }
    name_ = str;

    emit fullNameChanged(prefix_ + "/" + name_);
}

int Property::valueToInt(int def, int min, int max) const
{
    bool ok;
    int i = value_.toInt(&ok, 10);
    if (max > min) {
        i = i < min ? min : i;
        i = i > max ? max : i;
    }

    if (ok) {
        return i;
    } else {
        return def;
    }
}

void Property::setValue(const QString &str)
{
    if (str == value_) {
        return;
    }
    value_ = str;
    emit valueChanged(str);
}

namespace
{
struct Attr {
    Attr(int p = 1, const QString& n = "", const QString& v = "") : pos(p), name(n), lit(v), isVar(false) {}
    int pos;
    QString name;
    QString lit;
    bool isVar;
};

struct Var {
    Var(const QString& n, int po, const QString& pa) : name(n), pos(po), part(pa) {}
    Var(const QString& n, const QString& l, bool t = false) : name(n), pos(-1), part(""), lit(l), tag(t) {}
    QString name;
    int pos;
    QString part;
    QString lit;
    bool tag;
};

struct Word {
    Word(int p = -1, bool clip = true) : pos(p), clipWhole(clip), lit(""), isVar(false) {}
    Word(const QString& lem) : pos(-1), clipWhole(true), lit(lem), isVar(false) {}
    int pos;
    bool clipWhole;
    QString lit;
    bool isVar;
    QList<Attr> attrs;
};
}

class NodeXmlHandler : public QXmlDefaultHandler
{
public:
    NodeXmlHandler(const QString& file)
        : QXmlDefaultHandler()
        , file_(file)
        , stack_()
        , root_(new RootNode())
        , modeStack_()
        , patternSize_(0)
        , words_()
        , cond_(NULL)
        , symIndProp_()
        , symIndTag_()
        , attrs_(NULL)
        , noLemYet_(false)
        , noActionYet_(true)
    {
        modeStack_.append(NORMAL);
        stack_.append(root_);
    }

    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    bool comment(const QString &ch);

    RootNode* root() const { return root_; }

private:    
    static QString stripIgnorableWS(const QString &str);
    int mapIndex(int i) const;

    Node *buildSequence();

    enum Mode {
        NORMAL,
        ACTION,
        SYMBOL_DIRECT,
        SYMBOL_INDIRECT,
        VARIABLE,
        CONDITION
    };

    QString file_;
    QList<Node*> stack_;
    RootNode* root_;
    QList<Mode> modeStack_;
    int patternSize_;
    QVector<Word> words_;
    QList<Var> vars_;
    Node* cond_;
    QString symIndProp_;
    QString symIndTag_;
    Node* attrs_;
    bool noLemYet_;
    bool noActionYet_;
};


bool NodeXmlHandler::startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &atts)
{
    Node* n;

    if (qName == "action" || qName == "when" || qName == "otherwise") {
        modeStack_.append(ACTION);
        words_.clear();
        vars_.clear();
    } else if (qName == "cat-item") {
        modeStack_.append(SYMBOL_DIRECT);
    } else if (qName == "choose") {
        modeStack_.append(NORMAL);
    }

    if (modeStack_.back() == NORMAL) {
        if (root_->name().isEmpty()) {
            root_->setName(qName);
            root_->setFilePath(file_);
            n = root_;
        } else {
            n = Node::create(qName, false, stack_.back());
            stack_.back()->addChild(n);
            stack_.append(n);
        }
        for (int i = 0; i<atts.count(); i++) {
            Property *prop = new Property(n->name() + "/" + atts.qName(i), atts.value(i));
            n->addProperty(prop);
        }

        if (qName == "pattern") {
            patternSize_ = 0;
        } else if (qName == "pattern-item") {
            patternSize_++;
        } else if (qName == "section-def-attrs") {
            attrs_ = n;
        } else if (qName == "rule") {
            noActionYet_ = true;
        } else if (qName == "choose") {
            noActionYet_ = false;
        }

    } else if (modeStack_.back() == ACTION){
        if (qName == "lu") {
            words_.append(Word());
            noLemYet_ = true;
        } else if (qName == "clip") {
            QString part = atts.value("part");
            int pos = atts.value("pos").toInt();

            if (part == "whole" || part == "lem") {
                noLemYet_ = false;
                words_.back().pos = pos;
                if (part == "lem") {
                    words_.back().clipWhole = false;
                }
            } else {
                words_.back().attrs.append(Attr(pos, part));
            }
        } else if (qName == "lit") {
            QString lem = atts.value("v");
            words_.back().pos = -1;
            words_.back().lit = lem;
            words_.back().clipWhole = true;
        } else if (qName == "lit-tag") {
            QString tag = atts.value("v");
            QString aname;
            foreach (Node* n, attrs_->children()) {
                foreach (Node* k, n->children()) {
                    if (k->name() == ("__symbol_" + tag)) {
                        aname = n->property("n")->value();
                    }
                }
            }
            words_.back().attrs.append(Attr(-1, aname, tag));
        } else if (qName == "var") {
            QString name = atts.value("n");
            if (noLemYet_) {
                words_.back().lit = name;
                words_.back().pos = -1;
                words_.back().isVar = true;
            } else {
                Attr a(-1, name);
                a.isVar = true;
                words_.back().attrs.append(a);
            }
        } else if (qName == "let") {
            vars_.append(Var("", ""));
            modeStack_.append(VARIABLE);
        }
    } else if (modeStack_.back() == SYMBOL_DIRECT) {
        n = Node::create(qName);
        stack_.back()->addChild(n);
        stack_.append(n);

        VisualSchema::Tag tdef = appConfig().tag(qName);
        QStringList syms = atts.value(tdef.symProp).split(".");
        foreach (const QString& sym, syms) {
            n->addChild(Node::create("__symbol_" + sym));
        }
    } else if (modeStack_.back() == SYMBOL_INDIRECT) {
        if (qName == symIndTag_) {
            QString sym = "__symbol_" + atts.value(symIndProp_);
            n = Node::create(sym);
            stack_.back()->addChild(n);
        }
    } else if (modeStack_.back() == VARIABLE) {
        if (qName == "var") {
            vars_.back().name = atts.value("n");
        } else if (qName == "clip") {
            vars_.back().pos = atts.value("pos").toInt();
            vars_.back().part = atts.value("part");
        } else if (qName == "lit") {
            vars_.back().lit = atts.value("v");
            vars_.back().pos = -1;
            vars_.back().tag = false;
        } else if (qName == "lit-tag") {
            vars_.back().lit = atts.value("v");
            vars_.back().pos = -1;
            vars_.back().tag = true;
        }
    } else if (modeStack_.back() == CONDITION) {
        Node* cond = stack_.back();
        QString affix = cond->children().size() == 1 ? "2" : "1";

        if (qName == "clip") {
            cond->addProperty(new Property(cond->name() + "/pos" + affix, atts.value("pos")));
            cond->addProperty(new Property(cond->name() + "/part" + affix, atts.value("part")));
        } else if (qName == "lit") {
            cond->addProperty(new Property(cond->name() + "/lit", atts.value("v")));
        } else if (qName == "lit-tag") {
            cond->addProperty(new Property(cond->name() + "/lit-tag", atts.value("v")));
        }
    }

    if (qName == "def-attr") {
        VisualSchema::Tag tdef = appConfig().tag(qName);
        symIndProp_ = tdef.symProp;
        symIndTag_ = tdef.symIndProxy;
        modeStack_.append(SYMBOL_INDIRECT);
    }  else if (qName == "test") {
        cond_ = Node::create("test");
        stack_.append(cond_);
        modeStack_.append(NORMAL);
    } else if (qName == "equal" || qName == "begins-with" || qName == "ends-with") {
        modeStack_.append(CONDITION);
    }
    return true;
}

bool NodeXmlHandler::endElement(const QString &, const QString &, const QString &qName)
{

    if (qName == "rule") {
        if (noActionYet_) {
            stack_.back()->addChild(buildSequence());
        }
    } else if (qName == "action") {
        modeStack_.removeLast();
        return true;
    } else if (qName == "cat-item" || qName == "def-attr") {
        modeStack_.removeLast();
    } else if (qName == "let") {
        modeStack_.removeLast();
    } else if (qName == "test") {
        modeStack_.removeLast();
        stack_.removeLast();
    } else if (qName == "when") {
        Node* when = Node::create("when");
        stack_.back()->addChild(when);
        when->addChild(buildSequence());
        if (cond_ != NULL) {
            foreach (Node* ch, cond_->children()) {
                when->addChild(Node::clone(ch));
            }
            cond_->deleteLater();
        }
        modeStack_.removeLast();
        return true;
    } else if (qName == "otherwise") {
        Node* ot = Node::create("otherwise");
        stack_.back()->addChild(ot);
        ot->addChild(buildSequence());
        modeStack_.removeLast();
        return true;
    } else if (qName == "equal" || qName == "begins-with" || qName == "ends-with") {
        modeStack_.removeLast();
    }

    if (modeStack_.back() == NORMAL) {
        stack_.removeLast();
    }

    if (qName == "choose") {
        modeStack_.removeLast();
    }

    return true;
}

bool NodeXmlHandler::characters(const QString &ch)
{
    stack_.back()->appendCData(stripIgnorableWS(ch));
    return true;
}

bool NodeXmlHandler::comment(const QString &ch)
{
    if (ch.startsWith("[visruled settings]")) {
        int begin = ch.indexOf("sldict: ") + QString("sldict: ").size();
        int end = ch.indexOf("\n", begin);
        fileConfig(file_).setSlDictPath(ch.mid(begin, end - begin));

        begin = ch.indexOf("tldict: ") + QString("tldict: ").size();
        end = ch.indexOf("\n", begin);
        fileConfig(file_).setTlDictPath(ch.mid(begin, end - begin));

        begin = ch.indexOf("bidict: ") + QString("bidict: ").size();
        end = ch.indexOf("\n", begin);
        fileConfig(file_).setBiDictPath(ch.mid(begin, end - begin));
    }

    return true;
}

QString NodeXmlHandler::stripIgnorableWS(const QString &str)
{
    QString res = str;
    for (int pos = 0; pos<res.size(); pos++) {
        if (res[pos] != '\n') continue;
        while (res[pos].isSpace()) {
            res.remove(pos,1);
        }
    }

    return res;
}

int NodeXmlHandler::mapIndex(int i) const
{
    if (i < 0) {
        return i;
    }

    for (int j = 0; j<words_.size(); j++) {
        if (words_[j].pos < 0) {
            i++;
        }

        if (i == j) {
            return i;
        }
    }

    return i;
}

Node *NodeXmlHandler::buildSequence()
{
    Node* res = Node::create("action");

    //removals
    {
        QList<int> missingIndices;
        for (int i = 0; i<patternSize_; i++) {
            foreach (const Word& w, words_) {
                if (i == w.pos - 1) {
                    goto next;
                }
            }

            missingIndices.append(i);

            next:
            continue;
        }

        foreach (int i, missingIndices) {
            words_.insert(mapIndex(i), Word(i+1));
            Node* rm = Node::create("__action_remove");
            rm->addProperty(new Property("__action_remove/pos", QString::number(mapIndex(i)+1)));
            res->insertChild(rm, 0);
        }
    }

    //swaps
    {
        bool repeat = true;
        while (repeat) {
            repeat = false;
            for (int i = 0; i<words_.size(); i++) {
                const int pos = mapIndex(words_[i].pos - 1);
                if (pos != i && pos > 0) {
                    repeat = true;
                    Word tmp = words_[pos];
                    words_[pos] = words_[i];
                    words_[i] = tmp;
                    Node* swap = Node::create("__action_swap");
                    swap->addProperty(new Property("__action_swap/pos1", QString::number(i+1)));
                    swap->addProperty(new Property("__action_swap/pos2", QString::number(pos+1)));
                    res->insertChild(swap, 0);
                }
            }
        }
    }

    //reorders
    {
        for (int i = 0; i<words_.size(); i++) {
            if (!words_[i].clipWhole) {
                QString order;
                foreach (const Attr& a, words_[i].attrs) {
                    order += a.name + ",";
                }
                order.remove(order.size()-1, 1);
                Node* reorder = Node::create("__action_attrs");
                reorder->addProperty(new Property("__action_attrs/pos", QString::number(mapIndex(words_[i].pos))));
                reorder->addProperty(new Property("__action_attrs/order", order));
                res->insertChild(reorder,0);
            }
        }
    }

    //agreements
    {
        for (int i = 0; i<words_.size(); i++) {
            foreach (const Attr& a, words_[i].attrs) {
                const int apos = mapIndex(a.pos) - 1;
                const int wpos = mapIndex(words_[i].pos) - 1;
                if (apos != wpos && apos > 0) {
                    Node* agree = Node::create("__action_agree");
                    agree->addProperty(new Property("__action_agree/src", QString::number(apos+1)));
                    agree->addProperty(new Property("__action_agree/trg", QString::number(wpos+1)));
                    agree->addProperty(new Property("__action_agree/attr", a.name));
                    res->insertChild(agree,0);
                }
            }
        }
    }

    //tag insertions
    {
        for (int i = 0; i<words_.size(); i++) {
            for (int j = 0; j < words_[i].attrs.size(); j++) {
                const Attr& a = words_[i].attrs[j];
                if (a.pos < 0) {
                    Node* ins = Node::create("__action_insert_tag");
                    ins->addProperty(new Property("__action_insert_tag/pos", QString::number(i+1)));
                    if (!a.isVar) {
                        ins->addProperty(new Property("__action_insert_word/tag", a.lit));
                    } else {
                        ins->addProperty(new Property("__action_insert_tag/var", a.name));
                    }
                    res->insertChild(ins, 0);
                    words_[i].attrs.removeAt(j);
                    j--;
                }
            }
        }
    }

    //insertions
    {
        for (int i = 0; i<words_.size(); i++) {
            if (words_[i].pos < 0) {
                Node* ins = Node::create("__action_insert_word");
                ins->addProperty(new Property("__action_insert_word/pos", QString::number(i+1)));
                if (!words_[i].isVar) {
                    ins->addProperty(new Property("__action_insert_word/lem", words_[i].lit));
                } else {
                    ins->addProperty(new Property("__action_insert_word/var", words_[i].lit));
                }
                res->insertChild(ins, 0);
                words_.remove(i);
                i--;
            }
        }
    }

    //variables
    {
        foreach (const Var& v, vars_) {
            Node* var = Node::create("__action_assign_var");
            var->addProperty(new Property("__action_assign_var/name", v.name));
            if (v.pos < 0) {
                if (v.tag) {
                    var->addProperty(new Property("__action_assign_var/lit-tag", v.lit));
                } else {
                    var->addProperty(new Property("__action_assign_var/lit", v.lit));
                }
            } else {
                var->addProperty(new Property("__action_assign_var/pos", QString::number(v.pos)));
                var->addProperty(new Property("__action_assign_var/part", v.part));
            }
            res->insertChild(var, 0);
        }
    }

    return res;
}

RootNode* readXmlIntoNode(const QString &path)
{
    QFile file(path);
    QXmlInputSource xml(&file);

    QXmlSimpleReader reader;
    NodeXmlHandler handler(path);
    reader.setContentHandler(&handler);
    reader.setLexicalHandler(&handler);
    reader.parse(&xml);

    return handler.root();
}

Node::~Node()
{
    foreach(Node* ch, children_) {
        delete ch;
    }
}

Property *Node::property(const QString &name) const
{
    foreach(Property* prop, properties_) {
        if (name == prop->name()) {
            return prop;
        }
    }

    return NULL;
}

Node *Node::child(const QString &name) const
{
    foreach (Node* n, children_) {
        if (n->name() == name) {
            return n;
        }
    }

    return NULL;
}

void Node::insertChild(Node *ch, int index)
{
    if (!children_.contains(ch)) {
        ch->setParentNode(this);
        children_.insert(index, ch);
        emit childInserted(ch, index);
    }
}

QString Node::toXml() const
{
    QString res;
    res += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    res += toXmlIndented(0);

    return res;
}

QString Node::toXmlIndented(int level) const
{
    return toXmlIndentedDefault(this, level);
}

QString Node::toXmlIndentedDefault(const Node *n, int level)
{
    QString res;
    QString indent;
    for (int i = level; i>0; i--) {
        indent += "  ";
    }
    res += indent + "<" + n->name_;
    foreach (Property* prop, n->properties_) {
        res += " " + prop->name() + "=\"" + prop->value() + "\"";
    }
    if (n->cdata_.isEmpty() && n->children_.isEmpty()) {
        res += " />\n";
        return res;
    } else {
        res += ">\n";
    }

    if (!n->cdata_.isEmpty()) {
        res += indent + "  " + n->cdata_ + '\n';
    }

    foreach (Node* ch, n->children_) {
        res += ch->toXmlIndented(level+1);
    }
    res += indent + "</" + n->name_ + ">\n";

    return res;
}


QString RootNode::toXmlIndented(int level) const
{
    QString res;
    QString indent;
    for (int i = level; i>0; i--) {
        indent += "  ";
    }

    res += indent + "<!--[visruled settings] - do not modify manually!\n";
    res += indent + "     sldict: " + fileConfig(filePath_).slDictPath() + "\n";
    res += indent + "     tldict: " + fileConfig(filePath_).tlDictPath() + "\n";
    res += indent + "     bidict: " + fileConfig(filePath_).biDictPath() + "\n -->\n";

    res += Node::toXmlIndented(level);
    return res;
}


ActionNode::ActionNode(const QString &name, Node *parent)
    : Node(name, parent)
    , pattern_(NULL)
    , parentRule_(NULL)
    , attrs_(NULL)
{
    setParentNode(parent);
}

void ActionNode::trySetPattern(Node *n)
{
    if (n->name() != "pattern") {
        return;
    }

    pattern_ = n;
}

void ActionNode::tryRemovePattern(Node *n)
{
    if (n == pattern_) {
        pattern_ = NULL;
    }
}

Node* ActionNode::resolveSequence() const
{
    if (pattern_ == NULL) {
        return create("out");
    }

    QVector<Word> words(pattern_->children().size());
    QList<Var> vars;
    for (int i = 0; i<words.size(); i++) {
        words[i].pos = i+1;
        words[i].clipWhole = true;
    }
    foreach (Node* n, children()) {
        if (n->name() == "__action_swap") {
            Property* ppos1 = n->property("pos1");
            Property* ppos2 = n->property("pos2");
            if (ppos1 == NULL || ppos2 == NULL) {
                continue;
            }

            int pos1 = ppos1->valueToInt(1, 1, words.size()) - 1;
            int pos2 = ppos2->valueToInt(1, 1, words.size()) - 1;
            Word tmp = words[pos2];
            words[pos2] = words[pos1];
            words[pos1] = tmp;
        } else if (n->name() == "__action_agree") {
            Property* ppos1 = n->property("src");
            Property* ppos2 = n->property("trg");
            Property* pattr = n->property("attr");
            if (ppos1 == NULL || ppos2 == NULL || pattr == NULL) {
                continue;
            }

            int pos1 = ppos1->valueToInt(1, 1, words.size()) - 1;
            int pos2 = ppos2->valueToInt(1, 1, words.size()) - 1;
            QString attr = pattr->value();
            for (int i = 0; i<words[pos2].attrs.size(); i++) {
                if (words[pos2].attrs[i].name == attr) {
                    words[pos2].attrs.removeAt(i);
                }
            }
            words[pos2].attrs.append(Attr(words[pos1].pos, attr));
        } else if (n->name() == "__action_remove") {
            Property* ppos = n->property("pos");
            if (ppos == NULL) {
                continue;
            }

            int pos = ppos->valueToInt(1, 1, words.size()) - 1;
            words.remove(pos);
        } else if (n->name() == "__action_attrs") {
            Property* ppos = n->property("pos");
            Property* pord = n->property("order");
            if (ppos == NULL || pord == NULL) {
                continue;
            }

            int pos = ppos->valueToInt(1, 1, words.size()) -1;
            QStringList order = pord->value().split(",");
            QList<Attr> attrs;
            foreach (const QString& str, order) {
                foreach (const Attr& a, words[pos].attrs) {
                    if (a.name == str) {
                        attrs.append(a);
                        goto next;
                    }
                }

                attrs.append(Attr(words[pos].pos, str));

                next:
                continue;
            }
            words[pos].attrs = attrs;
            words[pos].clipWhole = false;
        } else if (n->name() == "__action_insert_word") {
            Property* ppos = n->property("pos");
            Property* plem = n->property("lem");
            Property* pvar = n->property("var");
            if (ppos == NULL) {
                continue;
            } else if (plem != NULL) {
                int pos = ppos->valueToInt(1, 1, words.size()+1)-1;
                QString lem = plem->value();
                words.insert(pos, Word(lem));
            } else if (pvar != NULL) {
                int pos = ppos->valueToInt(1, 1, words.size()+1)-1;
                QString var = pvar->value();
                Word w(var);
                w.isVar = true;
                words.insert(pos, w);
            }
        } else if (n->name() == "__action_insert_tag") {
            Property* ppos = n->property("pos");
            Property* ptag = n->property("tag");
            Property* pvar = n->property("var");
            if (ppos == NULL) {
                continue;
            } else if (ptag != NULL) {
                int pos = ppos->valueToInt(1, 1, words.size()+1)-1;
                QString tag = ptag->value();
                foreach (Node* adef, attrs_->children()) {
                    foreach (Node* a, adef->children()) {
                        if (a->name() == ("__symbol_" + tag)) {
                            words[pos].attrs.append(Attr(-1, adef->property("n")->value(), tag));
                        }
                    }
                }
            } else if (pvar != NULL) {
                int pos = ppos->valueToInt(1, 1, words.size()+1)-1;
                QString var = pvar->value();
                Attr a(-1, var);
                a.isVar = true;
                words[pos].attrs.append(a);
            }
        } else if (n->name() == "__action_assign_var") {
            Property* pname = n->property("name");
            Property* plit = n->property("lit");
            Property* plittag = n->property("lit-tag");
            Property* ppos = n->property("pos");
            Property* ppart = n->property("part");
            if (pname == NULL) {
                continue;
            } else if (plit != NULL) {
                vars.append(Var(pname->value(), plit->value()));
            } else if (plittag != NULL) {
                vars.append(Var(pname->value(), plittag->value(), true));
            } else if (ppos != NULL && ppart != NULL) {
                int pos = ppos->valueToInt(1, 1, words.size()+1)-1;
                QString part = ppart->value();
                QString name = pname->value();

                if (part == "whole") {
                    continue;
                } if (part == "lem") {
                    vars.append(Var(name, words[pos].pos, part));
                } else {
                    foreach (const Attr& a, words[pos].attrs) {
                        if (a.name == part) {
                            vars.append(Var(name, a.pos, part));
                            goto end;
                        }
                    }
                    vars.append(Var(name, words[pos].pos, part));

                    end:
                    continue;
                }
            }
        }
    }

    Node* action = create("dummy");
    action->setName(name());
    Node* out = create("out");
    foreach (const Var& v, vars) {
        Node* let = create("let");
        Node* var = create("var");
        var->addProperty(new Property("var/n", v.name));
        let->addChild(var);

        if (v.pos < 0) {
            if (v.tag) {
                Node* littag = create("lit-tag");
                littag->addProperty(new Property("lit-tag/v", v.lit));
                let->addChild(littag);
            } else {
                Node* lit = create("lit");
                lit->addProperty(new Property("lit/v", v.lit));
                let->addChild(lit);
            }
        } else {
            Node* clip = create("clip");
            clip->addProperty(new Property("clip/side", "tl"));
            clip->addProperty(new Property("clip/pos", QString::number(v.pos)));
            clip->addProperty(new Property("clip/part", v.part));
            let->addChild(clip);
        }
        action->addChild(let);
    }

    foreach (const Word& w, words) {
        Node* lu = create("lu");
        if (w.isVar) {
            Node* var = create("var");
            var->addProperty(new Property("var/n", w.lit));
            lu->addChild(var);
        } else if (w.pos != -1) {
            Node* lem = create("clip");
            lem->addProperty(new Property("clip/pos", QString::number(w.pos)));
            lem->addProperty(new Property("clip/side", "tl"));
            lem->addProperty(new Property("clip/part", w.clipWhole ? "whole" : "lem"));
            lu->addChild(lem);
        } else {
            Node* lem = create("lit");
            lem->addProperty(new Property("lit/v", w.lit));
            lu->addChild(lem);
        }

        foreach (const Attr& a, w.attrs) {
            if (a.isVar) {
                Node* var = create("var");
                var->addProperty(new Property("var/n", a.name));
                lu->addChild(var);
            } else if (a.pos >= 0) {
                Node* attr = create("clip");
                attr->addProperty(new Property("clip/pos", QString::number(a.pos)));
                attr->addProperty(new Property("clip/side", "tl"));
                attr->addProperty(new Property("clip/part", a.name));
                lu->addChild(attr);
            } else {
                Node* attr = create("lit-tag");
                attr->addProperty(new Property("lit-tag/v", a.lit));
                lu->addChild(attr);
            }
        }
        out->addChild(lu);
        out->addChild(create("b"));
    }
    out->children().removeLast();
    action->addChild(out);

    return action;
}

void ActionNode::setParentNode(Node *n)
{
    pattern_ = NULL;

    Node* par = parentNode();

    while (par != NULL) {
        if (par->name() == "rule") {
            disconnect(par, SIGNAL(childInserted(Node*,int)), this, SLOT(trySetPattern(Node*)));
            disconnect(par, SIGNAL(childRemoved(Node*)), this, SLOT(tryRemovePattern(Node*)));
            break;
        } else {
            par = par->parentNode();
        }
    }

    par = n;
    while (par != NULL) {
        if (par->name() == "rule") {
            connect(par, SIGNAL(childInserted(Node*,int)), this, SLOT(trySetPattern(Node*)));
            connect(par, SIGNAL(childRemoved(Node*)), this, SLOT(tryRemovePattern(Node*)));
            foreach (Node* nd, par->children()) {
                trySetPattern(nd);
            }

            Node* root = par;
            while (root->parentNode() != NULL) {
                root = root->parentNode();
            }
            attrs_ = root->child("section-def-attrs");
            break;
        } else {
            par = par->parentNode();
        }
    }

    Node::setParentNode(n);
}

QString ActionNode::toXmlIndented(int level) const
{
    Node* action = resolveSequence();
    QString res = Node::toXmlIndentedDefault(action, level);
    action->deleteLater();

    return res;
}


Node *Node::create(const QString &name, bool addMandProps, Node *parent)
{
    VisualSchema::Tag tdef = appConfig().tag(name);
    Node* res;
    switch (tdef.type) {
    case nodetype::STANDARD:
        res = new Node(name, parent);
        break;
    case nodetype::ACTION:
        res = new ActionNode(name, parent);
        break;
    case nodetype::SYMBOL_CONTAINER_DIRECT:
        res = new DirectSymbolContainerNode(name, tdef.symProp, parent);
        break;
    case nodetype::SYMBOL_CONTAINER_INDIRECT:
        res = new IndirectSymbolContainerNode(name, tdef.symIndProxy, tdef.symProp, parent);
        break;
    case nodetype::CONDITION:
        res = new ConditionNode(name, parent);
        break;
    case nodetype::WHEN:
        res = new WhenNode(name, parent);
        break;
    case nodetype::CHOOSE:
        res = new ChooseNode(name, parent);
    }

    if (addMandProps) {
        foreach (QString prop, tdef.properties) {
            VisualSchema::Property pdef = appConfig().property(prop);
            if (pdef.isMandatory) {
                res->addProperty(new Property(prop, ""));
            }
        }
    }

    return res;
}

Node *Node::clone(Node *n)
{
    Node* res = create(n->name());
    foreach (Property* p, n->properties()) {
        res->addProperty(new Property(p->fullName(), p->value()));
    }
    foreach (Node* nd, n->children()) {
        res->addChild(clone(nd));
    }

    return res;
}


Node::Node(const QString &name, Node *parent)
    : name_(name)
    , properties_()
    , children_()
    , cdata_()
    , parent_(parent)
{}


QString DirectSymbolContainerNode::toXmlIndented(int level) const
{
    QList<Property*> props = properties();
    QList<Node*> chs;
    QString val;
    foreach (Node* n, children()) {
        if (n->name().startsWith("__symbol_")) {
            val += n->name().mid(QString("__symbol_").size()) + ".";
        } else {
            chs.append(n);
        }
    }
    val.remove(val.size()-1, 1);
    Property tags(name() + "/tags", val);
    props.append(&tags);


    QString res;
    QString indent;
    for (int i = level; i>0; i--) {
        indent += "  ";
    }
    res += indent + "<" + name();
    foreach (Property* prop, props) {
        res += " " + prop->name() + "=\"" + prop->value() + "\"";
    }
    if (cdata().isEmpty() && chs.isEmpty()) {
        res += " />\n";
        return res;
    } else {
        res += ">\n";
    }

    if (!cdata().isEmpty()) {
        res += indent + "  " + cdata() + '\n';
    }

    foreach (const Node* ch, chs) {
        res += ch->toXmlIndentedDefault(ch, level+1);
    }
    res += indent + "</" + name() + ">\n";

    return res;
}


QString IndirectSymbolContainerNode::toXmlIndented(int level) const
{
    QList<Node*> chs;
    QList<Node*> toBeDeleted;
    foreach (Node* n, children()) {
        if (n->name().startsWith("__symbol_")) {
            Node* ch = Node::create(proxy_);
            Property* pr = new Property(proxy_ + "/" + prop_, n->name().mid(QString("__symbol_").size()));
            ch->addProperty(pr);
            chs.append(ch);
            toBeDeleted.append(ch);
        } else {
            chs.append(n);
        }
    }


    QString res;
    QString indent;
    for (int i = level; i>0; i--) {
        indent += "  ";
    }
    res += indent + "<" + name();
    foreach (Property* prop, properties()) {
        res += " " + prop->name() + "=\"" + prop->value() + "\"";
    }
    if (cdata().isEmpty() && chs.isEmpty()) {
        res += " />\n";
        return res;
    } else {
        res += ">\n";
    }

    if (!cdata().isEmpty()) {
        res += indent + "  " + cdata() + '\n';
    }

    foreach (const Node* ch, chs) {
        res += ch->toXmlIndentedDefault(ch, level+1);
    }
    res += indent + "</" + name() + ">\n";

    return res;
}

QString ConditionNode::toXmlIndented(int level) const
{
    QString indent = indentation(level);

    Property* ppos1 = property("pos1");
    Property* ppart1 = property("part1");
    Property* ppos2 = property("pos2");
    Property* ppart2 = property("part2");
    Property* plit = property("lit");
    Property* plittag = property("lit-tag");
    Node* first = NULL;
    Node* second = NULL;
    if (ppos1 == NULL || ppart1 == NULL) {
        return indent + "<" + name() + "/>\n";
    } else {
        first = create("clip");
        first->addProperty(new Property("clip/pos", ppos1->value()));
        first->addProperty(new Property("clip/part", ppart1->value()));
        first->addProperty(new Property("clip/side", "sl"));
    }

    if (ppos2 != NULL && ppart2 != NULL) {
        second = create("clip");
        second->addProperty(new Property("clip/pos", ppos2->value()));
        second->addProperty(new Property("clip/part", ppart2->value()));
        second->addProperty(new Property("clip/side", "sl"));
    } else if (plit != NULL) {
        second = create("lit");
        second->addProperty(new Property("lit/v", plit->value()));
    } else if (plittag != NULL) {
        second = create("lit-tag");
        second->addProperty(new Property("lit-tag/v", plittag->value()));
    } else {
        return indent + "<" + name() + "/>\n";
    }

    return indent + "<" + name() + ">\n" +
                toXmlIndentedDefault(first, level+1) +
                toXmlIndentedDefault(second, level+1) +
            indent + "</" + name() + ">\n";
}

QString WhenNode::toXmlIndented(int level) const
{
    Node* test = create("test");
    Node* action = NULL;
    foreach (Node* nd, children()) {
        if (nd->name() == "action") {
            action = static_cast<ActionNode*>(nd)->resolveSequence();
        } else {
            test->addChild(clone(nd));
        }
    }
    if (action == NULL) {
        return indentation(level) + "<" + name() + " />\n";
    }
    if (!test->children().isEmpty()) {
        action->insertChild(test, 0);
    }
    action->setName(name());
    return action->toXmlIndented(level);
}

QString ChooseNode::toXmlIndented(int level) const
{
    QString indent = indentation(level);

    return indent + "<action>\n" +
            toXmlIndentedDefault(this, level+1) +
            indent + "</action>\n";
}
