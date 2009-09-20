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

#include "abstracttool.h"
#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"
#include "toolmanager.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(0),
    mSelectedObjectGroupItem(0),
    mNewMapObjectItem(0),
    mActiveTool(0),
    mGridVisible(true)
{
    setBackgroundBrush(Qt::darkGray);

    ToolManager *toolManager = ToolManager::instance();
    connect(toolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            this, SLOT(setActiveTool(AbstractTool*)));
}

void MapScene::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);

        if (mActiveTool)
            mActiveTool->disable();
    }

    mMapDocument = mapDocument;
    refreshScene();

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(mapChanged()),
                this, SLOT(mapChanged()));
        connect(mMapDocument, SIGNAL(regionChanged(QRegion)),
                this, SLOT(repaintRegion(QRegion)));
        connect(mMapDocument, SIGNAL(layerAdded(int)),
                this, SLOT(layerAdded(int)));
        connect(mMapDocument, SIGNAL(layerRemoved(int)),
                this, SLOT(layerRemoved(int)));
        connect(mMapDocument, SIGNAL(layerChanged(int)),
                this, SLOT(layerChanged(int)));
        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(currentLayerChanged()));
        connect(mMapDocument, SIGNAL(objectsAdded(QList<MapObject*>)),
                this, SLOT(objectsAdded(QList<MapObject*>)));
        connect(mMapDocument, SIGNAL(objectsRemoved(QList<MapObject*>)),
                this, SLOT(objectsRemoved(QList<MapObject*>)));
        connect(mMapDocument, SIGNAL(objectsChanged(QList<MapObject*>)),
                this, SLOT(objectsChanged(QList<MapObject*>)));

        if (mActiveTool)
            mActiveTool->enable(this);
    }
}

void MapScene::setActiveTool(AbstractTool *tool)
{
    if (mActiveTool == tool)
        return;

    if (mMapDocument && mActiveTool)
        mActiveTool->disable();

    mActiveTool = tool;

    if (mMapDocument && mActiveTool)
        mActiveTool->enable(this);
}

void MapScene::refreshScene()
{
    mSelectedObjectGroupItem = 0;
    mLayerItems.clear();
    mObjectItems.clear();

    clear();

    if (!mMapDocument) {
        setSceneRect(QRectF());
        return;
    }

    const Map *map = mMapDocument->map();

    mLayerItems.resize(map->layerCount());

    setSceneRect(0, 0,
            map->width() * map->tileWidth(),
            map->height() * map->tileHeight());

    int layerIndex = 0;
    foreach (Layer *layer, map->layers()) {
        QGraphicsItem *layerItem = createLayerItem(layer);
        layerItem->setZValue(layerIndex);
        addItem(layerItem);
        mLayerItems[layerIndex] = layerItem;
        ++layerIndex;
    }

    TileSelectionItem *selectionItem = new TileSelectionItem(mMapDocument);
    selectionItem->setZValue(10000 - 1);
    addItem(selectionItem);
}

QGraphicsItem *MapScene::createLayerItem(Layer *layer)
{
    QGraphicsItem *layerItem = 0;

    if (TileLayer *tl = dynamic_cast<TileLayer*>(layer)) {
        layerItem = new TileLayerItem(tl, mMapDocument->renderer());
    } else if (ObjectGroup *og = dynamic_cast<ObjectGroup*>(layer)) {
        ObjectGroupItem *ogItem = new ObjectGroupItem(og);
        foreach (MapObject *object, og->objects()) {
            MapObjectItem *item = new MapObjectItem(object, ogItem);
            mObjectItems.insert(object, item);
        }
        layerItem = ogItem;
    }

    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());
    return layerItem;
}

void MapScene::repaintRegion(const QRegion &region)
{
    Map *map = mMapDocument->map();
    const int extra = map->maxTileHeight() - map->tileHeight();

    foreach (const QRect &r, region.rects())
        update(mMapDocument->toPixelCoordinates(r).adjusted(0, -extra, 0, 0));
}

/**
 * Adapts the scene to the currently selected layer. If an object group is
 * selected, it makes sure the objects in the group are movable.
 */
void MapScene::updateInteractionMode()
{
    ObjectGroupItem *ogItem = 0;

    const int index = mMapDocument->currentLayer();
    if (index != -1) {
        Layer *layer = mMapDocument->map()->layerAt(index);
        if (layer->isVisible() && dynamic_cast<ObjectGroup*>(layer))
            ogItem = static_cast<ObjectGroupItem*>(mLayerItems.at(index));
    }

    if (mSelectedObjectGroupItem == ogItem)
        return;

    if (mSelectedObjectGroupItem) {
        // This object group is no longer selected
        foreach (QGraphicsItem *item, mSelectedObjectGroupItem->childItems())
            static_cast<MapObjectItem*>(item)->setEditable(false);
    }

    if (ogItem) {
        // This is the newly selected object group
        foreach (QGraphicsItem *item, ogItem->childItems())
            static_cast<MapObjectItem*>(item)->setEditable(true);
    }

    mSelectedObjectGroupItem = ogItem;
}

void MapScene::currentLayerChanged()
{
    updateInteractionMode();
}

/**
 * Adapts the scene rect to the new map size.
 */
void MapScene::mapChanged()
{
    const Map *map = mMapDocument->map();
    setSceneRect(0, 0,
                 map->width() * map->tileWidth(),
                 map->height() * map->tileHeight());
}

void MapScene::layerAdded(int index)
{
    Layer *layer = mMapDocument->map()->layerAt(index);
    QGraphicsItem *layerItem = createLayerItem(layer);
    addItem(layerItem);
    mLayerItems.insert(index, layerItem);

    int z = 0;
    foreach (QGraphicsItem *item, mLayerItems)
        item->setZValue(z++);
}

void MapScene::layerRemoved(int index)
{
    QGraphicsItem *layerItem = mLayerItems.at(index);
    if (layerItem == mSelectedObjectGroupItem)
        mSelectedObjectGroupItem = 0;

    delete layerItem;
    mLayerItems.remove(index);
}

/**
 * A layer has changed. This can mean that the layer visibility has changed.
 */
void MapScene::layerChanged(int index)
{
    const Layer *layer = mMapDocument->map()->layerAt(index);
    QGraphicsItem *layerItem = mLayerItems.at(index);

    if (layer->isVisible() != layerItem->isVisible()) {
        layerItem->setVisible(layer->isVisible());
        updateInteractionMode();
    }
}

/**
 * Inserts map object items for the given objects.
 */
void MapScene::objectsAdded(const QList<MapObject*> &objects)
{
    foreach (MapObject *object, objects) {
        ObjectGroup *og = object->objectGroup();
        ObjectGroupItem *ogItem = 0;

        // Find the object group item for the map object's object group
        foreach (QGraphicsItem *item, mLayerItems) {
            if (ObjectGroupItem *ogi = dynamic_cast<ObjectGroupItem*>(item)) {
                if (ogi->objectGroup() == og) {
                    ogItem = ogi;
                    break;
                }
            }
        }

        Q_ASSERT(ogItem);

        MapObjectItem *item = new MapObjectItem(object);

        // Set parent item later to prevent snapping to grid when setting pos
        item->setParentItem(ogItem);

        mObjectItems.insert(object, item);
        if (ogItem == mSelectedObjectGroupItem)
            item->setEditable(true);
    }
}

/**
 * Removes the map object items related to the given objects.
 */
void MapScene::objectsRemoved(const QList<MapObject*> &objects)
{
    foreach (MapObject *o, objects) {
        ObjectItems::iterator i = mObjectItems.find(o);
        Q_ASSERT(i != mObjectItems.end());

        delete i.value();
        mObjectItems.erase(i);
    }
}

/**
 * Updates the map object items related to the given objects.
 */
void MapScene::objectsChanged(const QList<MapObject*> &objects)
{
    foreach (MapObject *o, objects) {
        MapObjectItem *item = mObjectItems.value(o);
        Q_ASSERT(item);

        item->syncWithMapObject();
    }
}

void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
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
    const int endX = qMin((int) std::ceil(rect.right()),
                          map->width() * tileWidth + 1);
    const int endY = qMin((int) std::ceil(rect.bottom()),
                          map->height() * tileHeight + 1);

    QColor gridColor(Qt::black);
    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);

    if ((int) rect.top() < endY) {
        gridPen.setDashOffset(rect.top());
        painter->setPen(gridPen);
        for (int x = startX; x < endX; x += tileWidth)
            painter->drawLine(x, (int) rect.top(), x, endY - 1);
    }

    if ((int) rect.left() < endX) {
        gridPen.setDashOffset(rect.left());
        painter->setPen(gridPen);
        for (int y = startY; y < endY; y += tileHeight)
            painter->drawLine((int) rect.left(), y, endX - 1, y);
    }
}

bool MapScene::event(QEvent *event)
{
    // Show and hide the brush cursor as the mouse enters and leaves the scene
    switch (event->type()) {
    case QEvent::Enter:
        if (mActiveTool)
            mActiveTool->enterEvent(event);
        break;
    case QEvent::Leave:
        if (mActiveTool)
            mActiveTool->leaveEvent(event);
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

    if (mNewMapObjectItem) {
        // Update the size of the new map object
        const QPoint pixelPos = mouseEvent->scenePos().toPoint();
        const QPoint origin = mNewMapObjectItem->pos().toPoint();
        QPoint newSize(qMax(0, pixelPos.x() - origin.x()),
                       qMax(0, pixelPos.y() - origin.y()));

        if (mGridVisible)
            newSize = mMapDocument->snapToTileGrid(newSize);

        mNewMapObjectItem->resize(QSize(newSize.x(), newSize.y()));
        mouseEvent->accept();
    } else if (mActiveTool) {
        mActiveTool->mouseMoveEvent(mouseEvent);
    }
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    // First check if we are creating a new map object
    if (mNewMapObjectItem) {
        if (mouseEvent->button() == Qt::RightButton)
            cancelNewMapObject();

        mouseEvent->accept();
        return;
    }

    QGraphicsScene::mousePressEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mouseEvent->button() == Qt::LeftButton
        && mSelectedObjectGroupItem
        && !mNewMapObjectItem)
    {
        startNewMapObject(mouseEvent->scenePos());
        mouseEvent->accept();
    } else if (mActiveTool) {
        mActiveTool->mousePressEvent(mouseEvent);
    }
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mouseEvent->button() == Qt::LeftButton && mNewMapObjectItem) {
        finishNewMapObject();
        mouseEvent->accept();
    } else if (mActiveTool) {
        mActiveTool->mouseReleaseEvent(mouseEvent);
    }
}

void MapScene::startNewMapObject(const QPointF &pos)
{
    Q_ASSERT(!mNewMapObjectItem);

    MapObject *newMapObject = new MapObject;

    ObjectGroup *objectGroup = mSelectedObjectGroupItem->objectGroup();
    objectGroup->addObject(newMapObject);

    mNewMapObjectItem = new MapObjectItem(newMapObject,
                                          mSelectedObjectGroupItem);
    mNewMapObjectItem->setEditable(true);
    mNewMapObjectItem->setPos(pos);
}

MapObject *MapScene::clearNewMapObjectItem()
{
    Q_ASSERT(mNewMapObjectItem);

    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    ObjectGroup *objectGroup = mSelectedObjectGroupItem->objectGroup();
    objectGroup->removeObject(newMapObject);

    delete mNewMapObjectItem;
    mNewMapObjectItem = 0;

    return newMapObject;
}

void MapScene::cancelNewMapObject()
{
    MapObject *newMapObject = clearNewMapObjectItem();
    delete newMapObject;
}

void MapScene::finishNewMapObject()
{
    ObjectGroup *objectGroup = mSelectedObjectGroupItem->objectGroup();
    MapObject *newMapObject = clearNewMapObjectItem();
    mMapDocument->undoStack()->push(new AddMapObject(mMapDocument,
                                                     objectGroup,
                                                     newMapObject));
}
