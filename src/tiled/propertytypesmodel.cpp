/*
 * propertytypesmodel.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertytypesmodel.h"

#include "containerhelpers.h"

using namespace Tiled;

static bool propertyTypeLessThan(const PropertyType &a, const PropertyType &b)
{
    return QString::localeAwareCompare(a.name, b.name) < 0;
}

void PropertyTypesModel::setPropertyTypes(const PropertyTypes &propertyTypes)
{
    beginResetModel();
    mPropertyTypes = propertyTypes;
    std::sort(mPropertyTypes.begin(), mPropertyTypes.end(), propertyTypeLessThan);
    endResetModel();
}

PropertyType PropertyTypesModel::propertyTypeAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return PropertyType();

    return mPropertyTypes.at(index.row());
}

int PropertyTypesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mPropertyTypes.size();
}

QVariant PropertyTypesModel::data(const QModelIndex &index, int role) const
{
    // QComboBox requests data for an invalid index when the model is empty
    if (!index.isValid())
        return QVariant();

    const PropertyType &propertyType = mPropertyTypes.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        if (index.column() == 0)
            return propertyType.name;

    return QVariant();
}

bool PropertyTypesModel::setData(const QModelIndex &index,
                                 const QVariant &value,
                                 int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        setPropertyTypeName(index.row(), value.toString());
        return true;
    }
    return false;
}

Qt::ItemFlags PropertyTypesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

void PropertyTypesModel::setPropertyTypeName(int row, const QString &name)
{
    PropertyType propertyType = mPropertyTypes.at(row);
    propertyType.name = name.trimmed();

    auto nextPropertyType = std::lower_bound(mPropertyTypes.constBegin(),
                                             mPropertyTypes.constEnd(),
                                             propertyType,
                                             propertyTypeLessThan);

    const int newRow = nextPropertyType - mPropertyTypes.constBegin();
    // QVector::move works differently from beginMoveRows
    const int moveToRow = newRow > row ? newRow - 1 : newRow;

    mPropertyTypes[row].name = propertyType.name;
    const auto index = this->index(row);
    emit nameChanged(index, mPropertyTypes[row]);
    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    if (moveToRow != row) {
        Q_ASSERT(newRow != row);
        Q_ASSERT(newRow != row + 1);
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), newRow);
        mPropertyTypes.move(row, moveToRow);
        endMoveRows();
    }
}

void PropertyTypesModel::setPropertyTypeValues(int index,
                                               const QStringList &values)
{
    mPropertyTypes[index].values = values;
}

void PropertyTypesModel::removePropertyTypes(const QModelIndexList &indexes)
{
    QVector<int> rows;
    for (const QModelIndex &index : indexes)
        rows.append(index.row());

    std::sort(rows.begin(), rows.end());

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mPropertyTypes.remove(row);
        endRemoveRows();
    }
}

QModelIndex PropertyTypesModel::addNewPropertyType()
{
    const int row = mPropertyTypes.size();
    beginInsertRows(QModelIndex(), row, row);

    PropertyType propertyType;
    propertyType.id = ++PropertyType::nextId;
    propertyType.name = nextPropertyTypeName();
    mPropertyTypes.append(propertyType);

    endInsertRows();
    return index(row, 0);
}

QString PropertyTypesModel::nextPropertyTypeName() const
{
    const auto baseText = tr("Enum");

    // Search for a unique value, starting from the current count
    int number = mPropertyTypes.count();
    QString name;
    do {
        name = baseText + QString::number(number++);
    } while (contains_where(mPropertyTypes,
                            [&] (const PropertyType &type) { return type.name == name; }));

    return name;
}
