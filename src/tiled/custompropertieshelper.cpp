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
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QScopedValueRollback>
#include <QDebug>

namespace Tiled {

CustomPropertiesHelper::CustomPropertiesHelper(QtAbstractPropertyBrowser *propertyBrowser,
                                               QObject *parent)
    : QObject(parent)
    , mPropertyBrowser(propertyBrowser)
    , mPropertyManager(new VariantPropertyManager(this))
{
    auto variantEditorFactory = new VariantEditorFactory(this);

    propertyBrowser->setFactoryForManager(mPropertyManager, variantEditorFactory);

    connect(mPropertyManager, &QtVariantPropertyManager::valueChanged,
            this, &CustomPropertiesHelper::onValueChanged);

    connect(variantEditorFactory, &VariantEditorFactory::resetProperty,
            this, &CustomPropertiesHelper::resetProperty);

    connect(Preferences::instance(), &Preferences::propertyTypesChanged,
            this, &CustomPropertiesHelper::propertyTypesChanged);
}

CustomPropertiesHelper::~CustomPropertiesHelper()
{
    mPropertyBrowser->unsetFactoryForManager(mPropertyManager);
}

QtVariantProperty *CustomPropertiesHelper::createProperty(const QString &name,
                                                          const QVariant &value)
{
    Q_ASSERT(!mProperties.contains(name));

    QScopedValueRollback<bool> updating(mUpdating, true);

    QtVariantProperty *property = createPropertyInternal(name, value);
    property->setValue(toDisplayValue(value));

    mProperties.insert(name, property);

    return property;
}

QtVariantProperty *CustomPropertiesHelper::createPropertyInternal(const QString &name,
                                                                  const QVariant &value)
{
    int type = value.userType();

    const PropertyType *propertyType = nullptr;

    if (type == propertyValueId()) {
        const PropertyValue propertyValue = value.value<PropertyValue>();
        propertyType = propertyValue.type();

        if (propertyType) {
            switch (propertyType->type) {
            case PropertyType::PT_Invalid:
                break;
            case PropertyType::PT_Class:
                type = VariantPropertyManager::unstyledGroupTypeId();
                break;
            case PropertyType::PT_Enum: {
                const auto &enumType = static_cast<const EnumPropertyType&>(*propertyType);
                if (enumType.valuesAsFlags)
                    type = QtVariantPropertyManager::flagTypeId();
                else
                    type = QtVariantPropertyManager::enumTypeId();
                break;
            }
            }
        }
    }

    if (type == objectRefTypeId())
        type = VariantPropertyManager::displayObjectRefTypeId();

    QtVariantProperty *property = mPropertyManager->addProperty(type, name);
    if (!property) {
        // fall back to string property for unsupported property types
        property = mPropertyManager->addProperty(QMetaType::QString, name);
    }

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

    return property;
}

void CustomPropertiesHelper::deleteProperty(QtProperty *property)
{
    Q_ASSERT(hasProperty(property));

    mProperties.remove(property->propertyName());
    deletePropertyInternal(property);
}

void CustomPropertiesHelper::deletePropertyInternal(QtProperty *property)
{
    Q_ASSERT(mPropertyTypeIds.contains(property));
    deleteSubProperties(property);
    mPropertyTypeIds.remove(property);
    delete property;
}

void CustomPropertiesHelper::deleteSubProperties(QtProperty *property)
{
    const auto subProperties = property->subProperties();
    for (QtProperty *subProperty : subProperties) {
        if (mPropertyParents.value(subProperty) == property) {
            deletePropertyInternal(subProperty);
            mPropertyParents.remove(subProperty);
        }
    }
}

void CustomPropertiesHelper::clear()
{
    QHashIterator<QtProperty *, int> it(mPropertyTypeIds);
    while (it.hasNext())
        delete it.next().key();

    mProperties.clear();
    mPropertyTypeIds.clear();
    mPropertyParents.clear();
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

    if (auto type = propertyType(property))
        value = type->wrap(value);

    return value;
}

void CustomPropertiesHelper::onValueChanged(QtProperty *property, const QVariant &value)
{
    if (!mPropertyTypeIds.contains(property))
        return;

    if (!mUpdating) {
        const auto propertyValue = fromDisplayValue(property, value);
        const auto path = propertyPath(property);

        QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
        emit propertyMemberValueChanged(path, propertyValue);
    }

    if (auto type = propertyType(property); type && type->isClass()) {
        // Apply the change to the children

        auto &members = static_cast<const ClassPropertyType&>(*type).members;

        const auto subProperties = property->subProperties();
        const auto map = value.toMap();

        QScopedValueRollback<bool> updating(mUpdating, true);

        for (QtProperty *subProperty : subProperties) {
            const auto name = subProperty->propertyName();
            const bool modified = map.contains(name);
            const auto value = modified ? map.value(name)
                                        : members.value(name);

            // Avoid setting child class members as modified, just because
            // the class definition sets different defaults on them.
            const bool isParentTopLevel = !mPropertyParents.contains(property);
            const bool isParentModified = property->isModified();
            subProperty->setModified(modified && (isParentTopLevel || isParentModified));

            static_cast<QtVariantProperty*>(subProperty)->setValue(toDisplayValue(value));
        }
    }
}

void CustomPropertiesHelper::resetProperty(QtProperty *property)
{
    // Reset class property value by removing it
    if (property->isModified()) {
        // Only nested properties are currently marked as "modified", so in
        // this case we rely on the handling of this signal
        emit propertyMemberValueChanged(propertyPath(property), QVariant());
        return;
    }

    // Some other type can reset their value
    auto typeId = mPropertyManager->propertyType(property);

    if (typeId == QMetaType::QColor)
        mPropertyManager->setValue(property, QColor());
    else if (typeId == VariantPropertyManager::displayObjectRefTypeId()) {
        mPropertyManager->setValue(property, toDisplayValue(QVariant::fromValue(ObjectRef())));
    } else {
        qWarning() << "Requested reset of unsupported type" << typeId << "for property" << property->propertyName();
    }
}

void CustomPropertiesHelper::propertyTypesChanged()
{
    // When this happens in response to emitting propertyValueChanged, it means
    // we have triggered a change in a class definition. In this case we should
    // not update ourselves.
    if (mEmittingValueChanged)
        return;

    QHashIterator<QString, QtVariantProperty *> it(mProperties);
    while (it.hasNext()) {
        it.next();
        const auto property = it.value();
        const auto typeId = mPropertyTypeIds.value(property);
        if (!typeId)
            continue;

        if (const auto type = Object::propertyTypes().findTypeById(typeId)) {
            setPropertyAttributes(property, *type);

            if (type->isClass()) {
                // Restore the existing member values
                QScopedValueRollback<bool> updating(mUpdating, true);
                onValueChanged(property, property->value());
            }
        }
    }
}

void CustomPropertiesHelper::setPropertyAttributes(QtVariantProperty *property,
                                                   const PropertyType &propertyType)
{
    switch (propertyType.type) {
    case Tiled::PropertyType::PT_Invalid:
        break;
    case Tiled::PropertyType::PT_Class: {
        const auto &classType = static_cast<const ClassPropertyType&>(propertyType);

        // Delete any existing sub-properties
        deleteSubProperties(property);

        // Create a sub-property for each member
        QMapIterator<QString, QVariant> it(classType.members);
        while (it.hasNext()) {
            it.next();
            const QString &name = it.key();
            const QVariant &value = it.value();

            auto subProperty = createPropertyInternal(name, value);
            property->addSubProperty(subProperty);
            mPropertyParents.insert(subProperty, property);
        }
        break;
    }
    case Tiled::PropertyType::PT_Enum: {
        const auto &enumType = static_cast<const EnumPropertyType&>(propertyType);
        const bool isFlags = property->propertyType() == QtVariantPropertyManager::flagTypeId();

        // Need to re-create the property when valuesAsFlags changed, but we
        // don't have access to the view.
        if (enumType.valuesAsFlags != isFlags) {
            emit recreateProperty(property, fromDisplayValue(property, property->value()));
            return;
        }

        // Setting these attributes leads to emission of valueChanged...
        QScopedValueRollback<bool> updating(mUpdating, true);

        if (enumType.valuesAsFlags) {
            mPropertyManager->setAttribute(property, QStringLiteral("flagNames"), enumType.values);
        } else {
            // TODO: Support icons for enum values
            mPropertyManager->setAttribute(property, QStringLiteral("enumNames"), enumType.values);
        }
        break;
    }
    }
}

const PropertyType *CustomPropertiesHelper::propertyType(QtProperty *property) const
{
    if (const auto typeId = mPropertyTypeIds.value(property))
        return Object::propertyTypes().findTypeById(typeId);
    return nullptr;
}

QStringList CustomPropertiesHelper::propertyPath(QtProperty *property) const
{
    QStringList path;

    if (auto parent = mPropertyParents.value(property))
        path = propertyPath(parent);

    path.append(property->propertyName());
    return path;
}

} // namespace Tiled

#include "moc_custompropertieshelper.cpp"
