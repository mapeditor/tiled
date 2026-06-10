/*
 * obliquerenderer.cpp
 * Copyright 2025, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "obliquerenderer.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"

#include <QtCore/qmath.h>

#include <algorithm>

using namespace Tiled;

QRect ObliqueRenderer::boundingRect(const QRect &rect) const
{
    const QPolygonF polygon = tileRectToScreenPolygon(rect);
    return polygon.boundingRect().toAlignedRect();
}

QRectF ObliqueRenderer::boundingRect(const MapObject *object) const
{
    if (object->shape() == MapObject::Text) {
        QRectF bounds { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));
        return bounds;
    }

    if (object->shape() == MapObject::Point) {
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));
        return shape(object).boundingRect()
                            .adjusted(-extraSpace,
                                      -extraSpace,
                                      extraSpace,
                                      extraSpace);
    }

    QRectF bounds = object->bounds();
    bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

    if (!object->cell().isEmpty()) {
        if (const Tile *tile = object->cell().tile()) {
            QPointF tileOffset = tile->offset();
            const QSize tileSize = tile->size();
            if (!tileSize.isNull()) {
                const QSizeF scale {
                    bounds.width() / tileSize.width(),
                    bounds.height() / tileSize.height()
                };
                tileOffset.rx() *= scale.width();
                tileOffset.ry() *= scale.height();
            }
            tileOffset = transform().map(tileOffset);
            bounds.translate(tileOffset);
        }

        QPainterPath path;
        path.addRect(bounds);
        return transform().map(path).boundingRect().adjusted(-1, -1, 1, 1);
    }

    const qreal extraSpace = qMax(objectLineWidth(), qreal(1));
    return shape(object).boundingRect()
                        .adjusted(-extraSpace,
                                  -extraSpace,
                                  extraSpace + 1,
                                  extraSpace + 1);
}

QPainterPath ObliqueRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    QRectF bounds = object->bounds();
    bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

    switch (object->shape()) {
    case MapObject::Rectangle: {
        if (bounds.isNull()) {
            path.addRect(object->x() - 10, object->y() - 10, 20, 20);
        } else {
            if (const Tile *tile = object->cell().tile()) {
                QPointF tileOffset = tile->offset();
                const QSize tileSize = tile->size();
                if (!tileSize.isNull()) {
                    const QSizeF scale {
                        bounds.width() / tileSize.width(),
                        bounds.height() / tileSize.height()
                    };
                    tileOffset.rx() *= scale.width();
                    tileOffset.ry() *= scale.height();
                }
                bounds.translate(tileOffset);
            }

            path.addRect(bounds);
        }
        break;
    }
    case MapObject::Polygon:
    case MapObject::Polyline: {
        const QPointF &pos = object->position();
        QPolygonF polygon = object->polygon().translated(pos);

        if (object->shape() == MapObject::Polygon && !polygon.isEmpty())
            polygon.append(polygon.first());

        path.addPolygon(polygon);
        break;
    }
    case MapObject::Ellipse: {
        if (bounds.isNull())
            path.addEllipse(bounds.topLeft(), 10, 10);
        else
            path.addEllipse(bounds);
        break;
    }
    case MapObject::Capsule: {
        const qreal rad = std::min(std::abs(bounds.height()),
                                   std::abs(bounds.width())) / 2;
        if (rad == 0.)
            path.addEllipse(bounds.topLeft(), 10, 10);
        else
            path.addRoundedRect(bounds, rad, rad, Qt::SizeMode::AbsoluteSize);
        break;
    }
    case MapObject::Point:
        return pointShape(object->position());  // don't transform shape of points
    case MapObject::Text:
        path.addRect(bounds);
        break;
    }

    return transform().map(path);
}

QPainterPath ObliqueRenderer::interactionShape(const MapObject *object) const
{
    QPainterPath path;

    switch (object->shape()) {
    case MapObject::Polyline: {
        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        for (int i = 1; i < polygon.size(); ++i) {
            const QPointF start = pixelToScreenCoords(polygon[i - 1]);
            const QPointF end = pixelToScreenCoords(polygon[i]);
            path.addPolygon(lineToPolygon(start, end));
        }
        path.setFillRule(Qt::WindingFill);
        break;
    }
    case MapObject::Rectangle:
    case MapObject::Polygon:
    case MapObject::Ellipse:
    case MapObject::Capsule:
    case MapObject::Text:
        path = shape(object);
        break;
    case MapObject::Point:
        path = pointInteractionShape(object);
        break;
    }

    return path;
}

void ObliqueRenderer::drawGrid(QPainter *painter, const QRectF &rect,
                               QColor gridColor, QSize gridMajor) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    const QRectF pixelRect = screenToPixelRect(rect);
    int startX = qFloor(pixelRect.left() / tileWidth);
    int startY = qFloor(pixelRect.top() / tileHeight);
    int endX = qCeil(pixelRect.right() / tileWidth);
    int endY = qCeil(pixelRect.bottom() / tileHeight);

    // Return immediately when there is nothing to draw
    if (startX > endX || startY > endY)
        return;

    if (!map()->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(endX, map()->width());
        endY = qMin(endY, map()->height());
    }

    if (startX > endX || startY > endY)
        return;

    QPen gridPen, majorGridPen;
    setupGridPens(painter->device(), gridColor, gridPen, majorGridPen, qMin(tileWidth, tileHeight), gridMajor);

    for (int y = startY; y <= endY; ++y) {
        const QPointF start = tileToScreenCoords(startX, y);
        const QPointF end = tileToScreenCoords(endX, y);

        painter->setPen(gridMajor.height() != 0 && y % gridMajor.height() == 0 ? majorGridPen : gridPen);
        painter->drawLine(start, end);
    }

    for (int x = startX; x <= endX; ++x) {
        const QPointF start = tileToScreenCoords(x, startY);
        const QPointF end = tileToScreenCoords(x, endY);

        painter->setPen(gridMajor.width() != 0 && x % gridMajor.width() == 0 ? majorGridPen : gridPen);
        painter->drawLine(start, end);
    }
}

void ObliqueRenderer::drawTileLayer(const RenderTileCallback &renderTile,
                                    const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    const QRectF pixelRect = screenToPixelRect(exposed);
    int startX = qFloor(pixelRect.left() / tileWidth);
    int startY = qFloor(pixelRect.top() / tileHeight);
    int endX = qCeil(pixelRect.right() / tileWidth);
    int endY = qCeil(pixelRect.bottom() / tileHeight);

    if (!map()->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(endX, map()->width());
        endY = qMin(endY, map()->height());
    }

    int incX = 1;
    int incY = 1;
    switch (map()->renderOrder()) {
    case Map::RightUp:
        std::swap(startY, endY);
        incY = -1;
        break;
    case Map::LeftDown:
        std::swap(startX, endX);
        incX = -1;
        break;
    case Map::LeftUp:
        std::swap(startX, endX);
        std::swap(startY, endY);
        incX = -1;
        incY = -1;
        break;
    case Map::RightDown:
        break;
    }

    if (incX > 0) {
        if (startX > endX)
            return;
    } else if (startX < endX) {
        std::swap(startX, endX);
    }

    if (incY > 0) {
        if (startY > endY)
            return;
    } else if (startY < endY) {
        std::swap(startY, endY);
    }

    endX += incX;
    endY += incY;

    for (int y = startY; y != endY; y += incY) {
        for (int x = startX; x != endX; x += incX)
            renderTile(QPoint(x, y), tileToScreenCoords(x, y + 1));
    }
}

void ObliqueRenderer::drawTileSelection(QPainter *painter,
                                        const QRegion &region,
                                        const QColor &color,
                                        const QRectF &exposed) const
{
    QPainterPath path;

    for (const QRect &r : region) {
        QPolygonF polygon = tileRectToScreenPolygon(r);
        if (QRectF(polygon.boundingRect()).intersects(exposed))
            path.addPolygon(polygon);
    }

    QColor penColor(color);
    penColor.setAlpha(255);

    QPen pen(penColor);
    pen.setCosmetic(true);

    painter->setPen(pen);
    painter->setBrush(color);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPath(path.simplified());
}

void ObliqueRenderer::drawMapObject(QPainter *painter,
                                    const MapObject *object,
                                    const MapObjectColors &colors) const
{
    painter->save();

    QPen pen(Qt::black);
    pen.setCosmetic(true);

    const Cell &cell = object->cell();

    if (!cell.isEmpty()) {
        QRectF bounds { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        CellRenderer(painter, this, object->objectGroup()->effectiveTintColor())
                .render(cell, bounds.topLeft(), bounds.size());

        if (testFlag(ShowTileObjectOutlines)) {
            if (const Tile *tile = object->cell().tile()) {
                QPointF tileOffset = tile->offset();
                const QSize tileSize = tile->size();
                if (!tileSize.isNull()) {
                    const QSizeF scale {
                        bounds.width() / tileSize.width(),
                        bounds.height() / tileSize.height()
                    };
                    tileOffset.rx() *= scale.width();
                    tileOffset.ry() *= scale.height();
                }
                tileOffset = transform().map(tileOffset);
                bounds.translate(tileOffset);
            }

            pen.setStyle(Qt::SolidLine);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);
            painter->drawRect(bounds);
            pen.setStyle(Qt::DotLine);
            pen.setColor(colors.main);
            painter->setPen(pen);
            painter->drawRect(bounds);
        }
    } else if (object->shape() == MapObject::Text) {
        QRectF bounds { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        const auto &textData = object->textData();
        painter->setFont(textData.font);
        painter->setPen(textData.color);
        painter->drawText(bounds, textData.text, textData.textOption());
    } else if (object->shape() == MapObject::Point) {
        painter->translate(pixelToScreenCoords(object->position()));
        painter->setRenderHint(QPainter::Antialiasing);
        drawPointObject(painter, colors.main);
    } else {
        const qreal lineWidth = objectLineWidth();
        const qreal scale = painterScale();
        const QPointF shadowOffset(0, (lineWidth == 0 ? 1 : lineWidth) / scale);

        QBrush brush = colors.fill.isValid() ? QBrush(colors.fill) : QBrush(Qt::NoBrush);

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(lineWidth);

        QPen colorPen(pen);
        colorPen.setColor(colors.main);

        painter->setRenderHint(QPainter::Antialiasing);

        QRectF bounds = object->bounds();
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        switch (object->shape()) {
        case MapObject::Rectangle:
        case MapObject::Ellipse:
        case MapObject::Capsule: {
            QPainterPath path;
            if (object->shape() == MapObject::Rectangle) {
                if (bounds.isNull())
                    bounds = QRectF(QPointF(-10, -10), QSizeF(20, 20));
                path.addRect(bounds);
            } else if (object->shape() == MapObject::Ellipse) {
                if (bounds.isNull())
                    bounds = QRectF(QPointF(-10, -10), QSizeF(20, 20));
                path.addEllipse(bounds);
            } else {
                if (bounds.isNull())
                    bounds = QRectF(QPointF(-10, -10), QSizeF(20, 20));
                const qreal rad = std::min(std::abs(bounds.height()),
                                           std::abs(bounds.width())) / 2;
                path.addRoundedRect(bounds, rad, rad);
            }

            path = transform().map(path);

            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path.translated(shadowOffset));

            painter->setPen(colorPen);
            painter->setBrush(brush);
            painter->drawPath(path);
            break;
        }
        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            const QPointF pointPos = screenPolygon.isEmpty() ? QPointF()
                                                             : screenPolygon.first();

            QPen thickPen(pen);
            QPen thickColorPen(colorPen);
            thickPen.setWidthF(thickPen.widthF() * 4);
            thickColorPen.setWidthF(thickColorPen.widthF() * 4);

            painter->setPen(pen);
            if (object->shape() == MapObject::Polygon)
                painter->drawPolygon(screenPolygon.translated(shadowOffset));
            else
                painter->drawPolyline(screenPolygon.translated(shadowOffset));
            painter->setPen(thickPen);
            painter->drawPoint(pointPos + shadowOffset);

            painter->setPen(colorPen);
            painter->setBrush(brush);

            if (object->shape() == MapObject::Polygon)
                painter->drawPolygon(screenPolygon);
            else
                painter->drawPolyline(screenPolygon);
            painter->setPen(thickColorPen);
            painter->drawPoint(pointPos);
            break;
        }
        case MapObject::Text:
        case MapObject::Point:
            break;
        }
    }

    painter->restore();
}


QPointF ObliqueRenderer::screenToTileCoords(qreal x, qreal y) const
{
    const QPointF pixel = screenToPixelCoords(x, y);
    return pixelToTileCoords(pixel);
}

QPointF ObliqueRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    return pixelToScreenCoords(tileToPixelCoords(x, y));
}

QPointF ObliqueRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    bool ok = false;
    const QTransform inv = transform().inverted(&ok);
    if (!ok)
        return QPointF(x, y);
    return inv.map(QPointF(x, y));
}

QPointF ObliqueRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    return transform().map(QPointF(x, y));
}

QTransform ObliqueRenderer::transform() const
{
    const qreal tileWidth = map()->tileWidth();
    const qreal tileHeight = map()->tileHeight();
    if (tileWidth == 0 || tileHeight == 0)
        return QTransform();

    const qreal shearX = map()->skewX() / tileHeight;
    const qreal shearY = map()->skewY() / tileWidth;
    QTransform transform;
    transform.shear(shearX, shearY);
    return transform;
}

QRectF ObliqueRenderer::screenToPixelRect(const QRectF &rect) const
{
    bool ok = false;
    const QTransform inv = transform().inverted(&ok);
    if (!ok)
        return rect;

    QPolygonF polygon;
    polygon << inv.map(rect.topLeft())
            << inv.map(rect.topRight())
            << inv.map(rect.bottomRight())
            << inv.map(rect.bottomLeft());
    return polygon.boundingRect();
}

QPolygonF ObliqueRenderer::tileRectToScreenPolygon(const QRect &rect) const
{
    // Since tileToScreenCoords returns the top-left of the tile, we need to
    // extend the bottom-right of the rect.
    const auto r = rect.adjusted(0, 0, 1, 1);

    QPolygonF polygon;
    polygon << tileToScreenCoords(r.topLeft())
            << tileToScreenCoords(r.topRight())
            << tileToScreenCoords(r.bottomRight())
            << tileToScreenCoords(r.bottomLeft());
    return polygon;
}
