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

#ifndef TOOLS_H
#define TOOLS_H

#include <QObject>
#include <QList>
#include <QProcess>

class ToolsManager : public QObject
{
    Q_OBJECT
public:
    ToolsManager(const QString& sldict, const QString& tldict, const QString& bidict, const QString& rules, QObject* parent = NULL)
        : QObject(parent)
        , sldict_(sldict)
        , tldict_(tldict)
        , bidict_(bidict)
        , rules_(rules)
        , slbin_(dirOfFile(sldict_) + "/sl.automorph.bin")
        , tlbin_(dirOfFile(tldict_) + "/tl.autogen.bin")
        , bibin_(dirOfFile(bidict_) + "/bilin.autobil.bin")
        , rulebin_(dirOfFile(rules_) + "/transfer.bin")
        , curProc_(NULL)
        , curType_(NONE)
    {}

    ToolsManager(QObject* parent = NULL)
        : QObject(parent)
        , sldict_()
        , tldict_()
        , bidict_()
        , rules_()
        , slbin_()
        , tlbin_()
        , bibin_()
        , rulebin_()
        , curProc_(NULL)
        , curType_(NONE)
    {}

    bool canLaunchNew() const { return curProc_ == NULL; }

    const QString& slDict() const { return sldict_; }
    const QString& tlDict() const { return tldict_; }
    const QString& biDict() const { return bidict_; }
    const QString& transferRules() const { return rules_; }

public slots:
    void setSlDict(const QString& str) { sldict_ = str; slbin_ = dirOfFile(sldict_) + "/sl.automorph.bin"; }
    void setTlDict(const QString& str) { tldict_ = str; tlbin_ = dirOfFile(tldict_) + "/tl.autogen.bin"; }
    void setBiDict(const QString& str) { bidict_ = str; bibin_ = dirOfFile(bidict_) + "/bilin.autobil.bin"; }
    void setTransferRules(const QString& str) { rules_ = str; rulebin_ = dirOfFile(rules_) + "/transfer.bin"; }

    void compile();
    void morph(const QString& text);
    void transfer(const QString& text);
    void generate(const QString& text);

signals:
    void compileFinished(bool successful);
    void morphFinished(const QString& out);
    void transferFinished(const QString& out);
    void generateFinished(const QString& out);

private slots:
    void finished(int retval);
    void outputReady();
    void compile2();
    void compile3();
    void compile4();

private:
    enum ProcType { NONE, COMPILE1, COMPILE2, COMPILE3, COMPILE4, MORPH, TRANSFER, GENERATE };

    QString dirOfFile(const QString& str) const;

    QString sldict_, tldict_, bidict_, rules_;
    QString slbin_, tlbin_, bibin_, rulebin_;
    QProcess* curProc_;
    ProcType curType_;
};

#endif // TOOLS_H
