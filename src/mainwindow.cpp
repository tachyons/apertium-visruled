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

#include <QFileDialog>
#include <QDebug>
#include <QTableWidget>
#include <QLabel>
#include <QXmlInputSource>
#include <QCloseEvent>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filetab.h"
#include "newfiledialog.h"
#include "resources.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    newFileDialog_(this),
    settingsDialog_(this),
    testDialog_(this),
    stack_(NULL),
    files_(NULL),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&newFileDialog_, SIGNAL(accepted()), this, SLOT(createNewFile()));
    connect(&settingsDialog_, SIGNAL(accepted()), this, SLOT(updateSettings()));

    files_ = findChild<QTabWidget*>("filesContainer");
    connect(files_, SIGNAL(currentChanged(int)), this, SLOT(updateActionStack()));
    connect(files_, SIGNAL(currentChanged(int)), this, SLOT(updateActionStack()));
    updateActionStack();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Apertium transfer rules (*.t1x *.t2x *.t3x)"));

    if (fileName.isEmpty()) {
        return;
    }

    FileTab* ft = new FileTab(this, readXmlIntoNode(fileName));

    connect(ft, SIGNAL(sectionChanged()), this, SLOT(updateActionStack()));
    connect(ft, SIGNAL(sectionChanged()), this, SLOT(updateSidebar()));
    connect(ft, SIGNAL(saveStateChanged(bool)), this, SLOT(updateFileTabLabels()));
    connect(&ft->toolsManager(), SIGNAL(compileFinished(bool)), this, SLOT(compileFinished(bool)));
    ft->setFilePath(fileName);
    files_->addTab(ft, ft->fileName());
    files_->setCurrentIndex(files_->count() - 1);
}

void MainWindow::on_actionClose_triggered()
{
    closeFile(files_->currentIndex());
}

void MainWindow::on_actionSave_triggered()
{
    saveFile(files_->currentIndex());
}

void MainWindow::on_actionSaveAs_triggered()
{
    saveFileAs(files_->currentIndex());
}

void MainWindow::on_actionNew_triggered()
{
    newFileDialog_.open();
}

void MainWindow::on_actionUndo_triggered()
{
    stack_->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    stack_->redo();
}

void MainWindow::on_actionSettings_triggered()
{
    if (files_->currentIndex() == -1) {
        settingsDialog_.updateValues(NULL);
    } else {
        settingsDialog_.updateValues(tab(files_->currentIndex()));
    }
    settingsDialog_.open();
}

void MainWindow::on_actionCompile_triggered()
{
    if (files_->currentIndex() == -1) {
        return;
    }

    FileTab* ft = tab(files_->currentIndex());
    ToolsManager& tm = ft->toolsManager();
    tm.compile();
}

void MainWindow::on_actionTest_triggered()
{
    if (files_->currentIndex() == -1) {
        return;
    }

    FileTab* ft = tab(files_->currentIndex());
    testDialog_.setTools(&ft->toolsManager());
    testDialog_.show();
}

void MainWindow::updateUndoRedo()
{
    QAction* undo = findChild<QAction*>("actionUndo");
    QAction* redo = findChild<QAction*>("actionRedo");

    if (stack_ == NULL) {
        undo->setEnabled(false);
        redo->setEnabled(false);
    } else {
        undo->setEnabled(stack_->canUndo());
        redo->setEnabled(stack_->canRedo());
    }
}

void MainWindow::updateActionStack()
{
    if (stack_ != NULL) {
        disconnect(stack_, SIGNAL(actionPerformed()), this, SLOT(updateUndoRedo()));
    }

    stack_ = currentActionStack();

    if (stack_ != NULL) {
        connect(stack_, SIGNAL(actionPerformed()), this, SLOT(updateUndoRedo()));
    }

    updateUndoRedo();
}

ActionStack *MainWindow::currentActionStack()
{
    QWidget* w = files_->currentWidget();

    if (w == NULL) {
        return NULL;
    }
    FileTab* ft = static_cast<FileTab*>(w);
    return ft->currentActionStack();
}

void MainWindow::createNewFile()
{
    RootNode *root = NULL;

    QString path = newFileDialog_.getChoice();

    root = readXmlIntoNode(path);

    FileTab* ft = new FileTab(this, root);
    ft->setUnsaved();
    connect(ft, SIGNAL(sectionChanged()), this, SLOT(updateActionStack()));
    connect(ft, SIGNAL(saveStateChanged(bool)), this, SLOT(updateFileTabLabels()));
    connect(&ft->toolsManager(), SIGNAL(compileFinished(bool)), this, SLOT(compileFinished(bool)));
    ft->setFilePath("");
    files_->addTab(ft, ft->fileName() + "*");
    files_->setCurrentIndex(files_->count() - 1);
    updateSidebar();
}

void MainWindow::compileFinished(bool successful)
{
    QStatusBar* sb = findChild<QStatusBar*>("statusBar");
    const QString msg = successful
            ? tr("Compilation successful")
            : tr("Compilation failed");

    sb->showMessage(msg, 5000);
}

void MainWindow::updateFileTabLabels()
{
    for (int i = 0; i<files_->count(); i++) {
        FileTab* ft = static_cast<FileTab*>(files_->widget(i));
        QString label = ft->fileName();
        label += ft->isSaved() ? "" : "*";
        files_->setTabText(i, label);
    }
}

void MainWindow::updateSidebar()
{
    FileTab* ft = tab(files_->currentIndex());
    SectionTab* st = ft->currentSection();

    if (st->selectedBox() == NULL) {
        boxbar()->clearItems();
        propertybar()->clearItems();
        return;
    }

    VisualSchema::Tag tdef = fileConfig(ft->filePath()).tag(st->selectedBox()->data()->name());
    boxbar()->clearItems();
    foreach (QString str, tdef.children) {
        VisualSchema::Tag cdef = fileConfig(ft->filePath()).tag(str);
        boxbar()->addItem(new BoxPreview(cdef));
    }

    propertybar()->clearItems();
    foreach (QString str, tdef.properties) {
        VisualSchema::Property pdef = fileConfig(ft->filePath()).property(str);
        propertybar()->addItem(new PropertyPreview(pdef));
    }
}

void MainWindow::updateSettings()
{
    if (files_->currentIndex() != -1) {
        FileTab* ft = tab(files_->currentIndex());
        ft->setSourceLangDictPath(settingsDialog_.sourceLangDict());
        ft->setTargetLangDictPath(settingsDialog_.targetLangDict());
        ft->setBilingualDictPath(settingsDialog_.bilinDict());
    }

    appConfig().setApertiumTransferPath(settingsDialog_.apertiumTransfer());
    appConfig().setLtCompPath(settingsDialog_.ltComp());
    appConfig().setLtProcPath(settingsDialog_.ltProc());
    appConfig().setApertiumPreprocTransPath(settingsDialog_.apertiumPreprocTrans());
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (!closeAllFiles()) {
        ev->ignore();
    }
}

void MainWindow::saveFile(int index)
{
    FileTab* ft = tab(index);

    if (!ft->filePath().isEmpty()) {
        QFile dst(ft->filePath());
        dst.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&dst);

        out << ft->rootNode()->toXml();
        out.flush();
        dst.close();
        ft->setSaved();
    } else {
        saveFileAs(index);
    }
}

void MainWindow::saveFileAs(int index)
{
    FileTab* ft =tab(index);

    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), "", tr("Apertium transfer rules (*.t1x *.t2x *.t3x)"));
    if (path.isEmpty()) {
        return;
    }

    QFile dst(path);
    dst.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&dst);

    out << ft->rootNode()->toXml();
    ft->setSaved();

    ft->setFilePath(path);
    files_->setTabText(files_->currentIndex(), ft->fileName());
}

bool MainWindow::closeFile(int index)
{
    FileTab* ft = tab(index);

    if (ft->isSaved()) {
        files_->removeTab(index);
        return true;
    }

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("This file has unsaved changes."));
    msg.setInformativeText(tr("Do you want to save before closing?"));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Save | QMessageBox::Discard);

    int res = msg.exec();

    switch (res) {
    case QMessageBox::Save:
        saveFile(index);
    case QMessageBox::Discard:
        files_->removeTab(index);
        return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

bool MainWindow::closeAllFiles()
{
    bool needConfirm = false;
    for (int i = 0; i<files_->count(); i++) {
        if (!tab(i)->isSaved()) {
            needConfirm = true;
            break;
        }
    }

    if (!needConfirm) {
        while (files_->count() > 0) { files_->removeTab(0); }
        return true;
    }

    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("Some files have unsaved changes."));
    msg.setInformativeText(tr("Do you want to save before closing?"));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::SaveAll | QMessageBox::Discard);

    int res = msg.exec();

    switch (res) {
    case QMessageBox::SaveAll:
        for (int i = 0; i<files_->count(); i++) { saveFile(i); }
    case QMessageBox::Discard:
        while (files_->count() > 0) { files_->removeTab(0); }
        return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}


