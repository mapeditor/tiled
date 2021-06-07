/*
 * customtypesmodel.cpp
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

#include "customtypesmodel.h"

using namespace Tiled;

static bool customTypeLessThan(const CustomType &a, const CustomType &b)
{
    return a.name.toLower() < b.name.toLower();
}

void CustomTypesModel::setCustomTypes(const CustomTypes &customTypes)
{
    beginResetModel();
    mCustomTypes = customTypes;
    std::sort(mCustomTypes.begin(), mCustomTypes.end(), customTypeLessThan);
    endResetModel();
}

CustomType CustomTypesModel::customTypeAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return CustomType();

    return mCustomTypes.at(index.row());
}

int CustomTypesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mCustomTypes.size();
}

int CustomTypesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant CustomTypesModel::headerData(int section,
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

QVariant CustomTypesModel::data(const QModelIndex &index, int role) const
{
    // QComboBox requests data for an invalid index when the model is empty
    if (!index.isValid())
        return QVariant();

    const CustomType &customType = mCustomTypes.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        if (index.column() == 0)
            return customType.name;

    if (role == ColorRole && index.column() == 1)
        return customType.color;

    return QVariant();
}

bool CustomTypesModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        const int oldRow = index.row();

        CustomType customType = mCustomTypes.at(oldRow);
        customType.name = value.toString().trimmed();

        auto nextCustomType = std::lower_bound(mCustomTypes.constBegin(),
                                               mCustomTypes.constEnd(),
                                               customType,
                                               customTypeLessThan);

        const int newRow = nextCustomType - mCustomTypes.constBegin();
        // QVector::move works differently from beginMoveRows
        const int moveToRow = newRow > oldRow ? newRow - 1 : newRow;

        mCustomTypes[oldRow].name = customType.name;
        emit dataChanged(index, index);

        if (moveToRow != oldRow) {
            Q_ASSERT(newRow != oldRow);
            Q_ASSERT(newRow != oldRow + 1);
            beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
            mCustomTypes.move(oldRow, moveToRow);
            endMoveRows();
        }
        return true;
    }
    return false;
}

Qt::ItemFlags CustomTypesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

void CustomTypesModel::setCustomTypeColor(int objectIndex, const QColor &color)
{
    mCustomTypes[objectIndex].color = color;

    const QModelIndex mi = index(objectIndex, 1);
    emit dataChanged(mi, mi);
}

void CustomTypesModel::setCustomTypeValues(int objectIndex,
                                           const QStringList &values)
{
    mCustomTypes[objectIndex].values = values;
    mCustomTypes[objectIndex].validateValues();
}

void CustomTypesModel::removeCustomTypes(const QModelIndexList &indexes)
{
    QVector<int> rows;
    for (const QModelIndex &index : indexes)
        rows.append(index.row());

    std::sort(rows.begin(), rows.end());

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mCustomTypes.remove(row);
        endRemoveRows();
    }
}

QModelIndex CustomTypesModel::addNewCustomType()
{
    beginInsertRows(QModelIndex(), 0, 0);
    mCustomTypes.prepend(CustomType());
    endInsertRows();
    return index(0, 0);
}
