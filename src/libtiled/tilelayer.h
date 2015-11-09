/*
 * tilelayer.h
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

#ifndef TILELAYER_H
#define TILELAYER_H

#include "tiled_global.h"

#include "layer.h"
#include "tiled.h"

#include <QMargins>
#include <QString>
#include <QVector>
#include <QSharedPointer>

#include <functional>

namespace Tiled {

class Tile;

/**
 * A cell on a tile layer grid.
 */
class Cell
{
public:
    Cell() :
        tile(nullptr),
        flippedHorizontally(false),
        flippedVertically(false),
        flippedAntiDiagonally(false)
    {}

    explicit Cell(Tile *tile) :
        tile(tile),
        flippedHorizontally(false),
        flippedVertically(false),
        flippedAntiDiagonally(false)
    {}

    bool isEmpty() const { return tile == nullptr; }

    bool operator == (const Cell &other) const
    {
        return tile == other.tile
                && flippedHorizontally == other.flippedHorizontally
                && flippedVertically == other.flippedVertically
                && flippedAntiDiagonally == other.flippedAntiDiagonally;
    }

    bool operator != (const Cell &other) const
    {
        return tile != other.tile
                || flippedHorizontally != other.flippedHorizontally
                || flippedVertically != other.flippedVertically
                || flippedAntiDiagonally != other.flippedAntiDiagonally;
    }

    Tile *tile;
    bool flippedHorizontally;
    bool flippedVertically;
    bool flippedAntiDiagonally;
};

/**
 * A tile layer is a grid of cells. Each cell refers to a specific tile, and
 * stores how the tile is flipped.
 *
 * Coordinates and regions passed to function parameters are in local
 * coordinates and do not take into account the position of the layer.
 */
class TILEDSHARED_EXPORT TileLayer : public Layer
{
public:
    /**
     * Constructor.
     */
    TileLayer(const QString &name, int x, int y, int width, int height);

    QMargins drawMargins() const;

    bool contains(int x, int y) const;
    bool contains(const QPoint &point) const;

    /**
     * Calculates the region of cells in this tile layer for which the given
     * \a condition returns true.
     */
    QRegion region(std::function<bool (const Cell &)> condition) const;

    /**
     * Calculates the region occupied by the tiles of this layer. Similar to
     * Layer::bounds(), but leaves out the regions without tiles.
     */
    QRegion region() const;

    const Cell &cellAt(int x, int y) const;
    const Cell &cellAt(const QPoint &point) const;

    void setCell(int x, int y, const Cell &cell);

    /**
     * Returns a copy of the area specified by the given \a region. The
     * caller is responsible for the returned tile layer.
     */
    TileLayer *copy(const QRegion &region) const;

    TileLayer *copy(int x, int y, int width, int height) const
    { return copy(QRegion(x, y, width, height)); }

    /**
     * Merges the given \a layer onto this layer at position \a pos. Parts that
     * fall outside of this layer will be lost and empty tiles in the given
     * layer will have no effect.
     */
    void merge(const QPoint &pos, const TileLayer *layer);

    /**
     * Removes all cells in the specified region.
     */
    void erase(const QRegion &region);

    /**
     * Sets the cells starting at the given position to the cells in the given
     * \a tileLayer. Parts that fall outside of this layer will be ignored.
     *
     * When a \a mask is given, only cells that fall within this mask are set.
     * The mask is applied in local coordinates.
     */
    void setCells(int x, int y, TileLayer *tileLayer,
                  const QRegion &mask = QRegion());

    /**
     * Flip this tile layer in the given \a direction. Direction must be
     * horizontal or vertical. This doesn't change the dimensions of the
     * tile layer.
     */
    void flip(FlipDirection direction);

    /**
     * Rotate this tile layer by 90 degrees left or right. The tile positions
     * are rotated within the layer, and the tiles themselves are rotated. The
     * dimensions of the tile layer are swapped.
     */
    void rotate(RotateDirection direction);

    /**
     * Computes and returns the set of tilesets used by this tile layer.
     */
    QSet<SharedTileset> usedTilesets() const override;

    /**
     * Returns whether this tile layer has any cell for which the given
     * \a condition returns true.
     */
    bool hasCell(std::function<bool (const Cell &)> condition) const;

    /**
     * Returns whether this tile layer is referencing the given tileset.
     */
    bool referencesTileset(const Tileset *tileset) const override;

    /**
     * Removes all references to the given tileset. This sets all tiles on this
     * layer that are from the given tileset to null.
     */
    void removeReferencesToTileset(Tileset *tileset);

    /**
     * Replaces all tiles from \a oldTileset with tiles from \a newTileset.
     */
    void replaceReferencesToTileset(Tileset *oldTileset,
                                    Tileset *newTileset) override;

    /**
     * Resizes this tile layer to \a size, while shifting all tiles by
     * \a offset.
     */
    void resize(const QSize &size, const QPoint &offset);

    /**
     * Offsets the tiles in this layer within \a bounds by \a offset,
     * and optionally wraps them.
     *
     * \sa ObjectGroup::offsetObjects()
     */
    void offsetTiles(const QPoint &offset,
                     const QRect &bounds,
                     bool wrapX, bool wrapY);

    bool canMergeWith(Layer *other) const override;
    Layer *mergedWith(Layer *other) const override;

    /**
     * Returns the region where this tile layer and the given tile layer
     * are different. The relative positions of the layers are taken into
     * account. The returned region is relative to this tile layer.
     */
    QRegion computeDiffRegion(const TileLayer *other) const;

    /**
     * Returns true if all tiles in the layer are empty.
     */
    bool isEmpty() const override;

    virtual Layer *clone() const override;

    // Enable easy iteration over cells with range-based for
    QVector<Cell>::iterator begin() { return mGrid.begin(); }
    QVector<Cell>::iterator end() { return mGrid.end(); }
    QVector<Cell>::const_iterator begin() const { return mGrid.begin(); }
    QVector<Cell>::const_iterator end() const { return mGrid.end(); }

protected:
    TileLayer *initializeClone(TileLayer *clone) const;

private:
    QVector<Cell> mGrid;
    mutable QSet<SharedTileset> mUsedTilesets;
    mutable bool mUsedTilesetsDirty;
};


/**
 * Returns whether (x, y) is inside this map layer.
 */
inline bool TileLayer::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

inline bool TileLayer::contains(const QPoint &point) const
{
    return contains(point.x(), point.y());
}

inline QRegion TileLayer::region() const
{
    return region([] (const Cell &cell) { return !cell.isEmpty(); });
}

/**
 * Returns a read-only reference to the cell at the given coordinates. The
 * coordinates have to be within this layer.
 */
inline const Cell &TileLayer::cellAt(int x, int y) const
{
    Q_ASSERT(contains(x, y));
    return mGrid.at(x + y * mWidth);
}

inline const Cell &TileLayer::cellAt(const QPoint &point) const
{
    return cellAt(point.x(), point.y());
}

typedef QSharedPointer<TileLayer> SharedTileLayer;

} // namespace Tiled

#endif // TILELAYER_H
