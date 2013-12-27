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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "config.h"
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
}

QString SettingsDialog::ltProc() const
{
    QLineEdit* qle = findChild<QLineEdit*>("ltProcPath");
    return qle->text();
}

QString SettingsDialog::ltComp() const
{
    QLineEdit* qle = findChild<QLineEdit*>("ltCompPath");
    return qle->text();
}

QString SettingsDialog::apertiumTransfer() const
{
    QLineEdit* qle = findChild<QLineEdit*>("apertiumTransferPath");
    return qle->text();
}

QString SettingsDialog::apertiumPreprocTrans() const
{
    QLineEdit* qle = findChild<QLineEdit*>("apertiumPreprocTransPath");
    return qle->text();
}

QString SettingsDialog::sourceLangDict() const
{
    QLineEdit* qle = findChild<QLineEdit*>("slDictPath");
    return qle->text();
}

QString SettingsDialog::targetLangDict() const
{
    QLineEdit* qle = findChild<QLineEdit*>("tlDictPath");
    return qle->text();
}

QString SettingsDialog::bilinDict() const
{
    QLineEdit* qle = findChild<QLineEdit*>("bilinDictPath");
    return qle->text();
}

void SettingsDialog::updateValues(FileTab *tab)
{
    QLineEdit* ltproc = findChild<QLineEdit*>("ltProcPath");
    QLineEdit* ltcomp = findChild<QLineEdit*>("ltCompPath");
    QLineEdit* apert = findChild<QLineEdit*>("apertiumTransferPath");
    QLineEdit* sldict = findChild<QLineEdit*>("slDictPath");
    QLineEdit* tldict = findChild<QLineEdit*>("tlDictPath");
    QLineEdit* bidict = findChild<QLineEdit*>("bilinDictPath");

    QPushButton* slbtn = findChild<QPushButton*>("browseSlDict");
    QPushButton* tlbtn = findChild<QPushButton*>("browseTlDict");
    QPushButton* bibtn = findChild<QPushButton*>("browseBiDict");

    if (tab != NULL) {
        sldict->setText(tab->sourceLangDictPath());
        tldict->setText(tab->targetLangDictPath());
        bidict->setText(tab->bilingualDictPath());
        sldict->setEnabled(true);
        tldict->setEnabled(true);
        bidict->setEnabled(true);
        slbtn->setEnabled(true);
        tlbtn->setEnabled(true);
        bibtn->setEnabled(true);
    } else {
        sldict->setDisabled(true);
        tldict->setDisabled(true);
        bidict->setDisabled(true);
        slbtn->setDisabled(true);
        tlbtn->setDisabled(true);
        bibtn->setDisabled(true);
    }

    ltproc->setText(appConfig().ltProcPath());
    ltcomp->setText(appConfig().ltCompPath());
    apert->setText(appConfig().apertiumTransferPath());
}

void SettingsDialog::setDictPath(const QString &entry)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Apertium dictionary ( *.dix )"));
    QLineEdit* qle = findChild<QLineEdit*>(entry);
    qle->setText(fileName);
}

void SettingsDialog::setExecPath(const QString &entry)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("All files ( * )"));
    QLineEdit* qle = findChild<QLineEdit*>(entry);
    qle->setText(fileName);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
