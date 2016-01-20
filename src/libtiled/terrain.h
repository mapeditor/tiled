/*
 * terrain.h
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "object.h"
#include "tileset.h"

#include <QMetaType>
#include <QString>
#include <QVector>

namespace Tiled {

/**
 * Represents a terrain type.
 */
class TILEDSHARED_EXPORT Terrain : public Object
{
public:
    Terrain(int id,
            Tileset *tileset,
            QString name,
            int imageTileId):
        Object(TerrainType),
        mId(id),
        mTileset(tileset),
        mName(std::move(name)),
        mImageTileId(imageTileId)
    {
    }

    int id() const;

    Tileset *tileset() const;
    QSharedPointer<Tileset> sharedTileset() const;

    QString name() const;
    void setName(const QString &name);

    int imageTileId() const;
    void setImageTileId(int imageTileId);

    Tile *imageTile() const;

    int transitionDistance(int targetTerrainType) const;
    void setTransitionDistance(int targetTerrainType, int distance);
    void setTransitionDistances(const QVector<int> &transitionDistances);

private:
    int mId;
    Tileset *mTileset;
    QString mName;
    int mImageTileId;
    QVector<int> mTransitionDistance;

    friend class Tileset; // To allow changing the terrain id
};

/**
 * Returns ID of this terrain type.
 */
inline int Terrain::id() const
{
    return mId;
}

/**
 * Returns the tileset this terrain type belongs to.
 */
inline Tileset *Terrain::tileset() const
{
    return mTileset;
}

/**
 * Returns the tileset this terrain type belongs to as a shared pointer.
 */
inline QSharedPointer<Tileset> Terrain::sharedTileset() const
{
    return mTileset->sharedPointer();
}

/**
 * Returns the name of this terrain type.
 */
inline QString Terrain::name() const
{
    return mName;
}

/**
 * Sets the name of this terrain type.
 */
inline void Terrain::setName(const QString &name)
{
    mName = name;
}

/**
 * Returns the index of the tile that visually represents this terrain type.
 */
inline int Terrain::imageTileId() const
{
    return mImageTileId;
}

/**
 * Sets the index of the tile that visually represents this terrain type.
 */
inline void Terrain::setImageTileId(int imageTileId)
{
    mImageTileId = imageTileId;
}

/**
 * Returns a Tile that represents this terrain type in the terrain palette.
 */
inline Tile *Terrain::imageTile() const
{
    return mTileset->findTile(mImageTileId);
}

/**
 * Returns the transition penalty(/distance) from this terrain type to another terrain type.
 */
inline int Terrain::transitionDistance(int targetTerrainType) const
{
    return mTransitionDistance[targetTerrainType + 1];
}

/**
 * Sets the transition penalty(/distance) from this terrain type to another terrain type.
 */
inline void Terrain::setTransitionDistance(int targetTerrainType, int distance)
{
    mTransitionDistance[targetTerrainType + 1] = distance;
}

/**
 * Returns the array of terrain penalties(/distances).
 */
inline void Terrain::setTransitionDistances(const QVector<int> &transitionDistances)
{
    mTransitionDistance = transitionDistances;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Terrain*)

#endif // TERRAIN_H
