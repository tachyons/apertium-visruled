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


#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>
#include "tools.h"

namespace Ui {
class TestDialog;
}

class TestDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TestDialog(QWidget *parent = 0);
    ~TestDialog();

    void setTools(ToolsManager* tm);
    ToolsManager* tools() const { return tools_; }

public slots:
    void on_goButton_pressed();
    
private slots:
    void morphFinished(QString str);
    void transferFinished(QString str);
    void generateFinished(QString str);

private:
    Ui::TestDialog *ui;
    ToolsManager* tools_;
};

#endif // TESTDIALOG_H
