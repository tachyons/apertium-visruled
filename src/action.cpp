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

#include "action.h"
#include "diagram.h"

ActionStack::ActionStack(QObject *parent)
    : QObject(parent)
    , undoStack_()
    , redoStack_()
{}

ActionStack::~ActionStack()
{
    foreach (Action* a, undoStack_) {
        delete a;
    }

    foreach (Action* a, redoStack_) {
        delete a;
    }
}

void ActionStack::push(Action *a)
{
    foreach (Action* a, redoStack_) {
        delete a;
    }
    redoStack_.clear();

    undoStack_.push(a);
    a->execute();
    emit actionPerformed();
}

void ActionStack::undo()
{
    Action* a = undoStack_.pop();
    a->undo();
    redoStack_.push(a);
    emit actionPerformed();
}

void ActionStack::redo()
{
    Action* a = redoStack_.pop();
    a->execute();
    undoStack_.push(a);
    emit actionPerformed();
}

bool ActionStack::canUndo() const
{
    return !undoStack_.isEmpty();
}

bool ActionStack::canRedo() const
{
    return !redoStack_.isEmpty();
}

namespace actions
{
void ChangeProperty::execute()
{
    prop_->setValue(newValue_);
}

void ChangeProperty::undo()
{
    prop_->setValue(oldValue_);
}

DeleteProperty::DeleteProperty(PropertyWidget *prop)
    : prop_(prop)
    , parent_(prop->parentBox())
    , executed_(false)
{}

void DeleteProperty::execute()
{
    parent_->removeProperty(prop_);
    parent_->data()->removeProperty(prop_->data());
    prop_->hide();
    executed_ = true;
}

void DeleteProperty::undo()
{
    parent_->addProperty(prop_);
    parent_->data()->addProperty(prop_->data());
    prop_->show();
    executed_ = false;
}

DeleteProperty::~DeleteProperty()
{
    if (executed_) {
        prop_->deleteLater();
        prop_->data()->deleteLater();
    }
}

void AddProperty::execute()
{
    parent_->addProperty(prop_);
    parent_->data()->addProperty(prop_->data());
    prop_->show();
    executed_ = true;
}

void AddProperty::undo()
{
    parent_->removeProperty(prop_);
    parent_->data()->removeProperty(prop_->data());
    prop_->hide();
    executed_ = false;
}

AddProperty::~AddProperty()
{
    if (!executed_) {
        prop_->deleteLater();
        prop_->data()->deleteLater();
    }
}

DeleteBox::DeleteBox(Box* b)
    : box_(b)
    , pos_(b->parentElement()->indexOf(b))
    , parent_(b->parentElement())
    , executed_(false)
{}

void DeleteBox::execute()
{
    parent_->removeBox(box_);
    parent_->data()->removeChild(box_->data());
    box_->hide();
    foreach (Box* b, box_->boxes()) {
        b->hide();
    }

    executed_ = true;
}

void DeleteBox::undo()
{
    parent_->insertBox(box_, pos_);
    parent_->data()->insertChild(box_->data(), pos_);
    box_->show();
    foreach (Box* b, box_->boxes()) {
        b->show();
    }
    executed_ = false;
}

DeleteBox::~DeleteBox()
{
    if (executed_) {
        box_->deleteLater();
        box_->data()->deleteLater();
    }
}



InsertBox::InsertBox(Box *b, int pos, DiagramElement *to)
    : box_(b)
    , pos_(pos)
    , parent_(to)
    , executed_(false)
{}

void InsertBox::execute()
{
    parent_->insertBox(box_, pos_);
    parent_->data()->insertChild(box_->data(), pos_);
    box_->show();
    foreach (Box* b, box_->boxes()) {
        b->show();
    }
    executed_ = true;
}

void InsertBox::undo()
{
    parent_->removeBox(box_);
    parent_->data()->removeChild(box_->data());
    box_->hide();
    foreach (Box* b, box_->boxes()) {
        b->hide();
    }
    executed_ = false;
}

InsertBox::~InsertBox()
{
    if (!executed_) {
        box_->deleteLater();
        box_->data()->deleteLater();
    }
}

MoveBox::MoveBox(Box *b, int pos)
    : box_(b)
    , src_(b->parentElement()->indexOf(b))
    , dst_(pos)
    , parent_(b->parentElement())
{}

void MoveBox::execute()
{
    int dst = dst_ > src_ ? dst_ - 1 : dst_;
    parent_->removeBox(box_, false);
    parent_->data()->removeChild(box_->data());
    parent_->insertBox(box_, dst);
    parent_->data()->insertChild(box_->data(), dst);
}

void MoveBox::undo()
{
    parent_->removeBox(box_, false);
    parent_->data()->removeChild(box_->data());
    parent_->insertBox(box_, src_);
    parent_->data()->insertChild(box_->data(), src_);
}
}
