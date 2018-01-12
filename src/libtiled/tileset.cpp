/*
 * tileset.cpp
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

#include "tileset.h"

#include "terrain.h"
#include "tile.h"
#include "tilesetformat.h"
#include "wangset.h"

#include <QBitmap>

using namespace Tiled;

Tileset::Tileset(QString name, int tileWidth, int tileHeight,
                 int tileSpacing, int margin):
    Object(TilesetType),
    mName(std::move(name)),
    mTileWidth(tileWidth),
    mTileHeight(tileHeight),
    mTileSpacing(tileSpacing),
    mMargin(margin),
    mOrientation(Orthogonal),
    mGridSize(tileWidth, tileHeight),
    mColumnCount(0),
    mExpectedColumnCount(0),
    mExpectedRowCount(0),
    mNextTileId(0),
    mMaximumTerrainDistance(0),
    mTerrainDistancesDirty(false),
    mStatus(LoadingReady)
{
    Q_ASSERT(tileSpacing >= 0);
    Q_ASSERT(margin >= 0);
}

Tileset::~Tileset()
{
    qDeleteAll(mTiles);
    qDeleteAll(mTerrainTypes);
    qDeleteAll(mWangSets);
}

void Tileset::setFormat(TilesetFormat *format)
{
    mFormat = format;
}

TilesetFormat *Tileset::format() const
{
    return mFormat;
}

/**
 * Sets the tile size of this tileset. Affects how image is cut in loadImage.
 *
 * @warning Invalid for image collection tilesets!
 */
void Tileset::setTileSize(QSize tileSize)
{
    Q_ASSERT(!tileSize.isEmpty());
    mTileWidth = tileSize.width();
    mTileHeight = tileSize.height();
}

/**
 * Sets the space in pixels between tiles in the tileset. Affects how image is
 * cut in loadImage.
 */
void Tileset::setTileSpacing(int tileSpacing)
{
    Q_ASSERT(tileSpacing >= 0);
    mTileSpacing = tileSpacing;
}

/**
 * Sets the margin used by the tileset image. This is the number of pixels
 * at the top-left border of the image that is skipped when cutting out tiles.
 * Affects how image is cut in loadImage.
 */
void Tileset::setMargin(int margin)
{
    Q_ASSERT(margin >= 0);
    mMargin = margin;
}

/**
 * Returns the tile with the given ID, creating it when it does not exist yet.
 */
Tile *Tileset::findOrCreateTile(int id)
{
    if (Tile *tile = mTiles.value(id))
        return tile;

    mNextTileId = std::max(mNextTileId, id + 1);
    return mTiles[id] = new Tile(id, this);
}

/**
 * Returns the number of tile rows in the tileset image.
 */
int Tileset::rowCount() const
{
    if (isCollection())
        return 1;

    return rowCountForHeight(mImageReference.size.height());
}

/**
 * Sets the transparent color. Pixels with this color will be masked out
 * when loadFromImage() is called.
 */
void Tileset::setTransparentColor(const QColor &c)
{
    mImageReference.transparentColor = c;
}

/**
 * Sets the image reference data for tileset image based tilesets.
 *
 * This function also sets the expected column count, which can be used later
 * for automatic adjustment of tile indexes in case the tileset image width has
 * changed.
 */
void Tileset::setImageReference(const ImageReference &reference)
{
    mImageReference = reference;
    mExpectedColumnCount = columnCountForWidth(mImageReference.size.width());
    mExpectedRowCount = rowCountForHeight(mImageReference.size.height());
}

/**
 * Load this tileset from the given tileset \a image. This will replace
 * existing tile images in this tileset with new ones. If the new image
 * contains more tiles than exist in the tileset new tiles will be
 * appended, if there are fewer tiles the excess images will be blanked.
 *
 * The tile width and height of this tileset must be higher than 0.
 *
 * @param image    the image to load the tiles from
 * @param source   the url to the image, which will be remembered as the
 *                 image source of this tileset.
 * @return <code>true</code> if loading was successful, otherwise
 *         returns <code>false</code>
 */
bool Tileset::loadFromImage(const QImage &image, const QUrl &source)
{
    mImageReference.source = source;

    if (image.isNull()) {
        mImageReference.status = LoadingError;
        return false;
    }

    const QSize tileSize = this->tileSize();
    const int margin = this->margin();
    const int spacing = this->tileSpacing();

    Q_ASSERT(tileSize.width() > 0 && tileSize.height() > 0);

    const int stopWidth = image.width() - tileSize.width();
    const int stopHeight = image.height() - tileSize.height();

    int tileNum = 0;

    for (int y = margin; y <= stopHeight; y += tileSize.height() + spacing) {
        for (int x = margin; x <= stopWidth; x += tileSize.width() + spacing) {
            const QImage tileImage = image.copy(x, y, tileSize.width(), tileSize.height());
            QPixmap tilePixmap = QPixmap::fromImage(tileImage);
            const QColor &transparent = mImageReference.transparentColor;

            if (transparent.isValid()) {
                const QImage mask = tileImage.createMaskFromColor(transparent.rgb());
                tilePixmap.setMask(QBitmap::fromImage(mask));
            }

            auto it = mTiles.find(tileNum);
            if (it != mTiles.end())
                it.value()->setImage(tilePixmap);
            else
                mTiles.insert(tileNum, new Tile(tilePixmap, tileNum, this));

            ++tileNum;
        }
    }

    // Blank out any remaining tiles to avoid confusion (todo: could be more clear)
    for (Tile *tile : mTiles) {
        if (tile->id() >= tileNum) {
            QPixmap tilePixmap = QPixmap(tileSize);
            tilePixmap.fill();
            tile->setImage(tilePixmap);
        }
    }

    mNextTileId = std::max(mNextTileId, tileNum);

    mImageReference.size = image.size();
    mColumnCount = columnCountForWidth(mImageReference.size.width());
    mImageReference.status = LoadingReady;

    return true;
}

/**
 * Exists only because the Python plugin interface does not handle QUrl (would
 * be nice to add this). Assumes \a source is a local file when it would
 * otherwise be a relative URL (without scheme).
 */
bool Tileset::loadFromImage(const QImage &image, const QString &source)
{
    const QUrl url(source);
    return loadFromImage(image, url.isRelative() ? QUrl::fromLocalFile(source) : url);
}

/**
 * Tries to load the image this tileset is referring to.
 *
 * @return <code>true</code> if loading was successful, otherwise
 *         returns <code>false</code>
 */
bool Tileset::loadImage()
{
    return loadFromImage(mImageReference.create(), mImageReference.source);
}

/**
 * Returns whether the tiles in \a candidate use the same images as the ones
 * in \a subject. Note that \a candidate is allowed to have additional tiles
 * as well.
 */
static bool sameTileImages(const Tileset &subject, const Tileset &candidate)
{
    for (const Tile *subjectTile : subject.tiles()) {
        const Tile *replacementTile = candidate.findTile(subjectTile->id());
        if (!replacementTile)
            return false;
        if (subjectTile->imageSource() != replacementTile->imageSource())
            return false;
    }

    return true;
}

/**
 * This checks if there is a similar tileset in the given list.
 * It is needed for replacing this tileset by its similar copy.
 */
SharedTileset Tileset::findSimilarTileset(const QVector<SharedTileset> &tilesets) const
{
    for (const SharedTileset &candidate : tilesets) {
        Q_ASSERT(candidate != this);

        if (candidate->tileCount() != tileCount())
            continue;
        if (candidate->imageSource() != imageSource())
            continue;
        if (candidate->tileSize() != tileSize())
            continue;
        if (candidate->tileSpacing() != tileSpacing())
            continue;
        if (candidate->margin() != margin())
            continue;
        if (candidate->tileOffset() != tileOffset())
            continue;

        // For an image collection tileset, check the image sources
        if (isCollection())
            if (!sameTileImages(*this, *candidate))
                continue;

        return candidate;
    }

    return SharedTileset();
}

/**
 * Changes the source of the tileset image.
 *
 * Only takes affect when loadImage is called.
 */
void Tileset::setImageSource(const QUrl &imageSource)
{
    mImageReference.source = imageSource;
}

/**
 * Returns the column count that this tileset would have if the tileset
 * image would have the given \a width. This takes into account the tile
 * size, margin and spacing.
 */
int Tileset::columnCountForWidth(int width) const
{
    if (mTileWidth <= 0)
        return 0;
    return (width - mMargin + mTileSpacing) / (mTileWidth + mTileSpacing);
}

/**
 * Returns the row count that this tileset would have if the tileset
 * image would have the given \a width. This takes into account the tile
 * size, margin and spacing.
 */
int Tileset::rowCountForHeight(int height) const
{
    if (mTileHeight <= 0)
        return 0;
    return (height - mMargin + mTileSpacing) / (mTileHeight + mTileSpacing);
}

/**
 * Adds a new terrain type.
 *
 * @param name      the name of the terrain
 * @param imageTile the id of the tile that represents the terrain visually
 * @return the created Terrain instance
 */
Terrain *Tileset::addTerrain(const QString &name, int imageTileId)
{
    Terrain *terrain = new Terrain(terrainCount(), this, name,
                                   imageTileId);
    insertTerrain(terrainCount(), terrain);
    return terrain;
}

/**
 * Adds the \a terrain type at the given \a index.
 *
 * The terrain should already have this tileset associated with it.
 */
void Tileset::insertTerrain(int index, Terrain *terrain)
{
    Q_ASSERT(terrain->tileset() == this);

    mTerrainTypes.insert(index, terrain);

    // Reassign terrain IDs
    for (int terrainId = index; terrainId < mTerrainTypes.size(); ++terrainId)
        mTerrainTypes.at(terrainId)->mId = terrainId;

    // Adjust tile terrain references
    for (Tile *tile : mTiles) {
        for (int corner = 0; corner < 4; ++corner) {
            const int terrainId = tile->cornerTerrainId(corner);
            if (terrainId >= index)
                tile->setCornerTerrainId(corner, terrainId + 1);
        }
    }

    mTerrainDistancesDirty = true;
}

/**
 * Removes the terrain type at the given \a index and returns it. The
 * caller becomes responsible for the lifetime of the terrain type.
 *
 * This will cause the terrain ids of subsequent terrains to shift up to
 * fill the space and the terrain information of all tiles in this tileset
 * will be updated accordingly.
 */
Terrain *Tileset::takeTerrainAt(int index)
{
    Terrain *terrain = mTerrainTypes.takeAt(index);

    // Reassign terrain IDs
    for (int terrainId = index; terrainId < mTerrainTypes.size(); ++terrainId)
        mTerrainTypes.at(terrainId)->mId = terrainId;

    // Clear and adjust tile terrain references
    for (Tile *tile : mTiles) {
        for (int corner = 0; corner < 4; ++corner) {
            const int terrainId = tile->cornerTerrainId(corner);
            if (terrainId == index)
                tile->setCornerTerrainId(corner, 0xFF);
            else if (terrainId > index)
                tile->setCornerTerrainId(corner, terrainId - 1);
        }
    }

    mTerrainDistancesDirty = true;

    return terrain;
}

/**
 * Swaps a terrain type at \a index with another index.
 */
void Tileset::swapTerrains(int index, int swapIndex)
{
    mTerrainTypes.swap(index, swapIndex);

    // Reassign terrain IDs
    mTerrainTypes.at(index)->mId = index;
    mTerrainTypes.at(swapIndex)->mId = swapIndex;

    // Clear and adjust tile terrain references
    for (Tile *tile : mTiles) {
        for (int corner = 0; corner < 4; ++corner) {
            const int terrainId = tile->cornerTerrainId(corner);
            if (terrainId == index)
                tile->setCornerTerrainId(corner, swapIndex);
            else if (terrainId == swapIndex)
                tile->setCornerTerrainId(corner, index);
        }
    }

    mTerrainDistancesDirty = true;
}

/**
 * Returns the transition penalty(/distance) between 2 terrains. -1 if no
 * transition is possible.
 */
int Tileset::terrainTransitionPenalty(int terrainType0, int terrainType1) const
{
    if (mTerrainDistancesDirty)
        const_cast<Tileset*>(this)->recalculateTerrainDistances();

    terrainType0 = terrainType0 == 255 ? -1 : terrainType0;
    terrainType1 = terrainType1 == 255 ? -1 : terrainType1;

    // Do some magic, since we don't have a transition array for no-terrain
    if (terrainType0 == -1 && terrainType1 == -1)
        return 0;
    if (terrainType0 == -1)
        return mTerrainTypes.at(terrainType1)->transitionDistance(terrainType0);
    return mTerrainTypes.at(terrainType0)->transitionDistance(terrainType1);
}

int Tileset::maximumTerrainDistance() const
{
    if (mTerrainDistancesDirty)
        const_cast<Tileset*>(this)->recalculateTerrainDistances();

    return mMaximumTerrainDistance;
}

/**
 * Calculates the transition distance matrix for all terrain types.
 */
void Tileset::recalculateTerrainDistances()
{
    // some fancy macros which can search for a value in each byte of a word simultaneously
    #define hasZeroByte(dword) (((dword) - 0x01010101UL) & ~(dword) & 0x80808080UL)
    #define hasByteEqualTo(dword, value) (hasZeroByte((dword) ^ (~0UL/255 * (value))))

    // Terrain distances are the number of transitions required before one terrain may meet another
    // Terrains that have no transition path have a distance of -1
    int maximumDistance = 1;

    for (int i = 0; i < terrainCount(); ++i) {
        Terrain *type = terrain(i);
        QVector<int> distance(terrainCount() + 1, -1);

        // Check all tiles for transitions to other terrain types
        for (const Tile *tile : mTiles) {
            if (!hasByteEqualTo(tile->terrain(), i))
                continue;

            // This tile has transitions, add the transitions as neightbours (distance 1)
            int tl = tile->cornerTerrainId(0);
            int tr = tile->cornerTerrainId(1);
            int bl = tile->cornerTerrainId(2);
            int br = tile->cornerTerrainId(3);

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

                    // We have found a common connection
                    int d = t0->transitionDistance(j);
                    Q_ASSERT(t1->transitionDistance(i) == d);

                    // If the new path is shorter, record the new distance
                    if (d == -1 || d0 + d1 < d) {
                        d = d0 + d1;
                        t0->setTransitionDistance(j, d);
                        t1->setTransitionDistance(i, d);
                        maximumDistance = qMax(maximumDistance, d);

                        // We're making progress, flag for another iteration...
                        bNewConnections = true;
                    }
                }
            }
        }

        // Repeat while we are still making new connections (could take a
        // number of iterations for distant terrain types to connect)
    } while (bNewConnections);

    mMaximumTerrainDistance = maximumDistance;
    mTerrainDistancesDirty = false;
}

void Tileset::addWangSet(WangSet *wangSet)
{
    Q_ASSERT(wangSet->tileset() == this);

    mWangSets.append(wangSet);
}

/**
 * @brief Tileset::insertWangSet Adds a wangSet.
 * @param wangSet A pointer to the wangset to add.
 */
void Tileset::insertWangSet(int index, WangSet *wangSet)
{
    Q_ASSERT(wangSet->tileset() == this);

    mWangSets.insert(index, wangSet);
}

/**
 * @brief Tileset::takeWangSetAt Removes the wangset at a given index
 *                               And returns it to the caller.
 * @param index Index to take at.
 * @return
 */
WangSet *Tileset::takeWangSetAt(int index)
{
    return mWangSets.takeAt(index);
}

/**
 * Adds a new tile to the end of the tileset.
 */
Tile *Tileset::addTile(const QPixmap &image, const QUrl &source)
{
    Tile *newTile = new Tile(takeNextTileId(), this);
    newTile->setImage(image);
    newTile->setImageSource(source);

    mTiles.insert(newTile->id(), newTile);
    if (mTileHeight < image.height())
        mTileHeight = image.height();
    if (mTileWidth < image.width())
        mTileWidth = image.width();
    return newTile;
}

/**
 * Adds the given list of tiles to this tileset.
 *
 * The tiles should already have unique tile IDs associated with them!
 */
void Tileset::addTiles(const QList<Tile *> &tiles)
{
    for (Tile *tile : tiles) {
        Q_ASSERT(!mTiles.contains(tile->id()));
        mTiles.insert(tile->id(), tile);
    }

    updateTileSize();
}

/**
 * Removes the given list of tiles from this tileset.
 *
 * @warning The tiles are not deleted!
 */
void Tileset::removeTiles(const QList<Tile *> &tiles)
{
    for (Tile *tile : tiles) {
        Q_ASSERT(mTiles.contains(tile->id()));
        mTiles.remove(tile->id());
    }

    updateTileSize();
}

/**
 * Removes the given tile from this set and deletes it.
 */
void Tileset::deleteTile(int id)
{
    delete mTiles.take(id);
}

/**
 * Sets the \a image to be used for the tile with the given \a id.
 *
 * This function makes sure the tile width and tile height properties of the
 * tileset reflect the maximum size. It is only expected to be used for
 * image collection tilesets.
 */
void Tileset::setTileImage(Tile *tile,
                           const QPixmap &image,
                           const QUrl &source)
{
    Q_ASSERT(isCollection());
    Q_ASSERT(mTiles.value(tile->id()) == tile);

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

void Tileset::swap(Tileset &other)
{
    const Properties p = properties();
    setProperties(other.properties());
    other.setProperties(p);

    std::swap(mFileName, other.mFileName);
    std::swap(mImageReference, other.mImageReference);
    std::swap(mTileWidth, other.mTileWidth);
    std::swap(mTileHeight, other.mTileHeight);
    std::swap(mTileSpacing, other.mTileSpacing);
    std::swap(mMargin, other.mMargin);
    std::swap(mTileOffset, other.mTileOffset);
    std::swap(mOrientation, other.mOrientation);
    std::swap(mGridSize, other.mGridSize);
    std::swap(mColumnCount, other.mColumnCount);
    std::swap(mExpectedColumnCount, other.mExpectedColumnCount);
    std::swap(mExpectedRowCount, other.mExpectedRowCount);
    std::swap(mTiles, other.mTiles);
    std::swap(mNextTileId, other.mNextTileId);
    std::swap(mTerrainTypes, other.mTerrainTypes);
    std::swap(mWangSets, other.mWangSets);
    std::swap(mTerrainDistancesDirty, other.mTerrainDistancesDirty);
    std::swap(mStatus, other.mStatus);
    std::swap(mBackgroundColor, other.mBackgroundColor);
    std::swap(mFormat, other.mFormat);

    // Don't swap mWeakPointer, since it's a reference to this.

    // Update back references from tiles and terrains
    for (auto tile : mTiles)
        tile->mTileset = this;
    for (auto terrain : mTerrainTypes)
        terrain->mTileset = this;
    for (auto wangSet : mWangSets)
        wangSet->setTileset(this);

    for (auto tile : other.mTiles)
        tile->mTileset = &other;
    for (auto terrain : other.mTerrainTypes)
        terrain->mTileset = &other;
    for (auto wangSet : other.mWangSets)
        wangSet->setTileset(&other);
}

SharedTileset Tileset::clone() const
{
    SharedTileset c = create(mName, mTileWidth, mTileHeight, mTileSpacing, mMargin);
    c->setProperties(properties());

    // mFileName stays empty
    c->mImageReference = mImageReference;
    c->mTileOffset = mTileOffset;
    c->mOrientation = mOrientation;
    c->mGridSize = mGridSize;
    c->mColumnCount = mColumnCount;
    c->mExpectedColumnCount = mExpectedColumnCount;
    c->mExpectedRowCount = mExpectedRowCount;
    c->mNextTileId = mNextTileId;
    c->mTerrainDistancesDirty = mTerrainDistancesDirty;
    c->mStatus = mStatus;
    c->mBackgroundColor = mBackgroundColor;
    c->mFormat = mFormat;

    QMapIterator<int, Tile*> tileIterator(mTiles);
    while (tileIterator.hasNext()) {
        tileIterator.next();

        const int id = tileIterator.key();
        const Tile *tile = tileIterator.value();

        c->mTiles.insert(id, tile->clone(c.data()));
    }

    c->mTerrainTypes.reserve(mTerrainTypes.size());
    for (Terrain *terrain : mTerrainTypes)
        c->mTerrainTypes.append(terrain->clone(c.data()));

    c->mWangSets.reserve(mWangSets.size());
    for (WangSet *wangSet : mWangSets)
        c->mWangSets.append(wangSet->clone(c.data()));

    return c;
}

/**
 * Sets tile size to the maximum size.
 */
void Tileset::updateTileSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    for (Tile *tile : mTiles) {
        const QSize size = tile->size();
        if (maxWidth < size.width())
            maxWidth = size.width();
        if (maxHeight < size.height())
            maxHeight = size.height();
    }
    mTileWidth = maxWidth;
    mTileHeight = maxHeight;
}


QString Tileset::orientationToString(Tileset::Orientation orientation)
{
    switch (orientation) {
    case Tileset::Orthogonal:
        return QLatin1String("orthogonal");
    case Tileset::Isometric:
        return QLatin1String("isometric");
    }
    return QString();
}

Tileset::Orientation Tileset::orientationFromString(const QString &string)
{
    Orientation orientation = Orthogonal;
    if (string == QLatin1String("isometric"))
        orientation = Isometric;
    return orientation;
}
