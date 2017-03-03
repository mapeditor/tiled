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

#include "map.h"
#include "tile.h"

using namespace Tiled;

TileLayer::TileLayer(const QString &name, int x, int y, int width, int height)
    : Layer(TileLayerType, name, x, y)
    , mWidth(width)
    , mHeight(height)
    , mGrid(width * height)
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

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            if (condition(cellAt(x, y))) {
                const int rangeStart = x;
                for (++x; x <= mWidth; ++x) {
                    if (x == mWidth || !condition(cellAt(x, y))) {
                        const int rangeEnd = x;
                        region += QRect(rangeStart + mX, y + mY,
                                        rangeEnd - rangeStart, 1);
                        break;
                    }
                }
            }
        }
    }

    return region;
}

/**
 * Sets the cell at the given coordinates.
 */
void Tiled::TileLayer::setCell(int x, int y, const Cell &cell)
{
    Q_ASSERT(contains(x, y));

    Cell &existingCell = mGrid[x + y * mWidth];

    if (!mUsedTilesetsDirty) {
        Tileset *oldTileset = existingCell.isEmpty() ? nullptr : existingCell.tileset();
        Tileset *newTileset = cell.tileset();
        if (oldTileset != newTileset) {
            if (oldTileset)
                mUsedTilesetsDirty = true;
            else if (newTileset)
                mUsedTilesets.insert(newTileset->sharedPointer());
        }
    }

    existingCell = cell;
}

TileLayer *TileLayer::copy(const QRegion &region) const
{
    const QRegion area = region.intersected(QRect(0, 0, width(), height()));
    const QRect bounds = region.boundingRect();
    const QRect areaBounds = area.boundingRect();
    const int offsetX = qMax(0, areaBounds.x() - bounds.x());
    const int offsetY = qMax(0, areaBounds.y() - bounds.y());

    TileLayer *copied = new TileLayer(QString(),
                                      0, 0,
                                      bounds.width(), bounds.height());

    for (const QRect &rect : area.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                copied->setCell(x - areaBounds.x() + offsetX,
                                y - areaBounds.y() + offsetY,
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
    // Determine the overlapping area
    QRegion area = QRect(x, y, layer->width(), layer->height());
    area &= QRect(0, 0, width(), height());

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
    QVector<Cell> newGrid(mWidth * mHeight);

    Q_ASSERT(direction == FlipHorizontally || direction == FlipVertically);

    const Map* const globalMap = map();
    const Map::Orientation currentOrientation = (globalMap ? globalMap->orientation() : Map::Unknown);

     if ( currentOrientation != Map::Hexagonal ) {

         for (int y = 0; y < mHeight; ++y) {
             for (int x = 0; x < mWidth; ++x) {
                 Cell &dest = newGrid[x + y * mWidth];
                 if (direction == FlipHorizontally) {
                     const Cell &source = cellAt(mWidth - x - 1, y);
                     dest = source;
                     dest.setFlippedHorizontally(!source.flippedHorizontally());
                 } else if (direction == FlipVertically) {
                     const Cell &source = cellAt(x, mHeight - y - 1);
                     dest = source;
                     dest.setFlippedVertically(!source.flippedVertically());
                 }
             }
         }

     } else {

         static const char flipMaskH[16] = { 4, 10, 9, 0, 0, 14, 15, 0, 12, 2, 1, 0, 8, 0, 5, 6 }; // 0<=>4; 10<=>1; 5<=>14; 12<=>8; 6<=>15; 9<=>2; skip: 3 7 11 13
         static const char flipMaskV[16] = { 8, 6, 5, 0, 12, 2, 1, 0, 0, 14, 15, 0, 4, 0, 9, 10 }; // 0<=>8; 10<=>15; 5<=>2; 12<=>4; 6<=>1; 9<=>14; skip: 3 7 11 13

         const char (&flipMask)[16] = (direction == FlipHorizontally ? flipMaskH : flipMaskV);

         for (int y = 0; y < mHeight; ++y) {
             for (int x = 0; x < mWidth; ++x) {
                 const Cell &source = (direction == FlipHorizontally ? cellAt(mWidth - x - 1, y) : cellAt(x, mHeight - y - 1));
                 Cell &dest = newGrid[x + y * mWidth];
                 dest = source;

                 unsigned char mask =
                         (static_cast<unsigned char>(dest.flippedHorizontally()) << 3) |
                         (static_cast<unsigned char>(dest.flippedVertically()) << 2) |
                         (static_cast<unsigned char>(dest.flippedAntiDiagonally()) << 1) |
                         (static_cast<unsigned char>(dest.flippedLeftHexAntiDiagonally()) << 0);

                 mask = flipMask[mask];

                 dest.setFlippedHorizontally((mask & 8) != 0);
                 dest.setFlippedVertically((mask & 4) != 0);
                 dest.setFlippedAntiDiagonally((mask & 2) != 0);
                 dest.setFlippedLeftHexAntiDiagonally((mask & 1) != 0);
             }
         }

     }

    mGrid = newGrid;
}

void TileLayer::rotate(RotateDirection direction)
{
    const Map* const globalMap = map();
    const Map::Orientation currentOrientation = (globalMap ? globalMap->orientation() : Map::Unknown);

    int newWidth = mHeight;
    int newHeight = mWidth;
    QVector<Cell> newGrid(newWidth * newHeight);

    if ( currentOrientation != Map::Hexagonal ) {

        static const char rotateRightMask[8] = { 5, 4, 1, 0, 7, 6, 3, 2 };
        static const char rotateLeftMask[8]  = { 3, 2, 7, 6, 1, 0, 5, 4 };

        const char (&rotateMask)[8] =
                (direction == RotateRight) ? rotateRightMask : rotateLeftMask;

        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                const Cell &source = cellAt(x, y);
                Cell dest = source;

                unsigned char mask =
                        (static_cast<unsigned char>(dest.flippedHorizontally()) << 2) |
                        (static_cast<unsigned char>(dest.flippedVertically()) << 1) |
                        (static_cast<unsigned char>(dest.flippedAntiDiagonally()) << 0);

                mask = rotateMask[mask];

                dest.setFlippedHorizontally((mask & 4) != 0);
                dest.setFlippedVertically((mask & 2) != 0);
                dest.setFlippedAntiDiagonally((mask & 1) != 0);

                if (direction == RotateRight)
                    newGrid[x * newWidth + (mHeight - y - 1)] = dest;
                else
                    newGrid[(mWidth - x - 1) * newWidth + y] = dest;
            }
        }

    } else {

        static const char rotateRightMask[16] = { 10, 4, 15, 0, 2, 12, 9, 0, 14, 0, 5, 0, 6, 0, 1, 8 }; // 0->10->5->12->6->9; 8->14->1->4->2->15; skip: 3 7 11 13
        static const char rotateLeftMask[16]  = { 9, 14, 4, 0, 1, 10, 12, 0, 15, 6, 0, 0, 5, 0, 8, 2 }; // 0->9->6->12->5->10; 8->15->2->4->1->14; skip: 3 7 11 13

        const char (&rotateMask)[16] =
                (direction == RotateRight) ? rotateRightMask : rotateLeftMask;

        for (int y = 0; y < mHeight; ++y) {
            for (int x = 0; x < mWidth; ++x) {
                const Cell &source = cellAt(x, y);
                Cell dest = source;

                unsigned char mask =
                        (static_cast<unsigned char>(dest.flippedHorizontally()) << 3) |
                        (static_cast<unsigned char>(dest.flippedVertically()) << 2) |
                        (static_cast<unsigned char>(dest.flippedAntiDiagonally()) << 1) |
                        (static_cast<unsigned char>(dest.flippedLeftHexAntiDiagonally()) << 0);

                mask = rotateMask[mask];

                dest.setFlippedHorizontally((mask & 8) != 0);
                dest.setFlippedVertically((mask & 4) != 0);
                dest.setFlippedAntiDiagonally((mask & 2) != 0);
                dest.setFlippedLeftHexAntiDiagonally((mask & 1) != 0);

                if (direction == RotateRight)
                    newGrid[x * newWidth + (mHeight - y - 1)] = dest;
                else
                    newGrid[(mWidth - x - 1) * newWidth + y] = dest;
            }
        }

    }

    mWidth = newWidth;
    mHeight = newHeight;
    mGrid = newGrid;
}


QSet<SharedTileset> TileLayer::usedTilesets() const
{
    if (mUsedTilesetsDirty) {
        QSet<SharedTileset> tilesets;

        for (const Cell &cell : mGrid)
            if (const Tile *tile = cell.tile())
                tilesets.insert(tile->sharedTileset());

        mUsedTilesets.swap(tilesets);
        mUsedTilesetsDirty = false;
    }

    return mUsedTilesets;
}

bool TileLayer::hasCell(std::function<bool (const Cell &)> condition) const
{
    for (const Cell &cell : mGrid)
        if (condition(cell))
            return true;

    return false;
}

bool TileLayer::referencesTileset(const Tileset *tileset) const
{
    return usedTilesets().contains(tileset->sharedPointer());
}

void TileLayer::removeReferencesToTileset(Tileset *tileset)
{
    for (int i = 0, i_end = mGrid.size(); i < i_end; ++i) {
        if (mGrid.at(i).tileset() == tileset)
            mGrid.replace(i, Cell());
    }

    mUsedTilesets.remove(tileset->sharedPointer());
}

void TileLayer::replaceReferencesToTileset(Tileset *oldTileset,
                                           Tileset *newTileset)
{
    for (Cell &cell : mGrid) {
        if (cell.tileset() == oldTileset)
            cell.setTile(newTileset, cell.tileId());
    }

    if (mUsedTilesets.remove(oldTileset->sharedPointer()))
        mUsedTilesets.insert(newTileset->sharedPointer());
}

void TileLayer::resize(const QSize &size, const QPoint &offset)
{
    if (this->size() == size && offset.isNull())
        return;

    QVector<Cell> newGrid(size.width() * size.height());

    // Copy over the preserved part
    const int startX = qMax(0, -offset.x());
    const int startY = qMax(0, -offset.y());
    const int endX = qMin(mWidth, size.width() - offset.x());
    const int endY = qMin(mHeight, size.height() - offset.y());

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const int index = x + offset.x() + (y + offset.y()) * size.width();
            newGrid[index] = cellAt(x, y);
        }
    }

    mGrid = newGrid;
    setSize(size);
}

void TileLayer::offsetTiles(const QPoint &offset,
                            const QRect &bounds,
                            bool wrapX, bool wrapY)
{
    QVector<Cell> newGrid(mWidth * mHeight);

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            // Skip out of bounds tiles
            if (!bounds.contains(x, y)) {
                newGrid[x + y * mWidth] = cellAt(x, y);
                continue;
            }

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
            if (contains(oldX, oldY) && bounds.contains(oldX, oldY))
                newGrid[x + y * mWidth] = cellAt(oldX, oldY);
            else
                newGrid[x + y * mWidth] = Cell();
        }
    }

    mGrid = newGrid;
}

bool TileLayer::canMergeWith(Layer *other) const
{
    return other->isTileLayer();
}

Layer *TileLayer::mergedWith(Layer *other) const
{
    Q_ASSERT(canMergeWith(other));

    const TileLayer *o = static_cast<TileLayer*>(other);
    const QRect unitedBounds = bounds().united(o->bounds());
    const QPoint offset = position() - unitedBounds.topLeft();

    TileLayer *merged = static_cast<TileLayer*>(clone());
    merged->resize(unitedBounds.size(), offset);
    merged->merge(o->position() - unitedBounds.topLeft(), o);
    return merged;
}

QRegion TileLayer::computeDiffRegion(const TileLayer *other) const
{
    QRegion ret;

    const int dx = other->x() - mX;
    const int dy = other->y() - mY;
    QRect r = QRect(0, 0, width(), height());
    r &= QRect(dx, dy, other->width(), other->height());

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
    for (const Cell &cell : mGrid)
        if (!cell.isEmpty())
            return false;

    return true;
}

/**
 * Returns a duplicate of this TileLayer.
 *
 * \sa Layer::clone()
 */
Layer *TileLayer::clone() const
{
    return initializeClone(new TileLayer(mName, mX, mY, mWidth, mHeight));
}

TileLayer *TileLayer::initializeClone(TileLayer *clone) const
{
    Layer::initializeClone(clone);
    clone->mGrid = mGrid;
    clone->mUsedTilesets = mUsedTilesets;
    clone->mUsedTilesetsDirty = mUsedTilesetsDirty;
    return clone;
}
