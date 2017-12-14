/*
 * mapitem.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "mapitem.h"

#include "grouplayer.h"
#include "grouplayeritem.h"
#include "imagelayeritem.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroupitem.h"
#include "objectselectionitem.h"
#include "preferences.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"

#include <QPen>

namespace Tiled {
namespace Internal {

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

MapItem::MapItem(MapDocument *mapDocument, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
    , mDarkRectangle(new QGraphicsRectItem(this))
{
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);

    createLayerItems(mapDocument->map()->layers());

    auto tileSelectionItem = new TileSelectionItem(mapDocument, this);
    tileSelectionItem->setZValue(10000 - 2);

    auto objectSelectionItem = new ObjectSelectionItem(mapDocument, this);
    objectSelectionItem->setZValue(10000 - 1);

    Preferences *prefs = Preferences::instance();

    MapRenderer *renderer = mapDocument->renderer();
    renderer->setObjectLineWidth(prefs->objectLineWidth());
    renderer->setFlag(ShowTileObjectOutlines, prefs->showTileObjectOutlines());

    connect(prefs, &Preferences::objectLineWidthChanged, this, &MapItem::setObjectLineWidth);
    connect(prefs, &Preferences::showTileObjectOutlinesChanged, this, &MapItem::setShowTileObjectOutlines);
    connect(prefs, &Preferences::highlightCurrentLayerChanged, this, &MapItem::updateCurrentLayerHighlight);
    connect(prefs, &Preferences::objectTypesChanged, this, &MapItem::syncAllObjectItems);

    connect(mapDocument, &MapDocument::mapChanged, this, &MapItem::mapChanged);
    connect(mapDocument, &MapDocument::regionChanged, this, &MapItem::repaintRegion);
    connect(mapDocument, &MapDocument::tileLayerChanged, this, &MapItem::tileLayerChanged);
    connect(mapDocument, &MapDocument::layerAdded, this, &MapItem::layerAdded);
    connect(mapDocument, &MapDocument::layerRemoved, this, &MapItem::layerRemoved);
    connect(mapDocument, &MapDocument::layerChanged, this, &MapItem::layerChanged);
    connect(mapDocument, &MapDocument::objectGroupChanged, this, &MapItem::objectGroupChanged);
    connect(mapDocument, &MapDocument::imageLayerChanged, this, &MapItem::imageLayerChanged);
    connect(mapDocument, &MapDocument::currentLayerChanged, this, &MapItem::currentLayerChanged);
    connect(mapDocument, &MapDocument::tilesetTileOffsetChanged, this, &MapItem::adaptToTilesetTileSizeChanges);
    connect(mapDocument, &MapDocument::tileImageSourceChanged, this, &MapItem::adaptToTileSizeChanges);
    connect(mapDocument, &MapDocument::tilesetReplaced, this, &MapItem::tilesetReplaced);
    connect(mapDocument, &MapDocument::objectsInserted, this, &MapItem::objectsInserted);
    connect(mapDocument, &MapDocument::objectsRemoved, this, &MapItem::objectsRemoved);
    connect(mapDocument, &MapDocument::objectsChanged, this, &MapItem::objectsChanged);
    connect(mapDocument, &MapDocument::objectsIndexChanged, this, &MapItem::objectsIndexChanged);

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(Qt::black);
    mDarkRectangle->setOpacity(darkeningFactor);
    mDarkRectangle->setRect(QRect(INT_MIN / 2, INT_MIN / 2, INT_MAX, INT_MAX));
    updateCurrentLayerHighlight();
}

QRectF MapItem::boundingRect() const
{
    return QRectF();
}

void MapItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void MapItem::repaintRegion(const QRegion &region, TileLayer *tileLayer)
{
    const MapRenderer *renderer = mMapDocument->renderer();
    const QMargins margins = mMapDocument->map()->drawMargins();
    TileLayerItem *tileLayerItem = static_cast<TileLayerItem*>(mLayerItems.value(tileLayer));

    for (const QRect &r : region.rects()) {
        QRectF boundingRect = renderer->boundingRect(r);
        boundingRect.adjust(-margins.left(),
                            -margins.top(),
                            margins.right(),
                            margins.bottom());

        tileLayerItem->update(boundingRect);
    }
}

void MapItem::currentLayerChanged()
{
    updateCurrentLayerHighlight();
}

void MapItem::mapChanged()
{
    for (QGraphicsItem *item : mLayerItems) {
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();
    }

    for (MapObjectItem *item : mObjectItems)
        item->syncWithMapObject();

}

void MapItem::tileLayerChanged(TileLayer *tileLayer)
{
    TileLayerItem *item = static_cast<TileLayerItem*>(mLayerItems.value(tileLayer));
    item->syncWithTileLayer();
}

void MapItem::layerAdded(Layer *layer)
{
    createLayerItem(layer);

    int z = 0;
    for (auto sibling : layer->siblings())
        mLayerItems.value(sibling)->setZValue(z++);
}

void MapItem::layerRemoved(Layer *layer)
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
void MapItem::layerChanged(Layer *layer)
{
    Preferences *prefs = Preferences::instance();
    QGraphicsItem *layerItem = mLayerItems.value(layer);
    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    qreal multiplier = 1;
    if (prefs->highlightCurrentLayer() && isAbove(mMapDocument->currentLayer(), layer))
        multiplier = opacityFactor;

    layerItem->setOpacity(layer->opacity() * multiplier);
    layerItem->setPos(layer->offset());
}

/**
 * When an object group has changed it may mean its color or drawing order
 * changed, which affects all its objects.
 */
void MapItem::objectGroupChanged(ObjectGroup *objectGroup)
{
    objectsChanged(objectGroup->objects());
    objectsIndexChanged(objectGroup, 0, objectGroup->objectCount() - 1);
}

/**
 * When an image layer has changed, it may change size and it may look
 * differently.
 */
void MapItem::imageLayerChanged(ImageLayer *imageLayer)
{
    ImageLayerItem *item = static_cast<ImageLayerItem*>(mLayerItems.value(imageLayer));
    item->syncWithImageLayer();
    item->update();
}

/**
 * This function should be called when any tiles in the given tileset may have
 * changed their size or offset or image.
 */
void MapItem::adaptToTilesetTileSizeChanges(Tileset *tileset)
{
    for (QGraphicsItem *item : mLayerItems)
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : mObjectItems) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tileset() == tileset)
            item->syncWithMapObject();
    }
}

void MapItem::adaptToTileSizeChanges(Tile *tile)
{
    for (QGraphicsItem *item : mLayerItems)
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : mObjectItems) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tile() == tile)
            item->syncWithMapObject();
    }
}

void MapItem::tilesetReplaced(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    adaptToTilesetTileSizeChanges(tileset);
}

/**
 * Inserts map object items for the given objects.
 */
void MapItem::objectsInserted(ObjectGroup *objectGroup, int first, int last)
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
void MapItem::objectsRemoved(const QList<MapObject*> &objects)
{
    for (MapObject *o : objects) {
        auto i = mObjectItems.find(o);
        Q_ASSERT(i != mObjectItems.end());

        delete i.value();
        mObjectItems.erase(i);
    }
}

/**
 * Updates the map object items related to the given objects.
 */
void MapItem::objectsChanged(const QList<MapObject*> &objects)
{
    for (MapObject *object : objects) {
        MapObjectItem *item = mObjectItems.value(object);
        Q_ASSERT(item);

        item->syncWithMapObject();
    }
}

/**
 * Updates the Z value of the objects when appropriate.
 */
void MapItem::objectsIndexChanged(ObjectGroup *objectGroup,
                                   int first, int last)
{
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    for (int i = first; i <= last; ++i) {
        MapObjectItem *item = mObjectItems.value(objectGroup->objectAt(i));
        Q_ASSERT(item);

        item->setZValue(i);
    }
}

void MapItem::syncAllObjectItems()
{
    for (MapObjectItem *item : mObjectItems)
        item->syncWithMapObject();
}


void MapItem::setObjectLineWidth(qreal lineWidth)
{
    mMapDocument->renderer()->setObjectLineWidth(lineWidth);

    // Changing the line width can change the size of the object items
    for (MapObjectItem *item : mObjectItems) {
        if (item->mapObject()->cell().isEmpty()) {
            item->syncWithMapObject();
            item->update();
        }
    }
}

void MapItem::setShowTileObjectOutlines(bool enabled)
{
    mMapDocument->renderer()->setFlag(ShowTileObjectOutlines, enabled);

    for (MapObjectItem *item : mObjectItems) {
        if (!item->mapObject()->cell().isEmpty())
            item->update();
    }
}

void MapItem::createLayerItems(const QList<Layer *> &layers)
{
    int layerIndex = 0;

    for (Layer *layer : layers) {
        LayerItem *layerItem = createLayerItem(layer);
        layerItem->setZValue(layerIndex);
        ++layerIndex;
    }
}

LayerItem *MapItem::createLayerItem(Layer *layer)
{
    LayerItem *layerItem = nullptr;

    QGraphicsItem *parent = this;
    if (layer->parentLayer())
        parent = mLayerItems.value(layer->parentLayer());

    switch (layer->layerType()) {
    case Layer::TileLayerType:
        layerItem = new TileLayerItem(static_cast<TileLayer*>(layer), mMapDocument, parent);
        break;

    case Layer::ObjectGroupType: {
        auto og = static_cast<ObjectGroup*>(layer);
        const ObjectGroup::DrawOrder drawOrder = og->drawOrder();
        ObjectGroupItem *ogItem = new ObjectGroupItem(og, parent);
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
        layerItem = new ImageLayerItem(static_cast<ImageLayer*>(layer), mMapDocument, parent);
        break;

    case Layer::GroupLayerType:
        layerItem = new GroupLayerItem(static_cast<GroupLayer*>(layer), parent);
        break;
    }

    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    mLayerItems.insert(layer, layerItem);

    if (GroupLayer *groupLayer = layer->asGroupLayer())
        createLayerItems(groupLayer->layers());

    return layerItem;
}

void MapItem::updateCurrentLayerHighlight()
{
    Preferences *prefs = Preferences::instance();
    const auto currentLayer = mMapDocument->currentLayer();

    if (!prefs->highlightCurrentLayer() || !currentLayer) {
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
    QGraphicsItem *parentItem = mLayerItems.value(parentLayer);
    if (!parentItem)
        parentItem = this;

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

} // namespace Internal
} // namespace Tiled
