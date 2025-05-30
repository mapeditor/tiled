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

#include "properties.h"
#include "propertytypesmodel.h"
#include "utils.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolButton>

namespace Tiled {

ListEdit::ListEdit(QWidget *parent)
    : QWidget{parent}
{
    auto *layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    mLabel = new QLabel{this};
    mLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    mAddMenu = new QMenu{this};

    mAddButton = new QToolButton{this};
    mAddButton->setIcon(QIcon(QStringLiteral(":/images/22/add.png")));
    mAddButton->setText(tr("Add"));
    mAddButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mAddButton->setMenu(mAddMenu);
    mAddButton->setPopupMode(QToolButton::MenuButtonPopup);
    Utils::setThemeIcon(mAddButton, "add");

    layout->addWidget(mLabel);
    layout->addWidget(mAddButton);

    setFocusProxy(mAddButton);
    setFocusPolicy(Qt::StrongFocus);

    connect(mAddButton, &QToolButton::clicked,
            this, &ListEdit::addButtonClicked);
    connect(mAddMenu, &QMenu::aboutToShow,
            this, &ListEdit::populateAddMenu);

    connect(mAddMenu, &QMenu::triggered, this, [this](QAction *action) {
        emit appendValue(action->data());
    });
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
    if (mValue.isEmpty()) {
        mAddButton->showMenu();
        return;
    }

    const auto &lastValue = mValue.last();
    QVariant newValue;

    if (lastValue.userType() == propertyValueId()) {
        const auto propertyValue = lastValue.value<PropertyValue>();
        if (auto type = propertyValue.type())
            newValue = type->wrap(type->defaultValue());
        else
            newValue = QVariant::fromValue(PropertyValue { QVariant(), propertyValue.typeId });
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        newValue = QVariant(lastValue.metaType());
#else
        newValue = QVariant(lastValue.userType(), nullptr);
#endif
    }
    emit appendValue(newValue);
}

void ListEdit::populateAddMenu()
{
    mAddMenu->clear();

    const QVariantList values = possiblePropertyValues(nullptr);
    for (const auto &value : values) {
        const QIcon icon = PropertyTypesModel::iconForProperty(value);
        auto action = mAddMenu->addAction(icon, userTypeName(value));
        action->setData(value);
    }
}

} // namespace Tiled

#include "moc_listedit.cpp"
