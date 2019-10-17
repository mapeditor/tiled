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
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <functional>

inline uint qHash(QPoint key, uint seed = 0) Q_DECL_NOTHROW
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
class TILEDSHARED_EXPORT Cell
{
    Q_GADGET

    Q_PROPERTY(int tileId READ tileId)
    Q_PROPERTY(bool empty READ isEmpty)
    Q_PROPERTY(bool flippedHorizontally READ flippedHorizontally WRITE setFlippedHorizontally)
    Q_PROPERTY(bool flippedVertically READ flippedVertically WRITE setFlippedVertically)
    Q_PROPERTY(bool flippedAntiDiagonally READ flippedAntiDiagonally WRITE setFlippedAntiDiagonally)
    Q_PROPERTY(bool rotatedHexagonal120 READ rotatedHexagonal120 WRITE setRotatedHexagonal120)

public:
    static Cell empty;

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

    Cell(Tileset *tileset, int tileId) :
        _tileset(tileset),
        _tileId(tileId),
        _flags(0)
    {}

    bool isEmpty() const { return _tileset == nullptr; }

    bool operator == (const Cell &other) const
    {
        return _tileset == other._tileset
                && _tileId == other._tileId
                && (_flags & VisualFlags) == (other._flags & VisualFlags);
    }

    bool operator != (const Cell &other) const
    {
        return !(*this == other);
    }

    Tileset *tileset() const { return _tileset; }
    int tileId() const { return _tileId; }

    bool flippedHorizontally() const { return _flags & FlippedHorizontally; }
    bool flippedVertically() const { return _flags & FlippedVertically; }
    bool flippedAntiDiagonally() const { return _flags & FlippedAntiDiagonally; }
    bool rotatedHexagonal120() const { return _flags & RotatedHexagonal120; }

    void setFlippedHorizontally(bool v) { v ? _flags |= FlippedHorizontally : _flags &= ~FlippedHorizontally; }
    void setFlippedVertically(bool v) { v ? _flags |= FlippedVertically : _flags &= ~FlippedVertically; }
    void setFlippedAntiDiagonally(bool v) { v ? _flags |= FlippedAntiDiagonally : _flags &= ~FlippedAntiDiagonally; }
    void setRotatedHexagonal120(bool v) { v ? _flags |= RotatedHexagonal120 : _flags &= ~RotatedHexagonal120; }

    bool checked() const { return _flags & Checked; }
    void setChecked(bool checked) { checked ? _flags |= Checked : _flags &= ~Checked; }

    Tile *tile() const;
    void setTile(Tileset *tileset, int tileId);
    void setTile(Tile *tile);
    bool refersTile(const Tile *tile) const;

private:
    enum Flags {
        FlippedHorizontally     = 0x01,
        FlippedVertically       = 0x02,
        FlippedAntiDiagonally   = 0x04,
        RotatedHexagonal120     = 0x08,
        Checked                 = 0x10,
        VisualFlags             = FlippedHorizontally | FlippedVertically | FlippedAntiDiagonally | RotatedHexagonal120
    };

    Tileset *_tileset;
    int _tileId;
    int _flags;
};

inline Tile *Cell::tile() const
{
    return _tileset ? _tileset->findTile(_tileId) : nullptr;
}

inline void Cell::setTile(Tileset *tileset, int tileId)
{
    _tileset = tileset;
    _tileId = tileId;
}

inline void Cell::setTile(Tile *tile)
{
    if (tile)
        setTile(tile->tileset(), tile->id());
    else
        setTile(nullptr, -1);
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
    const Cell &cellAt(QPoint point) const;

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

inline const Cell &Chunk::cellAt(QPoint point) const
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
    Q_OBJECT

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

    TileLayer(const QString &name = QString(),
              QPoint position = QPoint(),
              QSize size = QSize(0, 0));

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

    void setSize(QSize size);

    /**
     * Returns the bounds of this layer in map tile coordinates.
     */
    QRect bounds() const { return mBounds.translated(mX, mY); }

    /**
     * Returns the bounds of this layer in local tile coordinates.
     */
    QRect localBounds() const { return mBounds; }

    QRect rect() const { return QRect(mX, mY, mWidth, mHeight); }

    QMargins drawMargins() const;

    bool contains(int x, int y) const;
    bool contains(QPoint point) const;

    Chunk &chunk(int x, int y);

    const Chunk *findChunk(int x, int y) const;

    QRegion region(std::function<bool (const Cell &)> condition) const;
    QRegion region() const;

    const Cell &cellAt(int x, int y) const;
    const Cell &cellAt(QPoint point) const;

    void setCell(int x, int y, const Cell &cell);

    /**
     * Returns a copy of the area specified by the given \a region. The
     * caller is responsible for the returned tile layer.
     */
    std::unique_ptr<TileLayer> copy(const QRegion &region) const;

    std::unique_ptr<TileLayer> copy(int x, int y, int width, int height) const
    { return copy(QRegion(x, y, width, height)); }

    /**
     * Merges the given \a layer onto this layer at position \a pos. Parts that
     * fall outside of this layer will be lost and empty tiles in the given
     * layer will have no effect.
     */
    void merge(QPoint pos, const TileLayer *layer);

    /**
     * Removes all cells in the specified region.
     */
    void erase(const QRegion &region);

    void clear();

    /**
     * Sets the cells within the given \a area to the cells in the given
     * \a tileLayer. The tiles in \a tileLayer are offset by \a x and \a y.
     */
    void setCells(int x, int y, const TileLayer *tileLayer, const QRegion &area);

    /**
     * Sets the cells starting at the given position to the cells in the given
     * \a tileLayer.
     */
    void setCells(int x, int y, const TileLayer *tileLayer);

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
    void resize(QSize size, QPoint offset);

    /**
     * Offsets the tiles in this layer within \a bounds by \a offset,
     * and optionally wraps them.
     *
     * \sa ObjectGroup::offsetObjects()
     */
    void offsetTiles(QPoint offset,
                     QRect bounds,
                     bool wrapX, bool wrapY);

    /**
     * Offsets the tiles in this layer by \a offset.
     *
     * \sa ObjectGroup::offsetObjects()
     */
    void offsetTiles(QPoint offset);

    bool canMergeWith(const Layer *other) const override;
    Layer *mergedWith(const Layer *other) const override;

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

    QVector<QRect> sortedChunksToWrite(QSize chunkSize) const;

protected:
    TileLayer *initializeClone(TileLayer *clone) const;

private:
    int mWidth;
    int mHeight;
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
inline void TileLayer::setSize(QSize size)
{
    mWidth = size.width();
    mHeight = size.height();
}

inline bool TileLayer::contains(int x, int y) const
{
    return x >= 0 && y >= 0 && x < mWidth && y < mHeight;
}

inline bool TileLayer::contains(QPoint point) const
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

/**
 * Calculates the region occupied by the tiles of this layer. Similar to
 * Layer::bounds(), but leaves out the regions without tiles.
 */
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
        return Cell::empty;
}

inline const Cell &TileLayer::cellAt(QPoint point) const
{
    return cellAt(point.x(), point.y());
}

inline void TileLayer::setCells(int x, int y, const TileLayer *tileLayer)
{
    setCells(x, y, tileLayer,
             QRect(x, y, tileLayer->width(), tileLayer->height()));
}

typedef QSharedPointer<TileLayer> SharedTileLayer;

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Cell)
