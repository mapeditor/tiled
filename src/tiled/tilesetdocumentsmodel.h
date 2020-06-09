/*
 * tilesetdocumentsmodel.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetdocument.h"

#include <QAbstractListModel>
#include <QList>
#include <QSortFilterProxyModel>

namespace Tiled {

class Tileset;

class MapDocument;

/**
 * This model exposes the list of tileset documents. This includes opened
 * tileset files, as well as both internal and external tilesets loaded as part
 * of a map.
 *
 * It also owns the TilesetDocument instances.
 */
class TilesetDocumentsModel : public QAbstractListModel
{
public:
    enum {
        TilesetDocumentRole = Qt::UserRole,
        TilesetRole,
    };

    TilesetDocumentsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    const QVector<TilesetDocumentPtr> &tilesetDocuments() const;

    bool contains(TilesetDocument *tilesetDocument) const;
    void append(TilesetDocument *tilesetDocument);
    void insert(int index, TilesetDocument *tilesetDocument);
    void remove(TilesetDocument *tilesetDocument);
    void remove(int index);

private:
    void tilesetNameChanged(Tileset *tileset);
    void tilesetFileNameChanged();

    QVector<TilesetDocumentPtr> mTilesetDocuments;
};


inline const QVector<TilesetDocumentPtr> &TilesetDocumentsModel::tilesetDocuments() const
{
    return mTilesetDocuments;
}

inline bool TilesetDocumentsModel::contains(TilesetDocument *tilesetDocument) const
{
    return std::find(mTilesetDocuments.begin(),
                     mTilesetDocuments.end(),
                     tilesetDocument) != mTilesetDocuments.end();
}

inline void TilesetDocumentsModel::append(TilesetDocument *tilesetDocument)
{
    insert(mTilesetDocuments.size(), tilesetDocument);
}

inline void TilesetDocumentsModel::remove(TilesetDocument *tilesetDocument)
{
    remove(mTilesetDocuments.indexOf(tilesetDocument->sharedFromThis()));
}


/**
 * Sorts the tilesets in alphabetical order and filters out embedded tilesets
 * that are not part of the current map document.
 */
class TilesetDocumentsFilterModel : public QSortFilterProxyModel
{
public:
    TilesetDocumentsFilterModel(QObject *parent = nullptr);

    void setMapDocument(MapDocument *mapDocument);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    MapDocument *mMapDocument;
};

} // namespace Tiled
