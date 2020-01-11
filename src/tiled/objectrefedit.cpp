/*
 * objectrefedit.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "objectrefedit.h"

#include "objectrefdialog.h"
#include "addpropertydialog.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#include <QIntValidator>

namespace Tiled {

ObjectRefEdit::ObjectRefEdit(QWidget *parent)
    : QWidget(parent)
    , mLineEdit(new QLineEdit(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(mLineEdit);

    QToolButton *button = new QToolButton(this);
    button->setText(tr("..."));
    button->setAutoRaise(true);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mLineEdit);
    layout->addWidget(button);

    auto regex = QRegExp(QStringLiteral("[0-9]+(:[0-9]+)?"));
    mLineEdit->setValidator(new QRegExpValidator(regex, this));

    connect(button, &QToolButton::clicked, this, &ObjectRefEdit::onButtonClicked);
    connect(mLineEdit, &QLineEdit::editingFinished, this, &ObjectRefEdit::onEditFinished);
}

void ObjectRefEdit::setValue(const ObjectRef &value)
{
    mValue = value;

    if (mValue.tileset) {
        if (mValue.tileId < 0) {
            mValue.id = 0;
            mLineEdit->setText(QString::number(0));
        } else {
            mLineEdit->setText(QString::number(mValue.tileId) + QLatin1Char(':') + QString::number(mValue.id));
        }
    } else {
        mValue.tileId = -1;
        mLineEdit->setText(QString::number(mValue.id));
    }
    emit valueChanged(mValue);
}

void ObjectRefEdit::onButtonClicked()
{
    ObjectRefDialog dialog(mValue, this);

    if (dialog.exec() == QDialog::Accepted)
        setValue(dialog.value());
}

void ObjectRefEdit::onEditFinished()
{
    auto newValue = fromExportValue(mLineEdit->text(), objectRefTypeId()).value<ObjectRef>();
    newValue.tileset = mValue.tileset;
    setValue(newValue);
}

} // namespace Tiled
