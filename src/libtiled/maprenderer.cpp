/*
 * maprenderer.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "maprenderer.h"

#include "imagelayer.h"

#include <QVector2D>

using namespace Tiled;

QRectF MapRenderer::boundingRect(const ImageLayer *imageLayer) const
{
    return QRectF(imageLayer->position(),
                  imageLayer->image().size());
}

void MapRenderer::drawImageLayer(QPainter *painter,
                                 const ImageLayer *imageLayer,
                                 const QRectF &exposed)
{
    Q_UNUSED(exposed)

    painter->drawPixmap(imageLayer->position(),
                        imageLayer->image());
}

void MapRenderer::setFlag(RenderFlag flag, bool enabled)
{
    if (enabled)
        mFlags |= flag;
    else
        mFlags &= ~flag;
}

/**
 * Converts a line running from \a start to \a end to a polygon which
 * extends 5 pixels from the line in all directions.
 */
QPolygonF MapRenderer::lineToPolygon(const QPointF &start, const QPointF &end)
{
    QPointF direction = QVector2D(end - start).normalized().toPointF();
    QPointF perpendicular(-direction.y(), direction.x());

    const qreal thickness = 5.0f; // 5 pixels on each side
    direction *= thickness;
    perpendicular *= thickness;

    QPolygonF polygon(4);
    polygon[0] = start + perpendicular - direction;
    polygon[1] = start - perpendicular - direction;
    polygon[2] = end - perpendicular + direction;
    polygon[3] = end + perpendicular + direction;
    return polygon;
}
