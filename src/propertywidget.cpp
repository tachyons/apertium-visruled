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

#include "propertywidget.h"
#include "config.h"
#include "action.h"
#include "resources.h"
#include "diagram.h"
#include "action.h"
#include <QDebug>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QMenu>

namespace {
QString getLineEditValue(QWidget* w, Property*)
{
    QLineEdit* qle = dynamic_cast<QLineEdit*>(w);

    if (qle == NULL) {
        return "";
    } else {
        return qle->text();
    }
}

QString getComboBoxValue(QWidget* w, Property* p)
{
    QComboBox* qcb = dynamic_cast<QComboBox*>(w);

    if (qcb == NULL) {
        return "";
    }

    VisualSchema::Property pdef = appConfig().property(p->fullName());
    return appConfig().valueMap().rlist(pdef.valueListId)[qcb->currentText()];
}

QString getSpinBoxValue(QWidget* w, Property*)
{
    QSpinBox* qsb = dynamic_cast<QSpinBox*>(w);

    if (qsb == NULL) {
        return "";
    } else {
        return QString::number(qsb->value());
    }
}

void setLineEditValue(const QString& str, QWidget* w, Property*)
{
    QLineEdit* qle = dynamic_cast<QLineEdit*>(w);

    if (qle == NULL) {
        return;
    } else {
        qle->setText(str);
    }
}

void setComboBoxValue(const QString& str, QWidget* w, Property* p)
{
    QComboBox* qcb = dynamic_cast<QComboBox*>(w);

    if (qcb == NULL) {
        return;
    }

    VisualSchema::Property pdef = appConfig().property(p->fullName());
    QString val = appConfig().valueMap().list(pdef.valueListId)[str];
    for (int i = 0; i<qcb->count(); i++) {
        if (val == qcb->itemText(i)) {
            qcb->setCurrentIndex(i);
            break;
        }
    }
}

void setSpinBoxValue(const QString& str, QWidget* w, Property*)
{
    QSpinBox* qsb = dynamic_cast<QSpinBox*>(w);

    if (qsb == NULL) {
        return;
    } else {
        qsb->setValue(str.toInt());
    }
}

}

PropertyWidget::PropertyWidget(Property *prop, Box *parent)
    : QWidget(parent)
    , prop_(prop)
    , parent_(parent)
    , label_(new QLabel("", this))
    , value_(NULL)
    , setValueFunc_(NULL)
    , getValueFunc_(NULL)
{
    VisualSchema::Property pdef = appConfig().property(prop->fullName());
    switch (pdef.type) {
    case proptype::SELECTION:
    {
        QComboBox* cb = new QComboBox(this);
        QMap<QString, QString> vals = appConfig().valueMap().list(pdef.valueListId);
        for (QMap<QString, QString>::Iterator i = vals.begin(); i != vals.end(); i++) {
             cb->addItem(*i);
        }
        value_ = cb;
        getValueFunc_ = &getComboBoxValue;
        setValueFunc_ = &setComboBoxValue;
        connect(cb, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateValue()));
        break;
    }
    case proptype::INTEGER:
    {
        QSpinBox* sb = new QSpinBox(this);
        sb->setMinimum(1);
        sb->setMaximum(99);

        value_ = sb;
        getValueFunc_ = &getSpinBoxValue;
        setValueFunc_ = &setSpinBoxValue;
        connect(sb, SIGNAL(valueChanged(int)), this, SLOT(updateValue()));
        break;
    }
    case proptype::STRING:
    case proptype::UNKNOWN:
    default:
    {
        QLineEdit *qle = new QLineEdit(this);
        getValueFunc_ = &getLineEditValue;
        setValueFunc_ = &setLineEditValue;
        value_ = qle;
        connect(qle, SIGNAL(textChanged(QString)), this, SLOT(updateValue()));
        break;
    }
    }

    label_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(label_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(label_);
    layout->addWidget(value_);

    updateWidgets();
}

void PropertyWidget::updateWidgets()
{
    label_->setText(appearance::formatTextNormal(appConfig().property(prop_->fullName()).label));
    setValueFunc_(prop_->value(), value_, prop_);
    label_->show();
    value_->show();
}

void PropertyWidget::updateValue()
{
    //actions::ChangeProperty *cp = new actions::ChangeProperty(this, getValueFunc_(value_, prop_));
    //resources::actionStack.push(cp);

    prop_->setValue(getValueFunc_(value_, prop_));
}

void PropertyWidget::showContextMenu(const QPoint& where)
{
    QPoint pos = label_->mapToGlobal(where);
    QMenu menu(this);

    QAction *del = menu.addAction("Delete property");

    QAction *res = menu.exec(pos);
    if (res == del) {
        parent_->actionStack().push(new actions::DeleteProperty(this));
    }
}
