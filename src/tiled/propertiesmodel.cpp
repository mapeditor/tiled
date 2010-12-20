/*
 * propertiesmodel.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertiesmodel.h"

using namespace Tiled;
using namespace Tiled::Internal;

PropertiesModel::PropertiesModel(QObject *parent):
    QAbstractTableModel(parent)
{
}

int PropertiesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (mKeys.size() + 1);
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.row() < mKeys.size()) {
            const QString &key = mKeys.at(index.row());
            switch (index.column()) {
                case 0: return key;
                case 1: return mProperties.value(key);
            }
        } else if (index.column() == 0) {
            return (role == Qt::EditRole) ? QString() : tr("<new property>");
        }
    }
    return QVariant();
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.row() < mKeys.size() || index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0) { // Edit name
        QString text = value.toString();
        if (index.row() == mKeys.size()) {
            // Add a new property
            if (text.isEmpty())
                return false;
            mProperties.insert(text, QString());
        } else {
            const QString &key = mKeys.at(index.row());
            const QString propertyValue = mProperties.value(key);
            mProperties.remove(key);
            mProperties.insert(text, propertyValue);
        }
        // Have to request keys and reset because of possible reordering
        mKeys = mProperties.keys();
        reset();
        return true;
    }
    else if (index.column() == 1) { // Edit value
        const QString &key = mKeys.at(index.row());
        mProperties.insert(key, value.toString());
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

void PropertiesModel::deleteProperties(const QModelIndexList &indices)
{
    QList<QString> keys;
    foreach (const QModelIndex &index, indices) {
        if (index.row() < mKeys.size())
            keys.append(mKeys.at(index.row()));
    }
    foreach (const QString &key, keys) {
        const int row = mKeys.indexOf(key);
        beginRemoveRows(QModelIndex(), row, row);
        mProperties.remove(key);
        mKeys = mProperties.keys();
        endRemoveRows();
    }
}

QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    static QString sectionHeaders[] = {
        tr("Name"),
        tr("Value")
    };
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal
            && section < 2) {
        return sectionHeaders[section];
    }
    return QVariant();
}

void PropertiesModel::setProperties(const Properties &properties)
{
    mProperties = properties;
    mKeys = mProperties.keys();
    reset();
}

const Properties &PropertiesModel::properties() const
{
    return mProperties;
}
