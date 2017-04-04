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

    mGrid = newGrid;
}

void TileLayer::flipHexagonal(FlipDirection direction)
{
    QVector<Cell> newGrid(mWidth * mHeight);

    Q_ASSERT(direction == FlipHorizontally || direction == FlipVertically);

    // for more info see impl "void TileLayer::rotateHexagonal(RotateDirection direction)"
    static const char flipMaskH[16] = { 8, 6, 5, 4, 12, 2, 1, 0, 0, 14, 13, 12, 4, 10, 9, 8 }; // [0,15]<=>[8,7]; 2<=>5; 1<=>6; [12,3]<=>[4,11]; 14<=>9; 13<=>10;
    static const char flipMaskV[16] = { 4, 10, 9, 8, 0, 14, 13, 12, 12, 2, 1, 0, 8, 6, 5, 4 }; // [0,15]<=>[4,11]; 2<=>9; 1<=>10; [12,3]<=>[8,7]; 14<=>5; 13<=>6;

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
                    (static_cast<unsigned char>(dest.rotatedHexagonal120()) << 0);

            mask = flipMask[mask];

            dest.setFlippedHorizontally((mask & 8) != 0);
            dest.setFlippedVertically((mask & 4) != 0);
            dest.setFlippedAntiDiagonally((mask & 2) != 0);
            dest.setRotatedHexagonal120((mask & 1) != 0);
        }
    }

    mGrid = newGrid;
}

void TileLayer::rotate(RotateDirection direction)
{
    static const char rotateRightMask[8] = { 5, 4, 1, 0, 7, 6, 3, 2 };
    static const char rotateLeftMask[8]  = { 3, 2, 7, 6, 1, 0, 5, 4 };

    const char (&rotateMask)[8] =
            (direction == RotateRight) ? rotateRightMask : rotateLeftMask;

    int newWidth = mHeight;
    int newHeight = mWidth;
    QVector<Cell> newGrid(newWidth * newHeight);

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            const Cell &source = cellAt(x, y);
            Cell dest = source;

            unsigned char mask =
                    (dest.flippedHorizontally() << 2) |
                    (dest.flippedVertically() << 1) |
                    (dest.flippedAntiDiagonally() << 0);

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

    mWidth = newWidth;
    mHeight = newHeight;
    mGrid = newGrid;
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
    QVector<Cell> newGrid(newWidth * newHeight);

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

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            const Cell &source = cellAt(x, y);
            Cell dest = source;

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

            Hex rotatedHex(x, y, staggerIndex, staggerAxis);
            rotatedHex -= center;
            rotatedHex.rotate(direction);
            rotatedHex += newCenter;

            QPoint rotatedPoint = rotatedHex.toStaggered(staggerIndex, staggerAxis);

            int index = rotatedPoint.y() * newWidth + rotatedPoint.x();

            newGrid[index] = dest;
        }
    }

    mWidth = newWidth;
    mHeight = newHeight;
    mGrid = newGrid;

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
