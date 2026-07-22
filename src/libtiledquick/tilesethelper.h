/*
 * tilesethelper.h
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "tilelayer.h"

#include "mapitem.h"

#include <QSGTexture>

using namespace Tiled;
using namespace TiledQuick;

/**
 * This helper class exists mainly to avoid redoing calculations that only need
 * to be done once per tileset.
 */
class TilesetHelper
{
public:
    /**
     * Enumerates the different parts of a geometry object's texture.
     */
    enum ObjectColorIndex {
        Fill = 0,
        Border = 1,
        Shadow = 2,
        ColorCount
    };

    TilesetHelper(const MapItem *mapItem);

    static TilesetHelper &instance(const MapItem *mapItem);

    static void deleteInstance();

    inline Tileset *tileset() const { return mTileset; }
    inline QSGTexture *texture() const { return mTexture; }

    void setTileset(Tileset *tileset);

    void setTextureCoordinates(float &tx, float &ty, const Cell &cell) const;

private:
    QImage mObjectPalette;
    QQuickWindow *mWindow;
    Tileset *mTileset;
    QSGTexture *mTexture;
    int mMargin;
    int mTileHSpace;
    int mTileVSpace;
    int mTilesPerRow;

    static TilesetHelper *mInstance;
};