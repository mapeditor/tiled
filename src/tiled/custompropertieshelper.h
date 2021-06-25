/*
 * custompropertieshelper.h
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

#pragma once

#include <QHash>
#include <QVariant>

class QtProperty;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace Tiled {

class MapDocument;
class PropertyType;

class CustomPropertiesHelper : public QObject
{
    Q_OBJECT

public:
    CustomPropertiesHelper(QtVariantPropertyManager *propertyManager,
                           QObject *parent = nullptr);

    QtVariantProperty *createProperty(const QString &name, const QVariant &value);
    void deleteProperty(QtProperty *property);
    void clear();
    bool hasProperty(QtProperty *property) const;
    QtVariantProperty *property(const QString &name);

    QVariant toDisplayValue(QVariant value) const;
    QVariant fromDisplayValue(QtProperty *property, QVariant value) const;

    void setMapDocument(MapDocument *mapDocument);

private:
    void propertyTypesChanged();
    void setPropertyAttributes(QtProperty *property, const PropertyType &propertyType);

    QtVariantPropertyManager *mPropertyManager;
    MapDocument *mMapDocument = nullptr;
    QHash<QString, QtVariantProperty *> mProperties;
    QHash<QtProperty *, int> mPropertyTypeIds;
};

inline bool CustomPropertiesHelper::hasProperty(QtProperty *property) const
{
    return mPropertyTypeIds.contains(property);
}

inline QtVariantProperty *CustomPropertiesHelper::property(const QString &name)
{
    return mProperties.value(name);
}

inline void CustomPropertiesHelper::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
}

} // namespace Tiled
