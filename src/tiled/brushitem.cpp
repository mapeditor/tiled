/*
 * brushitem.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

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
 * Sets a tile layer representing this brush. When no tile layer is set,
 * the brush only draws the selection color.
 */
void BrushItem::setTileLayer(const SharedTileLayer &tileLayer)
{
    mTileLayer = tileLayer;
    mRegion = tileLayer ? tileLayer->region() : QRegion();

    updateBoundingRect();
    update();
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

/**
 * Changes the position of the tile layer, if one is set.
 */
void BrushItem::setTileLayerPosition(const QPoint &pos)
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
}

/**
 * Sets the layer offset used by the currently active layer.
 */
void BrushItem::setLayerOffset(const QPointF &offset)
{
    setPos(offset);
}

QRectF BrushItem::boundingRect() const
{
    return mBoundingRect;
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    QColor insideMapHighlight = QApplication::palette().highlight().color();
    insideMapHighlight.setAlpha(64);
    QColor outsideMapHighlight = QColor(255, 0, 0, 64);

    int mapWidth = mMapDocument->map()->width();
    int mapHeight = mMapDocument->map()->height();
    QRegion mapRegion = QRegion(0, 0, mapWidth, mapHeight);

    if (!mMapDocument->currentLayer()->isUnlocked())
        mapRegion = QRegion();

    QRegion insideMapRegion = mRegion.intersected(mapRegion);
    QRegion outsideMapRegion = mRegion.subtracted(mapRegion);

    if (mMapDocument->map()->infinite()) {
        insideMapRegion = mRegion;
        outsideMapRegion = QRegion();
    }

    const MapRenderer *renderer = mMapDocument->renderer();
    if (mTileLayer) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        renderer->drawTileLayer(painter, mTileLayer.data(), option->exposedRect);
        painter->setOpacity(opacity);
    }

    renderer->drawTileSelection(painter, insideMapRegion,
                                insideMapHighlight,
                                option->exposedRect);
    renderer->drawTileSelection(painter, outsideMapRegion,
                                outsideMapHighlight,
                                option->exposedRect);
}

void BrushItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    const QRect bounds = mRegion.boundingRect();
    mBoundingRect = mMapDocument->renderer()->boundingRect(bounds);

    // Adjust for amount of pixels tiles extend at the top and to the right
    if (mTileLayer) {
        QSize tileSize = mMapDocument->map()->tileSize();

        QMargins drawMargins = mTileLayer->drawMargins();
        drawMargins.setTop(drawMargins.top() - tileSize.height());
        drawMargins.setRight(drawMargins.right() - tileSize.width());

        // Since we're also drawing a tile selection, we should not apply
        // negative margins
        mBoundingRect.adjust(qMin(0, -drawMargins.left()),
                             qMin(0, -drawMargins.top()),
                             qMax(0, drawMargins.right()),
                             qMax(0, drawMargins.bottom()));
    }
}
