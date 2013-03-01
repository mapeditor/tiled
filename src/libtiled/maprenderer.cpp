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
#include "tile.h"
#include "tilelayer.h"

#include <QPainter>
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

/**
 * Draws a \a cell with the given \a origin at \a pos, taking into account the
 * flipping and tile offset.
 *
 * It leaves the painter transform set to the transform used to draw the cell.
 */
void MapRenderer::drawCell(QPainter *painter,
                           const Cell &cell,
                           const QPointF &pos,
                           Origin origin,
                           const QTransform &baseTransform)
{
    const QPixmap &img = cell.tile->image();
    const QPoint offset = cell.tile->tileset()->tileOffset();
    const QSize imgSize = img.size();

    qreal m11 = 1;      // Horizontal scaling factor
    qreal m12 = 0;      // Vertical shearing factor
    qreal m21 = 0;      // Horizontal shearing factor
    qreal m22 = 1;      // Vertical scaling factor
    qreal dx = offset.x() + pos.x();
    qreal dy = offset.y() + pos.y() - imgSize.height();

    if (origin == BottomCenter)
        dx += -imgSize.width() / 2;

    if (cell.flippedAntiDiagonally) {
        // Use shearing to swap the X/Y axis
        m11 = 0;
        m12 = 1;
        m21 = 1;
        m22 = 0;

        // Compensate for the swap of image dimensions
        dy += imgSize.height() - imgSize.width();
        if (origin == BottomCenter)
            dx += (imgSize.width() - imgSize.height()) / 2;
    }
    if (cell.flippedHorizontally) {
        m11 = -m11;
        m21 = -m21;
        dx += cell.flippedAntiDiagonally ? imgSize.height() : imgSize.width();
    }
    if (cell.flippedVertically) {
        m12 = -m12;
        m22 = -m22;
        dy += cell.flippedAntiDiagonally ? imgSize.width() : imgSize.height();
    }

    const QTransform transform(m11, m12, m21, m22, dx, dy);
    painter->setTransform(transform * baseTransform);
    painter->drawPixmap(QPointF(), img);
}
