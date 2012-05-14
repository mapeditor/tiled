/*
 * mapscene.cpp
 * Copyright 2008-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "preferences.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"
#include "imagelayer.h"
#include "imagelayeritem.h"
#include "toolmanager.h"
#include "tilesetmanager.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QApplication>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(0),
    mSelectedTool(0),
    mActiveTool(0),
    mGridVisible(true),
    mUnderMouse(false),
    mCurrentModifiers(Qt::NoModifier),
    mDarkRectangle(new QGraphicsRectItem),
    mDefaultBackgroundColor(Qt::darkGray)
{
    setBackgroundBrush(mDefaultBackgroundColor);

    TilesetManager *tilesetManager = TilesetManager::instance();
    connect(tilesetManager, SIGNAL(tilesetChanged(Tileset*)),
            this, SLOT(tilesetChanged(Tileset*)));

    Preferences *prefs = Preferences::instance();
    connect(prefs, SIGNAL(objectTypesChanged()), SLOT(syncAllObjectItems()));
    connect(prefs, SIGNAL(highlightCurrentLayerChanged(bool)),
            SLOT(setHighlightCurrentLayer(bool)));
    connect(prefs, SIGNAL(gridColorChanged(QColor)), SLOT(update()));

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(Qt::black);
    mDarkRectangle->setOpacity(darkeningFactor);
    addItem(mDarkRectangle);

    mHighlightCurrentLayer = prefs->highlightCurrentLayer();

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
    if (mMapDocument)
        mMapDocument->disconnect(this);

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
        connect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(currentLayerIndexChanged()));
        connect(mMapDocument, SIGNAL(objectsAdded(QList<MapObject*>)),
                this, SLOT(objectsAdded(QList<MapObject*>)));
        connect(mMapDocument, SIGNAL(objectsRemoved(QList<MapObject*>)),
                this, SLOT(objectsRemoved(QList<MapObject*>)));
        connect(mMapDocument, SIGNAL(objectsChanged(QList<MapObject*>)),
                this, SLOT(objectsChanged(QList<MapObject*>)));
        connect(mMapDocument, SIGNAL(selectedObjectsChanged()),
                this, SLOT(updateSelectedObjectItems()));
    }
}

void MapScene::setSelectedObjectItems(const QSet<MapObjectItem *> &items)
{
    // Inform the map document about the newly selected objects
    QList<MapObject*> selectedObjects;
#if QT_VERSION >= 0x040700
    selectedObjects.reserve(items.size());
#endif
    foreach (const MapObjectItem *item, items)
        selectedObjects.append(item->mapObject());
    mMapDocument->setSelectedObjects(selectedObjects);
}

void MapScene::setSelectedTool(AbstractTool *tool)
{
    mSelectedTool = tool;
}

void MapScene::refreshScene()
{
    mLayerItems.clear();
    mObjectItems.clear();

    removeItem(mDarkRectangle);
    clear();
    addItem(mDarkRectangle);

    if (!mMapDocument) {
        setSceneRect(QRectF());
        return;
    }

    const QSize mapSize = mMapDocument->renderer()->mapSize();
    setSceneRect(0, 0, mapSize.width(), mapSize.height());
    mDarkRectangle->setRect(0, 0, mapSize.width(), mapSize.height());

    const Map *map = mMapDocument->map();
    mLayerItems.resize(map->layerCount());

    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);

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

    updateCurrentLayerHighlight();
}

QGraphicsItem *MapScene::createLayerItem(Layer *layer)
{
    QGraphicsItem *layerItem = 0;

    if (TileLayer *tl = layer->asTileLayer()) {
        layerItem = new TileLayerItem(tl, mMapDocument->renderer());
    } else if (ObjectGroup *og = layer->asObjectGroup()) {
        ObjectGroupItem *ogItem = new ObjectGroupItem(og);
        foreach (MapObject *object, og->objects()) {
            MapObjectItem *item = new MapObjectItem(object, mMapDocument,
                                                    ogItem);
            mObjectItems.insert(object, item);
        }
        layerItem = ogItem;
    } else if (ImageLayer *il = layer->asImageLayer()) {
        layerItem = new ImageLayerItem(il, mMapDocument->renderer());
    }

    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());
    return layerItem;
}

void MapScene::updateCurrentLayerHighlight()
{
    if (!mMapDocument)
        return;

    const int currentLayerIndex = mMapDocument->currentLayerIndex();

    if (!mHighlightCurrentLayer || currentLayerIndex == -1) {
        mDarkRectangle->setVisible(false);

        // Restore opacity for all layers
        for (int i = 0; i < mLayerItems.size(); ++i) {
            const Layer *layer = mMapDocument->map()->layerAt(i);
            mLayerItems.at(i)->setOpacity(layer->opacity());
        }

        return;
    }

    // Darken layers below the current layer
    mDarkRectangle->setZValue(currentLayerIndex - 0.5);
    mDarkRectangle->setVisible(true);

    // Set layers above the current layer to half opacity
    for (int i = 1; i < mLayerItems.size(); ++i) {
        const Layer *layer = mMapDocument->map()->layerAt(i);
        const qreal multiplier = (currentLayerIndex < i) ? opacityFactor : 1;
        mLayerItems.at(i)->setOpacity(layer->opacity() * multiplier);
    }
}

void MapScene::repaintRegion(const QRegion &region)
{
    const MapRenderer *renderer = mMapDocument->renderer();
    const QMargins margins = mMapDocument->map()->drawMargins();

    foreach (const QRect &r, region.rects()) {
        update(renderer->boundingRect(r).adjusted(-margins.left(),
                                                  -margins.top(),
                                                  margins.right(),
                                                  margins.bottom()));
    }
}

void MapScene::enableSelectedTool()
{
    if (!mSelectedTool || !mMapDocument)
        return;

    mActiveTool = mSelectedTool;
    mActiveTool->activate(this);

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
    mActiveTool->deactivate(this);
    mActiveTool = 0;
}

void MapScene::currentLayerIndexChanged()
{
    updateCurrentLayerHighlight();
}

/**
 * Adapts the scene rect and layers to the new map size.
 */
void MapScene::mapChanged()
{
    const QSize mapSize = mMapDocument->renderer()->mapSize();
    setSceneRect(0, 0, mapSize.width(), mapSize.height());
    mDarkRectangle->setRect(0, 0, mapSize.width(), mapSize.height());

    foreach (QGraphicsItem *item, mLayerItems) {
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();
    }

    const Map *map = mMapDocument->map();
    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
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
    delete mLayerItems.at(index);
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

    layerItem->setVisible(layer->isVisible());

    qreal multiplier = 1;
    if (mHighlightCurrentLayer && mMapDocument->currentLayerIndex() < index)
        multiplier = opacityFactor;

    layerItem->setOpacity(layer->opacity() * multiplier);
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

        mSelectedObjectItems.remove(i.value());
        delete i.value();
        mObjectItems.erase(i);
    }
}

/**
 * Updates the map object items related to the given objects.
 */
void MapScene::objectsChanged(const QList<MapObject*> &objects)
{
    foreach (MapObject *object, objects) {
        MapObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        item->syncWithMapObject();
    }
}

void MapScene::updateSelectedObjectItems()
{
    const QList<MapObject *> &objects = mMapDocument->selectedObjects();

    QSet<MapObjectItem*> items;
    foreach (MapObject *object, objects) {
        MapObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        items.insert(item);
    }

    // Update the editable state of the items
    foreach (MapObjectItem *item, mSelectedObjectItems - items)
        item->setEditable(false);
    foreach (MapObjectItem *item, items - mSelectedObjectItems)
        item->setEditable(true);

    mSelectedObjectItems = items;
    emit selectedObjectItemsChanged();
}

void MapScene::syncAllObjectItems()
{
    foreach (MapObjectItem *item, mObjectItems)
        item->syncWithMapObject();
}

void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

void MapScene::setHighlightCurrentLayer(bool highlightCurrentLayer)
{
    if (mHighlightCurrentLayer == highlightCurrentLayer)
        return;

    mHighlightCurrentLayer = highlightCurrentLayer;
    updateCurrentLayerHighlight();
}

void MapScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (!mMapDocument || !mGridVisible)
        return;

    Preferences *prefs = Preferences::instance();
    mMapDocument->renderer()->drawGrid(painter, rect, prefs->gridColor());
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

    if (mActiveTool) {
        mActiveTool->mouseMoved(mouseEvent->scenePos(),
                                mouseEvent->modifiers());
        mouseEvent->accept();
    }
}

void MapScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mActiveTool) {
        mouseEvent->accept();
        mActiveTool->mousePressed(mouseEvent);
    }
}

void MapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    if (mActiveTool) {
        mouseEvent->accept();
        mActiveTool->mouseReleased(mouseEvent);
    }
}

/**
 * Override to ignore drag enter events.
 */
void MapScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->ignore();
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
