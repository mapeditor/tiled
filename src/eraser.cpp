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

#include "eraser.h"

#include "brushitem.h"
#include "erasetile.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "tilelayer.h"

#include <QGraphicsSceneMouseEvent>

using namespace Tiled;
using namespace Tiled::Internal;

Eraser::Eraser(QObject *parent)
    : AbstractTool(QObject::tr("Eraser"),
                   QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")),
                   parent)
    , mMapScene(0)
    , mBrushItem(new BrushItem)
    , mTileX(0), mTileY(0)
    , mErasing(false)
    , mBrushVisible(false)
{
    mBrushItem->setVisible(false);
    mBrushItem->setZValue(10000);
    mBrushItem->setTileSize(1, 1);
}

void Eraser::enable(MapScene *scene)
{
    mMapScene = scene;

    MapDocument *mapDocument = mMapScene->mapDocument();
    connect(mapDocument, SIGNAL(layerChanged(int)),
            this, SLOT(updateBrushVisibility()));
    connect(mapDocument, SIGNAL(currentLayerChanged(int)),
            this, SLOT(updateBrushVisibility()));

    mBrushItem->setMapDocument(mapDocument);
    mMapScene->addItem(mBrushItem);
    updateBrushVisibility();
}

void Eraser::disable()
{
    mMapScene->removeItem(mBrushItem);
    mBrushItem->setMapDocument(0);
    mMapScene = 0;
}

void Eraser::enterEvent(QEvent *)
{
    setBrushVisible(true);
}

void Eraser::leaveEvent(QEvent *)
{
    setBrushVisible(false);
}

void Eraser::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    const Map *map = mMapScene->mapDocument()->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const QPointF pos = mouseEvent->scenePos();
    const int tileX = ((int) pos.x()) / tileWidth;
    const int tileY = ((int) pos.y()) / tileHeight;

    if (mTileX != tileX || mTileY != tileY) {
        mTileX = tileX;
        mTileY = tileY;
        mBrushItem->setTilePos(mTileX, mTileY);

        if (mErasing)
            doErase();
    }
}

void Eraser::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton) {
        mErasing = true;
        doErase();
    }
}

void Eraser::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
        mErasing = false;
}

void Eraser::doErase()
{
    MapDocument *mapDocument = mMapScene->mapDocument();
    TileLayer *tileLayer = currentTileLayer();

    if (!tileLayer->bounds().contains(mTileX, mTileY))
        return;

    QUndoCommand *erase = new EraseTile(mapDocument, tileLayer,
                                        mTileX, mTileY);
    mapDocument->undoStack()->push(erase);
}

/* TODO: The methods below are copies of those in StampBrush. They should be in
 *       a baseclass.
 */

void Eraser::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateBrushVisibility();
}

void Eraser::updateBrushVisibility()
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
TileLayer *Eraser::currentTileLayer() const
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
