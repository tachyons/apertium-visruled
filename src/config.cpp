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

#include "config.h"
#include "filesystem.h"
#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>
#include <QDebug>
#include <QRgb>

namespace
{
Configuration* config = NULL;
QMap<QString, FileConfiguration>* fileConfigs = NULL;
}

namespace appearance
{
QString formatTextNormal(const QString &str)
{
    QString open = "<font color=\"" + fontColor().name() + "\">";
    QString end = "</font>";

    return open + str + end;
}

QString formatTextBold(const QString &str)
{
    return "<b>" + formatTextNormal(str) + "</b>";
}
}


VisualSchema::VisualSchema()
    : tags_()
    , props_()
    , cons_()
{}

VisualSchema::VisualSchema(const QXmlInputSource &file)
    : tags_()
    , props_()
    , cons_()
{
    QXmlSimpleReader reader;
    XmlHandler handler(this);
    reader.setContentHandler(&handler);
    reader.parse(&file);
}

bool VisualSchema::XmlHandler::startElement(const QString&, const QString&, const QString &name, const QXmlAttributes &atts)
{
    if (name == "schema") {
    } else if (name == "node") {
        currentTag_ = Tag();
        currentTagName_ = atts.value("name");
        currentTag_.reptype = toRepType(atts.value("rep"));
        currentTag_.boxColor = toColor(atts.value("color"));
        currentTag_.label = atts.value("label");
        currentTag_.nesting = atts.value("nesting").isEmpty() ? "vertical" : atts.value("nesting");
        currentTag_.name = currentTagName_;
        currentTag_.type = toNodeType(atts.value("type"));
        currentTag_.symIndProxy = atts.value("proxy");
        currentTag_.symProp = atts.value("prop-name");
    } else if (name == "child") {
        currentTag_.children.append(atts.value("name"));
        Connection c;
        c.type = toConType(atts.value("con"));
        vschema_->cons_[currentTagName_ + "/" + atts.value("name")] = c;
    } else if (name == "prop") {
        currentTag_.properties.append(currentTagName_ + "/" + atts.value("name"));
        Property p;
        p.type = toPropType(atts.value("type"));
        p.label = atts.value("label");
        p.valueListId = atts.value("list");
        p.fullName = currentTagName_ + "/" + atts.value("name");
        p.isMandatory = toBool(atts.value("mand"));
        vschema_->props_[p.fullName] = p;
    }
    return true;
}

bool VisualSchema::XmlHandler::endElement(const QString &, const QString &, const QString &name)
{
    if (name == "schema") {
        return true;
    } else if (currentTagName_.size() > 0) {
        vschema_->tags_[currentTagName_] = currentTag_;
        return true;
    } else {
        return false;
    }
}

reptype::Type VisualSchema::XmlHandler::toRepType(const QString &str)
{
    QString names[] = {"box", "tab", "hidden"};
    for (int i = 0; i<3; i++) {
        if (str == names[i]) {
            return reptype::Type(i);
        }
    }
    return reptype::UNKNOWN;
}

proptype::Type VisualSchema::XmlHandler::toPropType(const QString &str)
{
    QString names[] = {"string", "integer", "selection"};
    for (int i = 0; i<3; i++) {
        if (str == names[i]) {
            return proptype::Type(i);
        }
    }
    return proptype::UNKNOWN;
}

contype::Type VisualSchema::XmlHandler::toConType(const QString &str)
{
    QString names[] = {"nested", "arrow"};
    for (int i = 0; i<2; i++) {
        if (str == names[i]) {
            return contype::Type(i);
        }
    }
    return contype::UNKNOWN;
}

nodetype::Type VisualSchema::XmlHandler::toNodeType(const QString &str)
{
    QString names[] = {"standard", "action", "symbol-container-direct", "symbol-container-indirect", "condition", "when", "choose"};
    for (int i = 0; i<7; i++) {
        if (str == names[i]) {
            return nodetype::Type(i);
        }
    }
    return nodetype::STANDARD;
}

QColor VisualSchema::XmlHandler::toColor(const QString& str)
{
    QColor res("white");
    if (str.size() != 7) {
        return res;
    }
    if (str[0] != '#') {
        return res;
    }
    int rgb = 0;
    for (int i = 1; i<7; i++) {
        char c = str[i].toLatin1();
        if (c >= 'a' && c <= 'f') {
            int n = c - 'a' + 10;
            rgb += n << (6-i)*4;
        } else if (c >= 'A' && c <= 'F') {
            int n = c - 'A' + 10;
            rgb += n << (6-i)*4;
        } else if (c >= '0' && c <= '9') {
            int n = c - '0';
            rgb += n << (6-i)*4;
        } else {
            return res;
        }
    }
    res = QColor(QRgb(rgb));
    return res;
}


ValueMap::ValueMap()
    : lists_()
    , rlists_()
{}

ValueMap::ValueMap(const QXmlInputSource &file)
    : lists_()
    , rlists_()
{
    QXmlSimpleReader reader;
    XmlHandler handler(this);
    reader.setContentHandler(&handler);
    reader.parse(&file);
}

bool ValueMap::XmlHandler::startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "list") {
        currentListId_ = atts.value("id");
    } else if (qName == "item") {
        vmap_->lists_[currentListId_][atts.value("value")] = atts.value("label");
        vmap_->rlists_[currentListId_][atts.value("label")] = atts.value("value");
    } else if (qName == "lists") {
        return true;
    } else {
        return false;
    }

    return true;
}

bool Configuration::TemplatesXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "template") {
        QString name = atts.value("name");
        QString path = atts.value("file");
        (*map_)[name] = path;
    }

    return true;
}



Configuration &appConfig()
{
    if (config == NULL) {
        config = new Configuration;
    }
    return *config;
}

Configuration::Configuration()
    : vschema_()
    , vmap_()
    , templates_()
    , ltCompPath_("/usr/bin/lt-comp")
    , ltProcPath_("/usr/bin/lt-proc")
    , apertiumTransferPath_("/usr/bin/apertium-transfer")
    , apertiumPreprocTransPath_("/usr/bin/apertium-preprocess-transfer")
{
    QFile vsfile(fs::visualSchemaFile());
    QXmlInputSource vsinput(&vsfile);
    vschema_ = VisualSchema(vsinput);

    QFile listsfile(fs::valueMapFile());
    QXmlInputSource listsinput(&listsfile);
    vmap_ = ValueMap(listsinput);

    QFile tempsfile(fs::templatesFile());
    QXmlInputSource tempsinput(&tempsfile);
    QXmlSimpleReader reader;
    TemplatesXmlHandler handler(&templates_);
    reader.setContentHandler(&handler);
    reader.parse(&tempsinput);
}


VisualSchema::Tag FileConfiguration::tag(const QString &name) const
{
    if (vschema_.hasTag(name)) {
        return vschema_.tag(name);
    }

    return appConfig().tag(name);
}

VisualSchema::Property FileConfiguration::property(const QString &tagname, const QString &propname) const
{
    if (vschema_.hasProperty(tagname, propname)) {
        return vschema_.property(tagname, propname);
    }

    return appConfig().property(tagname, propname);
}

VisualSchema::Property FileConfiguration::property(const QString &tagperprop) const
{
    if (vschema_.hasProperty(tagperprop)) {
        return vschema_.property(tagperprop);
    }

    return appConfig().property(tagperprop);
}

VisualSchema::Connection FileConfiguration::connection(const QString &parent, const QString &child) const
{
    if (vschema_.hasConnection(parent, child)) {
        return vschema_.connection(parent, child);
    }

    return appConfig().connection(parent, child);
}

void FileConfiguration::setSlDictPath(const QString &str)
{
    QStringList symbols;
    extractSymbols(str, symbols);

    foreach(const QString& sym, symbols) {
        VisualSchema::Tag tag;
        tag.name = sym;
        tag.boxColor = QColor(0xff, 0x99, 0x99);
        tag.label = "&lt;" + sym.mid(QString("__symbol_").size()) + ">";

        vschema_.setTag(sym, tag);
    }

    VisualSchema::Tag defcat = appConfig().tag("cat-item");
    defcat.children << symbols;
    vschema_.setTag("cat-item", defcat);

    VisualSchema::Tag defattr = appConfig().tag("def-attr");
    defattr.children << symbols;
    vschema_.setTag("def-attr", defattr);

    slPath_ = str;
}

void FileConfiguration::setTlDictPath(const QString &str)
{
    QStringList symbols;
    extractSymbols(str, symbols);

    foreach(const QString& sym, symbols) {
        VisualSchema::Tag tag;
        tag.name = sym;
        tag.boxColor = QColor(0x24, 0x24, 0xb4);
        tag.label = "&lt;" + sym.mid(QString("__symbol_").size()) + ">";

        vschema_.setTag(sym, tag);
    }

    VisualSchema::Tag defattr = appConfig().tag("def-attr");
    defattr.children << symbols;
    vschema_.setTag("def-attr", defattr);

    tlPath_ = str;
}

void FileConfiguration::extractSymbols(const QString &file, QStringList &into)
{
    QFile f(file);
    if (!f.exists()) {
        return;
    }

    QXmlInputSource xml(&f);
    SymbolsXmlHandler handler(into);
    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.parse(&xml);
}


FileConfiguration &fileConfig(const QString &file)
{
    if (fileConfigs == NULL) {
        fileConfigs = new QMap<QString, FileConfiguration>();
    }

    return (*fileConfigs)[file];
}


bool FileConfiguration::SymbolsXmlHandler::startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &atts)
{
    if (qName == "sdef") {
        symbols_ << ("__symbol_" + atts.value("n"));
    }

    return true;
}
