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

#include "sidebar.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QDrag>
#include <QMimeData>

BoxPreview::BoxPreview(const VisualSchema::Tag &tag, QWidget* parent)
    : QWidget(parent)
    , def_(tag)
    , label_(appearance::formatTextBold(tag.label), this)
{
    QLayout *layout = new QHBoxLayout();
    setLayout(layout);
    layout->addWidget(&label_);
}

void BoxPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(def_.boxColor);

    painter.drawRect(0, 0, width(), height());
}

void BoxPreview::mousePressEvent(QMouseEvent *)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << def_.name;

    QMimeData* mdata = new QMimeData;
    mdata->setData("application/x-dnd-visruledbox", data);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

PropertyPreview::PropertyPreview(const VisualSchema::Property &prop, QWidget *parent)
    : QWidget(parent)
    , def_(prop)
    , label_(appearance::formatTextNormal(prop.label), this)
{
    QLayout *layout = new QHBoxLayout();
    setLayout(layout);
    layout->addWidget(&label_);
}

void PropertyPreview::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::lightGray);

    painter.drawRect(0, 0, width(), height());
}

void PropertyPreview::mousePressEvent(QMouseEvent *)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << def_.fullName;

    QMimeData* mdata = new QMimeData;
    mdata->setData("application/x-dnd-visruledprop", data);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mdata);
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

