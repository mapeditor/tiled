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

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

BrushItem::BrushItem():
    mTileX(0),
    mTileY(0),
    mStampX(0),
    mStampY(0),
    mMapDocument(0),
    mStamp(0),
    mExtend(0),
    mPainting(false),
    mCapturing(false)
{
}

BrushItem::~BrushItem()
{
    delete mStamp;
}

void BrushItem::setMapDocument(MapDocument *mapDocument)
{
    // A different map may have a different tile size
    prepareGeometryChange();

    // The tiles in the stamp may no longer be valid
    setStamp(0);

    mMapDocument = mapDocument;
}

void BrushItem::setStamp(TileLayer *stamp)
{
    if (mStamp == stamp)
        return;

    prepareGeometryChange();
    delete mStamp;
    mStamp = stamp;

    updateExtend();
    updatePosition();
    update();
}

void BrushItem::setTilePos(int x, int y)
{
    if (mTileX == x && mTileY == y)
        return;

    if (mCapturing)
        prepareGeometryChange();

    mTileX = x;
    mTileY = y;

    updatePosition();

    if (mPainting)
        doPaint();
}

void BrushItem::beginPaint()
{
    if (mPainting || mCapturing)
        return;

    mPainting = true;
    doPaint();
}

void BrushItem::endPaint()
{
    mPainting = false;
}

void BrushItem::beginCapture()
{
    if (mPainting || mCapturing)
        return;

    prepareGeometryChange();
    mCaptureStart = QPoint(mTileX, mTileY);
    mCapturing = true;
    updatePosition();
    updateExtend();
    update();
}

void BrushItem::endCapture()
{
    if (!mCapturing)
        return;

    prepareGeometryChange();
    mCapturing = false;

    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea();
    captured.intersect(QRect(tileLayer->x(), tileLayer->y(),
                             tileLayer->width(), tileLayer->height()));

    if (captured.isValid()) {
        captured.translate(-tileLayer->x(), -tileLayer->y());
        setStamp(tileLayer->copy(captured));
    } else {
        updatePosition();
        updateExtend();
        update();
    }
}

void BrushItem::doPaint()
{
    if (!mStamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->bounds().intersects(QRect(mStampX, mStampY,
                                              mStamp->width(),
                                              mStamp->height())))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mMapDocument, tileLayer,
                                               mStampX, mStampY, mStamp);
    mMapDocument->undoStack()->push(paint);
}

void BrushItem::updateExtend()
{
    if (mStamp && mMapDocument && !mCapturing) {
        const Map *map = mMapDocument->map();
        mExtend = qMax(0, mStamp->maxTileHeight() - map->tileHeight());
    } else {
        mExtend = 0;
    }
}

/**
 * Updates the pixel position.
 */
void BrushItem::updatePosition()
{
    if (!mMapDocument)
        return;

    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    if (mCapturing) {
        setPos(qMin(mTileX, mCaptureStart.x()) * tileWidth,
               qMin(mTileY, mCaptureStart.y()) * tileHeight);
    } else if (mStamp) {
        mStampX = mTileX - mStamp->width() / 2;
        mStampY = mTileY - mStamp->height() / 2;
        setPos(mStampX * tileWidth, mStampY * tileHeight);
    } else {
        setPos(mTileX * tileWidth, mTileY * tileHeight);
    }
}

QRectF BrushItem::boundingRect() const
{
    if (!mMapDocument)
        return QRectF();

    int w = 1;
    int h = 1;

    if (mCapturing) {
        const QRect area = capturedArea();
        w = area.width();
        h = area.height();
    } else if (mStamp) {
        w = mStamp->width();
        h = mStamp->height();
    }

    const Map *map = mMapDocument->map();
    return QRectF(0, -mExtend,
                  map->tileWidth() * w,
                  map->tileHeight() * h + mExtend);
}

void BrushItem::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
    Q_UNUSED(widget);

    if (mCapturing) {
        QColor blue(Qt::blue);
        blue.setAlpha(64);
        painter->fillRect(option->exposedRect, blue);
        return;
    }

    if (mStamp) {
        const qreal opacity = painter->opacity();
        painter->setOpacity(0.75);
        mMapDocument->renderer()->drawTileLayer(painter, mStamp);
        painter->setOpacity(opacity);
    }

    QRectF redraw = option->exposedRect;
    if (redraw.top() < 0)
        redraw.setTop(0);

    QColor red(Qt::red);
    red.setAlpha(64);
    painter->fillRect(redraw, red);
}

/**
 * Returns the current tile layer, or 0 if no tile layer is current selected.
 */
TileLayer *BrushItem::currentTileLayer()
{
    const int currentLayerIndex = mMapDocument->currentLayer();
    Layer *currentLayer = mMapDocument->map()->layerAt(currentLayerIndex);
    return dynamic_cast<TileLayer*>(currentLayer);
}

QRect BrushItem::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, QPoint(mTileX, mTileY)).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}
