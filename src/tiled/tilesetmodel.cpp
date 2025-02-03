/*
 * tilesetmodel.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#include "tilesetmodel.h"

#include "map.h"
#include "relocatetiles.h"
#include "tile.h"
#include "tiled.h"
#include "tileset.h"
#include "tilesetdocument.h"

#include <QMimeData>

using namespace Tiled;

inline uint qHash(const QPoint &p, uint seed = 0)
{
    return qHash(p.x(), seed) ^ qHash(p.y(), seed);
}

TilesetModel::TilesetModel(TilesetDocument *tilesetDocument, QObject *parent)
    : QAbstractListModel(parent)
    , mTilesetDocument(tilesetDocument)
{
    refreshTileIds();

    connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
            this, &TilesetModel::tileChanged);
    connect(tilesetDocument, &TilesetDocument::tileAnimationChanged,
            this, &TilesetModel::tileChanged);
}

QPoint TilesetModel::viewToAtlasCoords(int viewCol, int viewRow) const
{
    const int currentCols = columnCount();
    const int atlasCols = tileset()->columnCount();
    const int linearIndex = viewCol + viewRow * currentCols;
    return QPoint(linearIndex % atlasCols,
                 linearIndex / atlasCols);
}

QPoint TilesetModel::atlasToViewCoords(int atlasCol, int atlasRow) const
{
    const int currentCols = columnCount();
    const int atlasCols = tileset()->columnCount();
    const int linearIndex = atlasRow * atlasCols + atlasCol;
    return QPoint(linearIndex % currentCols,
                 linearIndex / currentCols);
}

int TilesetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (tileset()->isAtlas()) {
        const int currentCols = columnCount();
        if (currentCols <= 0)
            return 1;

        const int totalTiles = tileset()->rowCount() * tileset()->columnCount();
        return (totalTiles + currentCols - 1) / currentCols;
    }

    const int tileCount = mTileIds.size();
    const int columns = columnCount();

    int rows = 1;
    if (columns > 0) {
        rows = tileCount / columns;
        if (tileCount % columns > 0)
            ++rows;
    }

    return rows;
}

int TilesetModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    if (mColumnCountOverride > 0)
        return mColumnCountOverride;
    if (tileset()->columnCount())
        return tileset()->columnCount();
    // TODO: Non-table tilesets should use a different model.
    // For now use an arbitrary number of columns.
    return 5;
}

QVariant TilesetModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole) {
        if (Tile *tile = tileAt(index))
            return tile->image().copy(tile->imageRect());
    }

    return QVariant();
}

QVariant TilesetModel::headerData(int /* section */,
                                  Qt::Orientation /* orientation */,
                                  int role) const
{
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

QSize TilesetModel::tileSpanSize(const QModelIndex &index) const
{
    if (!tileset()->isAtlas())
        return QSize(1, 1);

    if (Tile *tile = tileAt(index)) {
        const int tileWidth = tileset()->tileWidth();
        const int tileHeight = tileset()->tileHeight();
        const QRect rect = tile->imageRect();
        return QSize(
            rect.width() / tileWidth,
            rect.height() / tileHeight
        );
    }

    return QSize(1, 1);
}

Tile *TilesetModel::findSpanningTile(const QModelIndex &index) const
{
    if (!tileset()->isAtlas())
        return nullptr;

    for (Tile *tile : tileset()->tiles()) {
        // Use tileIndex to get normalized grid position
        QModelIndex tilePos = tileIndex(tile);
        QSize span = tileSpanSize(tilePos);

        if (span.width() <= 1 && span.height() <= 1)
            continue;

        if (index.row() >= tilePos.row() && index.row() < tilePos.row() + span.height() &&
            index.column() >= tilePos.column() && index.column() < tilePos.column() + span.width()) {
            return tile;
        }
    }

    return nullptr;
}

Qt::ItemFlags TilesetModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (tileset()->isAtlas()) {
        if (!index.isValid())
            return defaultFlags;

        // If this cell contains a tile's origin or we are relocating, it's selectable
        if (mRelocating || tileAt(index))
            return defaultFlags | Qt::ItemIsSelectable;

        return defaultFlags & ~Qt::ItemIsSelectable;
    }

    defaultFlags |= Qt::ItemIsDropEnabled;
    if (index.isValid())
        defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

Qt::DropActions TilesetModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList TilesetModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String(TILES_MIMETYPE);
    return types;
}

QMimeData *TilesetModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray encodedData;

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
#else
    QDataStream stream(&encodedData, QDataStream::WriteOnly);
#endif

    for (const QModelIndex &index : indexes) {
        if (auto tile = tileAt(index))
            stream << tile->id();
    }

    if (encodedData.isEmpty())
        return nullptr;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TILES_MIMETYPE), encodedData);
    return mimeData;
}

bool TilesetModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                int row, int column,
                                const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (!data || action != Qt::MoveAction)
        return false;
    if (!data->hasFormat(QLatin1String(TILES_MIMETYPE)))
        return false;

    QByteArray encodedData = data->data(QLatin1String(TILES_MIMETYPE));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
#else
    QDataStream stream(&encodedData, QDataStream::ReadOnly);
#endif

    QList<Tile*> sourceTiles;

    while (!stream.atEnd()) {
        int sourceId;
        stream >> sourceId;

        if (stream.status() != QDataStream::Ok)
            break;

        if (Tile *sourceTile = tileset()->findTile(sourceId))
            sourceTiles.append(sourceTile);
    }

    if (!sourceTiles.isEmpty()) {
        Tile *destinationTile = tileAt(parent);
        int destinationIndex = destinationTile ? mTileIds.indexOf(destinationTile->id())
                                               : mTileIds.size() - 1;

        mTilesetDocument->undoStack()->push(new RelocateTiles(mTilesetDocument,
                                                              sourceTiles,
                                                              destinationIndex));
    }

    return true;
}

Tile *TilesetModel::tileAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (tileset()->isAtlas()) {
        const QPoint atlasPos = viewToAtlasCoords(index.column(), index.row());
        return mTileGrid.value(atlasPos);
    }

    const int tileIndex = index.column() + index.row() * columnCount();
    if (tileIndex < mTileIds.size()) {
        const int tileId = mTileIds.at(tileIndex);
        return tileset()->findTile(tileId);
    }
    return nullptr;
}

QModelIndex TilesetModel::tileIndex(const Tile *tile) const
{
    Q_ASSERT(tile->tileset() == tileset());
    if (tileset()->isAtlas()) {
        const QPoint tilePos = tileset()->pixelToGrid(tile->imageRect().topLeft());
        const QPoint viewPos = atlasToViewCoords(tilePos.x(), tilePos.y());
        return index(viewPos.y(), viewPos.x());
    }

    const int columnCount = TilesetModel::columnCount();

    // Can't yield a valid index with column count <= 0
    if (columnCount <= 0)
        return QModelIndex();

    const int tileIndex = mTileIds.indexOf(tile->id());
    // todo: this assertion was hit when testing tileset image size changes
    Q_ASSERT(tileIndex != -1);

    const int row = tileIndex / columnCount;
    const int column = tileIndex % columnCount;

    return index(row, column);
}

Tileset *TilesetModel::tileset() const
{
    return mTilesetDocument->tileset().data();
}

void TilesetModel::tilesetChanged()
{
    beginResetModel();
    refreshTileIds();
    endResetModel();
}

void TilesetModel::setColumnCountOverride(int columnCount)
{
    if (mColumnCountOverride == columnCount)
        return;

    beginResetModel();
    mColumnCountOverride = columnCount;
    endResetModel();
}

void TilesetModel::tilesChanged(const QList<Tile *> &tiles)
{
    if (tiles.first()->tileset() != tileset())
        return;

    QModelIndex topLeft;
    QModelIndex bottomRight;

    for (const Tile *tile : tiles) {
        const QModelIndex i = tileIndex(tile);

        if (!topLeft.isValid()) {
            topLeft = i;
            bottomRight = i;
            continue;
        }

        if (i.row() < topLeft.row() || i.column() < topLeft.column())
            topLeft = index(qMin(topLeft.row(), i.row()),
                            qMin(topLeft.column(), i.column()));

        if (i.row() > bottomRight.row() || i.column() > bottomRight.column())
            bottomRight = index(qMax(bottomRight.row(), i.row()),
                                qMax(bottomRight.column(), i.column()));
    }

    if (topLeft.isValid())
        emit dataChanged(topLeft, bottomRight);
}

void TilesetModel::tileChanged(Tile *tile)
{
    if (tileset()->isAtlas()) {
        const QPoint oldPos = mTileGrid.key(tile, QPoint(-1, -1));
        if (oldPos != QPoint(-1, -1))
            mTileGrid.remove(oldPos);
        const QPoint newPos = tileset()->pixelToGrid(tile->imageRect().topLeft());
        mTileGrid.insert(newPos, tile);
    }

    const QModelIndex i = tileIndex(tile);
    emit dataChanged(i, i);
}

void TilesetModel::refreshTileIds()
{
    mTileIds.clear();
    for (Tile *tile : tileset()->tiles()) {
        mTileIds.append(tile->id());
        const QPoint pos = tileset()->pixelToGrid(tile->imageRect().topLeft());
        mTileGrid.insert(pos, tile);
    }
}

#include "moc_tilesetmodel.cpp"
