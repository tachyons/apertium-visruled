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

#include "diagram.h"
#include "config.h"
#include "resources.h"
#include <cmath>
#include <QPainter>
#include <QMenu>
#include <QDebug>

DiagramElement::~DiagramElement()
{}

void DiagramElement::addBox(Box *b, bool repaint)
{
    if (!boxes_.contains(b)) {
        boxes_.append(b);
        b->setParentElement(this);
        b->setParent(this);
    }

    if (repaint) {
        updateLayout();
    }
}

void DiagramElement::insertBox(Box *b, int index, bool repaint)
{
    if (!boxes_.contains(b)) {
        boxes_.insert(index, b);
        b->setParentElement(this);
        b->setParent(this);
    }

    if (repaint) {
        updateLayout();
    }
}

void DiagramElement::removeBox(Box *b, bool repaint)
{
    removeArrows(b);
    boxes_.removeOne(b);

    if (repaint) {
        updateLayout();
    }
}

void DiagramElement::clearBoxes()
{
    boxes_.clear();
    updateLayout();
}

int DiagramElement::getInsertPosition(int x, int y) const
{
    int pos = 0;
    VisualSchema::Tag tdef = fileConfig(data_->filePath()).tag(data_->name());

    if (tdef.nesting == "horizontal") {
        foreach(Box* b, boxes_) {
            if (b->x() > x) {
                return pos;
            }
            pos++;
        }
    } else {
        foreach(Box* b, boxes_) {
            if (b->y() > y) {
                return pos;
            }
            pos++;
        }
    }

    return pos;
}

bool DiagramElement::isChildOf(DiagramElement *de, bool noArrow) const
{
    if (de == this) {
        return true;
    } else if (parent_ == NULL) {
        return false;
    } else {
        if (noArrow) {
            if (parent_->data() == 0) {
                return parent_->isChildOf(de, noArrow);
            }
            VisualSchema::Connection cdef = fileConfig(data_->filePath()).connection(parent_->data()->name(), data()->name());
            if (cdef.type == contype::ARROW) {
                return false;
            } else {
                return parent_->isChildOf(de, noArrow);
            }
        } else {
            return parent_->isChildOf(de, noArrow);
        }
    }
}

void DiagramElement::dragEnterEvent(QDragEnterEvent *ev)
{
    VisualSchema::Tag tdef = fileConfig(data_->filePath()).tag(data_->name());
    bool accept = false;
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledbox")) {
        QString name;
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledbox");
        QDataStream stream(&mdata, QIODevice::ReadOnly);
        stream >> name;
        accept = tdef.children.contains(name);
    } else if (ev->mimeData()->hasFormat("application/x-dnd-visruledmove")) {
        Box* src = static_cast<Box*>(ev->source());
        int pos = indexOf(src);
        int insertPos = getInsertPosition(ev->pos().x(), ev->pos().y());
        if (pos != -1 && (insertPos != pos) && (insertPos != pos+1)) {
            accept = true;
        }
    }

    if (accept) {
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}

void DiagramElement::dragMoveEvent(QDragMoveEvent *ev)
{
    VisualSchema::Tag tdef = fileConfig(data_->filePath()).tag(data_->name());
    bool accept = false;
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledbox")) {
        QString name;
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledbox");
        QDataStream stream(&mdata, QIODevice::ReadOnly);
        stream >> name;
        accept = tdef.children.contains(name);
    } else if (ev->mimeData()->hasFormat("application/x-dnd-visruledmove")) {
        Box* src = static_cast<Box*>(ev->source());
        int pos = indexOf(src);
        int insertPos = getInsertPosition(ev->pos().x(), ev->pos().y());
        if (pos != -1 && (insertPos != pos) && (insertPos != pos+1)) {
            accept = true;
        }
    }

    if (accept) {
        ev->acceptProposedAction();
    } else {
        ev->ignore();
    }
}

void DiagramElement::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledbox")) {
        ev->accept();
        QByteArray data = ev->mimeData()->data("application/x-dnd-visruledbox");
        QDataStream stream(&data, QIODevice::ReadOnly);

        QString name;
        stream >> name;

        Node* n = Node::create(name, true, data_);
        Box* newb = new Box(n, this);
        actionStack().push(new actions::InsertBox(newb, getInsertPosition(ev->pos().x(), ev->pos().y()), this));
    } else if (ev->mimeData()->hasFormat("application/x-dnd-visruledmove")) {
        Box* src = static_cast<Box*>(ev->source());
        int pos = getInsertPosition(ev->pos().x(), ev->pos().y());
        actionStack().push(new actions::MoveBox(src, pos));
    }
}

Box::Box(Node *data, DiagramElement *parent)
    : DiagramElement(data, parent, parent)
    , label_(new QLabel)
    , mainLayout_(NULL)
    , propsContainer_(new QWidget(this))
    , props_()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    foreach(Node* n, data->children()) {
        Box* ch = new Box(n, this);
        addBox(ch);
    }

    foreach(Property* p, data->properties()) {
        props_.append(new PropertyWidget(p, this));
    }

    propsContainer_->setLayout(new QHBoxLayout());

    updateLayout();
}

Box::~Box()
{}

void Box::addBox(Box *b, bool repaint)
{
    DiagramElement::addBox(b);
    VisualSchema::Connection cdef = fileConfig(data()->filePath()).connection(data()->name(), b->data()->name());
    if (cdef.type == contype::ARROW) {
        setArrow(this, b);
    }
    if (repaint) {
        updateLayout();
    }
}

void Box::insertBox(Box *b, int index, bool repaint)
{
    DiagramElement::insertBox(b, index);
    VisualSchema::Connection cdef = fileConfig(data()->filePath()).connection(data()->name(), b->data()->name());
    if (cdef.type == contype::ARROW) {
        setArrow(this, b);
    }
    if (repaint) {
        updateLayout();
    }
}

void Box::addProperty(PropertyWidget *pw)
{
    if (!props_.contains(pw)) {
        pw->setParent(this);
        pw->setParentBox(this);
        props_.append(pw);;
        updateLayout();
    }
}

void Box::removeProperty(PropertyWidget *pw)
{
    props_.removeOne(pw);
    updateLayout();
}

int Box::absX() const
{
    DiagramElement* de = static_cast<DiagramElement*>(parentWidget());
    return de->absX() + x();
}

int Box::absY() const
{
    DiagramElement* de = static_cast<DiagramElement*>(parentWidget());
    return de->absY() + y();
}

void Box::paintBackground(QPainter &qp)
{
    VisualSchema::Tag tagdef = fileConfig(data()->filePath()).tag(data()->name());
    qp.setBrush(tagdef.boxColor);

    qp.drawRect(absX(), absY(), width(), height());

    foreach (Box* box, boxes()) {
        box->paintBackground(qp);
    }
}

void Box::paintEvent(QPaintEvent *ev)
{
    DiagramElement::paintEvent(ev);

    QPainter painter(this);
    if (isSelected()) {
        painter.setPen(QPen(Qt::black, 4));
    } else {
        painter.setPen(QPen(Qt::black, 1));
    }

    painter.drawRect(0, 0, width(), height());
}

void Box::mousePressEvent(QMouseEvent *)
{
    registerSelection(this);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << data()->name();

    QMimeData* mdata = new QMimeData;
    mdata->setData("application/x-dnd-visruledmove", array);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->exec(Qt::MoveAction, Qt::MoveAction);
}

void Box::dragEnterEvent(QDragEnterEvent *ev)
{
    DiagramElement::dragEnterEvent(ev);
    bool accept = false;
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledprop")) {
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledprop");
        QDataStream stream(&mdata, QIODevice::ReadOnly);
        QString name;
        stream >> name;

        VisualSchema::Tag tdef = fileConfig(data()->filePath()).tag(data()->name());

        accept = tdef.properties.contains(name);
    }

    if (accept) {
        ev->acceptProposedAction();
    }
}

void Box::dragMoveEvent(QDragMoveEvent *ev)
{
    DiagramElement::dragMoveEvent(ev);
    bool accept = false;
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledprop")) {
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledprop");
        QDataStream stream(&mdata, QIODevice::ReadOnly);
        QString name;
        stream >> name;

        VisualSchema::Tag tdef = fileConfig(data()->filePath()).tag(data()->name());

        accept = tdef.properties.contains(name);
    }

    if (accept) {
        ev->acceptProposedAction();
    }
}

void Box::dropEvent(QDropEvent *ev)
{
    DiagramElement::dropEvent(ev);
    bool accept = false;
    if (ev->mimeData()->hasFormat("application/x-dnd-visruledprop")) {
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledprop");
        QDataStream stream(&mdata, QIODevice::ReadOnly);
        QString name;
        stream >> name;

        VisualSchema::Tag tdef = fileConfig(data()->filePath()).tag(data()->name());

        accept = tdef.properties.contains(name);
    }

    if (accept) {
        ev->accept();
        QByteArray mdata = ev->mimeData()->data("application/x-dnd-visruledprop");
        QDataStream stream(&mdata, QIODevice::ReadOnly);

        QString name;
        stream >> name;

        Property* pr = new Property(name, "");
        data()->addProperty(pr);
        PropertyWidget* pw = new PropertyWidget(pr, this);
        actionStack().push(new actions::AddProperty(pw, this));
    }
}

void Box::updateLayout()
{
    VisualSchema::Tag tagdef = fileConfig(data()->filePath()).tag(data()->name());

    if (mainLayout_ != NULL) {
        delete mainLayout_;
    }

    label_->setText(appearance::formatTextBold(tagdef.label));

    if (tagdef.nesting == "horizontal") {
        mainLayout_ = new QHBoxLayout(this);
    } else {
        mainLayout_ = new QVBoxLayout(this);
    }

    mainLayout_->addWidget(label_);
    if (!props_.isEmpty()) {
        mainLayout_->addWidget(propsContainer_);
    }

    foreach (Box* box, boxes()) {
        VisualSchema::Connection cdef = fileConfig(data()->filePath()).connection(data()->name(), box->data()->name());
        if (cdef.type != contype::ARROW) {
            mainLayout_->addWidget(box);
            box->show();
        }
    }

    QLayout* propsLayout = propsContainer_->layout();
    foreach (PropertyWidget *pr, props_) {
        propsLayout->addWidget(pr);
        pr->show();
    }
    propsContainer_->show();

    setMinimumSize(mainLayout_->sizeHint());
    updateGeometry();
    parentElement()->updateGeometry();
    parentElement()->updateLayout();
}

void Box::showContextMenu(const QPoint &where)
{
    QPoint pos = mapToGlobal(where);

    VisualSchema::Tag tdef = fileConfig(data()->filePath()).tag(data()->name());
    QMenu menu(this);

    QAction* del = menu.addAction("Delete");

    QMap<QAction*, QString> centries;
    if (!tdef.children.isEmpty()) {
        QMenu* cmenu = menu.addMenu("Add child");
        foreach(QString str, tdef.children) {
            QAction* a = cmenu->addAction(str);
            centries[a] = str;
        }
    }

    QMap<QAction*, QString> pentries;
    if (!tdef.properties.isEmpty()) {
        QMenu* pmenu = menu.addMenu("Add property");
        foreach(QString str, tdef.properties) {
            QAction* a = pmenu->addAction(str);
            pentries[a] = str;
        }
    }

    QAction* res = menu.exec(pos);

    if (res == del) {
        actionStack().push(new actions::DeleteBox(this));
    } else if (centries.contains(res)) {
        Node* n = Node::create(centries[res], true, data());
        data()->addChild(n);
        Box* newb = new Box(n, this);
        actionStack().push(new actions::InsertBox(newb, boxes().size(), this));
    } else if (pentries.contains(res)) {
        Property* pr = new Property(pentries[res], "");
        data()->addProperty(pr);
        PropertyWidget* pw = new PropertyWidget(pr, this);
        actionStack().push(new actions::AddProperty(pw, this));
    }
}

Diagram::Diagram(Node *data, QWidget *parent)
    : DiagramElement(data, NULL, parent)
    , arrows_()
    , stack_()
    , selectedBox_(this)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    if (data == NULL) {
        return;
    }

    foreach (Node* n, data->children()) {
        addBox(new Box(n, this));
    }
    updateLayout();
}

void Diagram::updateLayout()
{
    QPoint p(0, 0);
    int maxX = 50;
    foreach (Box* box, boxes()) {
        p.setX(0);
        QList<Box*> col;
        col.append(box);
        p = layoutBoxes(col, p);
        maxX = std::max(maxX, p.x());
    }

    setMinimumSize(maxX, p.y() + 50);
    updateGeometry();
    repaint();
}

void Diagram::setArrow(Box *from, Box *to)
{
    to->setParent(this);
    QPair<Box*, Box*> arrow(from, to);

    if (!arrows_.contains(arrow)) {
        arrows_.append(arrow);
    }
}

void Diagram::removeArrow(Box *from, Box *to)
{
    QPair<Box*, Box*> arrow(from, to);
    arrows_.removeOne(arrow);
}

void Diagram::removeArrows(Box *b)
{
    for (int i = 0; i < arrows_.size(); i++) {
        const QPair<Box*, Box*>& bb = arrows_[i];
        if (bb.first->isChildOf(b) || bb.second->isChildOf(b)) {
            arrows_.removeAt(i--);
        }
    }
}

void Diagram::paintEvent(QPaintEvent *ev)
{
    QWidget::paintEvent(ev);
    QPainter painter(this);
    QSize size = minimumSize();

    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, size.width(), size.height());

    foreach (Box* box, boxes()) {
        box->paintBackground(painter);
    }

    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (QList<QPair<Box*, Box*> >::Iterator pair = arrows_.begin(); pair != arrows_.end(); pair++) {
        QPointF from(pair->first->absX() + pair->first->width(), pair->first->absY() + pair->first->height()/2);
        QPointF to(pair->second->absX(), pair->second->absY() + pair->second->height()/2);

        double d = sqrt(pow(from.x() - to.x(), 2) + pow(from.y() - to.y(), 2));

        double bx = to.x() * (d - 5)/d + from.x() * 5/d;
        double by = to.y() * (d - 5)/d + from.y() * 5/d;
        double angle = asin((to.x() - from.x()) / d);

        if ((to.y() - from.y()) > 0) {
            angle *= -1;
        }

        QPointF arrowHead[3] = {
            to,
            QPointF(bx + 3*cos(angle), by + 3*sin(angle)),
            QPointF(bx - 3*cos(angle), by - 3*sin(angle))
        };

        painter.drawConvexPolygon(arrowHead, 3);

        painter.drawLine(from, to);
    }
}

QPoint Diagram::layoutBoxes(const QList<Box *> &bl, QPoint start)
{
    QList<Box*> nextColumn;

    start.setX(start.x() + 20);

    int curY = start.y() + 20;
    int maxWidth = 0;

    foreach (Box* b, bl) {
        QSize s = b->sizeHint();
        b->setGeometry(start.x(), curY, s.width(), s.height());
        b->show();
        maxWidth = std::max(maxWidth, b->width());
        curY += b->height() + 10;

        for (QList<QPair<Box*, Box*> >::Iterator i = arrows_.begin(); i != arrows_.end(); i++) {
            if (i->first->isChildOf(b, true)) {
                nextColumn.append(i->second);
            }
        }
    }

    QPoint next(start.x() + maxWidth + 50, -1);

    if (!nextColumn.isEmpty()) {
        next = layoutBoxes(nextColumn, QPoint(start.x() + maxWidth + 50, start.y()));
    }

    return QPoint(next.x(), std::max(curY, next.y()));
}

void Diagram::setData(Node *n)
{
    foreach(Box* b, boxes()) {
        delete b;
    }
    clearBoxes();

    if (n == NULL) {
        return;
    }

    foreach (Node* n, n->children()) {
        addBox(new Box(n, this));
    }
    DiagramElement::setData(n);
}

void Diagram::showContextMenu(const QPoint &where)
{
    QPoint pos = mapToGlobal(where);

    VisualSchema::Tag tdef = fileConfig(data()->filePath()).tag(data()->name());
    QMenu menu(this);

    QMap<QAction*, QString> centries;
    if (!tdef.children.isEmpty()) {
        QMenu* cmenu = menu.addMenu("Add child");
        foreach(QString str, tdef.children) {
            QAction* a = cmenu->addAction(str);
            centries[a] = str;
        }
    }

    QAction* res = menu.exec(pos);

    if (centries.contains(res)) {
        Node* n = Node::create(centries[res], true, data());
        data()->addChild(n);
        Box* b =new Box(n, this);
        actionStack().push(new actions::InsertBox(b, boxes().size(), this));
        updateLayout();
    }
}

void Diagram::registerSelection(DiagramElement *de)
{
    if (selectedBox_ != NULL) {
        selectedBox_->setSelected(false);
    }

    selectedBox_ = de;
    selectedBox_->setSelected();
    resources::mainWindow->updateSidebar();
    repaint();
}

void Diagram::mousePressEvent(QMouseEvent *)
{
    registerSelection(this);
}
