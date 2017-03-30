/*
 * minimaprenderer.h
 * Copyright 2017, Yuriy Natarov <natarur@gmail.com>
 * Based on a minimap by Christoph Schnackenberg and Thorbjørn Lindeijer
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

#include "mapdocument.h"

#include <QImage>

namespace Tiled {
namespace Internal {

class MiniMapRenderer
{
public:
    enum MiniMapRenderFlag {
        DrawObjects             = 0x0001,
        DrawTiles               = 0x0002,
        DrawImages              = 0x0004,
        IgnoreInvisibleLayer    = 0x0008,
        DrawGrid                = 0x0010
    };

    Q_DECLARE_FLAGS(MiniMapRenderFlags, MiniMapRenderFlag)

    static MiniMapRenderer* instance();
    void renderMinimapToImage(QImage& image, const MiniMapRenderFlags minimapRenderFlags) const;
    void setMapDocument(MapDocument* map);
private:
    MiniMapRenderer();
    ~MiniMapRenderer();
    MiniMapRenderer(MiniMapRenderer const&) = delete;
    MiniMapRenderer& operator= (MiniMapRenderer const&) = delete;

    MapDocument* mMapDocument;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Internal::MiniMapRenderer::MiniMapRenderFlags)

} // namespace Internal
} // namespace Tiled
