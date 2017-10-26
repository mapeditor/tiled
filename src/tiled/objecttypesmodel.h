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

#pragma once

#include "preferences.h"
#include "properties.h"

#include <QAbstractTableModel>

namespace Tiled {
namespace Internal {

class ObjectTypesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum { ColorRole = Qt::UserRole };

    ObjectTypesModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {}

    void setObjectTypes(const ObjectTypes &objectTypes);
    const ObjectTypes &objectTypes() const { return mObjectTypes; }

    ObjectType objectTypeAt(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setObjectTypeColor(int objectIndex, const QColor &color);
    void setObjectTypeProperties(int objectIndex, const Properties &properties);
    void removeObjectTypes(const QModelIndexList &indexes);

public slots:
    QModelIndex addNewObjectType();

private:
    ObjectTypes mObjectTypes;
};

} // namespace Internal
} // namespace Tiled
