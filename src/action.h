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


#ifndef ACTION_H
#define ACTION_H

#include <QStack>
#include "node.h"
#include "propertywidget.h"

class Action
{
public:
    virtual void execute() = 0;
    virtual void undo() = 0;

    virtual ~Action() {}

protected:
    Action() {}
};

class ActionStack : public QObject
{
    Q_OBJECT
public:
    ActionStack(QObject* parent = NULL);
    ~ActionStack();

    void push(Action* a);
    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

signals:
    void actionPerformed();

private:
    ActionStack(const ActionStack&);

    QStack<Action*> undoStack_;
    QStack<Action*> redoStack_;
};

class Box;
class DiagramElement;

namespace actions
{

class ChangeProperty : public Action
{
public:
    ChangeProperty(PropertyWidget* prop, const QString& newValue)
        : prop_(prop)
        , newValue_(newValue)
        , oldValue_(prop->value())
    {}

    virtual void execute();
    virtual void undo();

private:
    PropertyWidget* prop_;
    QString newValue_;
    QString oldValue_;
};

class DeleteProperty : public Action
{
public:
    DeleteProperty(PropertyWidget* prop);

    ~DeleteProperty();

    void execute();
    void undo();

private:
    PropertyWidget* prop_;
    Box* parent_;
    bool executed_;
};

class AddProperty : public Action
{
public:
    AddProperty(PropertyWidget* prop, Box* to)
        : prop_(prop)
        , parent_(to)
        , executed_(false)
    {}

    ~AddProperty();

    void execute();
    void undo();

private:
    PropertyWidget* prop_;
    Box* parent_;
    bool executed_;
};

class DeleteBox : public Action
{
public:
    DeleteBox(Box* b);

    ~DeleteBox();

    void execute();
    void undo();

private:
    Box* box_;
    int pos_;
    DiagramElement* parent_;
    bool executed_;
};

class InsertBox : public Action
{
public:
    InsertBox(Box* b, int pos, DiagramElement *to);

    ~InsertBox();

    void execute();
    void undo();

private:
    Box* box_;
    int pos_;
    DiagramElement* parent_;
    bool executed_;
};

class MoveBox : public Action
{
public:
    MoveBox(Box* b, int pos);

    void execute();
    void undo();

private:
    Box* box_;
    int src_;
    int dst_;
    DiagramElement* parent_;
};

}

#endif // ACTION_H
