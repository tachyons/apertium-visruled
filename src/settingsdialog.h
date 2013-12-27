#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
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

#include <QDialog>
#include "filetab.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    QString ltProc() const;
    QString ltComp() const;
    QString apertiumTransfer() const;
    QString apertiumPreprocTrans() const;

    QString sourceLangDict() const;
    QString targetLangDict() const;
    QString bilinDict() const;

    void updateValues(FileTab* tab);

private slots:
    void on_browseSlDict_pressed() { setDictPath("slDictPath"); }
    void on_browseTlDict_pressed() { setDictPath("tlDictPath"); }
    void on_browseBiDict_pressed() { setDictPath("bilinDictPath"); }
    void on_browseLtProc_pressed() { setExecPath("ltProcPath"); }
    void on_browseLtComp_pressed() { setExecPath("ltCompPath"); }
    void on_browseTransfer_pressed() { setExecPath("apertiumTransferPath"); }
    void on_browsePreproc_pressed() { setExecPath("apertiumPreprocTransPath"); }
    
private:
    void setDictPath(const QString& entry);
    void setExecPath(const QString& entry);

    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
