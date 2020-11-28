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

#include <QStringList>

#include "customproperties.h"
#include "properties.h"
#include "object.h"
#include <QAbstractTableModel>

namespace Tiled {

class CustomPropsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum { ColorRole = Qt::UserRole };

    CustomPropsModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {
    }

    void setCustomProps(const CustomProps &customProps);
    const CustomProps &customProps() const { return mCustomProps; }

    CustomProp customPropAt(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setCustomPropColor(int objectIndex, const QColor &color);
    void setCustomPropValues(int objectIndex, const QStringList &values);
    void removeCustomProps(const QModelIndexList &indexes);

public slots:
    QModelIndex addNewCustomProp();

private:
    CustomProps mCustomProps;
};

} // namespace Tiled
