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

#include "tile.h"
#include "tilesetmanager.h"
#include "wangset.h"

#include <QBitmap>

namespace Tiled {

Tileset::Tileset(QString name, int tileWidth, int tileHeight,
                 int tileSpacing, int margin)
    : Object(TilesetType)
    , mName(std::move(name))
    , mTileWidth(tileWidth)
    , mTileHeight(tileHeight)
    , mTileSpacing(tileSpacing)
    , mMargin(margin)
    , mGridSize(tileWidth, tileHeight)
{
    Q_ASSERT(tileSpacing >= 0);
    Q_ASSERT(margin >= 0);

    TilesetManager::instance()->addTileset(this);
}

Tileset::~Tileset()
{
    TilesetManager::instance()->removeTileset(this);
    qDeleteAll(mTiles);
    qDeleteAll(mWangSets);
}

void Tileset::setFormat(const QString &format)
{
    mFormat = format;
}

QString Tileset::format() const
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
 * Returns the location of the tile with the given ID.
 */
int Tileset::findTileLocation(Tile *tile) const
{
    return mTiles.indexOf(tile);
}

/**
 * Returns the tile with the given ID, creating it when it does not exist yet.
 */
Tile *Tileset::findOrCreateTile(int id)
{
    if (Tile *tile = mTilesById.value(id))
        return tile;

    mNextTileId = std::max(mNextTileId, id + 1);

    auto tile = new Tile(id, this);
    mTilesById[id] = tile;
    mTiles.append(tile);

    return tile;
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
    const QUrl oldImageSource = mImageReference.source;

    mImageReference = reference;
    mExpectedColumnCount = columnCountForWidth(mImageReference.size.width());
    mExpectedRowCount = rowCountForHeight(mImageReference.size.height());

    if (mImageReference.source != oldImageSource)
        TilesetManager::instance()->tilesetImageSourceChanged(*this, oldImageSource);
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
    const QUrl oldImageSource = mImageReference.source;

    mImageReference.source = source;

    if (mImageReference.source != oldImageSource)
        TilesetManager::instance()->tilesetImageSourceChanged(*this, oldImageSource);

    if (image.isNull()) {
        mImageReference.status = LoadingError;
        return false;
    }

    mImage = QPixmap::fromImage(image);

    initializeTilesetTiles();

    return true;
}

/**
 * Exists only because the Python plugin interface does not handle QUrl (would
 * be nice to add this). Assumes \a source is a local file when it would
 * otherwise be a relative URL (without scheme).
 *
 * \sa setImageSource
 */
bool Tileset::loadFromImage(const QImage &image, const QString &source)
{
    return loadFromImage(image, Tiled::toUrl(source));
}

/**
 * Convenience override that loads the image via the ImageCache.
 */
bool Tileset::loadFromImage(const QString &fileName)
{
    const QUrl oldImageSource = mImageReference.source;
    mImageReference.source = QUrl::fromLocalFile(fileName);
    if (mImageReference.source != oldImageSource)
        TilesetManager::instance()->tilesetImageSourceChanged(*this, oldImageSource);

    return loadImage();
}

/**
 * Tries to load the image this tileset is referring to, if any.
 *
 * @return <code>true</code> if loading was successful, otherwise
 *         returns <code>false</code>
 */
bool Tileset::loadImage()
{
    if (mImageReference.hasImage()) {
        mImage = mImageReference.create();
        if (mImage.isNull()) {
            mImageReference.status = LoadingError;
            return false;
        }
    }

    return initializeTilesetTiles();
}

bool Tileset::initializeTilesetTiles()
{
    if (mImage.isNull() || mTileWidth <= 0 || mTileHeight <= 0)
        return false;

    if (mImageReference.transparentColor.isValid())
        mImage.setMask(mImage.createMaskFromColor(mImageReference.transparentColor));

    QVector<QRect> tileRects;

    for (int y = mMargin; y <= mImage.height() - mTileHeight; y += mTileHeight + mTileSpacing)
        for (int x = mMargin; x <= mImage.width() - mTileWidth; x += mTileWidth + mTileSpacing)
            tileRects.append(QRect(x, y, mTileWidth, mTileHeight));

    for (int tileNum = 0; tileNum < tileRects.size(); ++tileNum) {
        auto it = mTilesById.find(tileNum);
        if (it != mTilesById.end()) {
            it.value()->setImage(QPixmap());    // make sure it uses the tileset's image
            it.value()->setImageRect(tileRects.at(tileNum));
        } else {
            auto tile = new Tile(tileNum, this);
            tile->setImageRect(tileRects.at(tileNum));
            mTilesById.insert(tileNum, tile);
            mTiles.insert(tileNum, tile);
        }
    }

    QPixmap blank;

    // Blank out any remaining tiles to avoid confusion (todo: could be more clear)
    for (Tile *tile : std::as_const(mTiles)) {
        if (tile->id() >= tileRects.size()) {
            if (blank.isNull()) {
                blank = QPixmap(mTileWidth, mTileHeight);
                blank.fill();
            }
            tile->setImage(blank);
            tile->setImageRect(QRect(0, 0, mTileWidth, mTileHeight));
        }
    }

    mNextTileId = std::max<int>(mNextTileId, tileRects.size());

    mImageReference.size = mImage.size();
    mColumnCount = columnCountForWidth(mImageReference.size.width());
    mImageReference.status = LoadingReady;
    return true;
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
    // The TilesetManager avoids loading the same external tileset twice, so
    // for external tilesets we don't need to look for "similar" tilesets.
    if (isExternal())
        return SharedTileset();

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
    if (mImageReference.source != imageSource) {
        const QUrl oldImageSource = mImageReference.source;
        mImageReference.source = imageSource;
        TilesetManager::instance()->tilesetImageSourceChanged(*this, oldImageSource);
    }
}

/**
 * Exists only because the Python plugin interface does not handle QUrl (would
 * be nice to add this). Assumes \a source is a local file when it is either
 * an absolute file path or would otherwise be a relative URL (without scheme).
 *
 * \sa loadFromImage
 */
void Tileset::setImageSource(const QString &source)
{
    setImageSource(Tiled::toUrl(source));
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

void Tileset::addWangSet(std::unique_ptr<WangSet> wangSet)
{
    Q_ASSERT(wangSet->tileset() == this);
    mWangSets.append(wangSet.release());
}

/**
 * Adds a WangSet.
 */
void Tileset::insertWangSet(int index, std::unique_ptr<WangSet> wangSet)
{
    Q_ASSERT(wangSet->tileset() == this);
    mWangSets.insert(index, wangSet.release());
}

/**
 * Removes the WangSet at a given \a index and returns it to the caller.
 */
std::unique_ptr<WangSet> Tileset::takeWangSetAt(int index)
{
    return std::unique_ptr<WangSet>(mWangSets.takeAt(index));
}

/**
 * Adds a new tile to the end of the tileset.
 */
Tile *Tileset::addTile(const QPixmap &image, const QUrl &source, const QRect &rect)
{
    Tile *newTile = new Tile(takeNextTileId(), this);
    newTile->setImage(image);
    newTile->setImageSource(source);
    newTile->setImageRect(rect.isNull() ? image.rect() : rect);

    mTilesById.insert(newTile->id(), newTile);
    mTiles.append(newTile);
    if (mTileHeight < newTile->height())
        mTileHeight = newTile->height();
    if (mTileWidth < newTile->width())
        mTileWidth = newTile->width();
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
        Q_ASSERT(tile->tileset() == this && !mTilesById.contains(tile->id()));
        mTilesById.insert(tile->id(), tile);
        mTiles.append(tile);
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
        Q_ASSERT(tile->tileset() == this && mTilesById.contains(tile->id()));
        mTilesById.remove(tile->id());
        mTiles.removeOne(tile);
    }

    updateTileSize();
}

/**
 * Removes the given tile from this set and deletes it.
 */
void Tileset::deleteTile(int id)
{
    auto tile = mTilesById.take(id);
    mTiles.removeOne(tile);
    delete tile;
}

/**
 * Moves the \a tiles from their current position the given \a position.
 *
 * Returns the previous locations of the moved tiles.
 */
QList<int> Tileset::relocateTiles(const QList<Tile *> &tiles, int location)
{
    QList<int> prevLocations;
    for (Tile *tile : tiles) {
        int fromIndex = mTiles.indexOf(tile);
        mTiles.move(fromIndex, location);
        if (fromIndex > location)
            ++location; // insert the next tile after the previous one

        prevLocations.append(fromIndex);
    }
    return prevLocations;
}

bool Tileset::anyTileOutOfOrder() const
{
    int tileId = 0;
    for (const Tile *tile : mTiles) {
        if (tile->id() != tileId)
            return true;
        ++tileId;
    }
    return false;
}

void Tileset::resetTileOrder()
{
    mTiles.clear();
    for (Tile *tile : std::as_const(mTilesById))
        mTiles.append(tile);
}

/**
 * Sets the \a image to be used for the given \a tile.
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
    Q_ASSERT(mTilesById.value(tile->id()) == tile);

    const QSize previousTileSize = tile->size();
    tile->setImage(image);
    tile->setImageSource(source);

    maybeUpdateTileSize(previousTileSize, tile->size());
}

void Tileset::setTileImageRect(Tile *tile, const QRect &imageRect)
{
    Q_ASSERT(mTilesById.value(tile->id()) == tile);

    const QSize previousTileSize = tile->size();
    tile->setImageRect(imageRect);

    maybeUpdateTileSize(previousTileSize, tile->size());
}

void Tileset::maybeUpdateTileSize(QSize previousTileSize, QSize newTileSize)
{
    if (previousTileSize == newTileSize)
        return;

    // Update our max. tile size
    if (previousTileSize.height() == mTileHeight ||
            previousTileSize.width() == mTileWidth) {
        // This used to be the max image; we have to recompute
        updateTileSize();
    } else {
        // Check if we have a new maximum
        if (mTileHeight < newTileSize.height())
            mTileHeight = newTileSize.height();
        if (mTileWidth < newTileSize.width())
            mTileWidth = newTileSize.width();
    }
}

void Tileset::setOriginalTileset(const SharedTileset &original)
{
    mOriginalTileset = original;
}

/**
 * When a tileset gets exported, a copy might be made to apply certain export
 * options. In this case, the copy will have a (weak) pointer to the original
 * tileset, to allow issues found during export to refer to this tileset.
 */
SharedTileset Tileset::originalTileset()
{
    SharedTileset original { mOriginalTileset };
    if (!original)
        original = sharedFromThis();
    return original;
}

void Tileset::swap(Tileset &other)
{
    const QString className = this->className();
    setClassName(other.className());
    other.setClassName(className);

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
    std::swap(mObjectAlignment, other.mObjectAlignment);
    std::swap(mOrientation, other.mOrientation);
    std::swap(mTileRenderSize, other.mTileRenderSize);
    std::swap(mFillMode, other.mFillMode);
    std::swap(mGridSize, other.mGridSize);
    std::swap(mColumnCount, other.mColumnCount);
    std::swap(mExpectedColumnCount, other.mExpectedColumnCount);
    std::swap(mExpectedRowCount, other.mExpectedRowCount);
    std::swap(mTilesById, other.mTilesById);
    std::swap(mTiles, other.mTiles);
    std::swap(mNextTileId, other.mNextTileId);
    std::swap(mWangSets, other.mWangSets);
    std::swap(mStatus, other.mStatus);
    std::swap(mBackgroundColor, other.mBackgroundColor);
    std::swap(mFormat, other.mFormat);

    // Don't swap mWeakPointer, since it's a reference to this.

    // Update back references from tiles and Wang sets
    for (auto tile : std::as_const(mTiles))
        tile->mTileset = this;
    for (auto wangSet : std::as_const(mWangSets))
        wangSet->setTileset(this);

    for (auto tile : std::as_const(other.mTiles))
        tile->mTileset = &other;
    for (auto wangSet : std::as_const(other.mWangSets))
        wangSet->setTileset(&other);
}

SharedTileset Tileset::clone() const
{
    SharedTileset c = create(mName, mTileWidth, mTileHeight, mTileSpacing, mMargin);
    c->setClassName(className());
    c->setProperties(properties());

    // mFileName stays empty
    c->mTileOffset = mTileOffset;
    c->mObjectAlignment = mObjectAlignment;
    c->mOrientation = mOrientation;
    c->mTileRenderSize = mTileRenderSize;
    c->mFillMode = mFillMode;
    c->mGridSize = mGridSize;
    c->mColumnCount = mColumnCount;
    c->mNextTileId = mNextTileId;
    c->mStatus = mStatus;
    c->mBackgroundColor = mBackgroundColor;
    c->mFormat = mFormat;
    c->mTransformationFlags = mTransformationFlags;

    for (auto tile : mTiles) {
        const int id = tile->id();
        Tile *clonedTile = tile->clone(c.data());

        c->mTilesById.insert(id, clonedTile);
        c->mTiles.append(clonedTile);
    }

    c->mWangSets.reserve(mWangSets.size());
    for (WangSet *wangSet : mWangSets)
        c->mWangSets.append(wangSet->clone(c.data()));

    // Call setter to please TilesetManager, which starts watching the image of
    // the tileset when it calls TilesetManager::tilesetImageSourceChanged.
    c->setImageReference(mImageReference);
    c->mImage = mImage;

    return c;
}

/**
 * Sets tile size to the maximum size.
 */
void Tileset::updateTileSize()
{
    int maxWidth = 0;
    int maxHeight = 0;
    for (Tile *tile : std::as_const(mTiles)) {
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
        return QStringLiteral("orthogonal");
    case Tileset::Isometric:
        return QStringLiteral("isometric");
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

QString Tileset::tileRenderSizeToString(TileRenderSize tileRenderSize)
{
    switch (tileRenderSize) {
    case TileSize:
        return QStringLiteral("tile");
    case GridSize:
        return QStringLiteral("grid");
    }
    return QString();
}

Tileset::TileRenderSize Tileset::tileRenderSizeFromString(const QString &string)
{
    if (string == QLatin1String("grid"))
        return GridSize;
    return TileSize;
}

QString Tileset::fillModeToString(FillMode fillMode)
{
    switch (fillMode) {
    case Stretch:
        return QStringLiteral("stretch");
    case PreserveAspectFit:
        return QStringLiteral("preserve-aspect-fit");
    }
    return QString();
}

Tileset::FillMode Tileset::fillModeFromString(const QString &string)
{
    if (string == QLatin1String("preserve-aspect-fit"))
        return PreserveAspectFit;
    return Stretch;
}

} // namespace Tiled
