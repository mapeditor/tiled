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

using namespace Tiled;

static bool propertyTypeLessThan(const PropertyType &a, const PropertyType &b)
{
    return a.name.toLower() < b.name.toLower();
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

int PropertyTypesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant PropertyTypesModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return tr("Type");
            case 1:
                return tr("Color");
            }
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }
    }

    return QVariant();
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

    if (role == ColorRole && index.column() == 1)
        return propertyType.color;

    return QVariant();
}

bool PropertyTypesModel::setData(const QModelIndex &index,
                                 const QVariant &value,
                                 int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        const int oldRow = index.row();

        PropertyType propertyType = mPropertyTypes.at(oldRow);
        propertyType.name = value.toString().trimmed();

        auto nextPropertyType = std::lower_bound(mPropertyTypes.constBegin(),
                                                 mPropertyTypes.constEnd(),
                                                 propertyType,
                                                 propertyTypeLessThan);

        const int newRow = nextPropertyType - mPropertyTypes.constBegin();
        // QVector::move works differently from beginMoveRows
        const int moveToRow = newRow > oldRow ? newRow - 1 : newRow;

        mPropertyTypes[oldRow].name = propertyType.name;
        emit dataChanged(index, index);

        if (moveToRow != oldRow) {
            Q_ASSERT(newRow != oldRow);
            Q_ASSERT(newRow != oldRow + 1);
            beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
            mPropertyTypes.move(oldRow, moveToRow);
            endMoveRows();
        }
        return true;
    }
    return false;
}

Qt::ItemFlags PropertyTypesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

void PropertyTypesModel::setPropertyTypeColor(int objectIndex, const QColor &color)
{
    mPropertyTypes[objectIndex].color = color;

    const QModelIndex mi = index(objectIndex, 1);
    emit dataChanged(mi, mi);
}

void PropertyTypesModel::setPropertyTypeValues(int objectIndex,
                                               const QStringList &values)
{
    mPropertyTypes[objectIndex].values = values;
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
    beginInsertRows(QModelIndex(), 0, 0);

    PropertyType propertyType;
    propertyType.id = ++PropertyType::nextId;
    mPropertyTypes.prepend(propertyType);

    endInsertRows();
    return index(0, 0);
}
