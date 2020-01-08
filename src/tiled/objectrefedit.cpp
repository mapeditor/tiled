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
#include "logginginterface.h"
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

    mLineEdit->setValidator(new QIntValidator(this));

    connect(button, &QToolButton::clicked, this, &ObjectRefEdit::onButtonClicked);
    connect(mLineEdit, &QLineEdit::editingFinished,
            [this] { setId(mLineEdit->text().toInt()); });
}

void ObjectRefEdit::setId(int id)
{
    if (mId == id)
        return;

    mId = id;
    mLineEdit->setText(QString::number(id));
    emit idChanged(mId);
}

void ObjectRefEdit::onButtonClicked()
{
    ObjectRefDialog dialog(this);
    dialog.setId(mId);

    if (dialog.exec() == QDialog::Accepted)
        setId(dialog.id());
}

} // namespace Tiled
