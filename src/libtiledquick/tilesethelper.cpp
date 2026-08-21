/*
 * tilesethelper.cpp
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "tilesethelper.h"

#include "tile.h"
#include "tileset.h"

/**
 * Returns the texture of a given tileset, or 0 if the image has not been
 * loaded yet.
 */
static inline QSGTexture *tilesetTexture(Tileset *tileset,
                                         QQuickWindow *window)
{
    static QHash<Tileset *, QSGTexture *> cache;

    QSGTexture *texture = cache.value(tileset);
    if (!texture) {
        const QString imagePath(Tiled::urlToLocalFileOrQrc(tileset->imageSource()));
        texture = window->createTextureFromImage(QImage(imagePath));
        cache.insert(tileset, texture);
    }
    return texture;
}

TilesetHelper *TilesetHelper::mInstance;

TilesetHelper::TilesetHelper(const MapItem *mapItem)
    : mObjectPalette(3, 1, QImage::Format_RGBA8888)
    , mWindow(mapItem->window())
    , mTileset(nullptr)
    , mTexture(nullptr)
    , mMargin(0)
    , mTileHSpace(0)
    , mTileVSpace(0)
    , mTilesPerRow(0)
{
    mObjectPalette.setPixelColor(Fill, 0, QColor(191, 191, 191, 63));
    mObjectPalette.setPixelColor(Border, 0, QColor(191, 191, 191));
    mObjectPalette.setPixelColor(Shadow, 0, Qt::black);
    mObjectTexture = mWindow->createTextureFromImage(mObjectPalette);
}

TilesetHelper &TilesetHelper::instance(const MapItem *mapItem)
{
    if (!mInstance)
        mInstance = new TilesetHelper(mapItem);
    return *mInstance;
};

void TilesetHelper::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
};

void TilesetHelper::setTileset(Tileset *tileset)
{
    mTileset = tileset;
    if (!tileset) {
        mTexture = nullptr;
        return;
    }

    mTexture = tilesetTexture(tileset, mWindow);
    if (!mTexture)
        return;

    const int tileSpacing = tileset->tileSpacing();
    mMargin = tileset->margin();
    mTileHSpace = tileset->tileWidth() + tileSpacing;
    mTileVSpace = tileset->tileHeight() + tileSpacing;

    const QSize tilesetSize = mTexture->textureSize();
    const int availableWidth = tilesetSize.width() + tileSpacing - mMargin;
    mTilesPerRow = qMax(availableWidth / mTileHSpace, 1);
}

void TilesetHelper::setTextureCoordinates(float &tx, float &ty, const Cell &cell) const
{
    if (cell.isEmpty())
        return;

    const int tileId = cell.tileId();
    const int column = tileId % mTilesPerRow;
    const int row = tileId / mTilesPerRow;

    tx = column * mTileHSpace + mMargin;
    ty = row * mTileVSpace + mMargin;
}