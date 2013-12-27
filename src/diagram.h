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


#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QWidget>
#include <QPaintEvent>
#include <QList>
#include <QBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QDrag>
#include "node.h"
#include "propertywidget.h"
#include "action.h"

class Diagram;
class Box;

class DiagramElement : public QWidget
{
    Q_OBJECT
public:
    virtual ~DiagramElement();

    virtual void setArrow(Box* from, Box* to) = 0;
    virtual void removeArrow(Box* from, Box* to) = 0;
    virtual void removeArrows(Box* b) = 0;
    virtual void clearArrows() = 0;

    virtual void setData(Node* n) { data_ = n; }
    virtual Node* data() const { return data_; }

    virtual void addBox(Box* b, bool repaint = true);
    virtual void insertBox(Box* b, int index, bool repaint = true);
    virtual void removeBox(Box* b, bool repaint = true);
    virtual void clearBoxes();
    const QList<Box*>& boxes() const { return boxes_; }
    int getInsertPosition(int x, int y) const;
    int indexOf(Box* b) const { return boxes_.indexOf(b); }

    virtual int absX() const = 0;
    virtual int absY() const = 0;

    virtual ActionStack& actionStack() = 0;

    virtual void updateLayout() = 0;

    bool isChildOf(DiagramElement* de, bool noArrow = false) const;

    bool isSelected() const { return selected_; }
    void setSelected(bool b = true) { selected_ = b; }
    virtual void registerSelection(DiagramElement* de) = 0;

    DiagramElement* parentElement() const { return parent_; }

protected:
    void setParentElement(DiagramElement* de) { parent_ = de; }

    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dragMoveEvent(QDragMoveEvent *ev);

    DiagramElement(Node* data, DiagramElement* parentE = NULL, QWidget* parent = NULL)
        : QWidget(parent)
        , data_(data)
        , parent_(parentE)
        , boxes_()
        , selected_(false)
    {
        setAcceptDrops(true);
    }

private:
    DiagramElement(const DiagramElement&);
    Node* data_;
    DiagramElement* parent_;
    QList<Box*> boxes_;
    bool selected_;
};

class Box : public DiagramElement
{
    Q_OBJECT
public:
    Box(Node* data, DiagramElement* parent = NULL);
    ~Box();

    void setArrow(Box *from, Box *to) { parentElement()->setArrow(from, to); }
    void removeArrow(Box *from, Box *to) { parentElement()->removeArrow(from, to); }
    void removeArrows(Box *b) { parentElement()->removeArrows(b); }
    void clearArrows() { parentElement()->clearArrows(); }

    virtual void addBox(Box *b, bool repaint = true);
    virtual void insertBox(Box *b, int index, bool repaint = true);
    void addProperty(PropertyWidget* pw);
    void removeProperty(PropertyWidget* pw);

    int absX() const;
    int absY() const;

    void paintBackground(QPainter& qp);

    ActionStack& actionStack() { return parentElement()->actionStack(); }

    void updateLayout();

    QSize minimumSizeHint() const { return mainLayout_->sizeHint(); }
    QSize sizeHint() const { return mainLayout_->sizeHint(); }

    void registerSelection(DiagramElement *de) { parentElement()->registerSelection(de); }

public slots:
    void showContextMenu(const QPoint& where);
    void dropEvent(QDropEvent *ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dragMoveEvent(QDragMoveEvent *ev);

protected:
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent * );

private:
    QLabel* label_;
    QBoxLayout* mainLayout_;
    QWidget* propsContainer_;
    QList<PropertyWidget*> props_;
};

class Diagram : public DiagramElement
{
    Q_OBJECT
public:
    Diagram(Node* data = NULL, QWidget* parent = NULL);

    void setArrow(Box* from, Box* to);
    void removeArrow(Box* from, Box* to);
    void removeArrows(Box* b);
    void clearArrows() { arrows_.clear(); }

    void setData(Node *n);

    int absX() const { return 0; }
    int absY() const { return 0; }

    void updateLayout();

    void paintEvent(QPaintEvent * ev);

    ActionStack& actionStack() { return stack_; }

    void registerSelection(DiagramElement *de);
    DiagramElement* selectedBox() const { return selectedBox_; }

public slots:
    void showContextMenu(const QPoint& where);

protected:
    void mousePressEvent(QMouseEvent *);

private:
    QPoint layoutBoxes(const QList<Box*>& bl, QPoint start);

    QList<QPair<Box*, Box*> > arrows_;
    ActionStack stack_;
    DiagramElement* selectedBox_;
};

#endif // DIAGRAM_H
