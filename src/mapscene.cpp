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

#include "mapscene.h"

#include "brushitem.h"
#include "layertablemodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(0),
    mSelectedObjectGroupItem(0),
    mMovingItem(0),
    mBrush(new BrushItem),
    mGridVisible(true),
    mBrushVisible(false),
    mPainting(false)
{
    setBackgroundBrush(Qt::darkGray);

    mBrush->setZValue(10000);
    mBrush->setVisible(false);
    addItem(mBrush);
}

void MapScene::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);
        mMapDocument->layerModel()->disconnect(this);
    }

    mMapDocument = mapDocument;
    refreshScene();

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(regionChanged(QRegion)),
                this, SLOT(repaintRegion(QRegion)));
        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(currentLayerChanged(int)));

        // TODO: This should really be more optimal (adding/removing as
        // necessary)
        LayerTableModel *layerModel = mMapDocument->layerModel();
        connect(layerModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(refreshScene()));
        connect(layerModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(refreshScene()));
    }

    mBrush->setMapDocument(mapDocument);
    updateBrushVisibility();
}

void MapScene::refreshScene()
{
    mSelectedObjectGroupItem = 0;
    mMovingItem = 0;

    // Clear any existing items, but don't delete the brush
    removeItem(mBrush);
    clear();
    addItem(mBrush);

    if (!mMapDocument) {
        setSceneRect(QRectF());
        return;
    }

    const Map *map = mMapDocument->map();

    mLayerItems.resize(map->layers().size());

    // The +1 is to allow space for the right and bottom grid lines
    setSceneRect(0, 0,
            map->width() * map->tileWidth() + 1,
            map->height() * map->tileHeight() + 1);

    int z = 0;
    int layerIndex = 0;
    foreach (Layer *layer, map->layers()) {
        QGraphicsItem *layerItem = 0;
        if (TileLayer *tl = dynamic_cast<TileLayer*>(layer)) {
            TileLayerItem *item = new TileLayerItem(tl);
            item->setZValue(z++);
            addItem(item);
            layerItem = item;
        } else if (ObjectGroup *og = dynamic_cast<ObjectGroup*>(layer)) {
            ObjectGroupItem *ogItem = new ObjectGroupItem(og);
            ogItem->setZValue(z++);
            foreach (MapObject *object, og->objects())
                new MapObjectItem(object, ogItem);
            addItem(ogItem);
            layerItem = ogItem;
        }
        mLayerItems[layerIndex] = layerItem;
        ++layerIndex;
    }

    TileSelectionItem *selectionItem = new TileSelectionItem(mMapDocument);
    selectionItem->setZValue(10000 - 1);
    addItem(selectionItem);
}

void MapScene::repaintRegion(const QRegion &region)
{
    // TODO: Adjust region to deal with high tiles
    foreach (const QRect &r, region.rects())
        update(mMapDocument->toPixelCoordinates(r));
}

void MapScene::currentTileChanged(Tile *tile)
{
    mBrush->setTile(tile);
}

void MapScene::currentLayerChanged(int index)
{
    updateBrushVisibility();

    ObjectGroupItem *ogItem = 0;

    if (index != -1) {
        Layer *layer = mMapDocument->map()->layers().at(index);
        if (dynamic_cast<ObjectGroup*>(layer))
            ogItem = static_cast<ObjectGroupItem*>(mLayerItems.at(index));
    }

    if (mSelectedObjectGroupItem == ogItem)
        return;

    if (mSelectedObjectGroupItem) {
        // This object group is no longer selected
        foreach (QGraphicsItem *item, mSelectedObjectGroupItem->childItems())
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
    }

    if (ogItem) {
        // This is the newly selected object group
        foreach (QGraphicsItem *item, ogItem->childItems())
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
    }

    mSelectedObjectGroupItem = ogItem;
}

void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

void MapScene::setBrushVisible(bool visible)
{
    if (mBrushVisible == visible)
        return;

    mBrushVisible = visible;
    updateBrushVisibility();
}

void MapScene::updateBrushVisibility()
{
    // Show the tile brush only when a tile layer is selected
    bool showBrush = false;
    if (mBrushVisible && mMapDocument) {
        const int currentLayer = mMapDocument->currentLayer();
        if (currentLayer >= 0) {
            Layer *layer = mMapDocument->map()->layers().at(currentLayer);
            if (dynamic_cast<TileLayer*>(layer))
                showBrush = true;
        }
    }
    mBrush->setVisible(showBrush);
}

void MapScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (!mMapDocument || !mGridVisible)
        return;

    Map *map = mMapDocument->map();

    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const int startX = (int) (rect.x() / tileWidth) * tileWidth;
    const int startY = (int) (rect.y() / tileHeight) * tileHeight;
    const int endX = qMin((int) rect.right(), map->width() * tileWidth + 1);
    const int endY = qMin((int) rect.bottom(),
                          map->height() * tileHeight + 1);

    painter->setPen(Qt::black);
    painter->setOpacity(0.5f);

    for (int x = startX; x < endX; x += tileWidth) {
        painter->drawLine(x, (int) rect.top(), x, endY - 1);
    }

    for (int y = startY; y < endY; y += tileHeight) {
        painter->drawLine((int) rect.left(), y, endX - 1, y);
    }
}

bool MapScene::event(QEvent *event)
{
    // Show and hide the brush cursor as the mouse enters and leaves the scene
    switch (event->type()) {
    case QEvent::Enter:
        setBrushVisible(true);
        break;
    case QEvent::Leave:
        setBrushVisible(false);
        break;
    default:
        break;
    }

    return QGraphicsScene::event(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (!mMapDocument)
        return;

    QGraphicsScene::mouseMoveEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const QPointF pos = mouseEvent->scenePos();
    const int tileX = ((int) pos.x()) / tileWidth;
    const int tileY = ((int) pos.y()) / tileHeight;
    mBrush->setTilePos(tileX, tileY);
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    // Remember the old position if a map object item may be dragged
    if (mSelectedObjectGroupItem && mouseEvent->button() == Qt::LeftButton) {
        QGraphicsItem *item = itemAt(mouseEvent->scenePos());
        if ((mMovingItem = dynamic_cast<MapObjectItem*>(item)))
            mOldPos = mMovingItem->pos();
    }

    QGraphicsScene::mousePressEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mBrush->isVisible()) {
        mBrush->beginPaint();
        mPainting = true;
    }
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    // If the position of the map object item changed, create an undo command
    if (mMovingItem && mouseEvent->button() == Qt::LeftButton) {
        if (mOldPos != mMovingItem->pos()) {
            QUndoCommand *command = new MoveMapObject(mMovingItem, mOldPos);
            mMapDocument->undoStack()->push(command);
        }
        mMovingItem = 0;
    }

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mPainting) {
        mBrush->endPaint();
        mPainting = false;
    }
}
