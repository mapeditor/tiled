/*
 * tile.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#ifndef TILE_H
#define TILE_H

#include "object.h"
#include "tileset.h"

#include <QPixmap>

namespace Tiled {

struct TILEDSHARED_EXPORT TileData : public QSharedData
{
    TileData(int id, Tileset *tileset, const QPixmap *image) :
        mId(id),
        mTileset(tileset),
        mImage(image),
        mTerrain(0xFFFFFFFF),
        mTerrainProbability(-1.f)
    {}

    TileData(const TileData &o);

    ~TileData();

    int mId;
    Tileset *mTileset;
    const QPixmap *mImage;
    unsigned int mTerrain;
    float mTerrainProbability;
};

class TILEDSHARED_EXPORT Tile : public Object
{
public:
    Tile()
        : d(null.d)
    {}

    Tile(const QPixmap &image, int id, Tileset *tileset)
        : d(new TileData(id, tileset, new QPixmap(image)))
    {}

    bool isNull() const
    { return d == null.d && Object::isNull(); }

    /**
     * Returns ID of this tile within its tileset.
     */
    int id() const { return d->mId; }

    /**
     * Returns the tileset that this tile is part of.
     */
    Tileset *tileset() const { return d->mTileset; }

    /**
     * Returns the image of this tile.
     */
    QPixmap image() const { return d->mImage ? *d->mImage : QPixmap(); }

    /**
     * Sets the image of this tile.
     */
    void setImage(const QPixmap &image);

    /**
     * Returns the width of this tile.
     */
    int width() const { return d->mImage ? d->mImage->width() : 0; }

    /**
     * Returns the height of this tile.
     */
    int height() const { return d->mImage ? d->mImage->height() : 0; }

    /**
     * Returns the size of this tile.
     */
    QSize size() const { return d->mImage ? d->mImage->size() : QSize(0, 0); }

    /**
     * Returns the Terrain of a given corner.
     */
    Terrain *terrainAtCorner(int corner) const;

    /**
     * Returns the terrain id at a given corner.
     */
    int cornerTerrainId(int corner) const
    {
        unsigned int t = (terrain() >> (3 - corner)*8) & 0xFF;
        return t == 0xFF ? -1 : (int)t;
    }

    /**
     * Set the terrain type of a given corner.
     */
    void setCornerTerrain(int corner, int terrainId)
    {
        Q_ASSERT(d->mTileset);
        unsigned int mask = 0xFF << (3 - corner)*8;
        unsigned int insert = terrainId << (3 - corner)*8;
        d->mTerrain = (d->mTerrain & ~mask) | (insert & mask);
    }

    /**
     * Functions to get various terrain type information from tiles.
     */
    unsigned short topEdge() const { return terrain() >> 16; }
    unsigned short bottomEdge() const { return terrain() & 0xFFFF; }
    unsigned short leftEdge() const { return((terrain() >> 16) & 0xFF00) | ((terrain() >> 8) & 0xFF); }
    unsigned short rightEdge() const { return ((terrain() >> 8) & 0xFF00) | (terrain() & 0xFF); }
    unsigned int terrain() const { return d->mTerrain; }

    /**
     * Returns the probability of this terrain type appearing while painting (0-100%).
     */
    float terrainProbability() const { return d->mTerrainProbability; }

    /**
     * Set the probability of this terrain type appearing while painting (0-100%).
     */
    void setTerrainProbability(float probability) { d->mTerrainProbability = probability; }

    // These comparators ignore the Object part since it doesn't matter.
    bool operator==(const Tile &other) const { return d == other.d; }
    bool operator!=(const Tile &other) const { return d != other.d; }

private:
    /** Only used to construct the shared 'null' tile. */
    explicit Tile(TileData *data):
        d(data)
    {}

    QSharedDataPointer<TileData> d;
    static const Tile null;
};

} // namespace Tiled

#endif // TILE_H
