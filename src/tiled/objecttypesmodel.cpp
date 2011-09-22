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

void ObjectTypesModel::setObjectTypes(const ObjectTypes &objectTypes)
{
    beginResetModel();
    mObjectTypes = objectTypes;
    endResetModel();
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
        mObjectTypes[index.row()].name = value.toString().trimmed();
        emit dataChanged(index, index);
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

void ObjectTypesModel::removeObjectTypes(const QModelIndexList &indexes)
{
    QVector<int> rows;
    foreach (const QModelIndex &index, indexes)
        rows.append(index.row());

    qSort(rows);

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mObjectTypes.remove(row);
        endRemoveRows();
    }
}

void ObjectTypesModel::appendNewObjectType()
{
    beginInsertRows(QModelIndex(), mObjectTypes.size(), mObjectTypes.size());
    mObjectTypes.append(ObjectType());
    endInsertRows();
}
