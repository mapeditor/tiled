/*
 * objecttypesmodel.cpp
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

#include "objecttypesmodel.h"

using namespace Tiled;
using namespace Tiled::Internal;

static bool objectTypeLessThan(const ObjectType &a, const ObjectType &b)
{
    return a.name.toLower() < b.name.toLower();
}

void ObjectTypesModel::setObjectTypes(const ObjectTypes &objectTypes)
{
    beginResetModel();
    mObjectTypes = objectTypes;
    qSort(mObjectTypes.begin(), mObjectTypes.end(), objectTypeLessThan);
    endResetModel();
}

ObjectType ObjectTypesModel::objectTypeAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return ObjectType();

    return mObjectTypes.at(index.row());
}

int ObjectTypesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mObjectTypes.size();
}

int ObjectTypesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant ObjectTypesModel::headerData(int section,
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

QVariant ObjectTypesModel::data(const QModelIndex &index, int role) const
{
    // QComboBox requests data for an invalid index when the model is empty
    if (!index.isValid())
        return QVariant();

    const ObjectType &objectType = mObjectTypes.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        if (index.column() == 0)
            return objectType.name;

    if (role == ColorRole && index.column() == 1)
        return objectType.color;

    return QVariant();
}

bool ObjectTypesModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        const int oldRow = index.row();

        ObjectType objectType = mObjectTypes.at(oldRow);
        objectType.name = value.toString().trimmed();

        auto nextObjectType = std::lower_bound(mObjectTypes.constBegin(),
                                               mObjectTypes.constEnd(),
                                               objectType,
                                               objectTypeLessThan);

        const int newRow = nextObjectType - mObjectTypes.constBegin();
        // QVector::move works differently from beginMoveRows
        const int moveToRow = newRow > oldRow ? newRow - 1 : newRow;

        mObjectTypes[oldRow].name = objectType.name;
        emit dataChanged(index, index);

        if (moveToRow != oldRow) {
            Q_ASSERT(newRow != oldRow);
            Q_ASSERT(newRow != oldRow + 1);
            beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
            mObjectTypes.move(oldRow, moveToRow);
            endMoveRows();
        }
        return true;
    }
    return false;
}

Qt::ItemFlags ObjectTypesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

void ObjectTypesModel::setObjectTypeColor(int objectIndex, const QColor &color)
{
    mObjectTypes[objectIndex].color = color;

    const QModelIndex mi = index(objectIndex, 1);
    emit dataChanged(mi, mi);
}

void ObjectTypesModel::setObjectTypeProperties(int objectIndex,
                                               const Properties &properties)
{
    mObjectTypes[objectIndex].defaultProperties = properties;
}

void ObjectTypesModel::removeObjectTypes(const QModelIndexList &indexes)
{
    QVector<int> rows;
    for (const QModelIndex &index : indexes)
        rows.append(index.row());

    qSort(rows);

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mObjectTypes.remove(row);
        endRemoveRows();
    }
}

QModelIndex ObjectTypesModel::addNewObjectType()
{
    beginInsertRows(QModelIndex(), 0, 0);
    mObjectTypes.prepend(ObjectType());
    endInsertRows();
    return index(0, 0);
}
