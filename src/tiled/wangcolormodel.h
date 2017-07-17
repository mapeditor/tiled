/*
 * wangcolormodel.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include <QAbstractItemModel>

namespace Tiled {

class WangSet;

namespace Internal {

class WangColorModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        ColorRole = Qt::UserRole,
        //where Edge is 0 and Corner is 1
        EdgeOrCornerRole = Qt::UserRole + 1
    };

    WangColorModel(QObject *parent = nullptr);
    ~WangColorModel() {}

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    QModelIndex edgeIndex(int color) const;
    QModelIndex cornerIndex(int color) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setWangSet(WangSet *wangSet);

private:
    WangSet *mWangSet;
    QString *mEdgeText;
    QString *mCornerText;
};

} // namespace Internal
} // namespace Tiled
