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

#ifndef FILETAB_H
#define FILETAB_H

#include <QWidget>
#include <QVector>

#include "node.h"
#include "sectiontab.h"
#include "tools.h"
#include "config.h"

namespace Ui {
class FileTab;
}

class FileTab : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileTab(QWidget *parent, RootNode *root);
    ~FileTab();

    int sectionCount() const { return sections_.size(); }
    QString sectionTitle(int index) const { return sections_[index].first; }
    SectionTab* sectionTab(int index) const { return sections_[index].second; }

    const QString& filePath() const { return fileRoot_->filePath(); }
    void setFilePath(const QString& path);

    QString fileName() const;

    RootNode* rootNode() const { return fileRoot_; }
    void setRootNode(RootNode* n) { fileRoot_ = n; }

    ActionStack* currentActionStack();

    SectionTab* currentSection();

    bool isSaved() const { return saved_; }

    const QString& sourceLangDictPath() const { return fileConfig(fileRoot_->filePath()).slDictPath(); }
    const QString& targetLangDictPath() const { return fileConfig(fileRoot_->filePath()).tlDictPath(); }
    const QString& bilingualDictPath() const { return fileConfig(fileRoot_->filePath()).biDictPath(); }

    void setSourceLangDictPath(const QString& str);
    void setTargetLangDictPath(const QString& str);
    void setBilingualDictPath(const QString& str);

    ToolsManager& toolsManager() { return tools_; }

public slots:
    void setUnsaved() { saved_ = false; emit saveStateChanged(false); }
    void setSaved() { saved_ = true; emit saveStateChanged(true); }

signals:
    void sectionChanged();
    void saveStateChanged(bool);

private slots:
    void on_sectionsContainer_currentChanged();

private:
    Ui::FileTab *ui;

    QVector<QPair<QString, SectionTab*> > sections_;
    RootNode* fileRoot_;
    bool saved_;

    ToolsManager tools_;
};

#endif // FILETAB_H
