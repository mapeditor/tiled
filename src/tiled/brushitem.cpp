/*
 * brushitem.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "brushitem.h"

#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QTimerEvent>
#include <QUndoStack>

#include <cmath>

using namespace Tiled;

BrushItem::BrushItem():
    mMapDocument(nullptr)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}

/**
 * Sets the map document this brush is operating on.
 */
void BrushItem::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    // The tiles in the stamp may no longer be valid
    clear();
}

/**
 * Clears the tile layer, map and region set on this item.
 */
void BrushItem::clear()
{
    mTileLayer.clear();
    mMap.clear();
    mRegion = QRegion();

    if (mAnimationTimer != -1) {
        killTimer(mAnimationTimer);
        mAnimationTimer = -1;
    }
    mTileOffset = QPoint();
    mDashOffset = 0;

    updateBoundingRect();
    update();
}

/**
 * Sets a tile layer representing this brush. When no tile layer is set,
 * the brush only draws the selection color.
 */
void BrushItem::setTileLayer(const SharedTileLayer &tileLayer)
{
    setTileLayer(tileLayer,
                 tileLayer ? tileLayer->modifiedRegion() : QRegion());
}

/**
 * Sets a tile layer as well as the region that should be highlighted along
 * with it. This allows highlighting of areas that are not covered by tiles in
 * the given tile layer.
 */
void BrushItem::setTileLayer(const SharedTileLayer &tileLayer,
                             const QRegion &region)
{
    mTileLayer = tileLayer;
    mRegion = region;

    updateBoundingRect();
    update();
}

void BrushItem::setMap(const SharedMap &map)
{
    setMap(map, map->modifiedTileRegion());
}

void BrushItem::setMap(const SharedMap &map, const QRegion &region)
{
    mMap = map;
    mRegion = region;

    updateBoundingRect();
    update();
}

/**
 * Changes the position of the tile layer, if one is set.
 */
void BrushItem::setTileLayerPosition(QPoint pos)
{
    if (!mTileLayer)
        return;

    const QPoint oldPosition(mTileLayer->x(), mTileLayer->y());
    if (oldPosition == pos)
        return;

    mRegion.translate(pos - oldPosition);
    mTileLayer->setX(pos.x());
    mTileLayer->setY(pos.y());
    updateBoundingRect();
}

/**
 * Sets the region of tiles that this brush item occupies.
 */
void BrushItem::setTileRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    updateBoundingRect();

    if (mAnimatedOutline) {
        if (!region.isEmpty() && mAnimationTimer == -1)
            mAnimationTimer = startTimer(AnimationInterval);
        else if (region.isEmpty() && mAnimationTimer != -1) {
            killTimer(mAnimationTimer);
            mAnimationTimer = -1;
        }
    }
}

/**
 * Sets the layer offset used by the currently active layer.
 */
void BrushItem::setLayerOffset(const QPointF &offset)
{
    setPos(offset);
}

void BrushItem::setAnimatedOutline(bool enabled)
{
    if (mAnimatedOutline == enabled)
        return;

    mAnimatedOutline = enabled;

    if (enabled && !mRegion.isEmpty()) {
        if (mAnimationTimer == -1)
            mAnimationTimer = startTimer(AnimationInterval);
    } else {
        if (mAnimationTimer != -1) {
            killTimer(mAnimationTimer);
            mAnimationTimer = -1;
        }
        mDashOffset = 0;
    }

    update();
}

void BrushItem::setTileOffset(QPoint offset)
{
    if (mTileOffset == offset)
        return;

    mTileOffset = offset;
    updateBoundingRect();
    update();
}

QRectF BrushItem::boundingRect() const
{
    return mBoundingRect;
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    if (!mMapDocument)
        return;

    if (mAnimatedOutline) {
        paintAnimatedOutline(painter, option);
        return;
    }

    QColor insideMapHighlight = QApplication::palette().highlight().color();
    insideMapHighlight.setAlpha(64);
    QColor outsideMapHighlight = QColor(255, 0, 0, 64);

    QRegion insideMapRegion = mRegion;
    QRegion outsideMapRegion;

    const auto currentLayer = mMapDocument->currentLayer();

    if (currentLayer && !currentLayer->isUnlocked()) {
        qSwap(insideMapRegion, outsideMapRegion);
    } else if (!mMapDocument->map()->infinite()) {
        int mapWidth = mMapDocument->map()->width();
        int mapHeight = mMapDocument->map()->height();
        QRegion mapRegion = QRegion(0, 0, mapWidth, mapHeight);

        insideMapRegion = mRegion.intersected(mapRegion);
        outsideMapRegion = mRegion.subtracted(mapRegion);
    }

    const MapRenderer *renderer = mMapDocument->renderer();
    if (mTileLayer) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        renderer->drawTileLayer(painter, mTileLayer.data(), option->exposedRect);
        painter->setOpacity(opacity);
    } else if (mMap) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        LayerIterator it(mMap.data(), Layer::TileLayerType);
        while (auto tileLayer = static_cast<TileLayer*>(it.next()))
            renderer->drawTileLayer(painter, tileLayer, option->exposedRect);
        painter->setOpacity(opacity);
    }

    renderer->drawTileSelection(painter, insideMapRegion,
                                insideMapHighlight,
                                option->exposedRect);
    renderer->drawTileSelection(painter, outsideMapRegion,
                                outsideMapHighlight,
                                option->exposedRect);
}

void BrushItem::paintAnimatedOutline(QPainter *painter,
                                     const QStyleOptionGraphicsItem *option)
{
    MapRenderer *renderer = mMapDocument->renderer();

    if (mTileLayer) {
        painter->save();
        QPointF pixelOffset = renderer->tileToPixelCoords(QPointF(mTileOffset));
        painter->translate(pixelOffset);
        painter->setOpacity(0.8);
        renderer->drawTileLayer(painter, mTileLayer.data(),
                                option->exposedRect.translated(-pixelOffset));
        painter->restore();
    }

    QRegion offsetRegion = mRegion.translated(mTileOffset);
    QPainterPath path = buildOutlinePath(renderer, offsetRegion);

    const qreal dpr = painter->device()->devicePixelRatioF();
    const qreal dashLength = std::ceil(2.0 * dpr);

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::white, 1.5 * dpr, Qt::SolidLine);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);

    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);
    pen.setDashPattern({dashLength, dashLength});
    pen.setDashOffset(mDashOffset);
    painter->setPen(pen);
    painter->drawPath(path);
}

QPainterPath BrushItem::buildOutlinePath(MapRenderer *renderer,
                                         const QRegion &region)
{
    QPainterPath path;
    const Map *map = mMapDocument->map();

    if (map->orientation() == Map::Orthogonal) {
        for (const QRect &r : region)
            path.addRect(QRectF(renderer->boundingRect(r)));

    } else if (auto *hexRenderer = dynamic_cast<HexagonalRenderer *>(renderer)) {
        for (const QRect &r : region)
            for (int y = r.top(); y <= r.bottom(); ++y)
                for (int x = r.left(); x <= r.right(); ++x)
                    path.addPolygon(hexRenderer->tileToScreenPolygon(x, y));

    } else if (auto *isoRenderer = dynamic_cast<IsometricRenderer *>(renderer)) {
        for (const QRect &r : region)
            path.addPolygon(isoRenderer->tileRectToScreenPolygon(r));

    } else {
        // Fallback: per-tile diamond from 4 corners
        for (const QRect &r : region) {
            for (int y = r.top(); y <= r.bottom(); ++y) {
                for (int x = r.left(); x <= r.right(); ++x) {
                    QPolygonF polygon;
                    polygon << renderer->tileToScreenCoords(x, y)
                            << renderer->tileToScreenCoords(x + 1, y)
                            << renderer->tileToScreenCoords(x + 1, y + 1)
                            << renderer->tileToScreenCoords(x, y + 1);
                    path.addPolygon(polygon);
                }
            }
        }
    }

    return path.simplified();
}

void BrushItem::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mAnimationTimer) {
        ++mDashOffset;
        update();
    }
}

void BrushItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    QRegion effectiveRegion = mRegion;
    if (mAnimatedOutline && !mTileOffset.isNull())
        effectiveRegion = mRegion.translated(mTileOffset);

    const QRect bounds = effectiveRegion.boundingRect();
    mBoundingRect = mMapDocument->renderer()->boundingRect(bounds);

    QMargins drawMargins;

    // Adjust for amount of pixels tiles extend at the top and to the right
    if (mTileLayer) {
        drawMargins = mTileLayer->drawMargins();

        QSize tileSize = mMapDocument->map()->tileSize();
        drawMargins.setTop(drawMargins.top() - tileSize.height());
        drawMargins.setRight(drawMargins.right() - tileSize.width());
    } else if (mMap) {
        drawMargins = mMap->drawMargins();
    } else if (!mAnimatedOutline) {
        return;
    }

    // Since we're also drawing a tile selection, we should not apply
    // negative margins
    mBoundingRect.adjust(qMin(0, -drawMargins.left()),
                         qMin(0, -drawMargins.top()),
                         qMax(0, drawMargins.right()),
                         qMax(0, drawMargins.bottom()));

    // Adjust for border drawn at tile selection edges
    const int pad = mAnimatedOutline ? 2 : 1;
    mBoundingRect.adjust(-pad, -pad, pad, pad);
}

#include "moc_brushitem.cpp"
