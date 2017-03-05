/*
 * mapscene.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "containerhelpers.h"
#include "grouplayer.h"
#include "grouplayeritem.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "objectselectionitem.h"
#include "preferences.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"
#include "imagelayer.h"
#include "imagelayeritem.h"
#include "stylehelper.h"
#include "toolmanager.h"
#include "tilesetmanager.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QPalette>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(nullptr),
    mSelectedTool(nullptr),
    mActiveTool(nullptr),
    mUnderMouse(false),
    mCurrentModifiers(Qt::NoModifier),
    mDarkRectangle(new QGraphicsRectItem),
    mObjectSelectionItem(nullptr)
{
    updateDefaultBackgroundColor();

    connect(StyleHelper::instance(), &StyleHelper::styleApplied,
            this, &MapScene::updateDefaultBackgroundColor);

    TilesetManager *tilesetManager = TilesetManager::instance();
    connect(tilesetManager, &TilesetManager::tilesetImagesChanged,
            this, &MapScene::repaintTileset);
    connect(tilesetManager, &TilesetManager::repaintTileset,
            this, &MapScene::repaintTileset);

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::showGridChanged, this, &MapScene::setGridVisible);
    connect(prefs, &Preferences::showTileObjectOutlinesChanged, this, &MapScene::setShowTileObjectOutlines);
    connect(prefs, &Preferences::objectTypesChanged, this, &MapScene::syncAllObjectItems);
    connect(prefs, &Preferences::highlightCurrentLayerChanged, this, &MapScene::setHighlightCurrentLayer);
    connect(prefs, SIGNAL(gridColorChanged(QColor)), this, SLOT(update()));
    connect(prefs, &Preferences::objectLineWidthChanged, this, &MapScene::setObjectLineWidth);

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(Qt::black);
    mDarkRectangle->setOpacity(darkeningFactor);
    addItem(mDarkRectangle);

    mGridVisible = prefs->showGrid();
    mObjectLineWidth = prefs->objectLineWidth();
    mShowTileObjectOutlines = prefs->showTileObjectOutlines();
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
    if (mMapDocument) {
        mMapDocument->disconnect(this);

        if (!mSelectedObjectItems.isEmpty()) {
            mSelectedObjectItems.clear();
            emit selectedObjectItemsChanged();
        }
    }

    mMapDocument = mapDocument;

    if (mMapDocument) {
        MapRenderer *renderer = mMapDocument->renderer();
        renderer->setObjectLineWidth(mObjectLineWidth);
        renderer->setFlag(ShowTileObjectOutlines, mShowTileObjectOutlines);

        connect(mMapDocument, &MapDocument::mapChanged,
                this, &MapScene::mapChanged);
        connect(mMapDocument, &MapDocument::regionChanged,
                this, &MapScene::repaintRegion);
        connect(mMapDocument, &MapDocument::tileLayerDrawMarginsChanged,
                this, &MapScene::tileLayerDrawMarginsChanged);
        connect(mMapDocument, &MapDocument::layerAdded,
                this, &MapScene::layerAdded);
        connect(mMapDocument, &MapDocument::layerRemoved,
                this, &MapScene::layerRemoved);
        connect(mMapDocument, &MapDocument::layerChanged,
                this, &MapScene::layerChanged);
        connect(mMapDocument, &MapDocument::objectGroupChanged,
                this, &MapScene::objectGroupChanged);
        connect(mMapDocument, &MapDocument::imageLayerChanged,
                this, &MapScene::imageLayerChanged);
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &MapScene::currentLayerChanged);
        connect(mMapDocument, &MapDocument::tilesetTileOffsetChanged,
                this, &MapScene::adaptToTilesetTileSizeChanges);
        connect(mMapDocument, &MapDocument::tileImageSourceChanged,
                this, &MapScene::adaptToTileSizeChanges);
        connect(mMapDocument, &MapDocument::tilesetReplaced,
                this, &MapScene::tilesetReplaced);
        connect(mMapDocument, &MapDocument::objectsInserted,
                this, &MapScene::objectsInserted);
        connect(mMapDocument, &MapDocument::objectsRemoved,
                this, &MapScene::objectsRemoved);
        connect(mMapDocument, &MapDocument::objectsChanged,
                this, &MapScene::objectsChanged);
        connect(mMapDocument, &MapDocument::objectsIndexChanged,
                this, &MapScene::objectsIndexChanged);
        connect(mMapDocument, &MapDocument::selectedObjectsChanged,
                this, &MapScene::updateSelectedObjectItems);
    }

    refreshScene();
}

void MapScene::setSelectedObjectItems(const QSet<MapObjectItem *> &items)
{
    // Inform the map document about the newly selected objects
    QList<MapObject*> selectedObjects;
    selectedObjects.reserve(items.size());

    for (const MapObjectItem *item : items)
        selectedObjects.append(item->mapObject());

    mMapDocument->setSelectedObjects(selectedObjects);

    // If all selected objects are in a single object group, make it active
    ObjectGroup *singleObjectGroup = nullptr;

    for (MapObject *object : selectedObjects) {
        ObjectGroup *currentObjectGroup = object->objectGroup();

        if (!singleObjectGroup) {
            singleObjectGroup = currentObjectGroup;
        } else if (singleObjectGroup != currentObjectGroup) {
            singleObjectGroup = nullptr;
            break;
        }
    }

    if (singleObjectGroup)
        mMapDocument->setCurrentLayer(singleObjectGroup);
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

    updateSceneRect();

    const Map *map = mMapDocument->map();

    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);

    createLayerItems(map->layers());

    TileSelectionItem *tileSelectionItem = new TileSelectionItem(mMapDocument);
    tileSelectionItem->setZValue(10000 - 2);
    addItem(tileSelectionItem);

    mObjectSelectionItem = new ObjectSelectionItem(mMapDocument);
    mObjectSelectionItem->setZValue(10000 - 1);
    addItem(mObjectSelectionItem);

    updateCurrentLayerHighlight();
}

void MapScene::createLayerItems(const QList<Layer *> &layers)
{
    int layerIndex = 0;

    for (Layer *layer : layers) {
        LayerItem *layerItem = createLayerItem(layer);
        layerItem->setZValue(layerIndex);
        ++layerIndex;
    }
}

LayerItem *MapScene::createLayerItem(Layer *layer)
{
    LayerItem *layerItem = nullptr;

    switch (layer->layerType()) {
    case Layer::TileLayerType:
        layerItem = new TileLayerItem(static_cast<TileLayer*>(layer), mMapDocument);
        break;

    case Layer::ObjectGroupType: {
        auto og = static_cast<ObjectGroup*>(layer);
        const ObjectGroup::DrawOrder drawOrder = og->drawOrder();
        ObjectGroupItem *ogItem = new ObjectGroupItem(og);
        int objectIndex = 0;
        for (MapObject *object : og->objects()) {
            MapObjectItem *item = new MapObjectItem(object, mMapDocument,
                                                    ogItem);
            if (drawOrder == ObjectGroup::TopDownOrder)
                item->setZValue(item->y());
            else
                item->setZValue(objectIndex);

            mObjectItems.insert(object, item);
            ++objectIndex;
        }
        layerItem = ogItem;
        break;
    }

    case Layer::ImageLayerType:
        layerItem = new ImageLayerItem(static_cast<ImageLayer*>(layer), mMapDocument);
        break;

    case Layer::GroupLayerType:
        layerItem = new GroupLayerItem(static_cast<GroupLayer*>(layer));
        break;
    }

    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    if (layer->parentLayer())
        layerItem->setParentItem(mLayerItems.value(layer->parentLayer()));
    else
        addItem(layerItem);

    mLayerItems.insert(layer, layerItem);

    if (GroupLayer *groupLayer = layer->asGroupLayer())
        createLayerItems(groupLayer->layers());

    return layerItem;
}

void MapScene::updateDefaultBackgroundColor()
{
    mDefaultBackgroundColor = QGuiApplication::palette().dark().color();

    if (!mMapDocument || !mMapDocument->map()->backgroundColor().isValid())
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::updateSceneRect()
{
    const QSize mapSize = mMapDocument->renderer()->mapSize();
    QRectF sceneRect(0, 0, mapSize.width(), mapSize.height());

    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    sceneRect.adjust(-margins.left(),
                     -margins.top(),
                     margins.right(),
                     margins.bottom());

    setSceneRect(sceneRect);
    mDarkRectangle->setRect(sceneRect);
}

void MapScene::updateCurrentLayerHighlight()
{
    if (!mMapDocument)
        return;

    const auto currentLayer = mMapDocument->currentLayer();

    if (!mHighlightCurrentLayer || !currentLayer) {
        if (mDarkRectangle->isVisible()) {
            mDarkRectangle->setVisible(false);

            // Restore opacity for all layers
            const auto layerItems = mLayerItems;
            for (auto layerItem : layerItems)
                layerItem->setOpacity(layerItem->layer()->opacity());
        }

        return;
    }

    // Darken layers below the current layer
    const int siblingIndex = currentLayer->siblingIndex();
    const auto parentLayer = currentLayer->parentLayer();
    const auto parentItem = mLayerItems.value(parentLayer);

    mDarkRectangle->setParentItem(parentItem);
    mDarkRectangle->setZValue(siblingIndex - 0.5);
    mDarkRectangle->setVisible(true);

    // Set layers above the current layer to reduced opacity
    LayerIterator iterator(mMapDocument->map());
    qreal multiplier = 1;

    while (Layer *layer = iterator.next()) {
        GroupLayer *groupLayer = layer->asGroupLayer();
        if (!groupLayer)
            mLayerItems.value(layer)->setOpacity(layer->opacity() * multiplier);

        if (layer == currentLayer)
            multiplier = opacityFactor;
    }
}

void MapScene::repaintRegion(const QRegion &region, Layer *layer)
{
    const MapRenderer *renderer = mMapDocument->renderer();
    const QMargins margins = mMapDocument->map()->drawMargins();

    for (const QRect &r : region.rects()) {
        QRectF boundingRect = renderer->boundingRect(r);

        boundingRect.adjust(-margins.left(),
                            -margins.top(),
                            margins.right(),
                            margins.bottom());

        boundingRect.translate(layer->totalOffset());

        update(boundingRect);
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
    mActiveTool = nullptr;
}

void MapScene::currentLayerChanged()
{
    updateCurrentLayerHighlight();

    // New layer may have a different offset, affecting the grid
    if (mGridVisible)
        update();
}

/**
 * Adapts the scene, layers and objects to new map size, orientation or
 * background color.
 */
void MapScene::mapChanged()
{
    updateSceneRect();

    for (QGraphicsItem *item : mLayerItems) {
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();
    }

    for (MapObjectItem *item : mObjectItems)
        item->syncWithMapObject();

    const Map *map = mMapDocument->map();
    if (map->backgroundColor().isValid())
        setBackgroundBrush(map->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
}

void MapScene::repaintTileset(Tileset *tileset)
{
    if (!mMapDocument)
        return;

    if (contains(mMapDocument->map()->tilesets(), tileset))
        update();
}

void MapScene::tileLayerDrawMarginsChanged(TileLayer *tileLayer)
{
    TileLayerItem *item = static_cast<TileLayerItem*>(mLayerItems.value(tileLayer));
    item->syncWithTileLayer();
}

void MapScene::layerAdded(Layer *layer)
{
    createLayerItem(layer);

    int z = 0;
    for (auto sibling : layer->siblings())
        mLayerItems.value(sibling)->setZValue(z++);
}

void MapScene::layerRemoved(Layer *layer)
{
    delete mLayerItems.take(layer);
}

// Returns whether layerB is drawn above layerA
static bool isAbove(Layer *layerA, Layer *layerB)
{
    int depthA = layerA->depth();
    int depthB = layerB->depth();

    // Make sure to start comparing at a common depth
    while (depthA > 0 && depthA > depthB) {
        layerA = layerA->parentLayer();
        --depthA;
    }
    while (depthB > 0 && depthB > depthA) {
        layerB = layerB->parentLayer();
        --depthB;
    }

    // One of the layers is a child of the other
    if (layerA == layerB)
        return false;

    // Move upwards until the layers have the same parent
    while (true) {
        GroupLayer *parentA = layerA->parentLayer();
        GroupLayer *parentB = layerB->parentLayer();

        if (parentA == parentB) {
            const auto &layers = layerA->siblings();
            const int indexA = layers.indexOf(layerA);
            const int indexB = layers.indexOf(layerB);
            return indexB > indexA;
        }

        layerA = parentA;
        layerB = parentB;
    }
}

/**
 * A layer has changed. This can mean that the layer visibility, opacity or
 * offset changed.
 */
void MapScene::layerChanged(Layer *layer)
{
    QGraphicsItem *layerItem = mLayerItems.value(layer);
    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    qreal multiplier = 1;
    if (mHighlightCurrentLayer && isAbove(mMapDocument->currentLayer(), layer))
        multiplier = opacityFactor;

    layerItem->setOpacity(layer->opacity() * multiplier);
    layerItem->setPos(layer->offset());

    // Layer offset may have changed, affecting the scene rect and grid
    updateSceneRect();
    if (mGridVisible)
        update();
}

/**
 * When an object group has changed it may mean its color or drawing order
 * changed, which affects all its objects.
 */
void MapScene::objectGroupChanged(ObjectGroup *objectGroup)
{
    objectsChanged(objectGroup->objects());
    objectsIndexChanged(objectGroup, 0, objectGroup->objectCount() - 1);
}

/**
 * When an image layer has changed, it may change size and it may look
 * differently.
 */
void MapScene::imageLayerChanged(ImageLayer *imageLayer)
{
    ImageLayerItem *item = static_cast<ImageLayerItem*>(mLayerItems.value(imageLayer));

    item->syncWithImageLayer();
    item->update();
}

/**
 * This function should be called when any tiles in the given tileset may have
 * changed their size or offset or image.
 */
void MapScene::adaptToTilesetTileSizeChanges(Tileset *tileset)
{
    update();

    for (QGraphicsItem *item : mLayerItems)
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : mObjectItems) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tileset() == tileset)
            item->syncWithMapObject();
    }
}

void MapScene::adaptToTileSizeChanges(Tile *tile)
{
    update();

    for (QGraphicsItem *item : mLayerItems)
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : mObjectItems) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tile() == tile)
            item->syncWithMapObject();
    }
}

void MapScene::tilesetReplaced(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    adaptToTilesetTileSizeChanges(tileset);
}

/**
 * Inserts map object items for the given objects.
 */
void MapScene::objectsInserted(ObjectGroup *objectGroup, int first, int last)
{
    ObjectGroupItem *ogItem = nullptr;

    // Find the object group item for the object group
    for (QGraphicsItem *item : mLayerItems) {
        if (ObjectGroupItem *ogi = dynamic_cast<ObjectGroupItem*>(item)) {
            if (ogi->objectGroup() == objectGroup) {
                ogItem = ogi;
                break;
            }
        }
    }

    Q_ASSERT(ogItem);

    const ObjectGroup::DrawOrder drawOrder = objectGroup->drawOrder();

    for (int i = first; i <= last; ++i) {
        MapObject *object = objectGroup->objectAt(i);

        MapObjectItem *item = new MapObjectItem(object, mMapDocument, ogItem);
        if (drawOrder == ObjectGroup::TopDownOrder)
            item->setZValue(item->y());
        else
            item->setZValue(i);

        mObjectItems.insert(object, item);
    }
}

/**
 * Removes the map object items related to the given objects.
 */
void MapScene::objectsRemoved(const QList<MapObject*> &objects)
{
    for (MapObject *o : objects) {
        auto i = mObjectItems.find(o);
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
    for (MapObject *object : objects) {
        MapObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        item->syncWithMapObject();
    }
}

/**
 * Updates the Z value of the objects when appropriate.
 */
void MapScene::objectsIndexChanged(ObjectGroup *objectGroup,
                                   int first, int last)
{
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    for (int i = first; i <= last; ++i) {
        MapObjectItem *item = itemForObject(objectGroup->objectAt(i));
        Q_ASSERT(item);

        item->setZValue(i);
    }
}

void MapScene::updateSelectedObjectItems()
{
    const QList<MapObject *> &objects = mMapDocument->selectedObjects();

    QSet<MapObjectItem*> items;
    for (MapObject *object : objects) {
        MapObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        items.insert(item);
    }

    mSelectedObjectItems = items;
    emit selectedObjectItemsChanged();
}

void MapScene::syncAllObjectItems()
{
    for (MapObjectItem *item : mObjectItems)
        item->syncWithMapObject();
}

/**
 * Sets whether the tile grid is visible.
 */
void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

void MapScene::setObjectLineWidth(qreal lineWidth)
{
    if (mObjectLineWidth == lineWidth)
        return;

    mObjectLineWidth = lineWidth;

    if (mMapDocument) {
        mMapDocument->renderer()->setObjectLineWidth(lineWidth);

        // Changing the line width can change the size of the object items
        if (!mObjectItems.isEmpty()) {
            for (MapObjectItem *item : mObjectItems)
                item->syncWithMapObject();

            update();
        }
    }
}

void MapScene::setShowTileObjectOutlines(bool enabled)
{
    if (mShowTileObjectOutlines == enabled)
        return;

    mShowTileObjectOutlines = enabled;

    if (mMapDocument) {
        mMapDocument->renderer()->setFlag(ShowTileObjectOutlines, enabled);
        if (!mObjectItems.isEmpty())
            update();
    }
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

    QPointF offset;

    // Take into account the offset of the current layer
    if (Layer *layer = mMapDocument->currentLayer()) {
        offset = layer->totalOffset();
        painter->translate(offset);
    }

    Preferences *prefs = Preferences::instance();
    mMapDocument->renderer()->drawGrid(painter,
                                       rect.translated(-offset),
                                       prefs->gridColor());
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

void MapScene::keyPressEvent(QKeyEvent *event)
{
    if (mActiveTool)
        mActiveTool->keyPressed(event);

    if (!(mActiveTool && event->isAccepted()))
        QGraphicsScene::keyPressEvent(event);
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
