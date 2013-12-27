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

#include "testdialog.h"
#include "ui_testdialog.h"

namespace {
void stripTrailingWhiteSpace(QString& str) {
    if (str.isEmpty()) {
        return;
    }
    while (str[str.size()-1].isSpace()) {
        str.remove(str.size()-1, 1);
        if (str.isEmpty()) {
            return;
        }
    }
}
}

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog),
    tools_(NULL)
{
    ui->setupUi(this);
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::setTools(ToolsManager *tm)
{
    if (tools_ != NULL) {
        disconnect(tools_, SIGNAL(morphFinished(QString)), this, SLOT(morphFinished(QString)));
        disconnect(tools_, SIGNAL(transferFinished(QString)), this, SLOT(transferFinished(QString)));
        disconnect(tools_, SIGNAL(generateFinished(QString)), this, SLOT(generateFinished(QString)));
    }

    tools_ = tm;

    if (tools_ != NULL) {
        connect(tools_, SIGNAL(morphFinished(QString)), this, SLOT(morphFinished(QString)));
        connect(tools_, SIGNAL(transferFinished(QString)), this, SLOT(transferFinished(QString)));
        connect(tools_, SIGNAL(generateFinished(QString)), this, SLOT(generateFinished(QString)));
    }
}

void TestDialog::on_goButton_pressed()
{
    QPlainTextEdit* in = findChild<QPlainTextEdit*>("input");
    tools_->morph(in->toPlainText());
}

void TestDialog::morphFinished(QString str)
{
    QRadioButton* radio = findChild<QRadioButton*>("morfRadio");
    if (radio->isChecked()) {
        QPlainTextEdit* out = findChild<QPlainTextEdit*>("output");
        stripTrailingWhiteSpace(str);
        out->setPlainText(str);
    } else {
        tools_->transfer(str);
    }
}

void TestDialog::transferFinished(QString str)
{
    QRadioButton* radio = findChild<QRadioButton*>("transferRadio");
    if (radio->isChecked()) {
        QPlainTextEdit* out = findChild<QPlainTextEdit*>("output");
        stripTrailingWhiteSpace(str);
        out->setPlainText(str);
    } else {
        tools_->generate(str);
    }
}

void TestDialog::generateFinished(QString str)
{
    QPlainTextEdit* out = findChild<QPlainTextEdit*>("output");
    stripTrailingWhiteSpace(str);
    out->setPlainText(str);
}
