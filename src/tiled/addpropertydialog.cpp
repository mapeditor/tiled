/*
 * addpropertydialog.cpp
 * Copyright 2015, CaptainFrog <jwilliam.perreault@gmail.com>
 * Copyright 2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addpropertydialog.h"
#include "ui_addpropertydialog.h"

#include "documentmanager.h"
#include "object.h"
#include "preferences.h"
#include "properties.h"
#include "propertytypesmodel.h"
#include "session.h"
#include "utils.h"

#include <QPushButton>

using namespace Tiled;

namespace session {
static SessionOption<QString> propertyType { "property.type", QStringLiteral("string") };
} // namespace session

AddPropertyDialog::AddPropertyDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::AddPropertyDialog)
{
    initialize(nullptr);
}

AddPropertyDialog::AddPropertyDialog(const ClassPropertyType *parentClassType, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::AddPropertyDialog)
{
    initialize(parentClassType);
}

void AddPropertyDialog::initialize(const Tiled::ClassPropertyType *parentClassType)
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    const QIcon plain(QStringLiteral("://images/scalable/property-type-plain.svg"));

    // Add possible types from QVariant
    mUi->typeBox->addItem(plain, typeToName(QMetaType::Bool),      false);
    mUi->typeBox->addItem(plain, typeToName(QMetaType::QColor),    QColor());
    mUi->typeBox->addItem(plain, typeToName(QMetaType::Double),    0.0);
    mUi->typeBox->addItem(plain, typeToName(filePathTypeId()),     QVariant::fromValue(FilePath()));
    mUi->typeBox->addItem(plain, typeToName(QMetaType::Int),       0);
    mUi->typeBox->addItem(plain, typeToName(objectRefTypeId()),    QVariant::fromValue(ObjectRef()));
    mUi->typeBox->addItem(plain, typeToName(QMetaType::QString),   QString());

    for (const auto propertyType : Object::propertyTypes()) {
        // Avoid suggesting the creation of circular dependencies between types
        if (parentClassType && !parentClassType->canAddMemberOfType(propertyType))
            continue;

        // Avoid suggesting classes not meant to be used as property value
        if (propertyType->isClass())
            if (!static_cast<const ClassPropertyType*>(propertyType)->isPropertyValueType())
                continue;

        const QVariant var = propertyType->wrap(propertyType->defaultValue());
        const QIcon icon = PropertyTypesModel::iconForPropertyType(propertyType->type);
        mUi->typeBox->addItem(icon, propertyType->name, var);
    }

    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    // Restore previously used type
    mUi->typeBox->setCurrentText(session::propertyType);

    connect(mUi->name, &QLineEdit::textChanged,
            this, &AddPropertyDialog::nameChanged);
    connect(mUi->typeBox, &QComboBox::currentTextChanged,
            this, &AddPropertyDialog::typeChanged);

    mUi->name->setFocus();
}

AddPropertyDialog::~AddPropertyDialog()
{
    delete mUi;
}

QString AddPropertyDialog::propertyName() const
{
    return mUi->name->text();
}

QVariant AddPropertyDialog::propertyValue() const
{
    return mUi->typeBox->currentData();
}

void AddPropertyDialog::nameChanged(const QString &text)
{
    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void AddPropertyDialog::typeChanged(const QString &text)
{
    session::propertyType = text;
}

#include "moc_addpropertydialog.cpp"
