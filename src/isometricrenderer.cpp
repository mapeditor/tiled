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
#include "tile.h"
#include "tilelayer.h"

using namespace Tiled::Internal;

QSize IsometricRenderer::mapSize() const
{
    // Map width and height contribute equally in both directions
    const int side = map()->height() + map()->width();
    return QSize(side * map()->tileWidth() / 2,
                 side * map()->tileHeight() / 2);
}

QRect IsometricRenderer::boundingRect(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const int originX = map()->height() * tileWidth / 2;
    const QPoint pos((rect.x() - (rect.y() + rect.height()))
                     * tileWidth / 2 + originX,
                     (rect.x() + rect.y()) * tileHeight / 2);

    const int side = rect.height() + rect.width();
    const QSize size(side * tileWidth / 2,
                     side * tileHeight / 2);

    return QRect(pos, size);
}

void IsometricRenderer::drawGrid(QPainter *painter, const QRectF &rect) const
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
                                      const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        rect = boundingRect(layer->bounds());

    const int extraHeight = layer->maxTileHeight() - tileHeight;
    rect.adjust(0, 0, 0, extraHeight);

    // Determine the tile and pixel coordinates to start at
    QPoint rowItr = screenToTileCoords(rect.x(), rect.y());
    QPoint startPos = tileToScreenCoords(rowItr);
    startPos.rx() -= tileWidth / 2;
    startPos.ry() += tileHeight;

    // Compensate for the layer position
    rowItr -= QPoint(layer->x(), layer->y());

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = startPos.y() - rect.y() > tileHeight / 2;
    const bool inLeftHalf = rect.x() - startPos.x() < tileWidth / 2;

    if (inUpperHalf) {
        if (inLeftHalf) {
            --rowItr.rx();
            startPos.rx() -= tileWidth / 2;
        } else {
            --rowItr.ry();
            startPos.rx() += tileWidth / 2;
        }
        startPos.ry() -= tileHeight / 2;
    }

    // Determine whether the current row is shifted half a tile to the right
    bool shifted = inUpperHalf ^ inLeftHalf;

    for (int y = startPos.y(); y - tileHeight < rect.bottom();
         y += tileHeight / 2)
    {
        QPoint columnItr = rowItr;

        for (int x = startPos.x(); x < rect.right(); x += tileWidth) {
            if (layer->contains(columnItr)) {
                if (const Tile *tile = layer->tileAt(columnItr)) {
                    const QPixmap &img = tile->image();
                    painter->drawPixmap(x, y - img.height(), img);
                }
            }

            // Advance to the next column
            ++columnItr.rx();
            --columnItr.ry();
        }

        // Advance to the next row
        if (!shifted) {
            ++rowItr.rx();
            startPos.rx() += tileWidth / 2;
            shifted = true;
        } else {
            ++rowItr.ry();
            startPos.rx() -= tileWidth / 2;
            shifted = false;
        }
    }
}

void IsometricRenderer::drawTileSelection(QPainter *painter,
                                          const QRegion &region,
                                          const QColor &color,
                                          const QRectF &exposed) const
{
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    foreach (const QRect &r, region.rects()) {
        QPolygon polygon = tileRectToPolygon(r);
        if (QRectF(polygon.boundingRect()).intersects(exposed))
            painter->drawConvexPolygon(polygon);
    }
}

QPoint IsometricRenderer::screenToTileCoords(int x, int y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const qreal ratio = (qreal) tileWidth / tileHeight;

    x -= map()->height() * tileWidth / 2;
    const int mx = y + (int) (x / ratio);
    const int my = y - (int) (x / ratio);

    return QPoint(((mx < 0) ? mx - tileHeight + 1 : mx) / tileHeight,
                  ((my < 0) ? my - tileHeight + 1 : my) / tileHeight);
}

QPoint IsometricRenderer::tileToScreenCoords(int x, int y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const int originX = map()->height() * tileWidth / 2;

    return QPoint((x - y) * tileWidth / 2 + originX,
                  (x + y) * tileHeight / 2);
}

QPolygon IsometricRenderer::tileRectToPolygon(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const QPoint topRight = tileToScreenCoords(rect.topRight());
    const QPoint bottomRight = tileToScreenCoords(rect.bottomRight());
    const QPoint bottomLeft = tileToScreenCoords(rect.bottomLeft());

    QPolygon polygon(4);
    polygon.setPoint(0, tileToScreenCoords(rect.topLeft()));
    polygon.setPoint(1, topRight.x() + tileWidth / 2,
                     topRight.y() + tileHeight / 2);
    polygon.setPoint(2, bottomRight.x(), bottomRight.y() + tileHeight);
    polygon.setPoint(3, bottomLeft.x() - tileWidth / 2,
                     bottomLeft.y() + tileHeight / 2);
    return polygon;
}
