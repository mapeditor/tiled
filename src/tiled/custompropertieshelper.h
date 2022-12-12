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

class QtAbstractPropertyBrowser;
class QtProperty;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace Tiled {

class MapDocument;
class PropertyType;
class VariantEditorFactory;

class CustomPropertiesHelper : public QObject
{
    Q_OBJECT

public:
    CustomPropertiesHelper(QtAbstractPropertyBrowser *propertyBrowser,
                           QObject *parent = nullptr);
    ~CustomPropertiesHelper() override;

    QtVariantProperty *createProperty(const QString &name, const QVariant &value);
    void deleteProperty(QtProperty *property);
    void clear();
    bool hasProperty(QtProperty *property) const;
    QtVariantProperty *property(const QString &name);
    const QHash<QString, QtVariantProperty *> &properties() const;

    QVariant toDisplayValue(QVariant value) const;
    QVariant fromDisplayValue(QtProperty *property, QVariant value) const;

    void setMapDocument(MapDocument *mapDocument);

signals:
    void propertyMemberValueChanged(const QStringList &path, const QVariant &value);
    void recreateProperty(QtVariantProperty *property, const QVariant &value);

private:
    QtVariantProperty *createPropertyInternal(const QString &name,
                                              const QVariant &value);
    void deletePropertyInternal(QtProperty *property);
    void deleteSubProperties(QtProperty *property);

    void onValueChanged(QtProperty *property, const QVariant &value);
    void resetProperty(QtProperty *property);
    void propertyTypesChanged();

    void setPropertyAttributes(QtVariantProperty *property,
                               const PropertyType &propertyType);

    const PropertyType *propertyType(QtProperty *property) const;
    QStringList propertyPath(QtProperty *property) const;

    QtAbstractPropertyBrowser *mPropertyBrowser;
    QtVariantPropertyManager *mPropertyManager;
    MapDocument *mMapDocument = nullptr;
    QHash<QString, QtVariantProperty *> mProperties;
    QHash<QtProperty *, int> mPropertyTypeIds;
    QHash<QtProperty *, QtProperty *> mPropertyParents;
    bool mUpdating = false;
    bool mEmittingValueChanged = false;
};

inline bool CustomPropertiesHelper::hasProperty(QtProperty *property) const
{
    return mPropertyTypeIds.contains(property) && !mPropertyParents.contains(property);
}

inline QtVariantProperty *CustomPropertiesHelper::property(const QString &name)
{
    return mProperties.value(name);
}

inline const QHash<QString, QtVariantProperty *> &CustomPropertiesHelper::properties() const
{
    return mProperties;
}

inline void CustomPropertiesHelper::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
}

} // namespace Tiled
