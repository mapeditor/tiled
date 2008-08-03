/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "propertiesmodel.h"

using namespace Tiled::Internal;

PropertiesModel::PropertiesModel(QObject *parent):
    QAbstractTableModel(parent)
{
}

int PropertiesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mKeys.size();
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const QString &key = mKeys.at(index.row());
        switch (index.column()) {
            case 0: return key;
            case 1: return mProperties.value(key);
        }
    }
    return QVariant();
}

QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("Value");
        }
    }
    return QVariant();
}

void PropertiesModel::setProperties(QMap<QString, QString> properties)
{
    mProperties = properties;
    mKeys = mProperties.keys();
    reset();
}
