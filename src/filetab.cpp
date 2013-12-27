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

#include "filetab.h"
#include "ui_filetab.h"
#include "config.h"
#include "resources.h"
#include <QDebug>
#include <QFileInfo>

FileTab::FileTab(QWidget *parent, RootNode *root) :
    QWidget(parent),
    ui(new Ui::FileTab),
    sections_(),
    fileRoot_(root),
    saved_(true),
    tools_(fileConfig(root->filePath()).slDictPath(), fileConfig(root->filePath()).tlDictPath(), fileConfig(root->filePath()).biDictPath(), root->filePath())
{
    ui->setupUi(this);

    for (Node::ChildIterator i = root->children().begin(); i != root->children().end(); i++) {
        if (appConfig().tag((*i)->name()).reptype == reptype::TAB) {
            SectionTab* st = new SectionTab(this, *i);
            connect(&(st->actionStack()), SIGNAL(actionPerformed()), this, SLOT(setUnsaved()));
            QString label = appConfig().tag((*i)->name()).label;
            sections_.append(QPair<QString, SectionTab*>(label, st));
            QTabWidget* tw = findChild<QTabWidget*>("sectionsContainer");
            tw->addTab(st, label);
        }
    }
}

FileTab::~FileTab()
{
    delete ui;
}

void FileTab::setFilePath(const QString &path)
{
    QString sl = sourceLangDictPath();
    QString tl = targetLangDictPath();
    QString bl = bilingualDictPath();
    fileRoot_->setFilePath(path);
    setSourceLangDictPath(sl);
    setTargetLangDictPath(tl);
    setBilingualDictPath(bl);
    tools_.setTransferRules(path);
}

QString FileTab::fileName() const
{
    const QString& filePath = fileRoot_->filePath();
    if (filePath.isEmpty()) {
        return tr("New file");
    }

    QFile f(filePath);

    return QFileInfo(f).fileName();
}

void FileTab::on_sectionsContainer_currentChanged()
{
    emit sectionChanged();
}

ActionStack *FileTab::currentActionStack()
{
    QTabWidget *tw = findChild<QTabWidget*>("sectionsContainer");
    QWidget* w = tw->currentWidget();
    if (w == NULL) {
        return NULL;
    }

    SectionTab *st = static_cast<SectionTab*>(tw->currentWidget());
    return &(st->actionStack());
}

SectionTab* FileTab::currentSection()
{
    QTabWidget* qtw = findChild<QTabWidget*>("sectionsContainer");
    QWidget* res = qtw->currentWidget();

    return res == NULL ? NULL : static_cast<SectionTab*>(res);
}

void FileTab::setSourceLangDictPath(const QString &str)
{
    fileConfig(fileRoot_->filePath()).setSlDictPath(str);
    tools_.setSlDict(str);
}

void FileTab::setTargetLangDictPath(const QString &str)
{
    fileConfig(fileRoot_->filePath()).setTlDictPath(str);
    tools_.setTlDict(str);
}

void FileTab::setBilingualDictPath(const QString &str)
{
    fileConfig(fileRoot_->filePath()).setBiDictPath(str);
    tools_.setBiDict(str);
}
