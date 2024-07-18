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

/**
 * Constructs a custom properties helper that manages properties in the given
 * \a propertyBrowser.
 *
 * Uses its own VariantPropertyManager to create the properties and
 * instantiates a VariantEditorFactory which creates the editor widgets.
 */
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
    connect(variantEditorFactory, &VariantEditorFactory::removeProperty,
            this, &CustomPropertiesHelper::removeProperty);

    connect(Preferences::instance(), &Preferences::propertyTypesChanged,
            this, &CustomPropertiesHelper::propertyTypesChanged);
}

CustomPropertiesHelper::~CustomPropertiesHelper()
{
    mPropertyBrowser->unsetFactoryForManager(mPropertyManager);
}

/**
 * Creates a top-level property with the given \a name and \a value.
 *
 * The property is not added to the property browser. It should be added either
 * directly or to a suitable group property.
 *
 * The name of the property needs to be unique.
 */
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

/**
 * Implementation of property creation. Used by createProperty for creating
 * top-level properties and by setPropertyAttributes for creating nested
 * properties.
 *
 * The \a value is only used for determining the type of the property, it is
 * not set on the property.
 *
 * This function works recursively, creating nested properties for class
 * properties.
 */
QtVariantProperty *CustomPropertiesHelper::createPropertyInternal(const QString &name,
                                                                  const QVariant &value)
{
    const PropertyType *propertyType = nullptr;
    const int type = propertyTypeForValue(value, propertyType);
    return createPropertyInternal(name, type, propertyType);
}

QtVariantProperty *CustomPropertiesHelper::createPropertyInternal(const QString &name,
                                                                  int type,
                                                                  const PropertyType *propertyType)
{
    QtVariantProperty *property = mPropertyManager->addProperty(type, name);

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

int CustomPropertiesHelper::propertyTypeForValue(const QVariant &value,
                                                 const PropertyType *&propertyType) const
{
    int type = value.userType();

    propertyType = nullptr;

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

    // fall back to string property for unsupported property types
    if (!mPropertyManager->isPropertyTypeSupported(type))
        type = QMetaType::QString;

    return type;
}

/**
 * Deletes the given top-level property.
 *
 * Should only be used for properties created with createProperty.
 */
void CustomPropertiesHelper::deleteProperty(QtProperty *property)
{
    Q_ASSERT(hasProperty(property));

    mProperties.remove(property->propertyName());
    deletePropertyInternal(property);
}

/**
 * Implementation of property deletion. Used by deleteProperty for deleting
 * top-level properties and by deleteSubProperties for deleting nested
 * properties.
 *
 * This function works recursively, also deleting all nested properties.
 */
void CustomPropertiesHelper::deletePropertyInternal(QtProperty *property)
{
    Q_ASSERT(mPropertyTypeIds.contains(property));
    deleteSubProperties(property);
    mPropertyTypeIds.remove(property);
    delete property;
}

/**
 * Deletes all sub-properties of the given \a property.
 *
 * Used when a property is being deleted or before refreshing the nested
 * properties that represent class members.
 */
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

/**
 * Removes all properties.
 */
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
        value = QVariant::fromValue(DisplayObjectRef {
                                        value.value<ObjectRef>(),
                                        mMapDocument
                                    });

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

void CustomPropertiesHelper::applyValueChange(QtProperty *property, const QVariant &displayValue)
{
    const auto propertyValue = fromDisplayValue(property, displayValue);

    if (auto parent = static_cast<QtVariantProperty*>(mPropertyParents.value(property))) {
        // If this is a list value, we need to apply the new value to the list.
        if (parent->propertyType() == QMetaType::QVariantList) {
            auto variantList = parent->value().toList();
            const int index = parent->subProperties().indexOf(property);
            if (index != -1) {
                variantList[index] = propertyValue;

                QScopedValueRollback<bool> applyToParent(mNoApplyToChildren, true);
                parent->setValue(variantList);
            }
            return;
        }

        // If this is a class member that is part of a list, we need to bubble
        // up the value change to the parent class first.
        if (isPartOfList(parent)) {
            auto variantMap = parent->value().toMap();
            variantMap.insert(property->propertyName(), propertyValue);

            QScopedValueRollback<bool> applyToParent(mNoApplyToChildren, true);
            parent->setValue(variantMap);

            property->setModified(!variantMap.isEmpty());
            return;
        }
    }

    // In all other cases, we emit the value change and let the listener handle
    // it. This allows to apply the change to possibly multiple selected
    // objects slightly differently.
    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit propertyMemberValueChanged(propertyPath(property), propertyValue);
}

void CustomPropertiesHelper::applyChangeToChildren(QtProperty *property, const QVariant &displayValue)
{
    if (auto type = propertyType(property); type && type->isClass()) {
        // Apply the change to the children

        auto parent = static_cast<QtVariantProperty*>(mPropertyParents.value(property));
        auto &members = static_cast<const ClassPropertyType&>(*type).members;

        const auto subProperties = property->subProperties();
        const auto map = displayValue.toMap();

        QScopedValueRollback<bool> updating(mUpdating, true);

        for (QtProperty *subProperty : subProperties) {
            const auto name = subProperty->propertyName();
            const bool modified = map.contains(name);
            const auto value = toDisplayValue(modified ? map.value(name)
                                                       : members.value(name));

            // Avoid setting child class members as modified, just because
            // the class definition sets different defaults on them.
            const bool isParentTopLevel = !parent;
            const bool isParentInList = parent && parent->propertyType() == QMetaType::QVariantList;
            const bool isParentModified = property->isModified();
            subProperty->setModified(modified && (isParentTopLevel ||
                                                  isParentInList ||
                                                  isParentModified));

            static_cast<QtVariantProperty*>(subProperty)->setValue(value);
            applyChangeToChildren(subProperty, value);
        }
    }

    if (displayValue.userType() == QMetaType::QVariantList) {
        const auto values = displayValue.toList();

        QScopedValueRollback<bool> updating(mUpdating, true);

        // Delete any superfluous sub-properties
        auto subProperties = property->subProperties();
        while (subProperties.size() > values.size()) {
            auto subProperty = subProperties.takeLast();
            if (mPropertyParents.value(subProperty) == property) {
                deletePropertyInternal(subProperty);
                mPropertyParents.remove(subProperty);
            }
        }

        QtProperty *previousProperty = nullptr;

        // Set sub-property values, (re)creating them if necessary
        for (int i = 0; i < values.size(); ++i) {
            const auto &value = values.at(i);
            auto subProperty = static_cast<QtVariantProperty*>(subProperties.value(i));

            const PropertyType *propertyType = nullptr;
            const int type = propertyTypeForValue(value, propertyType);

            // If the value is of a different type, delete the property
            if (subProperty && subProperty->propertyType() != type) {
                deletePropertyInternal(subProperty);
                subProperty = nullptr;
            }

            if (!subProperty) {
                // Create a new property
                subProperty = createPropertyInternal(QString::number(i), type, propertyType);
                property->insertSubProperty(subProperty, previousProperty);
                mPropertyParents.insert(subProperty, property);
            } else if (propertyType && mPropertyTypeIds.value(subProperty) != propertyType->id) {
                // Adjust property in case its property type changed
                mPropertyTypeIds.insert(subProperty, propertyType->id);
                setPropertyAttributes(subProperty, *propertyType);
            }

            const auto displayValue = toDisplayValue(value);
            static_cast<QtVariantProperty*>(subProperty)->setValue(displayValue);
            applyChangeToChildren(subProperty, displayValue);

            previousProperty = subProperty;
        }
    }
}

void CustomPropertiesHelper::onValueChanged(QtProperty *property, const QVariant &displayValue)
{
    if (!mPropertyTypeIds.contains(property))
        return;

    if (!mUpdating)
        applyValueChange(property, displayValue);

    if (!mNoApplyToChildren) {
        QScopedValueRollback<bool> applyingToChildren(mNoApplyToChildren, true);
        applyChangeToChildren(property, displayValue);
    }
}

void CustomPropertiesHelper::resetProperty(QtProperty *property)
{
    // Reset class property value by removing it
    if (property->isModified()) {
        // Only class members are currently marked as "modified", but if there
        // is a list involved, we need to bubble up the value change to the
        // parent.
        if (isPartOfList(property)) {
            auto parent = static_cast<QtVariantProperty*>(mPropertyParents.value(property));
            if (parent) {
                auto variantMap = parent->value().toMap();
                variantMap.remove(property->propertyName());

                // Not setting mNoApplyToChildren here since we need this
                // change to be applied to the children as well.
                parent->setValue(variantMap);
            }
        } else {
            // No lists involved, so we can rely on the handling of this signal
            emit propertyMemberValueChanged(propertyPath(property), QVariant());
        }
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

void CustomPropertiesHelper::removeProperty(QtProperty *property)
{
    // Removing is only supported for list items for now
    auto parent = static_cast<QtVariantProperty*>(mPropertyParents.value(property));
    if (!parent)
        return;
    if (parent->propertyType() != QMetaType::QVariantList)
        return;

    auto variantList = parent->value().toList();
    const int index = parent->subProperties().indexOf(property);
    if (index != -1) {
        variantList.removeAt(index);

        // Not setting mNoApplyToChildren here since we need this
        // change to be applied to the children as well.
        parent->setValue(variantList);
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

/**
 * When the given \a propertyType is an EnumPropertyType, sets the appropriate
 * attributes on the given \a property.
 *
 * Also creates sub-properties for members when the given \a propertyType is a
 * ClassPropertyType.
 *
 * Called after property creation, as well as when the property types changed.
 */
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

bool CustomPropertiesHelper::isPartOfList(QtProperty *property) const
{
    if (auto parent = static_cast<QtVariantProperty*>(mPropertyParents.value(property)))
        return parent->propertyType() == QMetaType::QVariantList || isPartOfList(parent);

    return false;
}

} // namespace Tiled

#include "moc_custompropertieshelper.cpp"
