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

#ifndef SECTIONTAB_H
#define SECTIONTAB_H

#include <QWidget>

#include "node.h"
#include "action.h"
#include "diagram.h"

namespace Ui {
class SectionTab;
}

class SectionTab : public QWidget
{
    Q_OBJECT
    
public:
    explicit SectionTab(QWidget *parent, Node *root);
    ~SectionTab();

    ActionStack& actionStack();

    DiagramElement* selectedBox() const;
    
private:
    Ui::SectionTab *ui;

    Node* sectionRoot_;
};

#endif // SECTIONTAB_H
