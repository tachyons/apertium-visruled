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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QLayout>
#include "config.h"

class BoxPreview : public QWidget
{
    Q_OBJECT
public:
    BoxPreview(const VisualSchema::Tag& tag, QWidget *parent = NULL);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    VisualSchema::Tag def_;
    QLabel label_;
};

class PropertyPreview : public QWidget
{
    Q_OBJECT
public:
    PropertyPreview(const VisualSchema::Property& prop, QWidget *parent = NULL);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

private:
    VisualSchema::Property def_;
    QLabel label_;
};

template<class T>
class Sidebar : public QWidget
{
public:
    Sidebar(QWidget *parent = NULL)
        : QWidget(parent)
    {
        setLayout(new QVBoxLayout());
    }

    ~Sidebar()
    {
        clearItems();
    }

    void addItem(T* t)
    {
        items_.append(t);
        layout()->addWidget(t);
    }

    void clearItems()
    {
        foreach(T* t, items_) {
            layout()->removeWidget(t);
            t->deleteLater();
        }
        items_.clear();
    }

private:
    QList<T*> items_;
};

typedef Sidebar<BoxPreview> Boxbar;
typedef Sidebar<PropertyPreview> Propertybar;

#endif // SIDEBAR_H
