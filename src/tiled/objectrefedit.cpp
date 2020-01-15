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

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

#include <climits>

namespace Tiled {

ObjectRefEdit::ObjectRefEdit(QWidget *parent)
    : QWidget(parent)
    , mLineEdit(new QLineEdit(this))
    , mObjectDialogButton(new QToolButton(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    setFocusProxy(mLineEdit);

    mObjectDialogButton->setText(QStringLiteral("..."));
    mObjectDialogButton->setAutoRaise(true);
    mObjectDialogButton->setEnabled(false);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mLineEdit);
    layout->addWidget(mObjectDialogButton);

    mLineEdit->setValidator(new QIntValidator(0, INT_MAX, this));

    connect(mObjectDialogButton, &QToolButton::clicked, this, &ObjectRefEdit::onButtonClicked);
    connect(mLineEdit, &QLineEdit::editingFinished, this, &ObjectRefEdit::onEditFinished);
}

void ObjectRefEdit::setValue(const DisplayObjectRef &value)
{
    if (mValue == value)
        return;

    mValue = value;
    mLineEdit->setText(QString::number(mValue.id()));
    mObjectDialogButton->setEnabled(mValue.mapDocument);

    emit valueChanged(mValue);
}

void ObjectRefEdit::onButtonClicked()
{
    if (!mValue.mapDocument)
        return;

    ObjectRefDialog dialog(mValue, this);

    if (dialog.exec() == QDialog::Accepted)
        setValue(dialog.value());
}

void ObjectRefEdit::onEditFinished()
{
    auto newValue = fromExportValue(mLineEdit->text(), objectRefTypeId()).value<ObjectRef>();
    setValue(DisplayObjectRef { newValue, mValue.mapDocument });
}

} // namespace Tiled
