/*
 * wangsetmodel.h
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
#include <tileset.h>

namespace Tiled {

class Tileset;
class WangSet;

class TilesetDocument;

class WangSetModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        WangSetRole = Qt::UserRole
    };

    WangSetModel(QAbstractItemModel *tilesetDocumentModel,
                 QObject *parent = nullptr);
    ~WangSetModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(Tileset *tileset) const;
    QModelIndex index(WangSet *wangSet) const;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Tileset *tilesetAt(const QModelIndex &index) const;
    WangSet *wangSetAt(const QModelIndex &index) const;

private:
    void onTilesetRowsInserted(const QModelIndex &parent, int first, int last);
    void onTilesetRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void onWangSetAboutToBeAdded(Tileset *tileset);
    void onWangSetAdded(Tileset *tileset);
    void onWangSetAboutToBeRemoved(WangSet *wangSet);
    void onWangSetRemoved(WangSet *wangSet);

    QAbstractItemModel *mTilesetDocumentsModel;
    QList<TilesetDocument*> mTilesetDocuments;
};

} // namespace Tiled
