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

#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLayout>
#include "node.h"

class Box;

class PropertyWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit PropertyWidget(Property *prop, Box *parent = 0);

    virtual QSize minimumSizeHint() const { return layout()->sizeHint(); }
    virtual QSize sizeHint() const { return layout()->sizeHint(); }

    Box* parentBox() const { return parent_; }
    void setParentBox(Box* b) { parent_ = b; }

    QString value() const { return getValueFunc_(value_, prop_); }

    Property* data() const { return prop_; }

public slots:
    void updateValue();
    void updateWidgets();
    void setValue(const QString& str) { setValueFunc_(str, value_, prop_); }

    void showContextMenu(const QPoint &where);
private:
    Property* prop_;
    Box* parent_;
    QLabel* label_;
    QWidget* value_;
    void (*setValueFunc_)(const QString&, QWidget*, Property*);
    QString (*getValueFunc_)(QWidget*, Property*);
};

#endif // PROPERTYWIDGET_H
