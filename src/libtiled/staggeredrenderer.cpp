/*
 * staggeredrenderer.cpp
 * Copyright 2011-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "staggeredrenderer.h"

#include <QtCore/qmath.h>

using namespace Tiled;

/**
 * Converts screen to tile coordinates. Sub-tile return values are not
 * supported by this renderer.
 *
 * This override exists because the method used by the HexagonalRenderer
 * does not produce nice results for isometric shapes in the tile corners.
 */
QPointF StaggeredRenderer::screenToTileCoords(qreal x, qreal y) const
{
    const RenderParams p(map());

    const int halfTileHeight = p.tileHeight / 2;
    const qreal ratio = (qreal) p.tileHeight / p.tileWidth;

    // Start with the coordinates of a grid-aligned tile
    const int tileX = qFloor(x / p.tileWidth);
    const int tileY = qFloor(y / p.tileHeight) * 2;

    // Relative x and y position on the base square of the grid-aligned tile
    const qreal relX = x - tileX * p.tileWidth;
    const qreal relY = y - (tileY / 2) * p.tileHeight;

    // Check whether the cursor is in any of the corners (neighboring tiles)
    if (halfTileHeight - relX * ratio > relY)
        return topLeft(tileX, tileY);
    if (-halfTileHeight + relX * ratio > relY)
        return topRight(tileX, tileY);
    if (halfTileHeight + relX * ratio < relY)
        return bottomLeft(tileX, tileY);
    if (halfTileHeight * 3 - relX * ratio < relY)
        return bottomRight(tileX, tileY);

    return QPoint(tileX, tileY);
}
