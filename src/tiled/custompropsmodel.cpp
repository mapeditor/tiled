/*
 * custompropsmodel.cpp
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

#include "custompropsmodel.h"

using namespace Tiled;

static bool customPropLessThan(const CustomProp &a, const CustomProp &b)
{
    return a.name.toLower() < b.name.toLower();
}

void CustomPropsModel::setCustomProps(const CustomProps &customProps)
{
    beginResetModel();
    mCustomProps = customProps;
    std::sort(mCustomProps.begin(), mCustomProps.end(), customPropLessThan);
    endResetModel();
}

CustomProp CustomPropsModel::customPropAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return CustomProp();

    return mCustomProps.at(index.row());
}

int CustomPropsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mCustomProps.size();
}

int CustomPropsModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant CustomPropsModel::headerData(int section,
                                      Qt::Orientation orientation,
                                      int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return tr("Property");
            case 1:
                return tr("Color");
            }
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignLeft;
        }
    }

    return QVariant();
}

QVariant CustomPropsModel::data(const QModelIndex &index, int role) const
{
    // QComboBox requests data for an invalid index when the model is empty
    if (!index.isValid())
        return QVariant();

    const CustomProp &customProp = mCustomProps.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        if (index.column() == 0)
            return customProp.name;

    if (role == ColorRole && index.column() == 1)
        return customProp.color;

    return QVariant();
}

bool CustomPropsModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        const int oldRow = index.row();

        CustomProp customProp = mCustomProps.at(oldRow);
        customProp.name = value.toString().trimmed();

        auto nextCustomProp = std::lower_bound(mCustomProps.constBegin(),
                                               mCustomProps.constEnd(),
                                               customProp,
                                               customPropLessThan);

        const int newRow = nextCustomProp - mCustomProps.constBegin();
        // QVector::move works differently from beginMoveRows
        const int moveToRow = newRow > oldRow ? newRow - 1 : newRow;

        mCustomProps[oldRow].name = customProp.name;
        emit dataChanged(index, index);

        if (moveToRow != oldRow) {
            Q_ASSERT(newRow != oldRow);
            Q_ASSERT(newRow != oldRow + 1);
            beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), newRow);
            mCustomProps.move(oldRow, moveToRow);
            endMoveRows();
        }
        return true;
    }
    return false;
}

Qt::ItemFlags CustomPropsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

void CustomPropsModel::setCustomPropColor(int objectIndex, const QColor &color)
{
    mCustomProps[objectIndex].color = color;

    const QModelIndex mi = index(objectIndex, 1);
    emit dataChanged(mi, mi);
}

void CustomPropsModel::setCustomPropValues(int objectIndex,
                                               const QStringList &values)
{
    mCustomProps[objectIndex].values = values;
    mCustomProps[objectIndex].validateValues();
}

void CustomPropsModel::removeCustomProps(const QModelIndexList &indexes)
{
    QVector<int> rows;
    for (const QModelIndex &index : indexes)
        rows.append(index.row());

    std::sort(rows.begin(), rows.end());

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mCustomProps.remove(row);
        endRemoveRows();
    }
}

QModelIndex CustomPropsModel::addNewCustomProp()
{
    beginInsertRows(QModelIndex(), 0, 0);
    mCustomProps.prepend(CustomProp());
    endInsertRows();
    return index(0, 0);
}


