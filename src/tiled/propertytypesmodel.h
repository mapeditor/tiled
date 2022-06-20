/*
 * propertytypesmodel.h
 * Copyright 2011-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "object.h"
#include "properties.h"
#include "propertytype.h"

#include <QAbstractListModel>
#include <QIcon>
#include <QStringList>

namespace Tiled {

class PropertyTypesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    PropertyTypesModel(QWidget *parent);

    void setPropertyTypes(const SharedPropertyTypes &propertyTypes);
    SharedPropertyTypes propertyTypes() const;

    PropertyType *propertyTypeAt(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setPropertyTypeName(int row, const QString &name);
    void removePropertyTypes(const QModelIndexList &indexes);

    QModelIndex addNewPropertyType(PropertyType::Type type);
    QModelIndex addPropertyType(std::unique_ptr<PropertyType> type);

    void importPropertyTypes(PropertyTypes typesToImport);
    void importObjectTypes(const QVector<ObjectType> &objectTypes);

    static QIcon iconForPropertyType(PropertyType::Type type);

signals:
    void nameChanged(const QModelIndex &index, const PropertyType &type);

private:
    bool checkTypeNameUnused(const QString &name) const;

    QString nextPropertyTypeName(PropertyType::Type type) const;

    QWidget *mParentWidget;
    SharedPropertyTypes mPropertyTypes;
};

inline SharedPropertyTypes PropertyTypesModel::propertyTypes() const
{
    return mPropertyTypes;
}

} // namespace Tiled
