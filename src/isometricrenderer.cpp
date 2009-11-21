/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "isometricrenderer.h"

#include "map.h"

using namespace Tiled::Internal;

QSize IsometricRenderer::mapSize() const
{
    // Map width and height contribute equally in both directions
    const int side = map()->height() + map()->width();
    return QSize(side * map()->tileWidth() / 2,
                 side * map()->tileHeight() / 2);
}

QRect IsometricRenderer::layerBoundingRect(const Layer *layer) const
{
    // TODO: Implement calculating the bounding rect of a layer
    return QRect();
}

void IsometricRenderer::drawGrid(QPainter *painter, const QRectF &rect)
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect r = rect.toAlignedRect();
    r.adjust(-tileWidth / 2, -tileHeight / 2,
             tileWidth / 2, tileHeight / 2);

    const int startX = qMax(0, screenToTileCoords(r.topLeft()).x());
    const int startY = qMax(0, screenToTileCoords(r.topRight()).y());
    const int endX = qMin(map()->width(),
                          screenToTileCoords(r.bottomRight()).x());
    const int endY = qMin(map()->height(),
                          screenToTileCoords(r.bottomLeft()).y());

    QColor gridColor(Qt::black);
    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);
    painter->setPen(gridPen);

    for (int y = startY; y <= endY; ++y) {
        const QPoint start = tileToScreenCoords(startX, y);
        const QPoint end = tileToScreenCoords(endX, y);
        painter->drawLine(start, end);
    }
    for (int x = startX; x <= endX; ++x) {
        const QPoint start = tileToScreenCoords(x, startY);
        const QPoint end = tileToScreenCoords(x, endY);
        painter->drawLine(start, end);
    }
}

void IsometricRenderer::drawTileLayer(QPainter *painter,
                                      const TileLayer *layer,
                                      const QRectF &exposed)
{
    // TODO: Implement tile layer rendering
}

QPoint IsometricRenderer::screenToTileCoords(int x, int y)
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const qreal ratio = (qreal) tileWidth / tileHeight;

    x -= map()->height() * tileWidth / 2;
    const int mx = y + (int) (x / ratio);
    const int my = y - (int) (x / ratio);

    return QPoint(((mx < 0) ? mx - tileHeight : mx) / tileHeight,
                  ((my < 0) ? my - tileHeight : my) / tileHeight);
}

QPoint IsometricRenderer::tileToScreenCoords(int x, int y)
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const int originX = map()->height() * tileWidth / 2;

    return QPoint(((x - y) * tileWidth / 2) + originX,
                  ((x + y) * tileHeight / 2));
}
