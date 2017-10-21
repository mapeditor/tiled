/*
 * tilelayer.cpp
 * Copyright 2008-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jksb@member.fsf.org>
 *
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

#include "tilelayer.h"

#include "tile.h"
#include "hex.h"

#include <algorithm>

using namespace Tiled;

QRegion Chunk::region(std::function<bool (const Cell &)> condition) const
{
    QRegion region;

    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            if (condition(cellAt(x, y))) {
                const int rangeStart = x;
                for (++x; x <= CHUNK_SIZE; ++x) {
                    if (x == CHUNK_SIZE || !condition(cellAt(x, y))) {
                        const int rangeEnd = x;
                        region += QRect(rangeStart, y, rangeEnd - rangeStart, 1);
                        break;
                    }
                }
            }
        }
    }

    return region;
}

void Chunk::setCell(int x, int y, const Cell &cell)
{
    int index = x + y * CHUNK_SIZE;

    mGrid[index] = cell;
}

bool Chunk::isEmpty() const
{
    for (int y = 0; y < CHUNK_SIZE; ++y) {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            if (!cellAt(x, y).isEmpty())
                return false;
        }
    }

    return true;
}

bool Chunk::hasCell(std::function<bool (const Cell &)> condition) const
{
    for (const Cell &cell : mGrid)
        if (condition(cell))
            return true;

    return false;
}

void Chunk::removeReferencesToTileset(Tileset *tileset)
{
    for (int i = 0, i_end = mGrid.size(); i < i_end; ++i) {
        if (mGrid.at(i).tileset() == tileset)
            mGrid.replace(i, Cell());
    }
}

void Chunk::replaceReferencesToTileset(Tileset *oldTileset, Tileset *newTileset)
{
    for (Cell &cell : mGrid) {
        if (cell.tileset() == oldTileset)
            cell.setTile(newTileset, cell.tileId());
    }
}

TileLayer::TileLayer(const QString &name, int x, int y, int width, int height)
    : Layer(TileLayerType, name, x, y)
    , mWidth(width)
    , mHeight(height)
    , mUsedTilesetsDirty(false)
{
    Q_ASSERT(width >= 0);
    Q_ASSERT(height >= 0);
}

static QMargins maxMargins(const QMargins &a,
                           const QMargins &b)
{
    return QMargins(qMax(a.left(), b.left()),
                    qMax(a.top(), b.top()),
                    qMax(a.right(), b.right()),
                    qMax(a.bottom(), b.bottom()));
}

static QMargins computeDrawMargins(const QSet<SharedTileset> &tilesets)
{
    int maxTileSize = 0;
    QMargins offsetMargins;

    for (const SharedTileset &tileset : tilesets) {
        const QPoint offset = tileset->tileOffset();
        const QSize tileSize = tileset->tileSize();

        maxTileSize = std::max(maxTileSize, std::max(tileSize.width(),
                                                     tileSize.height()));

        offsetMargins = maxMargins(QMargins(-offset.x(),
                                            -offset.y(),
                                            offset.x(),
                                            offset.y()),
                                   offsetMargins);
    }

    return QMargins(offsetMargins.left(),
                    offsetMargins.top() + maxTileSize,
                    offsetMargins.right() + maxTileSize,
                    offsetMargins.bottom());
}

QMargins TileLayer::drawMargins() const
{
    return computeDrawMargins(usedTilesets());
}

QRegion TileLayer::region(std::function<bool (const Cell &)> condition) const
{
    QRegion region;

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        region += it.value().region(condition).translated(it.key().x() * CHUNK_SIZE + mX,
                                                          it.key().y() * CHUNK_SIZE + mY);
    }

    return region;
}

/**
 * Sets the cell at the given coordinates.
 */
void Tiled::TileLayer::setCell(int x, int y, const Cell &cell)
{
    if (!findChunk(x, y)) {
        if (cell == mEmptyCell) {
            return;
        } else {
            mBounds = mBounds.united(QRect(x - (x & CHUNK_MASK),
                                           y - (y & CHUNK_MASK),
                                           CHUNK_SIZE,
                                           CHUNK_SIZE));
        }
    }

    Chunk &_chunk = chunk(x, y);

    if (!mUsedTilesetsDirty) {
        Tileset *oldTileset = _chunk.cellAt(x & CHUNK_MASK, y & CHUNK_MASK).tileset();
        Tileset *newTileset = cell.tileset();
        if (oldTileset != newTileset) {
            if (oldTileset)
                mUsedTilesetsDirty = true;
            else if (newTileset)
                mUsedTilesets.insert(newTileset->sharedPointer());
        }
    }

    _chunk.setCell(x & CHUNK_MASK, y & CHUNK_MASK, cell);
}

TileLayer *TileLayer::copy(const QRegion &region) const
{
    const QRect areaBounds = region.boundingRect();

    TileLayer *copied = new TileLayer(QString(),
                                      0, 0,
                                      areaBounds.width(), areaBounds.height());

    for (const QRect &rect : region.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                copied->setCell(x - areaBounds.x(),
                                y - areaBounds.y(),
                                cellAt(x, y));

    return copied;
}

void TileLayer::merge(const QPoint &pos, const TileLayer *layer)
{
    // Determine the overlapping area
    QRect area = QRect(pos, QSize(layer->width(), layer->height()));
    area &= QRect(0, 0, width(), height());

    for (int y = area.top(); y <= area.bottom(); ++y) {
        for (int x = area.left(); x <= area.right(); ++x) {
            const Cell &cell = layer->cellAt(x - pos.x(),
                                             y - pos.y());
            if (!cell.isEmpty())
                setCell(x, y, cell);
        }
    }
}

void TileLayer::setCells(int x, int y, TileLayer *layer,
                         const QRegion &mask)
{
    QRegion area = QRect(x, y, layer->width(), layer->height());

    if (!mask.isEmpty())
        area &= mask;

    for (const QRect &rect : area.rects())
        for (int _x = rect.left(); _x <= rect.right(); ++_x)
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y)
                setCell(_x, _y, layer->cellAt(_x - x, _y - y));
}

/**
 * Sets the tiles in the given \a area to \a tile. Flipping flags are
 * preserved.
 */
void TileLayer::setTiles(const QRegion &area, Tile *tile)
{
    Q_ASSERT(area.subtracted(QRegion(0, 0, mWidth, mHeight)).isEmpty());

    for (const QRect &rect : area.rects()) {
        for (int x = rect.left(); x <= rect.right(); ++x) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                Cell cell = cellAt(x, y);
                cell.setTile(tile);
                setCell(x, y, cell);
            }
        }
    }
}

void TileLayer::erase(const QRegion &area)
{
    const Cell emptyCell;
    for (const QRect &rect : area.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                setCell(x, y, emptyCell);
}

void TileLayer::flip(FlipDirection direction)
{
    QScopedPointer<TileLayer> newLayer(new TileLayer(QString(), 0, 0, mWidth, mHeight));

    Q_ASSERT(direction == FlipHorizontally || direction == FlipVertically);

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                int _x = it.key().x() * CHUNK_SIZE + x;
                int _y = it.key().y() * CHUNK_SIZE + y;

                Cell dest(it.value().cellAt(x, y));

                if (dest.isEmpty())
                    continue;

                if (direction == FlipHorizontally) {
                    dest.setFlippedHorizontally(!dest.flippedHorizontally());
                    newLayer->setCell(mWidth - _x - 1, _y, dest);
                } else if (direction == FlipVertically) {
                    dest.setFlippedVertically(!dest.flippedVertically());
                    newLayer->setCell(_x, mHeight - _y - 1, dest);
                }
            }
        }
    }

    mChunks = newLayer->mChunks;
}

void TileLayer::flipHexagonal(FlipDirection direction)
{
    QScopedPointer<TileLayer> newLayer(new TileLayer(QString(), 0, 0, mWidth, mHeight));

    Q_ASSERT(direction == FlipHorizontally || direction == FlipVertically);

    // for more info see impl "void TileLayer::rotateHexagonal(RotateDirection direction)"
    static const char flipMaskH[16] = { 8, 6, 5, 4, 12, 2, 1, 0, 0, 14, 13, 12, 4, 10, 9, 8 }; // [0,15]<=>[8,7]; 2<=>5; 1<=>6; [12,3]<=>[4,11]; 14<=>9; 13<=>10;
    static const char flipMaskV[16] = { 4, 10, 9, 8, 0, 14, 13, 12, 12, 2, 1, 0, 8, 6, 5, 4 }; // [0,15]<=>[4,11]; 2<=>9; 1<=>10; [12,3]<=>[8,7]; 14<=>5; 13<=>6;

    const char (&flipMask)[16] = (direction == FlipHorizontally ? flipMaskH : flipMaskV);

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                int _x = it.key().x() * CHUNK_SIZE + x;
                int _y = it.key().y() * CHUNK_SIZE + y;

                Cell dest(it.value().cellAt(x, y));

                if (dest.isEmpty())
                    continue;

                unsigned char mask =
                        (static_cast<unsigned char>(dest.flippedHorizontally()) << 3) |
                        (static_cast<unsigned char>(dest.flippedVertically()) << 2) |
                        (static_cast<unsigned char>(dest.flippedAntiDiagonally()) << 1) |
                        (static_cast<unsigned char>(dest.rotatedHexagonal120()) << 0);

                mask = flipMask[mask];

                dest.setFlippedHorizontally((mask & 8) != 0);
                dest.setFlippedVertically((mask & 4) != 0);
                dest.setFlippedAntiDiagonally((mask & 2) != 0);
                dest.setRotatedHexagonal120((mask & 1) != 0);

                if (direction == FlipHorizontally)
                    newLayer->setCell(mWidth - _x - 1, _y, dest);
                else
                    newLayer->setCell(_x, mHeight - _y - 1, dest);
            }
        }
    }

    mChunks = newLayer->mChunks;
}

void TileLayer::rotate(RotateDirection direction)
{
    static const char rotateRightMask[8] = { 5, 4, 1, 0, 7, 6, 3, 2 };
    static const char rotateLeftMask[8]  = { 3, 2, 7, 6, 1, 0, 5, 4 };

    const char (&rotateMask)[8] =
            (direction == RotateRight) ? rotateRightMask : rotateLeftMask;

    int newWidth = mHeight;
    int newHeight = mWidth;
    QScopedPointer<TileLayer> newLayer(new TileLayer(QString(), 0, 0, newWidth, newHeight));

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                int _x = it.key().x() * CHUNK_SIZE + x;
                int _y = it.key().y() * CHUNK_SIZE + y;

                Cell dest(it.value().cellAt(x, y));

                if (dest.isEmpty())
                    continue;

                unsigned char mask =
                        (dest.flippedHorizontally() << 2) |
                        (dest.flippedVertically() << 1) |
                        (dest.flippedAntiDiagonally() << 0);

                mask = rotateMask[mask];

                dest.setFlippedHorizontally((mask & 4) != 0);
                dest.setFlippedVertically((mask & 2) != 0);
                dest.setFlippedAntiDiagonally((mask & 1) != 0);

                if (direction == RotateRight)
                    newLayer->setCell(mHeight - _y - 1, _x, dest);
                else
                    newLayer->setCell(_y, mWidth - _x - 1, dest);
            }
        }
    }

    mWidth = newWidth;
    mHeight = newHeight;
    mChunks = newLayer->mChunks;
}

void TileLayer::rotateHexagonal(RotateDirection direction, Map *map)
{
    Map::StaggerIndex staggerIndex = map->staggerIndex();
    Map::StaggerAxis staggerAxis = map->staggerAxis();

    Hex bottomRight(mWidth, mHeight, staggerIndex, staggerAxis);
    Hex topRight(mWidth, 0, staggerIndex, staggerAxis);
    Hex center(mWidth / 2, mHeight / 2, staggerIndex, staggerAxis);

    bottomRight -= center;
    topRight -= center;

    bottomRight.rotate(RotateRight);
    topRight.rotate(RotateRight);

    int newWidth = topRight.toStaggered(staggerIndex, staggerAxis).x() * 2 + 2;
    int newHeight = bottomRight.toStaggered(staggerIndex, staggerAxis).y() * 2 + 2;
    QScopedPointer<TileLayer> newLayer(new TileLayer(QString(), 0, 0, newWidth, newHeight));

    Hex newCenter(newWidth / 2, newHeight / 2, staggerIndex, staggerAxis);

    /* https://github.com/bjorn/tiled/pull/1447

  0 or 15     0: None or (Rotated60 | Rotated120 | FlippedVertically | FlippedHorizontally)
     2       60: Rotated60
     1      120: Rotated120
 12 or 3    180: (FlippedHorizontally | FlippedVertically) or (Rotated60 | Rotated120)
    14      240: Rotated60 | FlippedHorizontally | FlippedVertically
    13      300: Rotated120 | FlippedHorizontally | FlippedVertically

  8 or 7      0: FlippedHorizontally or (Rotated60 | Rotated120 | FlippedVertically)
    10       60: Rotated60 | FlippedHorizontally
     9      120: Rotated120 | FlippedHorizontally
  4 or 11   180: (FlippedVertically) or (Rotated60 | Rotated120 | FlippedHorizontally)
     6      240: Rotated60 | FlippedVertically
     5      300: Rotated120 | FlippedVertically

    */

    static const char rotateRightMask[16] = { 2, 12, 1, 14, 6, 8, 5, 10, 10,  4, 9, 0, 14,  0, 13,  2 }; // [0,15]->2->1->[12,3]->14->13; [8,7]->10->9->[4,11]->6->5;
    static const char rotateLeftMask[16]  = { 13, 2, 0,  1, 9, 6, 4,  5,  5, 10, 8, 9,  1, 14, 12, 13 }; // [0,15]->13->14->[12,3]->1->2; [8,7]->5->6->[4,11]->9->10;

    const char (&rotateMask)[16] =
            (direction == RotateRight) ? rotateRightMask : rotateLeftMask;

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        for (int y = 0; y < CHUNK_SIZE; ++y) {
            for (int x = 0; x < CHUNK_SIZE; ++x) {
                int _x = it.key().x() * CHUNK_SIZE + x;
                int _y = it.key().y() * CHUNK_SIZE + y;

                Cell dest(it.value().cellAt(x, y));

                if (dest.isEmpty())
                    continue;

                unsigned char mask =
                        (static_cast<unsigned char>(dest.flippedHorizontally()) << 3) |
                        (static_cast<unsigned char>(dest.flippedVertically()) << 2) |
                        (static_cast<unsigned char>(dest.flippedAntiDiagonally()) << 1) |
                        (static_cast<unsigned char>(dest.rotatedHexagonal120()) << 0);

                mask = rotateMask[mask];

                dest.setFlippedHorizontally((mask & 8) != 0);
                dest.setFlippedVertically((mask & 4) != 0);
                dest.setFlippedAntiDiagonally((mask & 2) != 0);
                dest.setRotatedHexagonal120((mask & 1) != 0);

                Hex rotatedHex(_x, _y, staggerIndex, staggerAxis);
                rotatedHex -= center;
                rotatedHex.rotate(direction);
                rotatedHex += newCenter;

                QPoint rotatedPoint = rotatedHex.toStaggered(staggerIndex, staggerAxis);

                newLayer->setCell(rotatedPoint.x(), rotatedPoint.y(), dest);
            }
        }
    }

    mWidth = newWidth;
    mHeight = newHeight;
    mChunks = newLayer->mChunks;

    QRect filledRect = region().boundingRect();

    if (staggerAxis == Map::StaggerY) {
        if (filledRect.y() & 1)
            map->invertStaggerIndex();
    } else {
        if (filledRect.x() & 1)
            map->invertStaggerIndex();
    }

    resize(filledRect.size(), -filledRect.topLeft());
}


QSet<SharedTileset> TileLayer::usedTilesets() const
{
    if (mUsedTilesetsDirty) {
        QSet<SharedTileset> tilesets;

        for (const Chunk &chunk : mChunks) {
            for (const Cell &cell : chunk)
                if (const Tile *tile = cell.tile())
                    tilesets.insert(tile->sharedTileset());

            mUsedTilesets.swap(tilesets);
            mUsedTilesetsDirty = false;
        }
    }

    return mUsedTilesets;
}

bool TileLayer::hasCell(std::function<bool (const Cell &)> condition) const
{
    for (const Chunk &chunk : mChunks) {
        if (chunk.hasCell(condition))
            return true;
    }

    return false;
}

bool TileLayer::referencesTileset(const Tileset *tileset) const
{
    return usedTilesets().contains(tileset->sharedPointer());
}

void TileLayer::removeReferencesToTileset(Tileset *tileset)
{
    for (Chunk &chunk : mChunks)
        chunk.removeReferencesToTileset(tileset);

    mUsedTilesets.remove(tileset->sharedPointer());
}

void TileLayer::replaceReferencesToTileset(Tileset *oldTileset,
                                           Tileset *newTileset)
{
    for (Chunk &chunk : mChunks)
        chunk.replaceReferencesToTileset(oldTileset, newTileset);

    if (mUsedTilesets.remove(oldTileset->sharedPointer()))
        mUsedTilesets.insert(newTileset->sharedPointer());
}

void TileLayer::resize(const QSize &size, const QPoint &offset)
{
    if (this->size() == size && offset.isNull())
        return;

    QScopedPointer<TileLayer> newLayer(new TileLayer(QString(), 0, 0, size.width(), size.height()));

    // Copy over the preserved part
    QRect area = mBounds.translated(offset).intersected(newLayer->rect());
    for (int y = area.top(); y <= area.bottom(); ++y)
        for (int x = area.left(); x <= area.right(); ++x)
            newLayer->setCell(x, y, cellAt(x - offset.x(), y - offset.y()));

    mChunks = newLayer->mChunks;
    mBounds = newLayer->mBounds;
    setSize(size);
}

void TileLayer::offsetTiles(const QPoint &offset,
                            const QRect &bounds,
                            bool wrapX, bool wrapY)
{
    QScopedPointer<TileLayer> newLayer(clone());

    for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            // Get position to pull tile value from
            int oldX = x - offset.x();
            int oldY = y - offset.y();

            // Wrap x value that will be pulled from
            if (wrapX && bounds.width() > 0) {
                while (oldX < bounds.left())
                    oldX += bounds.width();
                while (oldX > bounds.right())
                    oldX -= bounds.width();
            }

            // Wrap y value that will be pulled from
            if (wrapY && bounds.height() > 0) {
                while (oldY < bounds.top())
                    oldY += bounds.height();
                while (oldY > bounds.bottom())
                    oldY -= bounds.height();
            }

            // Set the new tile
            if (bounds.contains(oldX, oldY))
                newLayer->setCell(x, y, cellAt(oldX, oldY));
            else
                newLayer->setCell(x, y, Cell());
        }
    }

    mChunks = newLayer->mChunks;
    mBounds = newLayer->mBounds;
}

bool TileLayer::canMergeWith(Layer *other) const
{
    return other->isTileLayer();
}

Layer *TileLayer::mergedWith(Layer *other) const
{
    Q_ASSERT(canMergeWith(other));

    const TileLayer *o = static_cast<TileLayer*>(other);
    const QRect unitedRect = rect().united(o->rect());
    const QPoint offset = position() - unitedRect.topLeft();

    TileLayer *merged = clone();
    merged->resize(unitedRect.size(), offset);
    merged->merge(o->position() - unitedRect.topLeft(), o);
    return merged;
}

QRegion TileLayer::computeDiffRegion(const TileLayer *other) const
{
    QRegion ret;

    const int dx = other->x() - mX;
    const int dy = other->y() - mY;

    const QRect r = bounds().united(other->bounds()).translated(-position());

    for (int y = r.top(); y <= r.bottom(); ++y) {
        for (int x = r.left(); x <= r.right(); ++x) {
            if (cellAt(x, y) != other->cellAt(x - dx, y - dy)) {
                const int rangeStart = x;
                while (x <= r.right() &&
                       cellAt(x, y) != other->cellAt(x - dx, y - dy)) {
                    ++x;
                }
                const int rangeEnd = x;
                ret += QRect(rangeStart, y, rangeEnd - rangeStart, 1);
            }
        }
    }

    return ret;
}

bool TileLayer::isEmpty() const
{
    for (const Chunk &chunk : mChunks)
        if (!chunk.isEmpty())
            return false;

    return true;
}

static bool compareRectPos(const QRect &a, const QRect &b)
{
    if (a.y() != b.y())
        return a.y() < b.y();
    return a.x() < b.x();
}

/**
 * Returns a list of rectangles that cover all the used area of this layer.
 * The list is sorted by the top-left of each rectangle.
 *
 * This function is used to determine the chunks to write when saving a tile
 * layer.
 */
QVector<QRect> TileLayer::sortedChunksToWrite() const
{
    QVector<QRect> chunksToWrite;
    chunksToWrite.reserve(mChunks.size());

    QHashIterator<QPoint, Chunk> it(mChunks);
    while (it.hasNext()) {
        it.next();
        if (!it.value().isEmpty()) {
            const QPoint p = it.key();
            chunksToWrite.append(QRect(p.x() * CHUNK_SIZE,
                                       p.y() * CHUNK_SIZE,
                                       CHUNK_SIZE, CHUNK_SIZE));
        }
    }

    std::sort(chunksToWrite.begin(), chunksToWrite.end(), compareRectPos);

    return chunksToWrite;
}

/**
 * Returns a duplicate of this TileLayer.
 *
 * \sa Layer::clone()
 */
TileLayer *TileLayer::clone() const
{
    return initializeClone(new TileLayer(mName, mX, mY, mWidth, mHeight));
}

TileLayer *TileLayer::initializeClone(TileLayer *clone) const
{
    Layer::initializeClone(clone);
    clone->mChunks = mChunks;
    clone->mBounds = mBounds;
    clone->mUsedTilesets = mUsedTilesets;
    clone->mUsedTilesetsDirty = mUsedTilesetsDirty;
    return clone;
}
