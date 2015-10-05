/*
 * tileset.h
 * Copyright 2008-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef TILESET_H
#define TILESET_H

#include "imagereference.h"
#include "object.h"

#include <QColor>
#include <QList>
#include <QVector>
#include <QPoint>
#include <QSharedPointer>
#include <QString>
#include <QPixmap>

class QImage;

namespace Tiled {

class Tile;
class Tileset;
class Terrain;

typedef QSharedPointer<Tileset> SharedTileset;

/**
 * A tileset, representing a set of tiles.
 *
 * This class is meant to be used by either loading tiles from a tileset image
 * (using loadFromImage) or by adding/removing individual tiles (using
 * addTile, insertTiles and removeTiles). These two use-cases are not meant to
 * be mixed.
 */
class TILEDSHARED_EXPORT Tileset : public Object
{
public:
    /**
     * Creates a new tileset with the given parameters. Using this function
     * makes sure the internal weak pointer is initialized, which enables the
     * sharedPointer() function.
     *
     * @param name        the name of the tileset
     * @param tileWidth   the width of the tiles in the tileset
     * @param tileHeight  the height of the tiles in the tileset
     * @param tileSpacing the spacing between the tiles in the tileset image
     * @param margin      the margin around the tiles in the tileset image
     */
    static SharedTileset create(const QString &name,
                                int tileWidth,
                                int tileHeight,
                                int tileSpacing = 0,
                                int margin = 0)
    {
        SharedTileset tileset(new Tileset(name, tileWidth, tileHeight,
                                          tileSpacing, margin));
        tileset->mWeakPointer = tileset;
        return tileset;
    }

private:
    /**
     * Private constructor. Use create() instead.
     */
    Tileset(QString name, int tileWidth, int tileHeight,
            int tileSpacing = 0, int margin = 0):
        Object(TilesetType),
        mName(std::move(name)),
        mTileWidth(tileWidth),
        mTileHeight(tileHeight),
        mTileSpacing(tileSpacing),
        mMargin(margin),
        mColumnCount(0),
        mTerrainDistancesDirty(false)
    {
        Q_ASSERT(tileSpacing >= 0);
        Q_ASSERT(margin >= 0);
    }

public:
    ~Tileset();

    const QString &name() const;
    void setName(const QString &name);

    const QString &fileName() const;
    void setFileName(const QString &fileName);
    bool isExternal() const;

    int tileWidth() const;
    int tileHeight() const;
    QSize tileSize() const;

    int tileSpacing() const;
    int margin() const;

    QPoint tileOffset() const;
    void setTileOffset(QPoint offset);

    const QList<Tile*> &tiles() const;
    bool hasTile(int id) const;
    Tile *tileAt(int id) const;
    int tileCount() const;
    void expandTiles(int size);

    int columnCount() const;

    int imageWidth() const;
    int imageHeight() const;

    QColor transparentColor() const;
    void setTransparentColor(const QColor &c);

    void setImageReference(const ImageReference &reference);

    bool loadFromImage(const QImage &image, const QString &fileName);
    bool loadFromImage(const QString &fileName);
    bool loadImage();

    SharedTileset findSimilarTileset(const QVector<SharedTileset> &tilesets) const;

    const QString &imageSource() const;

    int columnCountForWidth(int width) const;

    const QList<Terrain*> &terrains() const;
    int terrainCount() const;
    Terrain *terrain(int index) const;

    Terrain *addTerrain(const QString &name, int imageTileId);
    void insertTerrain(int index, Terrain *terrain);
    Terrain *takeTerrainAt(int index);

    int terrainTransitionPenalty(int terrainType0, int terrainType1) const;

    Tile *addTile(const QPixmap &image, const QString &source = QString());
    void insertTiles(int index, const QList<Tile*> &tiles);
    void removeTiles(int index, int count);

    void setTileImage(int id,
                      const QPixmap &image,
                      const QString &source = QString());

    void markTerrainDistancesDirty();

    SharedTileset sharedPointer() const;

private:
    void updateTileSize();
    void recalculateTerrainDistances();

    QString mName;
    QString mFileName;
    ImageReference mImageReference;
    int mTileWidth;
    int mTileHeight;
    int mTileSpacing;
    int mMargin;
    QPoint mTileOffset;
    int mColumnCount;
    QList<Tile*> mTiles;
    QList<Terrain*> mTerrainTypes;
    bool mTerrainDistancesDirty;

    QWeakPointer<Tileset> mWeakPointer;
};


/**
 * Returns the name of this tileset.
 */
inline const QString &Tileset::name() const
{
    return mName;
}

/**
 * Sets the name of this tileset.
 */
inline void Tileset::setName(const QString &name)
{
    mName = name;
}

/**
 * Returns the file name of this tileset. When the tileset isn't an
 * external tileset, the file name is empty.
 */
inline const QString &Tileset::fileName() const
{
    return mFileName;
}

/**
 * Sets the filename of this tileset.
 */
inline void Tileset::setFileName(const QString &fileName)
{
    mFileName = fileName;
}

/**
 * Returns whether this tileset is external.
 */
inline bool Tileset::isExternal() const
{
    return !mFileName.isEmpty();
}

/**
 * Returns the maximum width of the tiles in this tileset.
 */
inline int Tileset::tileWidth() const
{
    return mTileWidth;
}

/**
 * Returns the maximum height of the tiles in this tileset.
 */
inline int Tileset::tileHeight() const
{
    return mTileHeight;
}

/**
 * Returns the maximum size of the tiles in this tileset.
 */
inline QSize Tileset::tileSize() const
{
    return QSize(mTileWidth, mTileHeight);
}

/**
 * Returns the spacing between the tiles in the tileset image.
 */
inline int Tileset::tileSpacing() const
{
    return mTileSpacing;
}

/**
 * Returns the margin around the tiles in the tileset image.
 */
inline int Tileset::margin() const
{
    return mMargin;
}

/**
 * Returns the offset that is applied when drawing the tiles in this
 * tileset.
 */
inline QPoint Tileset::tileOffset() const
{
    return mTileOffset;
}

/**
 * @see tileOffset
 */
inline void Tileset::setTileOffset(QPoint offset)
{
    mTileOffset = offset;
}

/**
 * Returns a const reference to the list of tiles in this tileset.
 */
inline const QList<Tile *> &Tileset::tiles() const
{
    return mTiles;
}

inline bool Tileset::hasTile(int id) const
{
    return id >= 0 && id < mTiles.size();
}

/**
 * Returns the tile at the given tile ID.
 * The tile ID is local to this tileset, which means the IDs are in range
 * [0, tileCount() - 1].
 */
inline Tile *Tileset::tileAt(int id) const
{
    return mTiles.at(id);
}

/**
 * Returns the number of tiles in this tileset.
 */
inline int Tileset::tileCount() const
{
    return mTiles.size();
}

/**
 * Returns the number of tile columns in the tileset image.
 */
inline int Tileset::columnCount() const
{
    return mColumnCount;
}

/**
 * Returns the width of the tileset image.
 */
inline int Tileset::imageWidth() const
{
    return mImageReference.width;
}

/**
 * Returns the height of the tileset image.
 */
inline int Tileset::imageHeight() const
{
    return mImageReference.height;
}

/**
 * Returns the transparent color, or an invalid color if no transparent
 * color is used.
 */
inline QColor Tileset::transparentColor() const
{
    return mImageReference.transparentColor;
}

/**
 * Convenience override that loads the image using the QImage constructor.
 */
inline bool Tileset::loadFromImage(const QString &fileName)
{
    return loadFromImage(QImage(fileName), fileName);
}

/**
 * Returns the file name of the external image that contains the tiles in
 * this tileset. Is an empty string when this tileset doesn't have a
 * tileset image.
 */
inline const QString &Tileset::imageSource() const
{
    return mImageReference.source;
}

/**
 * Returns a const reference to the list of terrains in this tileset.
 */
inline const QList<Terrain *> &Tileset::terrains() const
{
    return mTerrainTypes;
}

/**
 * Returns the number of terrain types in this tileset.
 */
inline int Tileset::terrainCount() const
{
    return mTerrainTypes.size();
}

/**
 * Returns the terrain type at the given \a index.
 */
inline Terrain *Tileset::terrain(int index) const
{
    return index >= 0 ? mTerrainTypes[index] : nullptr;
}

/**
 * Used by the Tile class when its terrain information changes.
 */
inline void Tileset::markTerrainDistancesDirty()
{
    mTerrainDistancesDirty = true;
}

inline SharedTileset Tileset::sharedPointer() const
{
    return SharedTileset(mWeakPointer);
}

} // namespace Tiled

#endif // TILESET_H
