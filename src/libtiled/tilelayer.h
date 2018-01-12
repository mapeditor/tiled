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

#pragma once

#include "tiled_global.h"

#include "layer.h"
#include "tiled.h"
#include "tile.h"
#include "tileset.h"

#include <QHash>
#include <QMargins>
#include <QPoint>
#include <QString>
#include <QVector>
#include <QSharedPointer>

#include <functional>

inline uint qHash(const QPoint &key, uint seed = 0) Q_DECL_NOTHROW
{
    uint h1 = qHash(key.x(), seed);
    uint h2 = qHash(key.y(), seed);
    return ((h1 << 16) | (h1 >> 16)) ^ h2 ^ seed;
}

namespace Tiled {

class Tile;

/**
 * A cell on a tile layer grid.
 */
class Cell
{
public:
    Cell() :
        _tileset(nullptr),
        _tileId(-1),
        _flags(0)
    {}

    explicit Cell(Tile *tile) :
        _tileset(tile ? tile->tileset() : nullptr),
        _tileId(tile ? tile->id() : -1),
        _flags(0)
    {}

    bool isEmpty() const { return _tileset == nullptr; }

    bool operator == (const Cell &other) const
    {
        return _tileset == other._tileset
                && _tileId == other._tileId
                && _flags == other._flags;
    }

    bool operator != (const Cell &other) const
    {
        return _tileset != other._tileset
                || _tileId != other._tileId
                || _flags != other._flags;
    }

    Tileset *tileset() const { return _tileset; }
    int tileId() const { return _tileId; }

    bool flippedHorizontally() const { return f._flippedHorizontally; }
    bool flippedVertically() const { return f._flippedVertically; }
    bool flippedAntiDiagonally() const { return f._flippedAntiDiagonally; }
    bool rotatedHexagonal120() const { return f._rotatedHexagonal120; }

    void setFlippedHorizontally(bool v) { f._flippedHorizontally = v; }
    void setFlippedVertically(bool v) { f._flippedVertically = v; }
    void setFlippedAntiDiagonally(bool v) { f._flippedAntiDiagonally = v; }
    void setRotatedHexagonal120(bool v) { f._rotatedHexagonal120 = v; }

    bool checked() const { return f._checked; }
    void setChecked(bool checked) { f._checked = checked; }

    Tile *tile() const;
    void setTile(Tile *tile);
    void setTile(Tileset *tileset, int tileId);
    bool refersTile(const Tile *tile) const;

private:
    Tileset *_tileset;
    int _tileId;

    struct Flags {
        bool _flippedHorizontally : 1;
        bool _flippedVertically : 1;
        bool _flippedAntiDiagonally : 1;
        bool _rotatedHexagonal120 : 1;
        bool _checked : 1;
    };

    union {
        unsigned _flags;
        Flags f;
    };
};

inline Tile *Cell::tile() const
{
    return _tileset ? _tileset->findTile(_tileId) : nullptr;
}

inline void Cell::setTile(Tile *tile)
{
    if (tile)
        setTile(tile->tileset(), tile->id());
    else
        setTile(nullptr, -1);
}

inline void Cell::setTile(Tileset *tileset, int tileId)
{
    _tileset = tileset;
    _tileId = tileId;
}

inline bool Cell::refersTile(const Tile *tile) const
{
    return _tileset == tile->tileset() && _tileId == tile->id();
}


/**
 * A Chunk is a grid of cells of size CHUNK_SIZExCHUNK_SIZE.
 */
class TILEDSHARED_EXPORT Chunk
{
public:
    Chunk() :
        mGrid(CHUNK_SIZE * CHUNK_SIZE)
    {}

    QRegion region(std::function<bool (const Cell &)> condition) const;

    const Cell &cellAt(int x, int y) const;
    const Cell &cellAt(const QPoint &point) const;

    void setCell(int x, int y, const Cell &cell);

    bool isEmpty() const;

    bool hasCell(std::function<bool (const Cell &)> condition) const;

    void removeReferencesToTileset(Tileset *tileset);

    void replaceReferencesToTileset(Tileset *oldTileset, Tileset *newTileset);

    QVector<Cell>::iterator begin() { return mGrid.begin(); }
    QVector<Cell>::iterator end() { return mGrid.end(); }
    QVector<Cell>::const_iterator begin() const { return mGrid.begin(); }
    QVector<Cell>::const_iterator end() const { return mGrid.end(); }

private:
    QVector<Cell> mGrid;
};

inline const Cell &Chunk::cellAt(int x, int y) const
{
    return mGrid.at(x + y * CHUNK_SIZE);
}

inline const Cell &Chunk::cellAt(const QPoint &point) const
{
    return cellAt(point.x(), point.y());
}

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
    class iterator
    {
    public:
        iterator(QHash<QPoint, Chunk>::iterator it, QHash<QPoint, Chunk>::iterator end)
            : mChunkPointer(it)
            , mChunkEndPointer(end)
        {
            if (it != end)
                mCellPointer = it.value().begin();
        }

        iterator operator++(int)
        {
            iterator it = *this;
            advance();
            return it;
        }

        iterator &operator++()
        {
            advance();
            return *this;
        }

        Cell &operator*() { return *mCellPointer; }

        QVector<Cell>::iterator operator->() const { return mCellPointer; }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
        {
            if (lhs.mChunkPointer == lhs.mChunkEndPointer || rhs.mChunkPointer == rhs.mChunkEndPointer)
                return lhs.mChunkPointer == rhs.mChunkPointer;
            else
                return lhs.mCellPointer == rhs.mCellPointer;
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
        {
            if (lhs.mChunkPointer == lhs.mChunkEndPointer || rhs.mChunkPointer == rhs.mChunkEndPointer)
                return lhs.mChunkPointer != rhs.mChunkPointer;
            else
                return lhs.mCellPointer != rhs.mCellPointer;
        }

        Cell &value() const { return *mCellPointer; }

        QPoint key() const;

    private:
        void advance();

        QHash<QPoint, Chunk>::iterator mChunkPointer;
        QHash<QPoint, Chunk>::iterator mChunkEndPointer;
        QVector<Cell>::iterator mCellPointer;
    };

    class const_iterator
    {
    public:
        const_iterator(QHash<QPoint, Chunk>::const_iterator it, QHash<QPoint, Chunk>::const_iterator end)
            : mChunkPointer(it)
            , mChunkEndPointer(end)
        {
            if (it != end)
                mCellPointer = it.value().begin();
        }

        const_iterator operator++(int)
        {
            const_iterator it = *this;
            advance();
            return it;
        }

        const_iterator &operator++()
        {
            advance();
            return *this;
        }

        const Cell &operator*() { return *mCellPointer; }

        QVector<Cell>::const_iterator operator->() const { return mCellPointer; }

        friend bool operator==(const const_iterator& lhs, const const_iterator& rhs)
        {
            if (lhs.mChunkPointer == lhs.mChunkEndPointer || rhs.mChunkPointer == rhs.mChunkEndPointer)
                return lhs.mChunkPointer == rhs.mChunkPointer;
            else
                return lhs.mCellPointer == rhs.mCellPointer;
        }

        friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs)
        {
            if (lhs.mChunkPointer == lhs.mChunkEndPointer || rhs.mChunkPointer == rhs.mChunkEndPointer)
                return lhs.mChunkPointer != rhs.mChunkPointer;
            else
                return lhs.mCellPointer != rhs.mCellPointer;
        }

        const Cell &value() const { return *mCellPointer; }

        QPoint key() const;

    private:
        void advance();

        QHash<QPoint, Chunk>::const_iterator mChunkPointer;
        QHash<QPoint, Chunk>::const_iterator mChunkEndPointer;
        QVector<Cell>::const_iterator mCellPointer;
    };

    /**
     * Constructor.
     */
    TileLayer(const QString &name, int x, int y, int width, int height);

    /**
     * Returns the width of this layer.
     */
    int width() const { return mWidth; }

    /**
     * Returns the height of this layer.
     */
    int height() const { return mHeight; }

    /**
     * Returns the size of this layer.
     */
    QSize size() const { return QSize(mWidth, mHeight); }

    void setSize(const QSize &size);

    /**
     * Returns the bounds of this layer.
     */
    QRect bounds() const { return mBounds.translated(mX, mY); }

    QRect rect() const { return QRect(mX, mY, mWidth, mHeight); }

    QMargins drawMargins() const;

    bool contains(int x, int y) const;
    bool contains(const QPoint &point) const;

    Chunk &chunk(int x, int y);

    const Chunk *findChunk(int x, int y) const;

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

    void setTiles(const QRegion &area, Tile *tile);

    /**
     * Flip this tile layer in the given \a direction. Direction must be
     * horizontal or vertical. This doesn't change the dimensions of the
     * tile layer.
     */
    void flip(FlipDirection direction);

    /**
     * Hexagonal flip this tile layer in the given \a direction. Direction must be
     * horizontal or vertical. This doesn't change the dimensions of the
     * tile layer.
     */
    void flipHexagonal(FlipDirection direction);

    /**
     * Rotate this tile layer by 90 degrees left or right. The tile positions
     * are rotated within the layer, and the tiles themselves are rotated. The
     * dimensions of the tile layer are swapped.
     */
    void rotate(RotateDirection direction);

    /**
     * Hexagonal rotate this tile layer by 60 degrees left or right. The tile positions
     * are rotated within the layer, and the tiles themselves are rotated.
     * As a temporary measure, a Map* is passed to give information about stagger index
     * and axis, which affects rotation. The stagger index of this map can change.
     */
    void rotateHexagonal(RotateDirection direction, Map *map);

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

    TileLayer *clone() const override;

    iterator begin() { return iterator(mChunks.begin(), mChunks.end()); }
    iterator end() { return iterator(mChunks.end(), mChunks.end()); }
    const_iterator begin() const { return const_iterator(mChunks.begin(), mChunks.end()); }
    const_iterator end() const { return const_iterator(mChunks.end(), mChunks.end()); }

    QVector<QRect> sortedChunksToWrite() const;

protected:
    TileLayer *initializeClone(TileLayer *clone) const;

private:
    int mWidth;
    int mHeight;
    Cell mEmptyCell;
    QHash<QPoint, Chunk> mChunks;
    QRect mBounds;
    mutable QSet<SharedTileset> mUsedTilesets;
    mutable bool mUsedTilesetsDirty;
};

inline QPoint TileLayer::iterator::key() const
{
    QPoint chunkStart = mChunkPointer.key();

    int index = mCellPointer - mChunkPointer.value().begin();
    chunkStart += QPoint(index & CHUNK_MASK, index / CHUNK_SIZE);

    return chunkStart;
}

inline void TileLayer::iterator::advance()
{
    if (mChunkPointer != mChunkEndPointer) {
        if (++mCellPointer == mChunkPointer.value().end()) {
            mChunkPointer++;
            if (mChunkPointer != mChunkEndPointer)
                mCellPointer = mChunkPointer.value().begin();
        }
    }
}

inline QPoint TileLayer::const_iterator::key() const
{
    QPoint chunkStart = mChunkPointer.key();

    int index = mCellPointer - mChunkPointer.value().begin();
    chunkStart += QPoint(index & CHUNK_MASK, index / CHUNK_SIZE);

    return chunkStart;
}

inline void TileLayer::const_iterator::advance()
{
    if (mChunkPointer != mChunkEndPointer) {
        if (++mCellPointer == mChunkPointer.value().end()) {
            mChunkPointer++;
            if (mChunkPointer != mChunkEndPointer)
                mCellPointer = mChunkPointer.value().begin();
        }
    }
}

/**
 * Sets the size of this layer.
 */
inline void TileLayer::setSize(const QSize &size)
{
    mWidth = size.width();
    mHeight = size.height();
}

inline bool TileLayer::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

inline bool TileLayer::contains(const QPoint &point) const
{
    return contains(point.x(), point.y());
}

inline Chunk& TileLayer::chunk(int x, int y)
{
    QPoint chunkCoordinates(x < 0 ? (x + 1) / CHUNK_SIZE - 1 : x / CHUNK_SIZE,
                            y < 0 ? (y + 1) / CHUNK_SIZE - 1 : y / CHUNK_SIZE);
    return mChunks[chunkCoordinates];
}

inline const Chunk* TileLayer::findChunk(int x, int y) const
{
    QPoint chunkCoordinates(x < 0 ? (x + 1) / CHUNK_SIZE - 1 : x / CHUNK_SIZE,
                            y < 0 ? (y + 1) / CHUNK_SIZE - 1 : y / CHUNK_SIZE);
    auto it = mChunks.find(chunkCoordinates);
    return it != mChunks.end() ? &it.value() : nullptr;
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
    if (const Chunk *chunk = findChunk(x, y))
        return chunk->cellAt(x & CHUNK_MASK, y & CHUNK_MASK);
    else
        return mEmptyCell;
}

inline const Cell &TileLayer::cellAt(const QPoint &point) const
{
    return cellAt(point.x(), point.y());
}

typedef QSharedPointer<TileLayer> SharedTileLayer;

} // namespace Tiled
