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

#include "stampbrush.h"

#include "brushitem.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"

#include <QGraphicsSceneMouseEvent>

using namespace Tiled;
using namespace Tiled::Internal;

StampBrush::StampBrush(QObject *parent)
    : AbstractTool(tr("Stamp Brush"),
                   QIcon(QLatin1String(":images/22x22/stock-tool-clone.png")),
                   parent)
    , mMapScene(0)
    , mMapDocument(0)
    , mBrushItem(new BrushItem)
    , mStamp(0)
    , mBrushVisible(false)
    , mPainting(false)
    , mCapturing(false)
    , mTileX(0), mTileY(0)
    , mStampX(0), mStampY(0)
{
    mBrushItem->setVisible(false);
    mBrushItem->setZValue(10000);
    mBrushItem->setTileSize(1, 1);
}

StampBrush::~StampBrush()
{
    delete mBrushItem;
    delete mStamp;
}

void StampBrush::enable(MapScene *scene)
{
    mMapScene = scene;
    setMapDocument(scene->mapDocument());

    connect(mMapDocument, SIGNAL(layerChanged(int)),
            this, SLOT(updateBrushVisibility()));
    connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
            this, SLOT(updateBrushVisibility()));

    mMapScene->addItem(mBrushItem);
    updateBrushVisibility();
}

void StampBrush::disable()
{
    // Remove the brush from the scene
    mMapScene->removeItem(mBrushItem);

    // Make sure we no longer refer to the scene
    mMapScene = 0;
}

void StampBrush::enterEvent(QEvent *)
{
    setBrushVisible(true);
}

void StampBrush::leaveEvent(QEvent *)
{
    setBrushVisible(false);
}

void StampBrush::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const QPointF pos = mouseEvent->scenePos();
    const int tileX = ((int) pos.x()) / tileWidth;
    const int tileY = ((int) pos.y()) / tileHeight;

    if (mTileX != tileX || mTileY != tileY) {
        mTileX = tileX;
        mTileY = tileY;
        updatePosition();

        if (mPainting) {
            doPaint();
        } else if (mCapturing) {
            mBrushItem->setTileSize(capturedArea().size());
        }
    }
}

void StampBrush::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mBrushItem->isVisible()) {
        if (mouseEvent->button() == Qt::LeftButton)
            beginPaint();
        else if (mouseEvent->button() == Qt::RightButton)
            beginCapture();
        mouseEvent->accept();
    }
}

void StampBrush::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mPainting && mouseEvent->button() == Qt::LeftButton) {
        endPaint();
        mouseEvent->accept();
    } else if (mCapturing && mouseEvent->button() == Qt::RightButton) {
        endCapture();
        mouseEvent->accept();
    }
}

void StampBrush::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;
    mBrushItem->setMapDocument(mMapDocument);

    // Reset the brush, since it probably became invalid
    mBrushItem->setTileSize(1, 1);
    setStamp(0);
}

void StampBrush::setStamp(TileLayer *stamp)
{
    if (mStamp == stamp)
        return;

    mBrushItem->setTileLayer(stamp);
    delete mStamp;
    mStamp = stamp;

    updatePosition();
}

void StampBrush::beginPaint()
{
    if (mPainting || mCapturing)
        return;

    mPainting = true;
    doPaint();
}

void StampBrush::endPaint()
{
    mPainting = false;
}

void StampBrush::beginCapture()
{
    if (mPainting || mCapturing)
        return;

    mCaptureStart = QPoint(mTileX, mTileY);
    mCapturing = true;

    setStamp(0);
    mBrushItem->setTileSize(1, 1);
}

void StampBrush::endCapture()
{
    if (!mCapturing)
        return;

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
    }
}

QRect StampBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, QPoint(mTileX, mTileY)).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

void StampBrush::doPaint()
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

/**
 * Updates the position of the brush item.
 */
void StampBrush::updatePosition()
{
    QPoint newPos;

    if (mCapturing) {
        newPos = QPoint(qMin(mTileX, mCaptureStart.x()),
                        qMin(mTileY, mCaptureStart.y()));
    } else if (mStamp) {
        mStampX = mTileX - mStamp->width() / 2;
        mStampY = mTileY - mStamp->height() / 2;
        newPos = QPoint(mStampX, mStampY);
    } else {
        newPos = QPoint(mTileX, mTileY);
    }

    mBrushItem->setTilePos(newPos);
}

void StampBrush::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateBrushVisibility();
}

void StampBrush::updateBrushVisibility()
{
    // Show the tile brush only when a visible tile layer is selected
    bool showBrush = false;
    if (mBrushVisible) {
        if (Layer *layer = currentTileLayer()) {
            if (layer->isVisible())
                showBrush = true;
        }
    }
    mBrushItem->setVisible(showBrush);
}

/**
 * Returns the current tile layer, or 0 if no tile layer is current selected.
 */
TileLayer *StampBrush::currentTileLayer() const
{
    if (!mMapScene)
        return 0;

    MapDocument *mapDocument = mMapScene->mapDocument();
    const int currentLayerIndex = mapDocument->currentLayer();
    if (currentLayerIndex < 0)
        return 0;
    Layer *currentLayer = mapDocument->map()->layerAt(currentLayerIndex);
    return dynamic_cast<TileLayer*>(currentLayer);
}
