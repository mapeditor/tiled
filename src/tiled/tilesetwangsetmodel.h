/*
 * tilesetwangsetmodel.h
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

class Tileset;
class WangSet;
class WangColor;

class TilesetDocument;

class TilesetWangSetModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UserRoles {
        WangSetRole = Qt::UserRole
    };

    explicit TilesetWangSetModel(TilesetDocument *mapDocument,
                                 QObject *parent = nullptr);
    ~TilesetWangSetModel() override;

    using QAbstractListModel::index;
    QModelIndex index(WangSet *wangSet);

    /**
     * Returns the number of rows. For the root, this is the number of wangSets
     * in the tileset.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    WangSet *wangSetAt(const QModelIndex &index) const;

    void insertWangSet(int index, WangSet *wangSet);
    WangSet *takeWangSetAt(int index);
    void setWangSetName(WangSet *wangSet, const QString &name);
    void setWangSetEdges(WangSet *wangSet, int value);
    void setWangSetCorners(WangSet *wangSet, int value);
    void setWangSetImage(WangSet *wangSet, int tileId);
    void insertWangColor(WangSet *wangSet, const QSharedPointer<WangColor> &wangColor);
    void removeWangColorAt(WangSet *wangSet, int color, bool isEdge);

signals:
    void wangSetAboutToBeAdded(Tileset *tileset);
    void wangSetAdded(Tileset *tileset);
    void wangSetAboutToBeRemoved(WangSet *wangSet);
    void wangSetRemoved(WangSet *wangSet);

    /**
     * Emitted when either the name, image, edgeCount, or corner count
     * of a wangSet changed.
     */
    void wangSetChanged(Tileset *tileset, int index);

private:
    void emitWangSetChange(WangSet *wangSet);

    TilesetDocument *mTilesetDocument;
};

} // namespace Tiled
