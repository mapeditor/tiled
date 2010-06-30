/*
 * mapscene.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "mapscene.h"

#include "abstracttool.h"
#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"
#include "toolmanager.h"
#include "tilesetmanager.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(0),
    mSelectedObjectGroupItem(0),
    mNewMapObjectItem(0),
    mSelectedTool(0),
    mActiveTool(0),
    mGridVisible(true),
    mUnderMouse(false),
    mCurrentModifiers(Qt::NoModifier)
{
    setBackgroundBrush(Qt::darkGray);

    ToolManager *toolManager = ToolManager::instance();
    connect(toolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            this, SLOT(setSelectedTool(AbstractTool*)));

    TilesetManager *tilesetManager = TilesetManager::instance();
    connect(tilesetManager, SIGNAL(tilesetChanged(Tileset*)),
            this, SLOT(tilesetChanged(Tileset*)));

    // Install an event filter so that we can get key events on behalf of the
    // active tool without having to have the current focus.
    qApp->installEventFilter(this);
}

MapScene::~MapScene()
{
    qApp->removeEventFilter(this);
}

void MapScene::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);

        disableSelectedTool();
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

        enableSelectedTool();
    }
}

void MapScene::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    disableSelectedTool();
    mSelectedTool = tool;
    enableSelectedTool();
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

    const QSize mapSize = mMapDocument->renderer()->mapSize();
    setSceneRect(0, 0, mapSize.width(), mapSize.height());

    const Map *map = mMapDocument->map();
    mLayerItems.resize(map->layerCount());

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
            MapObjectItem *item = new MapObjectItem(object, mMapDocument,
                                                    ogItem);
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
    const MapRenderer *renderer = mMapDocument->renderer();
    const QSize extra = mMapDocument->map()->extraTileSize();

    foreach (const QRect &r, region.rects())
        update(renderer->boundingRect(r)
               .adjusted(0, -extra.height(), extra.width(), 0));
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

void MapScene::enableSelectedTool()
{
    if (!mSelectedTool || !mMapDocument)
        return;

    mActiveTool = mSelectedTool;
    mActiveTool->enable(this);

    mCurrentModifiers = QApplication::keyboardModifiers();
    if (mCurrentModifiers != Qt::NoModifier)
        mActiveTool->modifiersChanged(mCurrentModifiers);

    if (mUnderMouse) {
        mActiveTool->mouseEntered();
        mActiveTool->mouseMoved(mLastMousePos, Qt::KeyboardModifiers());
    }
}

void MapScene::disableSelectedTool()
{
    if (!mActiveTool)
        return;

    if (mUnderMouse)
        mActiveTool->mouseLeft();
    mActiveTool->disable();
    mActiveTool = 0;
}

void MapScene::currentLayerChanged()
{
    updateInteractionMode();
}

/**
 * Adapts the scene rect and layers to the new map size.
 */
void MapScene::mapChanged()
{
    const QSize mapSize = mMapDocument->renderer()->mapSize();
    setSceneRect(0, 0, mapSize.width(), mapSize.height());

    foreach (QGraphicsItem *item, mLayerItems) {
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();
    }
}

void MapScene::tilesetChanged(Tileset *tileset)
{
    if (!mMapDocument)
        return;

    if (mMapDocument->map()->tilesets().contains(tileset))
        update();
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
 * A layer has changed. This can mean that the layer visibility or opacity has
 * changed.
 */
void MapScene::layerChanged(int index)
{
    const Layer *layer = mMapDocument->map()->layerAt(index);
    QGraphicsItem *layerItem = mLayerItems.at(index);

    if (layer->isVisible() != layerItem->isVisible()) {
        layerItem->setVisible(layer->isVisible());
        updateInteractionMode();
    }
    if (layer->opacity() != layerItem->opacity())
        layerItem->setOpacity(layer->opacity());
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

        MapObjectItem *item = new MapObjectItem(object, mMapDocument, ogItem);

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

    mMapDocument->renderer()->drawGrid(painter, rect);
}

bool MapScene::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        mUnderMouse = true;
        if (mActiveTool)
            mActiveTool->mouseEntered();
        break;
    case QEvent::Leave:
        mUnderMouse = false;
        if (mActiveTool)
            mActiveTool->mouseLeft();
        break;
    default:
        break;
    }

    return QGraphicsScene::event(event);
}

void MapScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    mLastMousePos = mouseEvent->scenePos();

    if (!mMapDocument)
        return;

    QGraphicsScene::mouseMoveEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mNewMapObjectItem) {
        const MapRenderer *renderer = mMapDocument->renderer();

        // Update the size of the new map object
        const QPointF pixelPos = mouseEvent->scenePos();
        const QPointF tileCoords = renderer->pixelToTileCoords(pixelPos);
        const QPointF objectPos = mNewMapObjectItem->mapObject()->position();
        QSizeF newSize(qMax(qreal(0), tileCoords.x() - objectPos.x()),
                       qMax(qreal(0), tileCoords.y() - objectPos.y()));

        if (mouseEvent->modifiers() & Qt::ControlModifier)
            newSize = newSize.toSize();

        mNewMapObjectItem->resize(newSize);
        mouseEvent->accept();
    } else if (mActiveTool) {
        mActiveTool->mouseMoved(mouseEvent->scenePos(),
                                mouseEvent->modifiers());
        mouseEvent->accept();
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
        const MapRenderer *renderer = mMapDocument->renderer();
        const QPointF scenePos = mouseEvent->scenePos();
        QPointF tileCoords = renderer->pixelToTileCoords(scenePos);
        if (mouseEvent->modifiers() & Qt::ControlModifier)
            tileCoords = tileCoords.toPoint();

        startNewMapObject(tileCoords);
        mouseEvent->accept();
    } else if (mActiveTool) {
        mActiveTool->mousePressed(mouseEvent->scenePos(), mouseEvent->button(),
                                  mouseEvent->modifiers());
        mouseEvent->accept();
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
        mActiveTool->mouseReleased(mouseEvent->scenePos(), mouseEvent->button());
        mouseEvent->accept();
    }
}

/**
 * Override to ignore drag enter events.
 */
void MapScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->ignore();
}

void MapScene::startNewMapObject(const QPointF &pos)
{
    Q_ASSERT(!mNewMapObjectItem);

    MapObject *newMapObject = new MapObject;
    newMapObject->setPosition(pos);

    ObjectGroup *objectGroup = mSelectedObjectGroupItem->objectGroup();
    objectGroup->addObject(newMapObject);

    mNewMapObjectItem = new MapObjectItem(newMapObject,
                                          mMapDocument,
                                          mSelectedObjectGroupItem);
    mNewMapObjectItem->setEditable(true);
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

bool MapScene::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers newModifiers = keyEvent->modifiers();

            if (mActiveTool && newModifiers != mCurrentModifiers) {
                mActiveTool->modifiersChanged(newModifiers);
                mCurrentModifiers = newModifiers;
            }
        }
        break;
    default:
        break;
    }

    return false;
}
