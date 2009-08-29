/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tilelayer.h"

#include "map.h"
#include "tile.h"

using namespace Tiled;

TileLayer::TileLayer(const QString &name, int x, int y, int width, int height):
    Layer(name, x, y, width, height),
    mMaxTileHeight(0),
    mTiles(width * height)
{
}

Tile *TileLayer::tileAt(int x, int y) const
{
    return mTiles.at(x + y * mWidth);
}

void TileLayer::setTile(int x, int y, Tile *tile)
{
    if (tile && tile->height() > mMaxTileHeight) {
        mMaxTileHeight = tile->height();
        if (mMap)
            mMap->adjustMaxTileHeight(mMaxTileHeight);
    }

    mTiles[x + y * mWidth] = tile;
}

TileLayer *TileLayer::copy(const QRect &rect) const
{
    const QRect area = rect.intersected(QRect(0, 0, width(), height()));
    if (area.isEmpty())
        return 0;

    TileLayer *copied = new TileLayer(QString(),
                                      0, 0,
                                      area.width(), area.height());

    for (int x = area.left(); x <= area.right(); ++x)
        for (int y = area.top(); y <= area.bottom(); ++y)
            copied->setTile(x - area.x(), y - area.y(), tileAt(x, y));

    return copied;
}

void TileLayer::resize(const QSize &size, const QPoint &offset)
{
    QVector<Tile*> newTiles(size.width() * size.height());

    // Copy over the preserved part
    const int startX = qMax(0, -offset.x());
    const int startY = qMax(0, -offset.y());
    const int endX = qMin(mWidth, size.width() - offset.x());
    const int endY = qMin(mHeight, size.height() - offset.y());

    for (int x = startX; x < endX; ++x) {
        for (int y = startY; y < endY; ++y) {
            const int index = x + offset.x() + (y + offset.y()) * size.width();
            newTiles[index] = tileAt(x, y);
        }
    }

    mTiles = newTiles;
    Layer::resize(size, offset);
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
    clone->mMaxTileHeight = mMaxTileHeight;
    return clone;
}
