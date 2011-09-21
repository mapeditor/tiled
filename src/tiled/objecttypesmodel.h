/*
 * objecttypesmodel.h
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

#ifndef OBJECTTYPESMODEL_H
#define OBJECTTYPESMODEL_H

#include "preferences.h"

#include <QAbstractTableModel>

namespace Tiled {
namespace Internal {

class ObjectTypesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum { ColorRole = Qt::UserRole };

    ObjectTypesModel(QObject *parent = 0)
        : QAbstractTableModel(parent)
    {}

    void setObjectTypes(const ObjectTypes &objectTypes);

    const ObjectTypes &objectTypes() const { return mObjectTypes; }

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setObjectTypeColor(int objectIndex, const QColor &color);
    void removeObjectTypes(const QModelIndexList &indexes);

public slots:
    void appendNewObjectType();

private:
    ObjectTypes mObjectTypes;
};

} // namespace Internal
} // namespace Tiled

#endif // OBJECTTYPESMODEL_H
