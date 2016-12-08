/*
 * objectidedit.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * Based loosely on the TextPropertyEditor and TextEditor classes from
 * Qt Designer (Copyright (C) 2015 The Qt Company Ltd., LGPLv2.1).
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


#include "objectidedit.h"

#include "objectiddialog.h"

#include <QSpinBox>
#include <QHBoxLayout>
#include <QToolButton>

namespace Tiled {
namespace Internal {

ObjectIdEdit::ObjectIdEdit(QWidget *parent)
    : QWidget(parent)
    , mSpinBox(new QSpinBox(this))
    , mId(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(mSpinBox);

    QToolButton *button = new QToolButton(this);
    button->setText(tr("..."));
    button->setAutoRaise(true);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mSpinBox);
    layout->addWidget(button);

    connect(button, &QToolButton::clicked, this, &ObjectIdEdit::onButtonClicked);
    connect(mSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ObjectIdEdit::onIdChanged);
}

int ObjectIdEdit::id() const
{
    return mId;
}

void ObjectIdEdit::setId(int id)
{
    mId = id;
    mSpinBox->setValue(id);
}

void ObjectIdEdit::onIdChanged(int id)
{
    mId = id;
    emit idChanged(mId);
}

void ObjectIdEdit::onButtonClicked()
{
    ObjectIdDialog dialog(this);
    dialog.setId(mId);

    if (dialog.exec() != QDialog::Accepted)
        return;

    int newId = dialog.id();

    if (newId != mId) {
        setId(newId);
        emit idChanged(mId);
    }
}

} // namespace Internal
} // namespace Tiled
