/*
 * tilelayer.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QString>
#include <QVector>

namespace Tiled {

class Tile;
class Tileset;

/**
 * A tile layer.
 */
class TILEDSHARED_EXPORT TileLayer : public Layer
{
public:
    /**
     * Constructor.
     */
    TileLayer(const QString &name, int x, int y, int width, int height);

    /**
     * Returns the maximum tile size of this layer. Used by the layer
     * rendering code to determine the area that needs to be redrawn.
     */
    QSize maxTileSize() const { return mMaxTileSize; }

    /**
     * Returns whether (x, y) is inside this map layer.
     */
    bool contains(int x, int y) const
    { return x >= 0 && y >= 0 && x < mWidth && y < mHeight; }

    bool contains(const QPoint &point) const
    { return contains(point.x(), point.y()); }

    /**
     * Calculates the region occupied by the tiles of this layer. Similar to
     * Layer::bounds(), but leaves out the regions without tiles.
     */
    QRegion region() const;

    /**
     * Returns the tile at the given coordinates. The coordinates have to
     * be within this layer.
     */
    Tile *tileAt(int x, int y) const
    { return mTiles.at(x + y * mWidth); }

    Tile *tileAt(const QPoint &point) const
    { return tileAt(point.x(), point.y()); }

    /**
     * Sets the tile for the given coordinates.
     */
    void setTile(int x, int y, Tile *tile);

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
     * Computes and returns the set of tilesets used by this tile layer.
     */
    QSet<Tileset*> usedTilesets() const;

    /**
     * Returns whether this tile layer is referencing the given tileset.
     */
    bool referencesTileset(const Tileset *tileset) const;

    /**
     * Returns the region of tiles coming from the given \a tileset.
     */
    QRegion tilesetReferences(Tileset *tileset) const;

    /**
     * Removes all references to the given tileset. This sets all tiles on this
     * layer that are from the given tileset to null.
     */
    void removeReferencesToTileset(Tileset *tileset);

    /**
     * Replaces all tiles from \a oldTileset with tiles from \a newTileset.
     */
    void replaceReferencesToTileset(Tileset *oldTileset, Tileset *newTileset);

    /**
     * Resizes this tile layer to \a size, while shifting all tiles by
     * \a offset.
     *
     * \sa Layer::resize()
     */
    virtual void resize(const QSize &size, const QPoint &offset);

    /**
     * Offsets the objects in this group by \a offset, within \bounds
     * and optionally wraps it.
     *
     * \sa Layer::offset()
     */
    virtual void offset(const QPoint &offset,
                        const QRect &bounds,
                        bool wrapX, bool wrapY);

    bool canMergeWith(Layer *other) const;
    Layer *mergedWith(Layer *other) const;

    /**
     * Returns true if all tiles in the layer are empty.
     */
    bool isEmpty() const;

    virtual Layer *clone() const;

    virtual TileLayer *asTileLayer() { return this; }

protected:
    TileLayer *initializeClone(TileLayer *clone) const;

private:
    QSize mMaxTileSize;
    QVector<Tile*> mTiles;
};

} // namespace Tiled

#endif // TILELAYER_H
