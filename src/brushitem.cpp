/*
 * Tiled Map Editor (Qt)
 * Copyright 2008-2009 Tiled (Qt) developers (see AUTHORS file)
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
    mMapDocument(0),
    mTileLayer(0)
{
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
#endif
}

void BrushItem::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    // The tiles in the stamp may no longer be valid
    setTileLayer(0);
    updateBoundingRect();
}

void BrushItem::setTileLayer(TileLayer *tileLayer)
{
    if (mTileLayer == tileLayer)
        return;

    mTileLayer = tileLayer;
    mRegion = tileLayer ? tileLayer->region() : QRegion();
    updateBoundingRect();
    update();
}

void BrushItem::setTileLayerPosition(const QPoint &pos)
{
    if (!mTileLayer)
        return;

    const QPoint oldPosition(mTileLayer->x(), mTileLayer->y());
    if (oldPosition == pos)
        return;

    mRegion.translate(pos - oldPosition);
    updateBoundingRect();
}

void BrushItem::setTileRegion(const QRegion &region)
{
    if (mRegion == region)
        return;

    mRegion = region;
    updateBoundingRect();
}

QRectF BrushItem::boundingRect() const
{
    return mBoundingRect;
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *)
{
    QColor highlight = QApplication::palette().highlight().color();
    highlight.setAlpha(64);

    const MapRenderer *renderer = mMapDocument->renderer();

    if (mTileLayer) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        renderer->drawTileLayer(painter, mTileLayer, option->exposedRect);
        painter->setOpacity(opacity);

        renderer->drawTileSelection(painter, mRegion, highlight,
                                    option->exposedRect);
    } else {
        renderer->drawTileSelection(painter, mRegion, highlight,
                                    option->exposedRect);
    }
}

void BrushItem::updateBoundingRect()
{
    prepareGeometryChange();

    if (!mMapDocument) {
        mBoundingRect = QRectF();
        return;
    }

    QRect bounds = mRegion.boundingRect();
    if (mTileLayer) {
        bounds.setSize(QSize(mTileLayer->width(), mTileLayer->height()));
        mTileLayer->setX(bounds.x());
        mTileLayer->setY(bounds.y());
    }

    mBoundingRect = mMapDocument->renderer()->boundingRect(bounds);

    // Adjust for amount of pixels tiles extend above the brush
    if (mTileLayer) {
        const int tileHeight = mMapDocument->map()->tileHeight();
        const int extra = qMax(0, mTileLayer->maxTileHeight() - tileHeight);
        mBoundingRect.adjust(0, -extra, 0, 0);
    }
}
