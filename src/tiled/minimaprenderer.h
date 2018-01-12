/*
 * minimaprenderer.h
 * Copyright 2017, Yuriy Natarov <natarur@gmail.com>
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QImage>

namespace Tiled {

class Map;
class MapRenderer;

namespace Internal {

class MiniMapRenderer
{
public:
    enum RenderFlag {
        DrawMapObjects          = 0x0001,
        DrawTileLayers          = 0x0002,
        DrawImageLayers         = 0x0004,
        IgnoreInvisibleLayer    = 0x0008,
        DrawGrid                = 0x0010,
        DrawBackground          = 0x0020,
        SmoothPixmapTransform   = 0x0040,
        IncludeOverhangingTiles = 0x0080
    };

    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    MiniMapRenderer(Map *map);
    ~MiniMapRenderer();

    QImage render(QSize size, RenderFlags renderFlags) const;

    void renderToImage(QImage &image, RenderFlags renderFlags) const;

private:
    Map *mMap;
    MapRenderer *mRenderer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Internal::MiniMapRenderer::RenderFlags)

} // namespace Internal
} // namespace Tiled
