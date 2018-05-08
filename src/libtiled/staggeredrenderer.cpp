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
 * Converts screen to tile coordinates.
 *
 * This override exists because the method used by the HexagonalRenderer
 * does not produce nice results for isometric shapes in the tile corners.
 */
QPointF StaggeredRenderer::screenToTileCoords(qreal x, qreal y) const
{
    const RenderParams p(map());

    qreal alignedX = x, alignedY = y;
    if (p.staggerX)
        alignedX -= p.staggerEven ? p.sideOffsetX : 0;
    else
        alignedY -= p.staggerEven ? p.sideOffsetY : 0;

    // Start with the coordinates of a grid-aligned tile
    QPoint referencePoint = QPoint(qFloor(alignedX / p.tileWidth),
                                   qFloor(alignedY / p.tileHeight));

    // Relative x and y position on the base square of the grid-aligned tile
    const QPointF rel(alignedX - referencePoint.x() * p.tileWidth,
                      alignedY - referencePoint.y() * p.tileHeight);

    // Adjust the reference point to the correct tile coordinates
    int &staggerAxisIndex = p.staggerX ? referencePoint.rx() : referencePoint.ry();
    staggerAxisIndex *= 2;
    if (p.staggerEven)
        ++staggerAxisIndex;

    const qreal y_pos = rel.x() * ((qreal) p.tileHeight / p.tileWidth);

    // Check whether the cursor is in any of the corners (neighboring tiles)
    if (p.sideOffsetY - y_pos > rel.y())
        referencePoint = topLeft(referencePoint.x(), referencePoint.y());
    if (-p.sideOffsetY + y_pos > rel.y())
        referencePoint = topRight(referencePoint.x(), referencePoint.y());
    if (p.sideOffsetY + y_pos < rel.y())
        referencePoint = bottomLeft(referencePoint.x(), referencePoint.y());
    if (p.sideOffsetY * 3 - y_pos < rel.y())
        referencePoint = bottomRight(referencePoint.x(), referencePoint.y());

    QPointF newRel = tileToScreenCoords(referencePoint.x(), referencePoint.y());
    newRel = QPointF(x - newRel.x(), y - newRel.y());
    QPointF tileLocal = newRel - QPointF(p.tileWidth / 2, 0);

    tileLocal.ry() *= (qreal) p.tileWidth / p.tileHeight;
    QTransform t;
    t.rotate(-45);
    tileLocal = t.map(tileLocal);
    tileLocal /= p.tileWidth/sqrt(2);

    return tileLocal + referencePoint;
}
