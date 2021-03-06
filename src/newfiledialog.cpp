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

#include <QRadioButton>

#include "newfiledialog.h"
#include "ui_newfiledialog.h"
#include "config.h"
#include "filesystem.h"

NewFileDialog::NewFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFileDialog)
{
    ui->setupUi(this);
    QListWidget* list = findChild<QListWidget*>("optionList");

    foreach(QString str, appConfig().templates().keys()) {
        list->addItem(str);
    }
}

NewFileDialog::~NewFileDialog()
{
    delete ui;
}

QString NewFileDialog::getChoice() const
{
    QListWidget* list = findChild<QListWidget*>("optionList");

    return fs::templatesDir() + "/" + appConfig().templates()[list->currentItem()->text()];
}
