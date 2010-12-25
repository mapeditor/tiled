/*
 * tilelayer.cpp
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

#include "tilelayer.h"

#include "map.h"
#include "tile.h"
#include "tileset.h"

using namespace Tiled;

TileLayer::TileLayer(const QString &name, int x, int y, int width, int height):
    Layer(name, x, y, width, height),
    mMaxTileSize(0, 0),
    mTiles(width * height)
{
}

QRegion TileLayer::region() const
{
    QRegion region;

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            if (tileAt(x, y)) {
                const int rangeStart = x;
                for (++x; x <= mWidth; ++x) {
                    if (x == mWidth || !tileAt(x, y)) {
                        const int rangeEnd = x;
                        region += QRect(rangeStart + mX, y + mY,
                                        rangeEnd - rangeStart, 1);
                        break;
                    }
                }
            }
        }
    }

    return region;
}

void TileLayer::setTile(int x, int y, Tile *tile)
{
    if (tile) {
        if (tile->width() > mMaxTileSize.width()) {
            mMaxTileSize.setWidth(tile->width());
            if (mMap)
                mMap->adjustMaxTileSize(mMaxTileSize);
        }
        if (tile->height() > mMaxTileSize.height()) {
            mMaxTileSize.setHeight(tile->height());
            if (mMap)
                mMap->adjustMaxTileSize(mMaxTileSize);
        }
    }

    mTiles[x + y * mWidth] = tile;
}

TileLayer *TileLayer::copy(const QRegion &region) const
{
    const QRegion area = region.intersected(QRect(0, 0, width(), height()));
    const QRect bounds = region.boundingRect();
    const QRect areaBounds = area.boundingRect();
    const int offsetX = qMax(0, areaBounds.x() - bounds.x());
    const int offsetY = qMax(0, areaBounds.y() - bounds.y());

    TileLayer *copied = new TileLayer(QString(),
                                      0, 0,
                                      bounds.width(), bounds.height());

    foreach (const QRect &rect, area.rects())
        for (int x = rect.left(); x <= rect.right(); ++x)
            for (int y = rect.top(); y <= rect.bottom(); ++y)
                copied->setTile(x - areaBounds.x() + offsetX,
                                y - areaBounds.y() + offsetY,
                                tileAt(x, y));

    return copied;
}

void TileLayer::merge(const QPoint &pos, const TileLayer *layer)
{
    // Determine the overlapping area
    QRect area = QRect(pos, QSize(layer->width(), layer->height()));
    area &= QRect(0, 0, width(), height());

    for (int y = area.top(); y <= area.bottom(); ++y)
        for (int x = area.left(); x <= area.right(); ++x)
            if (Tile *tile = layer->tileAt(x - area.left(), y - area.top()))
                setTile(x, y, tile);
}

QSet<Tileset*> TileLayer::usedTilesets() const
{
    QSet<Tileset*> tilesets;

    for (int i = 0, i_end = mTiles.size(); i < i_end; ++i)
        if (const Tile *tile = mTiles.at(i))
            tilesets.insert(tile->tileset());

    return tilesets;
}

bool TileLayer::referencesTileset(Tileset *tileset) const
{
    for (int i = 0, i_end = mTiles.size(); i < i_end; ++i) {
        const Tile *tile = mTiles.at(i);
        if (tile && tile->tileset() == tileset)
            return true;
    }
    return false;
}

QRegion TileLayer::tilesetReferences(Tileset *tileset) const
{
    QRegion region;

    for (int y = 0; y < mHeight; ++y)
        for (int x = 0; x < mWidth; ++x)
            if (const Tile *tile = tileAt(x, y))
                if (tile->tileset() == tileset)
                    region += QRegion(x + mX, y + mY, 1, 1);

    return region;
}

void TileLayer::removeReferencesToTileset(Tileset *tileset)
{
    for (int i = 0, i_end = mTiles.size(); i < i_end; ++i) {
        const Tile *tile = mTiles.at(i);
        if (tile && tile->tileset() == tileset)
            mTiles.replace(i, 0);
    }
}

void TileLayer::replaceReferencesToTileset(Tileset *oldTileset,
                                           Tileset *newTileset)
{
    for (int i = 0, i_end = mTiles.size(); i < i_end; ++i) {
        const Tile *tile = mTiles.at(i);
        if (tile && tile->tileset() == oldTileset)
            mTiles.replace(i, newTileset->tileAt(tile->id()));
    }
}

void TileLayer::resize(const QSize &size, const QPoint &offset)
{
    QVector<Tile*> newTiles(size.width() * size.height());

    // Copy over the preserved part
    const int startX = qMax(0, -offset.x());
    const int startY = qMax(0, -offset.y());
    const int endX = qMin(mWidth, size.width() - offset.x());
    const int endY = qMin(mHeight, size.height() - offset.y());

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const int index = x + offset.x() + (y + offset.y()) * size.width();
            newTiles[index] = tileAt(x, y);
        }
    }

    mTiles = newTiles;
    Layer::resize(size, offset);
}

void TileLayer::offset(const QPoint &offset,
                       const QRect &bounds,
                       bool wrapX, bool wrapY)
{
    QVector<Tile*> newTiles(mWidth * mHeight);

    for (int y = 0; y < mHeight; ++y) {
        for (int x = 0; x < mWidth; ++x) {
            // Skip out of bounds tiles
            if (!bounds.contains(x, y)) {
                newTiles[x + y * mWidth] = tileAt(x, y);
                continue;
            }

            // Get position to pull tile value from
            int oldX = x - offset.x();
            int oldY = y - offset.y();

            // Wrap x value that will be pulled from
            if (wrapX && bounds.width() > 0) {
                while (oldX < bounds.left())
                    oldX += bounds.width();
                while (oldX > bounds.right())
                    oldX -= bounds.width();
            }

            // Wrap y value that will be pulled from
            if (wrapY && bounds.height() > 0) {
                while (oldY < bounds.top())
                    oldY += bounds.height();
                while (oldY > bounds.bottom())
                    oldY -= bounds.height();
            }

            // Set the new tile
            if (contains(oldX, oldY) && bounds.contains(oldX, oldY))
                newTiles[x + y * mWidth] = tileAt(oldX, oldY);
            else
                newTiles[x + y * mWidth] = 0;
        }
    }

    mTiles = newTiles;
}

bool TileLayer::isEmpty() const
{
    for (int y = 0; y < mHeight; ++y)
        for (int x = 0; x < mWidth; ++x)
            if (tileAt(x, y))
                return false;

    return true;
}

/**
 * Returns a duplicate of this TileLayer.
 *
 * \sa Layer::clone()
 */
Layer *TileLayer::clone() const
{
    return initializeClone(new TileLayer(mName, mX, mY, mWidth, mHeight));
}

TileLayer *TileLayer::initializeClone(TileLayer *clone) const
{
    Layer::initializeClone(clone);
    clone->mTiles = mTiles;
    clone->mMaxTileSize = mMaxTileSize;
    return clone;
}
