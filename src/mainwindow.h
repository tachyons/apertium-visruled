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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

#include "node.h"
#include "filetab.h"
#include "newfiledialog.h"
#include "settingsdialog.h"
#include "testdialog.h"
#include "sidebar.h"

class ActionStack;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    ActionStack* currentActionStack();

    Boxbar* boxbar() const { return findChild<Boxbar*>("boxPreviewsContainer"); }
    Propertybar* propertybar() const { return findChild<Propertybar*>("propPreviewsContainer"); }

public slots:
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionNew_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionSettings_triggered();
    void on_actionCompile_triggered();
    void on_actionTest_triggered();

    void updateUndoRedo();
    void updateActionStack();
    void updateFileTabLabels();
    void updateSidebar();
    void updateSettings();

protected:
    void closeEvent(QCloseEvent * ev);
    
private slots:
    void createNewFile();
    bool closeFile(int index);
    bool closeAllFiles();
    void saveFile(int index);
    void saveFileAs(int index);
    void compileFinished(bool successful);


private:
    FileTab* tab(int index) const { return static_cast<FileTab*>(files_->widget(index)); }

    NewFileDialog newFileDialog_;
    SettingsDialog settingsDialog_;
    TestDialog testDialog_;
    ActionStack* stack_;
    QTabWidget* files_;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
