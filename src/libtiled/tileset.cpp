/*
 * tileset.cpp
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

#include "tileset.h"
#include "tile.h"
#include "terrain.h"

#include <QBitmap>

using namespace Tiled;

Tileset::~Tileset()
{
    qDeleteAll(mTiles);
    qDeleteAll(mTerrainTypes);
}

Tile *Tileset::tileAt(int id) const
{
    return (id < mTiles.size()) ? mTiles.at(id) : 0;
}

bool Tileset::loadFromImage(const QImage &image, const QString &fileName)
{
    Q_ASSERT(mTileWidth > 0 && mTileHeight > 0);

    if (image.isNull())
        return false;

    const int stopWidth = image.width() - mTileWidth;
    const int stopHeight = image.height() - mTileHeight;

    int oldTilesetSize = mTiles.size();
    int tileNum = 0;

    for (int y = mMargin; y <= stopHeight; y += mTileHeight + mTileSpacing) {
        for (int x = mMargin; x <= stopWidth; x += mTileWidth + mTileSpacing) {
            const QImage tileImage = image.copy(x, y, mTileWidth, mTileHeight);
            QPixmap tilePixmap = QPixmap::fromImage(tileImage);

            if (mTransparentColor.isValid()) {
                const QImage mask =
                        tileImage.createMaskFromColor(mTransparentColor.rgb());
                tilePixmap.setMask(QBitmap::fromImage(mask));
            }

            if (tileNum < oldTilesetSize) {
                mTiles.at(tileNum)->setImage(tilePixmap);
            } else {
                mTiles.append(new Tile(tilePixmap, tileNum, this));
            }
            ++tileNum;
        }
    }

    // Blank out any remaining tiles to avoid confusion
    while (tileNum < oldTilesetSize) {
        QPixmap tilePixmap = QPixmap(mTileWidth, mTileHeight);
        tilePixmap.fill();
        mTiles.at(tileNum)->setImage(tilePixmap);
        ++tileNum;
    }

    mImageWidth = image.width();
    mImageHeight = image.height();
    mColumnCount = columnCountForWidth(mImageWidth);
    mImageSource = fileName;
    return true;
}

bool Tileset::loadFromImage(const QString &fileName)
{
    return loadFromImage(QImage(fileName), fileName);
}

Tileset *Tileset::findSimilarTileset(const QList<Tileset*> &tilesets) const
{
    foreach (Tileset *candidate, tilesets) {
        if (candidate != this
            && candidate->imageSource() == imageSource()
            && candidate->tileWidth() == tileWidth()
            && candidate->tileHeight() == tileHeight()
            && candidate->tileSpacing() == tileSpacing()
            && candidate->margin() == margin()) {
                return candidate;
        }
    }
    return 0;
}

int Tileset::columnCountForWidth(int width) const
{
    Q_ASSERT(mTileWidth > 0);
    return (width - mMargin + mTileSpacing) / (mTileWidth + mTileSpacing);
}

Terrain *Tileset::addTerrain(const QString &name, int imageTileId)
{
    Terrain *terrain = new Terrain(terrainCount(), this, name, imageTileId);
    insertTerrain(terrainCount(), terrain);
    return terrain;
}

void Tileset::insertTerrain(int index, Terrain *terrain)
{
    Q_ASSERT(terrain->tileset() == this);

    mTerrainTypes.insert(index, terrain);

    // Reassign terrain IDs
    for (int terrainId = index; terrainId < mTerrainTypes.size(); ++terrainId)
        mTerrainTypes.at(terrainId)->setId(terrainId);

    // Adjust tile terrain references
    foreach (Tile *tile, mTiles) {
        for (int corner = 0; corner < 4; ++corner) {
            const int terrainId = tile->cornerTerrainId(corner);
            if (terrainId >= index)
                tile->setCornerTerrain(corner, terrainId + 1);
        }
    }

    mTerrainDistancesDirty = true;
}

Terrain *Tileset::takeTerrainAt(int index)
{
    Terrain *terrain = mTerrainTypes.takeAt(index);

    // Reassign terrain IDs
    for (int terrainId = index; terrainId < mTerrainTypes.size(); ++terrainId)
        mTerrainTypes.at(terrainId)->setId(terrainId);

    // Clear and adjust tile terrain references
    foreach (Tile *tile, mTiles) {
        for (int corner = 0; corner < 4; ++corner) {
            const int terrainId = tile->cornerTerrainId(corner);
            if (terrainId == index)
                tile->setCornerTerrain(corner, 0xFF);
            else if (terrainId > index)
                tile->setCornerTerrain(corner, terrainId - 1);
        }
    }

    mTerrainDistancesDirty = true;

    return terrain;
}

int Tileset::terrainTransitionPenalty(int terrainType0, int terrainType1)
{
    if (mTerrainDistancesDirty) {
        recalculateTerrainDistances();
        mTerrainDistancesDirty = false;
    }

    terrainType0 = terrainType0 == 255 ? -1 : terrainType0;
    terrainType1 = terrainType1 == 255 ? -1 : terrainType1;

    // Do some magic, since we don't have a transition array for no-terrain
    if (terrainType0 == -1 && terrainType1 == -1)
        return 0;
    if (terrainType0 == -1)
        return mTerrainTypes.at(terrainType1)->transitionDistance(terrainType0);
    return mTerrainTypes.at(terrainType0)->transitionDistance(terrainType1);
}

void Tileset::recalculateTerrainDistances()
{
    // some fancy macros which can search for a value in each byte of a word simultaneously
    #define hasZeroByte(dword) (((dword) - 0x01010101UL) & ~(dword) & 0x80808080UL)
    #define hasByteEqualTo(dword, value) (hasZeroByte((dword) ^ (~0UL/255 * (value))))

    // Terrain distances are the number of transitions required before one terrain may meet another
    // Terrains that have no transition path have a distance of -1

    for (int i = 0; i < terrainCount(); ++i) {
        Terrain *type = terrain(i);
        QVector<int> distance(terrainCount() + 1, -1);

        // Check all tiles for transitions to other terrain types
        for (int j = 0; j < tileCount(); ++j) {
            Tile *t = tileAt(j);

            if (!hasByteEqualTo(t->terrain(), i))
                continue;

            // This tile has transitions, add the transitions as neightbours (distance 1)
            int tl = t->cornerTerrainId(0);
            int tr = t->cornerTerrainId(1);
            int bl = t->cornerTerrainId(2);
            int br = t->cornerTerrainId(3);

            // Terrain on diagonally opposite corners are not actually a neighbour
            if (tl == i || br == i) {
                distance[tr + 1] = 1;
                distance[bl + 1] = 1;
            }
            if (tr == i || bl == i) {
                distance[tl + 1] = 1;
                distance[br + 1] = 1;
            }

            // terrain has at least one tile of its own type
            distance[i + 1] = 0;
        }

        type->setTransitionDistances(distance);
    }

    // Calculate indirect transition distances
    bool bNewConnections;
    do {
        bNewConnections = false;

        // For each combination of terrain types
        for (int i = 0; i < terrainCount(); ++i) {
            Terrain *t0 = terrain(i);
            for (int j = 0; j < terrainCount(); ++j) {
                if (i == j)
                    continue;
                Terrain *t1 = terrain(j);

                // Scan through each terrain type, and see if we have any in common
                for (int t = -1; t < terrainCount(); ++t) {
                    int d0 = t0->transitionDistance(t);
                    int d1 = t1->transitionDistance(t);
                    if (d0 == -1 || d1 == -1)
                        continue;

                    // We have cound a common connection
                    int d = t0->transitionDistance(j);
                    Q_ASSERT(t1->transitionDistance(i) == d);

                    // If the new path is shorter, record the new distance
                    if (d == -1 || d0 + d1 < d) {
                        d = d0 + d1;
                        t0->setTransitionDistance(j, d);
                        t1->setTransitionDistance(i, d);

                        // We're making progress, flag for another iteration...
                        bNewConnections = true;
                    }
                }
            }
        }

        // Repeat while we are still making new connections (could take a
        // number of iterations for distant terrain types to connect)
    } while (bNewConnections);
}

Tile *Tileset::addTile(const QPixmap &image, const QString &source)
{
    Tile *newTile = new Tile(image, source, tileCount(), this);
    mTiles.append(newTile);
    if (mTileHeight < image.height())
        mTileHeight = image.height();
    if (mTileWidth < image.width())
        mTileWidth = image.width();
    return newTile;
}

void Tileset::insertTiles(int index, const QList<Tile *> &tiles)
{
    const int count = tiles.count();
    for (int i = 0; i < count; ++i)
        mTiles.insert(index + i, tiles.at(i));

    // Adjust the tile IDs of the remaining tiles
    for (int i = index + count; i < mTiles.size(); ++i)
        mTiles.at(i)->mId += count;

    updateTileSize();
}

void Tileset::removeTiles(int index, int count)
{
    const QList<Tile*>::iterator first = mTiles.begin() + index;

    QList<Tile*>::iterator last = first + count;
    last = mTiles.erase(first, last);

    // Adjust the tile IDs of the remaining tiles
    for (; last != mTiles.end(); ++last)
        (*last)->mId -= count;

    updateTileSize();
}

void Tileset::setTileImage(int id, const QPixmap &image,
                           const QString &source)
{
    // This operation is not supposed to be used on tilesets that are based
    // on a single image
    Q_ASSERT(mImageSource.isEmpty());

    Tile *tile = tileAt(id);
    if (!tile)
        return;

    const QSize previousImageSize = tile->image().size();
    const QSize newImageSize = image.size();

    tile->setImage(image);
    tile->setImageSource(source);

    if (previousImageSize != newImageSize) {
        // Update our max. tile size
        if (previousImageSize.height() == mTileHeight ||
                previousImageSize.width() == mTileWidth) {
            // This used to be the max image; we have to recompute
            updateTileSize();
        } else {
            // Check if we have a new maximum
            if (mTileHeight < newImageSize.height())
                mTileHeight = newImageSize.height();
            if (mTileWidth < newImageSize.width())
                mTileWidth = newImageSize.width();
        }
    }
}

void Tileset::updateTileSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    foreach (Tile *tile, mTiles) {
        const QSize size = tile->size();
        if (maxWidth < size.width())
            maxWidth = size.width();
        if (maxHeight < size.height())
            maxHeight = size.height();
    }
    mTileWidth = maxWidth;
    mTileHeight = maxHeight;
}
