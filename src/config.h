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

#ifndef CONFIG_H
#define CONFIG_H


#include <QColor>
#include <QXmlDefaultHandler>
#include <QFont>
#include "node.h"

namespace reptype
{
enum Type
{
    BOX = 0,
    TAB,
    HIDDEN,
    UNKNOWN
};
}

namespace contype
{
enum Type
{
    NESTED = 0,
    ARROW,
    UNKNOWN
};
}

namespace proptype
{
enum Type
{
    STRING = 0,
    INTEGER,
    SELECTION,
    UNKNOWN
};
}

namespace nodetype
{
enum Type
{
    STANDARD,
    ACTION,
    SYMBOL_CONTAINER_DIRECT,
    SYMBOL_CONTAINER_INDIRECT,
    CONDITION,
    WHEN,
    CHOOSE

};
}

class VisualSchema
{
public:
    VisualSchema(const QXmlInputSource& file);
    VisualSchema();

    struct Tag
    {
        reptype::Type reptype;
        nodetype::Type type;
        QColor boxColor;
        QString label;
        QStringList children;
        QStringList properties;
        QString nesting;
        QString name;
        QString symIndProxy;
        QString symProp;
    };

    struct Property
    {
        proptype::Type type;
        bool isMandatory;
        QString label;
        QString valueListId;
        QString fullName;
    };

    struct Connection
    {
        contype::Type type;
    };

    const Tag tag(const QString& name) const { return tags_[name]; }
    const Property property(const QString& tagname, const QString& propname) const { return props_[tagname + "/" + propname]; }
    const Property property(const QString& tagperprop) const { return props_[tagperprop]; }
    const Connection connection(const QString& parent, const QString& child) const { return cons_[parent + "/" + child]; }

    void setTag(const QString& name, const Tag& val) { tags_[name] = val; }
    void setProperty(const QString& tagname, const QString& propname, const Property& val) { props_[tagname + "/" + propname] = val; }
    void setProperty(const QString& tagperprop, const Property& val) { props_[tagperprop] = val; }
    void setConnection(const QString& parent, const QString& child, const Connection& val) { cons_[parent + "/" + child] = val; }

    bool hasTag(const QString& name) const { return tags_.contains(name); }
    bool hasProperty(const QString& tagname, const QString& propname) const { return props_.contains(tagname + "/" + propname); }
    bool hasProperty(const QString& tagperprop) const { return props_.contains(tagperprop); }
    bool hasConnection(const QString& parent, const QString& child) const { return cons_.contains(parent + "/" + child); }

private:
    class XmlHandler : public QXmlDefaultHandler
    {
    public:
        XmlHandler(VisualSchema* vs)
            : QXmlDefaultHandler()
            , vschema_(vs) {}

        virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
        virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);

    private:
        static reptype::Type toRepType(const QString& str);
        static contype::Type toConType(const QString& str);
        static proptype::Type toPropType(const QString& str);
        static nodetype::Type toNodeType(const QString& str);
        static QColor toColor(const QString& str);
        static bool toBool(const QString& str) { if (str == "true") return true; return false; }

        Tag currentTag_;
        QString currentTagName_;
        VisualSchema* vschema_;
    };

    QMap<QString, Tag> tags_;
    QMap<QString, Property> props_;
    QMap<QString, Connection> cons_;
};

class ValueMap
{
public:
    ValueMap(const QXmlInputSource& file);
    ValueMap();

    QMap<QString, QString> list(const QString& id) const { return lists_[id]; }
    QMap<QString, QString> rlist(const QString& id) const { return rlists_[id]; }

private:
    class XmlHandler : public QXmlDefaultHandler
    {
    public:
        XmlHandler(ValueMap* vl)
            : vmap_(vl)
            , currentListId_()
        {}

        virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);

    private:
        ValueMap* vmap_;
        QString currentListId_;
    };

    QMap<QString, QMap<QString, QString> > lists_;
    QMap<QString, QMap<QString, QString> > rlists_;
};

class Configuration;
class FileConfiguration;

Configuration& appConfig();
FileConfiguration& fileConfig(const QString& file);

class Configuration
{
    friend Configuration& appConfig();
public:
    VisualSchema::Tag tag(const QString& name) const { return vschema_.tag(name); }
    VisualSchema::Property property(const QString& tagname, const QString& propname) const { return vschema_.property(tagname, propname); }
    VisualSchema::Property property(const QString& tagperprop) const { return vschema_.property(tagperprop); }
    VisualSchema::Connection connection(const QString& parent, const QString& child) const { return vschema_.connection(parent, child); }

    virtual const ValueMap& valueMap() const { return vmap_; }
    const QMap<QString,QString>& templates() const { return templates_; }

    QString ltCompPath() const { return ltCompPath_; }
    QString ltProcPath() const { return ltProcPath_; }
    QString apertiumTransferPath() const { return apertiumTransferPath_; }
    QString apertiumPreprocTransPath() const { return apertiumPreprocTransPath_; }

    void setLtCompPath(const QString& str) { ltCompPath_ = str; }
    void setLtProcPath(const QString& str) { ltProcPath_ = str; }
    void setApertiumTransferPath(const QString& str) { apertiumTransferPath_ = str; }
    void setApertiumPreprocTransPath(const QString& str) { apertiumPreprocTransPath_ = str; }

private:
    Configuration();

    class TemplatesXmlHandler : public QXmlDefaultHandler
    {
    public:
        TemplatesXmlHandler(QMap<QString,QString>* map)
            : map_(map)
        {}

        virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);

    private:
        QMap<QString,QString>* map_;
    };

    VisualSchema vschema_;
    ValueMap vmap_;
    QMap<QString, QString> templates_;

    QString ltCompPath_;
    QString ltProcPath_;
    QString apertiumTransferPath_;
    QString apertiumPreprocTransPath_;
};

class FileConfiguration
{
public:
    VisualSchema::Tag tag(const QString &name) const;
    VisualSchema::Property property(const QString &tagname, const QString &propname) const;
    VisualSchema::Property property(const QString &tagperprop) const;
    VisualSchema::Connection connection(const QString &parent, const QString &child) const;

    void setTag(const QString& name, const VisualSchema::Tag& val) { vschema_.setTag(name, val); }
    void setProperty(const QString& tagname, const QString& propname, const VisualSchema::Property& val) { vschema_.setProperty(tagname, propname, val); }
    void setProperty(const QString& tagperprop, const VisualSchema::Property& val) { vschema_.setProperty(tagperprop, val); }
    void setConnection(const QString& parent, const QString& child, const VisualSchema::Connection& val) { vschema_.setConnection(parent, child, val); }

    const QString& slDictPath() const { return slPath_; }
    const QString& tlDictPath() const { return tlPath_; }
    const QString& biDictPath() const { return biPath_; }

    void setSlDictPath(const QString& str);
    void setTlDictPath(const QString& str);
    void setBiDictPath(const QString& str) { biPath_ = str; }

private:
    class SymbolsXmlHandler : public QXmlDefaultHandler
    {
    public:
        SymbolsXmlHandler(QStringList& dst)
            : symbols_(dst)
        {}

        bool startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &atts);

    private:
        QStringList& symbols_;
    };

    void extractSymbols(const QString& file, QStringList& into);

    VisualSchema vschema_;
    QString slPath_, tlPath_, biPath_;
};

namespace appearance
{
inline QColor fontColor() { return Qt::black; }
QString formatTextNormal(const QString& str);
QString formatTextBold(const QString& str);
}

#endif // CONFIG_H
