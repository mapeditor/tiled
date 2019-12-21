/*
 * objectrefedit.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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


#include "objectrefedit.h"

#include "objectrefdialog.h"
#include "logginginterface.h"
#include "addpropertydialog.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QToolButton>
#include <QIntValidator>

namespace Tiled {

ObjectRefEdit::ObjectRefEdit(QWidget *parent)
    : QWidget(parent)
    , mLine(new QLineEdit(this))
    , mId(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(mLine);

    QToolButton *button = new QToolButton(this);
    button->setText(tr("..."));
    button->setAutoRaise(true);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mLine);
    layout->addWidget(button);

    mLine->setValidator(new QIntValidator(this));

    connect(button, &QToolButton::clicked, this, &ObjectRefEdit::onButtonClicked);
    connect(mLine, &QLineEdit::editingFinished, this, &ObjectRefEdit::onIdChanged);
}

int ObjectRefEdit::id() const
{
    return mId;
}

void ObjectRefEdit::setId(int id)
{
    mId = id;
    mLine->setText(QString::number(id));
    emit idChanged(mId);
}

void ObjectRefEdit::onIdChanged()
{
    setId(mLine->text().toInt());
}

void ObjectRefEdit::onButtonClicked()
{
    ObjectRefDialog dialog(this);
    dialog.setId(mId);

    if (dialog.exec() != QDialog::Accepted)
        return;

    int newId = dialog.id();

    if (newId != mId) {
        setId(newId);
    }
}

} // namespace Tiled
