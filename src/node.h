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

#ifndef NODE_H
#define NODE_H

#include <QString>
#include <QList>
#include <QXmlInputSource>
#include <QXmlDefaultHandler>

class Property : public QObject
{
    Q_OBJECT
public:
    Property(const QString& fullName, const QString& value);
    Property();

    virtual ~Property() {}

    QString fullName() const { return prefix_ + "/" + name_; }
    void setFullName(const QString& str);

    QString name() const { return name_; }
    void setName(const QString& str);

    const QString& value() const { return value_; }
    int valueToInt(int def = 0, int min = 0, int max = -1) const;
    void setValue(const QString& str);

signals:
    void valueChanged(QString);
    void fullNameChanged(QString);

private:
    Property(const Property&);
    QString name_;
    QString prefix_;
    QString value_;
};

class Node : public QObject
{
    Q_OBJECT
public:
    static Node* create(const QString& name, bool addMandProps = false, Node* parent = NULL);
    static Node* clone(Node* n);

    typedef QList<Node*>::Iterator ChildIterator;
    typedef QList<Node*>::ConstIterator ConstChildIterator;
    typedef QList<Property*>::Iterator PropertyIterator;
    typedef QList<Property*>::ConstIterator ConstPropertyIterator;

    virtual ~Node();

    const QString& name() const { return name_; }
    void setName(const QString& str) { name_ = str; emit nameChanged(str);}

    const QList<Property*>& properties() const { return properties_; }
    Property *property(const QString& name) const;

    QList<Node*>& children() { return children_; }
    const QList<Node*>& children() const { return children_; }

    Node* child(const QString& name) const;

    const QString& cdata() const { return cdata_; }
    virtual void setCData(const QString& str) { cdata_ = str; }
    virtual void appendCData(const QString& str) { cdata_ += str; }

    Node* parentNode() const { return parent_; }

    virtual const QString& filePath() const { return parent_->filePath(); }

    QString toXml() const;
    virtual QString toXmlIndented(int level) const;

public slots:
    virtual void addChild(Node* ch) { insertChild(ch, children_.size()); }
    virtual void insertChild(Node* ch, int index);
    virtual void removeChild(Node* n) { children_.removeOne(n); emit childRemoved(n); }
    virtual void addProperty(Property* pr) { if (!properties_.contains(pr)) { properties_.append(pr); } }
    virtual void removeProperty(Property* pr) { properties_.removeOne(pr); }

signals:
    void childInserted(Node* ch, int pos);
    void childRemoved(Node* ch);
    void nameChanged(const QString& str);

protected:
    Node(const QString& name, Node* parent = NULL);

    Node(Node* parent = NULL)
        : name_()
        , properties_()
        , children_()
        , cdata_()
        , parent_(parent)
    {}


    static QString toXmlIndentedDefault(const Node* n, int level);
    virtual void setParentNode(Node* n) { parent_ = n; }

    static QString indentation(int level) { return QString(level*2, ' '); }
private:
    const Node& operator =(const Node& n);
    Node(const Node&);

    QString name_;
    QList<Property*> properties_;
    QList<Node*> children_;
    QString cdata_;
    Node* parent_;
};


class RootNode : public Node
{
    Q_OBJECT
public:
    RootNode(const QString& name = "")
        : Node(name, NULL)
        , filePath_()
    {}

    virtual const QString& filePath() const { return filePath_; }
    void setFilePath(const QString& str) { filePath_ = str; }
    QString toXmlIndented(int level) const;

protected:


    QString filePath_;
};

class WhenNode;

class ActionNode : public Node
{
    friend class WhenNode;
    Q_OBJECT
public:
    ActionNode(const QString& name, Node* parent = NULL);
    QString toXmlIndented(int level) const;

protected:
    virtual void setParentNode(Node* n);
    Node *resolveSequence() const;

private slots:
    void trySetPattern(Node* n);
    void tryRemovePattern(Node* n);

private:

    Node* pattern_;
    Node* parentRule_;
    Node* attrs_;
};

class DirectSymbolContainerNode : public Node
{
    Q_OBJECT
public:
    DirectSymbolContainerNode(const QString& name, const QString& prop, Node* parent = NULL)
        : Node(name, parent)
        , propName_(prop) {}

protected:
    QString toXmlIndented(int level) const;

private:
    QString propName_;
};

class IndirectSymbolContainerNode : public Node
{
    Q_OBJECT
public:
    IndirectSymbolContainerNode(const QString& name, const QString& proxy, const QString& prop, Node* parent = NULL)
        : Node(name, parent)
        , proxy_(proxy)
        , prop_(prop)
    {}

    QString toXmlIndented(int level) const;

private:
    QString proxy_, prop_;
};

class ConditionNode : public Node
{
    Q_OBJECT
public:
    ConditionNode(const QString& name, Node* parent = NULL)
        : Node(name, parent)
    {}

    QString toXmlIndented(int level) const;

};

class WhenNode : public Node
{
    Q_OBJECT
public:
    WhenNode(const QString& name, Node* parent = NULL)
        : Node(name, parent)
    {}

    QString toXmlIndented(int level) const;
};

class ChooseNode : public Node
{
    Q_OBJECT
public:
    ChooseNode(const QString& name, Node* parent = NULL)
        : Node(name, parent)
    {}

    QString toXmlIndented(int level) const;
};

RootNode* readXmlIntoNode(const QString &path);

#endif // NODE_H
