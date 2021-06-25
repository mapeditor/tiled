/*
 * custompropertieshelper.cpp
 * Copyright 2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "custompropertieshelper.h"

#include "object.h"
#include "preferences.h"
#include "propertytype.h"
#include "variantpropertymanager.h"

namespace Tiled {

CustomPropertiesHelper::CustomPropertiesHelper(QtVariantPropertyManager *propertyManager,
                                               QObject *parent)
    : QObject(parent)
    , mPropertyManager(propertyManager)
{
    connect(Preferences::instance(), &Preferences::propertyTypesChanged,
            this, &CustomPropertiesHelper::propertyTypesChanged);
}

QtVariantProperty *CustomPropertiesHelper::createProperty(const QString &name,
                                                          const QVariant &value)
{
    Q_ASSERT(!mProperties.contains(name));

    int type = value.userType();

    const PropertyType *propertyType = nullptr;

    if (type == propertyValueId()) {
        const PropertyValue propertyValue = value.value<PropertyValue>();
        propertyType = propertyValue.type();
        // todo: support more than just enum properties
        type = QtVariantPropertyManager::enumTypeId();
    }

    if (type == objectRefTypeId())
        type = VariantPropertyManager::displayObjectRefTypeId();

    QtVariantProperty *property = mPropertyManager->addProperty(type, name);
    if (!property) {
        // fall back to string property for unsupported property types
        property = mPropertyManager->addProperty(QMetaType::QString, name);
    }

    mProperties.insert(name, property);

    if (type == QMetaType::Bool)
        property->setAttribute(QLatin1String("textVisible"), false);
    if (type == QMetaType::QString)
        property->setAttribute(QLatin1String("multiline"), true);
    if (type == QMetaType::Double)
        property->setAttribute(QLatin1String("decimals"), 9);

    if (propertyType) {
        mPropertyTypeIds.insert(property, propertyType->id);
        setPropertyAttributes(property, *propertyType);
    } else {
        mPropertyTypeIds.insert(property, 0);
    }

    property->setValue(toDisplayValue(value));

    return property;
}

void CustomPropertiesHelper::deleteProperty(QtProperty *property)
{
    Q_ASSERT(mPropertyTypeIds.contains(property));
    mProperties.remove(property->propertyName());
    mPropertyTypeIds.remove(property);
    delete property;
}

void CustomPropertiesHelper::clear()
{
    qDeleteAll(mProperties);
    mProperties.clear();
    mPropertyTypeIds.clear();
}

QVariant CustomPropertiesHelper::toDisplayValue(QVariant value) const
{
    if (value.userType() == propertyValueId())
        value = value.value<PropertyValue>().value;

    if (value.userType() == objectRefTypeId())
        value = QVariant::fromValue(DisplayObjectRef { value.value<ObjectRef>(), mMapDocument });

    return value;
}

QVariant CustomPropertiesHelper::fromDisplayValue(QtProperty *property,
                                                  QVariant value) const
{
    if (value.userType() == VariantPropertyManager::displayObjectRefTypeId())
        value = QVariant::fromValue(value.value<DisplayObjectRef>().ref);

    if (const auto typeId = mPropertyTypeIds.value(property))
        if (auto type = Object::propertyType(typeId))
            value = type->wrap(value);

    return value;
}

void CustomPropertiesHelper::propertyTypesChanged()
{
    for (const auto &type : Object::propertyTypes()) {
        QHashIterator<QtProperty *, int> it(mPropertyTypeIds);
        while (it.hasNext()) {
            it.next();

            if (it.value() == type.id)
                setPropertyAttributes(it.key(), type);
        }
    }
}

void CustomPropertiesHelper::setPropertyAttributes(QtProperty *property, const PropertyType &propertyType)
{
    // TODO: Support icons for enum values
    mPropertyManager->setAttribute(property, QStringLiteral("enumNames"), propertyType.values);
}

} // namespace Tiled

#include "moc_custompropertieshelper.cpp"
