/*
 * tilesetwangsetmodel.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <QAbstractItemModel>

namespace Tiled {

class Tileset;
class WangSet;

namespace Internal {

class TilesetDocument;

class TilesetWangSetModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UserRoles {
        WangSetRole = Qt::UserRole
    };

    TilesetWangSetModel(TilesetDocument *mapDocument,
                        QObject *parent = nullptr);
    ~TilesetWangSetModel();

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

    void insertWangSet(WangSet *wangSet);
    WangSet *takeWangSetAt(int index);
    void setWangSetName(int index, const QString &name);
    void setWangSetEdges(int index, int value);
    void setWangSetCorners(int index, int value);
    void setWangSetImage(int index, int tileId);

signals:
    void wangSetAboutToBeAdded(Tileset *tileset);
    void wangSetAdded(Tileset *tileset);
    void wangSetAboutToBeRemoved(WangSet *wangSet);
    void wangSetRemoved(WangSet *wangSet);

    /**
     * Emitted when either the name or the image of a terrain changed.
     */
    void wangSetChanged(Tileset *tileset, int index);

private:
    void emitWangSetChange(WangSet *wangSet);

    TilesetDocument *mTilesetDocument;
};

} // namespace Internal
} // namespace Tiled
