/*
 * tile.cpp
 * Copyright 2012-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tile.h"

#include "objectgroup.h"
#include "tileset.h"

#include <QBitmap>

using namespace Tiled;

Tile::Tile(int id, Tileset *tileset):
    Object(TileType),
    mId(id),
    mTileset(tileset),
    mImageStatus(LoadingReady),
    mProbability(1.0),
    mCurrentFrameIndex(0),
    mUnusedTime(0)
{}

Tile::Tile(const QPixmap &image, int id, Tileset *tileset):
    Object(TileType),
    mId(id),
    mTileset(tileset),
    mImage(image),
    mImageStatus(image.isNull() ? LoadingError : LoadingReady),
    mProbability(1.0),
    mCurrentFrameIndex(0),
    mUnusedTime(0)
{}

Tile::~Tile()
{
}

/**
 * Returns the tileset that this tile is part of as a shared pointer.
 */
QSharedPointer<Tileset> Tile::sharedTileset() const
{
    return mTileset->sharedFromThis();
}

/**
 * Returns the image of this tile, or the image of its tileset if it doesn't
 * have an individual one.
 */
const QPixmap &Tile::image() const
{
    return mImage.isNull() ? mTileset->image() : mImage;
}

// Using some internal Qt API here, but this is the function that is also used
// by QGraphicsPixmapItem, so my assumption is that it is better suited for
// this task than using QPainterPath::addRegion.
extern QPainterPath qt_regionToPath(const QRegion &region);

const QPainterPath &Tile::imageShape() const
{
    if (!mImageShape.has_value()) {
#if 1
        const QBitmap mask = image().mask();
        if (mask.isNull()) {
            // Can happen when the image has no alpha channel, in which case we
            // want to be able to select the entire image.
            mImageShape = QPainterPath();
            mImageShape->addRect(image().rect());
        } else {
            mImageShape = qt_regionToPath(mask);
        }
#else
        // Public Qt API alternative, which generally produces more
        // heavy-weight paths since it just uses rectangles.
        mImageShape.emplace().addRegion(mImage.mask());
#endif

        if (mImageRect != image().rect()) {
            QPainterPath rect;
            rect.addRect(mImageRect);
            *mImageShape &= rect;
            mImageShape->translate(-mImageRect.topLeft());
        }
    }
    return *mImageShape;
}

/**
 * Sets the image of this tile.
 */
void Tile::setImage(const QPixmap &image)
{
    // Initialize or auto-adjust the image rect
    if (mImageRect.isNull() || mImageRect == mImage.rect())
        mImageRect = image.rect();

    mImage = image;
    mImageStatus = image.isNull() ? LoadingError : LoadingReady;
    mImageShape.reset();
}

/**
 * Returns the tile to render when taking into account tile animations.
 *
 * \warning May return null when the tileset is invalid or the image could
 *          not be loaded.
 */
const Tile *Tile::currentFrameTile() const
{
    if (isAnimated()) {
        const Frame &frame = mFrames.at(mCurrentFrameIndex);
        return mTileset->findTile(frame.tileId);
    }
    return this;
}

void Tile::setImageRect(const QRect &imageRect)
{
    if (mImageRect == imageRect)
        return;

    mImageRect = imageRect;
    mImageShape.reset();
}

/**
 * Returns the drawing offset of the tile (in pixels).
 */
QPoint Tile::offset() const
{
    return mTileset->tileOffset();
}

/**
 * Sets \a objectGroup to be the group of objects associated with this tile.
 * The Tile takes ownership over the ObjectGroup and it can't also be part of
 * a map.
 */
void Tile::setObjectGroup(std::unique_ptr<ObjectGroup> objectGroup)
{
    Q_ASSERT(!objectGroup || !objectGroup->map());

    if (mObjectGroup == objectGroup)
        return;

    mObjectGroup = std::move(objectGroup);
}

/**
 * Swaps the object group of this tile with \a objectGroup. The tile releases
 * ownership over its existing object group and takes ownership over the new
 * one.
 */
void Tile::swapObjectGroup(std::unique_ptr<ObjectGroup> &objectGroup)
{
    std::swap(mObjectGroup, objectGroup);
}

/**
 * Sets the animation frames to be used by this tile. Resets any currently
 * running animation.
 */
void Tile::setFrames(const QVector<Frame> &frames)
{
    resetAnimation();
    mFrames = frames;
}

/**
 * Resets the tile animation. Returns whether this caused the current tileId to
 * change.
 */
bool Tile::resetAnimation()
{
    if (!isAnimated())
        return false;

    Frame previousFrame = mFrames.at(mCurrentFrameIndex);
    Frame currentFrame = mFrames.at(0);

    mCurrentFrameIndex = 0;
    mUnusedTime = 0;

    return previousFrame.tileId != currentFrame.tileId;
}

/**
 * Advances this tile animation by the given amount of milliseconds. Returns
 * whether this caused the current tileId to change.
 */
bool Tile::advanceAnimation(int ms)
{
    if (!isAnimated())
        return false;

    mUnusedTime += ms;

    Frame frame = mFrames.at(mCurrentFrameIndex);
    const int previousTileId = frame.tileId;

    while (frame.duration > 0 && mUnusedTime > frame.duration) {
        mUnusedTime -= frame.duration;
        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrames.size();

        frame = mFrames.at(mCurrentFrameIndex);
    }

    return previousTileId != frame.tileId;
}

/**
 * Returns a duplicate of this tile, to be added to the given \a tileset.
 */
Tile *Tile::clone(Tileset *tileset) const
{
    Tile *c = new Tile(mImage, mId, tileset);
    c->setClassName(className());
    c->setProperties(properties());

    c->mImageSource = mImageSource;
    c->mImageRect = mImageRect;
    c->mImageStatus = mImageStatus;
    c->mProbability = mProbability;

    if (mObjectGroup)
        c->mObjectGroup.reset(mObjectGroup->clone());

    c->mFrames = mFrames;
    c->mCurrentFrameIndex = mCurrentFrameIndex;
    c->mUnusedTime = mUnusedTime;

    return c;
}
