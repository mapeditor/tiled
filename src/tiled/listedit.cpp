/*
 * listedit.h
 * Copyright 2024, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "listedit.h"

#include "utils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace Tiled {

ListEdit::ListEdit(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mLabel = new QLabel{this};
    mLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    QToolButton *addButton = new QToolButton{this};
    addButton->setIcon(QIcon(QStringLiteral(":/images/22/add.png")));
    Utils::setThemeIcon(addButton, "add");

    QToolButton *editButton = new QToolButton{this};
    editButton->setSizePolicy(QSizePolicy{QSizePolicy::Fixed, QSizePolicy::Preferred});
    editButton->setText(tr("Edit..."));
    layout->addWidget(mLabel);
    layout->addWidget(addButton);
    layout->addWidget(editButton);

    setFocusProxy(editButton);
    setFocusPolicy(Qt::StrongFocus);

    connect(addButton, &QToolButton::clicked,
            this, &ListEdit::addButtonClicked);
    connect(editButton, &QToolButton::clicked,
            this, &ListEdit::editButtonClicked);
}

void ListEdit::setValue(const QVariantList &value)
{
    mValue = value;
    mLabel->setText(valueText(value));
}

QString ListEdit::valueText(const QVariantList &value)
{
    return value.isEmpty() ? tr("<empty>")
                           : tr("%1 items").arg(value.count());
}

void ListEdit::addButtonClicked()
{
    // todo: spawn a kind of "add property" dialog, but without a name field?
    // or maybe add button is a dropdown with the available types?
    mValue.append(0);
    mLabel->setText(valueText(mValue));
    emit valueChanged(mValue);
}

void ListEdit::editButtonClicked()
{
    // todo: spawn list edit dialog
}

} // namespace Tiled

#include "moc_listedit.cpp"
