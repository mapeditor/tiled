/*
 * variantpropertymanager.cpp
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "variantpropertymanager.h"

namespace Tiled {
namespace Internal {

class FilePathPropertyType
{
};

int VariantPropertyManager::filePathTypeId()
{
    return qMetaTypeId<FilePathPropertyType>();
}

bool VariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == filePathTypeId())
        return true;
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantPropertyManager::valueType(int propertyType) const
{
    if (propertyType == filePathTypeId())
        return QVariant::String;
    return QtVariantPropertyManager::valueType(propertyType);
}

QVariant VariantPropertyManager::value(const QtProperty *property) const
{
    if (mValues.contains(property))
        return mValues[property].value;
    return QtVariantPropertyManager::value(property);
}

QStringList VariantPropertyManager::attributes(int propertyType) const
{
    if (propertyType == filePathTypeId()) {
        QStringList attr;
        attr << QLatin1String("filter");
        return attr;
    }
    return QtVariantPropertyManager::attributes(propertyType);
}

int VariantPropertyManager::attributeType(int propertyType,
                                          const QString &attribute) const
{
    if (propertyType == filePathTypeId()) {
        if (attribute == QLatin1String("filter"))
            return QVariant::String;
        return 0;
    }
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant VariantPropertyManager::attributeValue(const QtProperty *property,
                                                const QString &attribute) const
{
    if (mValues.contains(property)) {
        if (attribute == QLatin1String("filter"))
            return mValues[property].filter;
        return QVariant();
    }
    if (attribute == mSuggestionsAttribute && mSuggestions.contains(property))
        return mSuggestions[property];

    return QtVariantPropertyManager::attributeValue(property, attribute);
}

QString VariantPropertyManager::valueText(const QtProperty *property) const
{
    if (mValues.contains(property))
        return mValues[property].value;
    return QtVariantPropertyManager::valueText(property);
}

void VariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
    if (mValues.contains(property)) {
        if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
            return;
        QString str = qVariantValue<QString>(val);
        Data d = mValues[property];
        if (d.value == str)
            return;
        d.value = str;
        mValues[property] = d;
        emit propertyChanged(property);
        emit valueChanged(property, str);
        return;
    }
    QtVariantPropertyManager::setValue(property, val);
}

void VariantPropertyManager::setAttribute(QtProperty *property,
                                          const QString &attribute,
                                          const QVariant &val)
{
    if (mValues.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
                return;
            QString str = qVariantValue<QString>(val);
            Data d = mValues[property];
            if (d.filter == str)
                return;
            d.filter = str;
            mValues[property] = d;
            emit attributeChanged(property, attribute, str);
        }
        return;
    }

    if (attribute == mSuggestionsAttribute && mSuggestions.contains(property))
        mSuggestions[property] = qVariantValue<QStringList>(val);

    QtVariantPropertyManager::setAttribute(property, attribute, val);
}

void VariantPropertyManager::initializeProperty(QtProperty *property)
{
    const int type = propertyType(property);
    if (type == filePathTypeId())
        mValues[property] = Data();
    else if (type == QVariant::String)
        mSuggestions[property] = QStringList();

    QtVariantPropertyManager::initializeProperty(property);
}

void VariantPropertyManager::uninitializeProperty(QtProperty *property)
{
    mValues.remove(property);
    mSuggestions.remove(property);
    QtVariantPropertyManager::uninitializeProperty(property);
}

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::FilePathPropertyType)
