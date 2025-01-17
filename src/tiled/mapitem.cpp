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

#include "abstractworldtool.h"
#include "changeevents.h"
#include "changeworld.h"
#include "documentmanager.h"
#include "grouplayer.h"
#include "grouplayeritem.h"
#include "imagelayeritem.h"
#include "mapeditor.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroupitem.h"
#include "objectselectionitem.h"
#include "preferences.h"
#include "tilelayer.h"
#include "tilelayeritem.h"
#include "tileselectionitem.h"
#include "world.h"
#include "worldmanager.h"
#include "zoomable.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

#include <memory>

namespace Tiled {

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

class TileGridItem : public QGraphicsObject
{
    Q_OBJECT

public:
    TileGridItem(MapDocument *mapDocument, QGraphicsItem *parent)
        : QGraphicsObject(parent)
        , mMapDocument(mapDocument)
    {
        Q_ASSERT(mapDocument);

        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

        Preferences *prefs = Preferences::instance();
        connect(prefs, &Preferences::showGridChanged, this, [this] (bool visible) { setVisible(visible); });
        connect(prefs, &Preferences::gridColorChanged, this, [this] { update(); });
        connect(prefs, &Preferences::gridMajorChanged, this, [this] { update(); });

        // New layer may have a different offset
        connect(mapDocument, &MapDocument::currentLayerChanged,
                this, [this] { update(); });

        // Offset of current layer may have changed
        connect(mapDocument, &Document::changed,
                this, [this] (const ChangeEvent &change) {
            if (change.type == ChangeEvent::LayerChanged) {
                auto &layerChange = static_cast<const LayerChangeEvent&>(change);
                if (layerChange.properties & LayerChangeEvent::PositionProperties)
                    if (Layer *currentLayer = mMapDocument->currentLayer())
                        if (currentLayer->isParentOrSelf(layerChange.layer))
                            updateOffset();
            }
        });
        connect(mapDocument, &MapDocument::currentLayerChanged,
                this, &TileGridItem::updateOffset);

        setVisible(prefs->showGrid());
    }

    QRectF boundingRect() const override
    {
        return QRectF(INT_MIN / 512, INT_MIN / 512,
                      INT_MAX / 256, INT_MAX / 256);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) override
    {
        // Take into account the offset of the current layer
        painter->translate(mOffset);

        Preferences *prefs = Preferences::instance();
        mMapDocument->renderer()->drawGrid(painter,
                                           option->exposedRect.translated(-mOffset),
                                           prefs->gridColor(), prefs->gridMajor());
    }

    void updateOffset()
    {
        if (Layer *currentLayer = mMapDocument->currentLayer()) {
            QPointF offset = static_cast<MapScene*>(scene())->absolutePositionForLayer(*currentLayer);
            if (mOffset != offset) {
                mOffset = offset;
                update();
            }
        }
    }

private:
    MapDocument *mMapDocument;
    QPointF mOffset;
};

MapItem::MapItem(const MapDocumentPtr &mapDocument, DisplayMode displayMode,
                 QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
    , mDarkRectangle(new QGraphicsRectItem(this))
    , mBorderRectangle(new QGraphicsRectItem(this))
    , mDisplayMode(Editable)
{
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);
    setAcceptHoverEvents(true);

    createLayerItems(mapDocument->map()->layers());

    Preferences *prefs = Preferences::instance();

    MapRenderer *renderer = mapDocument->renderer();
    renderer->setObjectLineWidth(prefs->objectLineWidth());
    renderer->setFlag(ShowTileObjectOutlines, prefs->showTileObjectOutlines());

    connect(prefs, &Preferences::objectLineWidthChanged, this, &MapItem::setObjectLineWidth);
    connect(prefs, &Preferences::showTileObjectOutlinesChanged, this, &MapItem::setShowTileObjectOutlines);
    connect(prefs, &Preferences::highlightCurrentLayerChanged, this, &MapItem::updateSelectedLayersHighlight);
    connect(prefs, &Preferences::propertyTypesChanged, this, &MapItem::syncAllObjectItems);
    connect(prefs, &Preferences::backgroundFadeColorChanged, this, [this] (QColor color) { mDarkRectangle->setBrush(color); });

    connect(mapDocument.data(), &Document::changed, this, &MapItem::documentChanged);
    connect(mapDocument.data(), &MapDocument::mapResized, this, &MapItem::mapChanged);
    connect(mapDocument.data(), &MapDocument::regionChanged, this, &MapItem::repaintRegion);
    connect(mapDocument.data(), &MapDocument::tileLayerChanged, this, &MapItem::tileLayerChanged);
    connect(mapDocument.data(), &MapDocument::layerAdded, this, &MapItem::layerAdded);
    connect(mapDocument.data(), &MapDocument::layerAboutToBeRemoved, this, &MapItem::layerAboutToBeRemoved);
    connect(mapDocument.data(), &MapDocument::layerRemoved, this, &MapItem::layerRemoved);
    connect(mapDocument.data(), &MapDocument::selectedLayersChanged, this, &MapItem::updateSelectedLayersHighlight);
    connect(mapDocument.data(), &MapDocument::tilesetTilePositioningChanged, this, &MapItem::adaptToTilesetTileSizeChanges);
    connect(mapDocument.data(), &MapDocument::tileImageSourceChanged, this, &MapItem::adaptToTileSizeChanges);
    connect(mapDocument.data(), &MapDocument::tileObjectGroupChanged, this, &MapItem::tileObjectGroupChanged);
    connect(mapDocument.data(), &MapDocument::tilesetReplaced, this, &MapItem::tilesetReplaced);
    connect(mapDocument.data(), &MapDocument::objectsInserted, this, &MapItem::objectsInserted);
    connect(mapDocument.data(), &MapDocument::objectsIndexChanged, this, &MapItem::objectsIndexChanged);

    updateBoundingRect();

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(prefs->backgroundFadeColor());
    mDarkRectangle->setOpacity(darkeningFactor);
    mDarkRectangle->setRect(QRectF(INT_MIN / 512, INT_MIN / 512,
                                   INT_MAX / 256, INT_MAX / 256));

    auto updateBorder = [this] (QColor color) {
        QPen pen(color);
        pen.setCosmetic(true);
        mBorderRectangle->setPen(pen);
    };

    updateBorder(prefs->gridColor());
    connect(prefs, &Preferences::gridColorChanged, this, updateBorder);

    mBorderRectangle->setZValue(10000 - 3);

    if (displayMode == ReadOnly) {
        setDisplayMode(displayMode);
    } else {
        updateSelectedLayersHighlight();

        mTileSelectionItem = std::make_unique<TileSelectionItem>(mapDocument.data(), this);
        mTileSelectionItem->setZValue(10000 - 2);

        mTileGridItem = std::make_unique<TileGridItem>(mapDocument.data(), this);
        mTileGridItem->setZValue(10000 - 2);

        mObjectSelectionItem = std::make_unique<ObjectSelectionItem>(mapDocument.data(), this);
        mObjectSelectionItem->setZValue(10000 - 1);
    }
}

MapItem::~MapItem()
{
}

void MapItem::setDisplayMode(DisplayMode displayMode)
{
    if (mDisplayMode == displayMode)
        return;

    mDisplayMode = displayMode;

    // Enabled state is checked by selection tools
    for (LayerItem *layerItem : std::as_const(mLayerItems))
        layerItem->setEnabled(displayMode == Editable);

    if (displayMode == ReadOnly) {
        setZValue(-1);

        mBorderRectangle->setBrush(QColor(0, 0, 0, 64));

        mTileSelectionItem.reset();
        mTileGridItem.reset();
        mObjectSelectionItem.reset();
    } else {
        unsetCursor();

        setZValue(0);

        mBorderRectangle->setBrush(Qt::NoBrush);

        mTileSelectionItem = std::make_unique<TileSelectionItem>(mapDocument(), this);
        mTileSelectionItem->setZValue(10000 - 3);

        mTileGridItem = std::make_unique<TileGridItem>(mapDocument(), this);
        mTileGridItem->setZValue(10000 - 2);

        mObjectSelectionItem = std::make_unique<ObjectSelectionItem>(mapDocument(), this);
        mObjectSelectionItem->setZValue(10000 - 1);
    }

    updateSelectedLayersHighlight();
}

void MapItem::setShowTileCollisionShapes(bool enabled)
{
    mapDocument()->renderer()->setFlag(ShowTileCollisionShapes, enabled);

    for (MapObjectItem *item : std::as_const(mObjectItems))
        if (Tile *tile = item->mapObject()->cell().tile())
            if (tile->objectGroup() && !tile->objectGroup()->isEmpty())
                item->syncWithMapObject();

    for (LayerItem *item : std::as_const(mLayerItems))
        if (item->layer()->isTileLayer())
            item->update();
}

void MapItem::updateLayerPositions()
{
    const MapScene *mapScene = static_cast<MapScene*>(scene());

    for (LayerItem *layerItem : std::as_const(mLayerItems)) {
        const Layer &layer = *layerItem->layer();
        layerItem->setPos(mapScene->layerItemPosition(layer));
    }

    if (mDisplayMode == Editable) {
        mTileSelectionItem->updatePosition();
        mTileGridItem->updateOffset();
        mObjectSelectionItem->updateItemPositions();
    }
}

QRectF MapItem::boundingRect() const
{
    return mBoundingRect;
}

void MapItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void MapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (mDisplayMode == ReadOnly) {
        mBorderRectangle->setBrush(QColor(0, 0, 0, 32));
        setCursor(Qt::PointingHandCursor);
        mIsHovered = true;
    }
}

void MapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (mDisplayMode == ReadOnly) {
        mBorderRectangle->setBrush(QColor(0, 0, 0, 64));
        unsetCursor();
        mIsHovered = false;
    }
}

bool MapItem::isWorldToolSelected() const
{
    Editor *currentEditor = DocumentManager::instance()->currentEditor();
    if (auto currentMapEditor = qobject_cast<MapEditor*>(currentEditor)) {
        if (qobject_cast<AbstractWorldTool*>(currentMapEditor->selectedTool()))
            return true;
    }
    return false;
}

void MapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (isWorldToolSelected()) {
        // the world tool has it's own handling for hovered maps
        QGraphicsItem::mousePressEvent(event);
        return;
    }
    if (mDisplayMode != ReadOnly || event->button() != Qt::LeftButton || !mIsHovered)
        QGraphicsItem::mousePressEvent(event);
}

/**
 * Switches from the current mapitem to this one,
 * tries to select similar layers and tileset by name.
 */
void MapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (mDisplayMode == ReadOnly && event->button() == Qt::LeftButton && isUnderMouse()) {
        MapView *view = static_cast<MapView*>(event->widget()->parent());
        QRectF viewRect { view->viewport()->rect() };
        QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
        DocumentManager::instance()->switchToDocumentAndHandleSimiliarTileset(mMapDocument.data(),
                                                                              sceneViewRect.center() - pos(),
                                                                              view->zoomable()->scale());
        return;
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

void MapItem::repaintRegion(const QRegion &region, TileLayer *tileLayer)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    const QMargins margins = mapDocument()->map()->drawMargins();
    TileLayerItem *tileLayerItem = static_cast<TileLayerItem*>(mLayerItems.value(tileLayer));

    for (const QRect &r : region) {
        QRectF boundingRect = renderer->boundingRect(r).marginsAdded(margins);
        tileLayerItem->update(boundingRect);
    }
}

void MapItem::documentChanged(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::DocumentAboutToReload:
        for (Layer *layer : mMapDocument->map()->layers())
            deleteLayerItems(layer);
        break;
    case ChangeEvent::DocumentReloaded: {
        // The renderer has been re-created
        auto lineWidth = Preferences::instance()->objectLineWidth();
        mapDocument()->renderer()->setObjectLineWidth(lineWidth);

        createLayerItems(mMapDocument->map()->layers());

        updateBoundingRect();
        updateLayerPositions();
        break;
    }
    case ChangeEvent::ObjectsChanged: {
        auto &objectsChange = static_cast<const ObjectsChangeEvent&>(change);
        if (!objectsChange.objects.isEmpty() && (objectsChange.properties & ObjectsChangeEvent::ClassProperty)) {
            const auto typeId = objectsChange.objects.first()->typeId();
            if (typeId == Object::MapObjectType) {
                for (Object *object : objectsChange.objects)
                    mObjectItems.value(static_cast<MapObject*>(object))->syncWithMapObject();
            } else if (typeId == Object::TileType) {
                if (mapDocument()->renderer()->testFlag(ShowTileObjectOutlines))
                    for (MapObjectItem *item : std::as_const(mObjectItems))
                        if (item->mapObject()->isTileObject())
                            item->syncWithMapObject();
            }
        }

        break;
    }
    case ChangeEvent::MapChanged: {
        auto &mapChange = static_cast<const MapChangeEvent&>(change);
        switch (mapChange.property) {
        case Map::TileSizeProperty:
        case Map::InfiniteProperty:
        case Map::HexSideLengthProperty:
        case Map::StaggerAxisProperty:
        case Map::StaggerIndexProperty:
        case Map::ParallaxOriginProperty:
        case Map::OrientationProperty:
            mapChanged();
            break;
        case Map::RenderOrderProperty:
        case Map::BackgroundColorProperty:
        case Map::LayerDataFormatProperty:
        case Map::CompressionLevelProperty:
        case Map::ChunkSizeProperty:
            break;
        }
        break;
    }
    case ChangeEvent::LayerChanged:
        layerChanged(static_cast<const LayerChangeEvent&>(change));
        break;
    case ChangeEvent::TileLayerChanged: {
        auto &e = static_cast<const TileLayerChangeEvent&>(change);
        if (e.properties & TileLayerChangeEvent::SizeProperty)
            tileLayerChanged(e.tileLayer(), MapDocument::TileLayerChangeFlags());
        break;
    }
    case ChangeEvent::ImageLayerChanged:
        imageLayerChanged(static_cast<const ImageLayerChangeEvent&>(change).imageLayer());
        break;
    case ChangeEvent::MapObjectAboutToBeRemoved: {
        auto &e = static_cast<const MapObjectEvent&>(change);
        deleteObjectItem(e.objectGroup->objectAt(e.index));
        break;
    }
    case ChangeEvent::MapObjectsChanged:
        syncObjectItems(static_cast<const MapObjectsChangeEvent&>(change).mapObjects);
        break;
    case ChangeEvent::ObjectGroupChanged: {
        auto &objectGroupChange = static_cast<const ObjectGroupChangeEvent&>(change);
        auto objectGroup = objectGroupChange.objectGroup;

        bool sync = (objectGroupChange.properties & ObjectGroupChangeEvent::ColorProperty) != 0;

        if (objectGroupChange.properties & ObjectGroupChangeEvent::DrawOrderProperty) {
            if (objectGroup->drawOrder() == ObjectGroup::IndexOrder)
                objectsIndexChanged(objectGroup, 0, objectGroup->objectCount() - 1);
            else
                sync = true;
        }

        if (sync)
            syncObjectItems(objectGroup->objects());

        break;
    }
    case ChangeEvent::TilesetChanged: {
        auto &tilesetChange = static_cast<const TilesetChangeEvent&>(change);
        if (tilesetChange.property == Tileset::TileRenderSizeProperty) {
            // This might affect the draw margins
            for (QGraphicsItem *item : std::as_const(mLayerItems)) {
                if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
                    tli->syncWithTileLayer();
            }
        }
        break;
    }
    default:
        break;
    }
}

/**
 * Adapts the layers and objects to new map size or orientation.
 */
void MapItem::mapChanged()
{
    for (QGraphicsItem *item : std::as_const(mLayerItems)) {
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();
    }

    syncAllObjectItems();
    updateBoundingRect();

    // When this map is part of a world, update that map's rect when necessary
    const QString &mapFileName = mapDocument()->fileName();
    if (auto worldDocument = WorldManager::instance().worldForMap(mapFileName)) {
        World *world = worldDocument->world();
        if (world->canBeModified()) {
            const QRect currentRectInWorld = world->mapRect(mapFileName);
            QRect resizedRect = mapDocument()->renderer()->mapBoundingRect();
            if (currentRectInWorld.size() != resizedRect.size()) {
                resizedRect.translate(currentRectInWorld.topLeft());
                auto undoStack = worldDocument->undoStack();
                undoStack->push(new SetMapRectCommand(worldDocument.data(), mapFileName, resizedRect));
            }
        }
    }
}

void MapItem::tileLayerChanged(TileLayer *tileLayer, MapDocument::TileLayerChangeFlags flags)
{
    TileLayerItem *item = static_cast<TileLayerItem*>(mLayerItems.value(tileLayer));
    item->syncWithTileLayer();

    if (flags & MapDocument::LayerBoundsChanged)
        updateBoundingRect();
}

void MapItem::layerAdded(Layer *layer)
{
    createLayerItem(layer);

    int z = 0;
    const auto siblings = layer->siblings();
    for (auto sibling : siblings)
        mLayerItems.value(sibling)->setZValue(z++);

    updateBoundingRect();
    updateSelectedLayersHighlight();
}

void MapItem::layerAboutToBeRemoved(GroupLayer *parentLayer, int index)
{
    // Fix up the Z value of the items of the siblings, before the layer is removed
    const auto siblings = parentLayer ? parentLayer->layers()
                                      : mMapDocument->map()->layers();
    const Layer *layer = siblings.at(index);

    int z = 0;
    for (auto sibling : siblings)
        if (sibling != layer)
            mLayerItems.value(sibling)->setZValue(z++);
}

void MapItem::layerRemoved(Layer *layer)
{
    deleteLayerItems(layer);
    updateBoundingRect();
    updateSelectedLayersHighlight();
}

/**
 * A layer has changed. This can mean that the layer visibility, opacity,
 * offset or parallax factor changed.
 */
void MapItem::layerChanged(const LayerChangeEvent &change)
{
    Layer *layer = change.layer;
    Preferences *prefs = Preferences::instance();
    QGraphicsItem *layerItem = mLayerItems.value(layer);
    Q_ASSERT(layerItem);

    if (change.properties & (LayerChangeEvent::TintColorProperty |
                             LayerChangeEvent::BlendModeProperty)) {
        updateLayerItems(layer);
    }

    layerItem->setVisible(layer->isVisible());

    qreal multiplier = 1;

    if (prefs->highlightCurrentLayer()) {
        const auto &selectedLayers = mapDocument()->selectedLayers();
        bool isAbove = false;

        LayerIterator iterator(mapDocument()->map());
        iterator.toBack();
        while (Layer *l = iterator.previous()) {
            if (selectedLayers.contains(l))
                break;
            if (l == layer) {
                isAbove = true;
                break;
            }
        }

        if (isAbove)
            multiplier = opacityFactor;
    }

    layerItem->setOpacity(layer->opacity() * multiplier);

    if (layer->isGroupLayer() && (change.properties & LayerChangeEvent::ParallaxFactorProperty))
        updateLayerPositions();
    else
        layerItem->setPos(static_cast<MapScene*>(scene())->layerItemPosition(*layer));

    updateBoundingRect();   // possible layer offset change
}

void MapItem::updateLayerItems(Layer *layer)
{
    switch (layer->layerType()) {
    case Layer::TileLayerType:
    case Layer::ImageLayerType:
        mLayerItems.value(layer)->update();
        break;
    case Layer::ObjectGroupType:
        for (MapObject *mapObject : static_cast<const ObjectGroup&>(*layer)) {
            if (mapObject->isTileObject())
                mObjectItems.value(mapObject)->update();
        }
        break;
    case Layer::GroupLayerType:
        // Recurse into group layers since tint color is inherited
        for (auto childLayer : static_cast<GroupLayer*>(layer)->layers())
            updateLayerItems(childLayer);
        break;
    }
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
    for (QGraphicsItem *item : std::as_const(mLayerItems))
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : std::as_const(mObjectItems)) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tileset() == tileset)
            item->syncWithMapObject();
    }
}

void MapItem::adaptToTileSizeChanges(Tile *tile)
{
    for (QGraphicsItem *item : std::as_const(mLayerItems))
        if (TileLayerItem *tli = dynamic_cast<TileLayerItem*>(item))
            tli->syncWithTileLayer();

    for (MapObjectItem *item : std::as_const(mObjectItems)) {
        const Cell &cell = item->mapObject()->cell();
        if (cell.tile() == tile)
            item->syncWithMapObject();
    }
}

void MapItem::tileObjectGroupChanged(Tile *tile)
{
    if (!Preferences::instance()->showTileCollisionShapes())
        return;

    for (MapObjectItem *item : std::as_const(mObjectItems)) {
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
    // Find the object group item for the object group
    auto ogItem = static_cast<ObjectGroupItem*>(mLayerItems.value(objectGroup));
    Q_ASSERT(ogItem);

    const ObjectGroup::DrawOrder drawOrder = objectGroup->drawOrder();

    for (int i = first; i <= last; ++i) {
        MapObject *object = objectGroup->objectAt(i);

        MapObjectItem *item = new MapObjectItem(object, mapDocument(), ogItem);
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
void MapItem::deleteObjectItem(MapObject *object)
{
    auto item = mObjectItems.take(object);
    Q_ASSERT(item);
    delete item;
}

/**
 * Updates the map object items related to the given objects.
 */
void MapItem::syncObjectItems(const QList<MapObject*> &objects)
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
    for (MapObjectItem *item : std::as_const(mObjectItems))
        item->syncWithMapObject();
}

void MapItem::setObjectLineWidth(qreal lineWidth)
{
    mapDocument()->renderer()->setObjectLineWidth(lineWidth);

    // Changing the line width can change the size of the object items
    for (MapObjectItem *item : std::as_const(mObjectItems)) {
        if (item->mapObject()->cell().isEmpty()) {
            item->syncWithMapObject();
            item->update();
        }
    }
}

void MapItem::setShowTileObjectOutlines(bool enabled)
{
    mapDocument()->renderer()->setFlag(ShowTileObjectOutlines, enabled);

    for (MapObjectItem *item : std::as_const(mObjectItems)) {
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
        layerItem = new TileLayerItem(static_cast<TileLayer*>(layer), mapDocument(), parent);
        break;

    case Layer::ObjectGroupType: {
        auto og = static_cast<ObjectGroup*>(layer);
        const ObjectGroup::DrawOrder drawOrder = og->drawOrder();
        ObjectGroupItem *ogItem = new ObjectGroupItem(og, parent);
        int objectIndex = 0;
        for (MapObject *object : og->objects()) {
            MapObjectItem *item = new MapObjectItem(object, mapDocument(),
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
        layerItem = new ImageLayerItem(static_cast<ImageLayer*>(layer), mapDocument(), parent);
        break;

    case Layer::GroupLayerType:
        layerItem = new GroupLayerItem(static_cast<GroupLayer*>(layer), parent);
        break;
    }

    Q_ASSERT(layerItem);

    // If we're not yet part of the MapScene, it means this happens in the
    // MapItem constructor and the layer will be positioned by a call to
    // updateLayerPositions from the MapScene.
    if (const MapScene *mapScene = static_cast<MapScene*>(scene()))
        layerItem->setPos(mapScene->layerItemPosition(*layer));

    layerItem->setVisible(layer->isVisible());
    layerItem->setEnabled(mDisplayMode == Editable);

    mLayerItems.insert(layer, layerItem);

    if (GroupLayer *groupLayer = layer->asGroupLayer())
        createLayerItems(groupLayer->layers());

    return layerItem;
}

void MapItem::deleteLayerItems(Layer *layer)
{
    switch (layer->layerType()) {
    case Layer::TileLayerType:
    case Layer::ImageLayerType:
        break;
    case Layer::ObjectGroupType:
        // Delete any object items
        for (auto object : static_cast<ObjectGroup*>(layer)->objects())
            delete mObjectItems.take(object);
        break;
    case Layer::GroupLayerType:
        // Recurse into group layers
        for (auto childLayer : static_cast<GroupLayer*>(layer)->layers())
            deleteLayerItems(childLayer);
        break;
    }

    delete mLayerItems.take(layer);
}

void MapItem::updateBoundingRect()
{
    QRect boundingRect = mapDocument()->renderer()->mapBoundingRect();

    // This rectangle represents the map boundary and as such is unaffected
    // by layer offsets or image layers.
    mBorderRectangle->setRect(boundingRect);

    mMapDocument->map()->adjustBoundingRectForOffsetsAndImageLayers(boundingRect);

    if (mBoundingRect != boundingRect) {
        prepareGeometryChange();
        mBoundingRect = boundingRect;
        emit boundingRectChanged();
    }
}

void MapItem::updateSelectedLayersHighlight()
{
    Preferences *prefs = Preferences::instance();
    const auto selectedLayers = mapDocument()->selectedLayers();

    if (!prefs->highlightCurrentLayer() || selectedLayers.isEmpty() || mDisplayMode == ReadOnly) {
        if (mDarkRectangle->isVisible()) {
            mDarkRectangle->setVisible(false);
            mDarkRectangle->setParentItem(this);    // avoid automatic deletion

            // Restore opacity for all layers
            for (auto layerItem : std::as_const(mLayerItems))
                layerItem->setOpacity(layerItem->layer()->opacity());
        }

        return;
    }

    Layer *lowestSelectedLayer = nullptr;
    LayerIterator iterator(mapDocument()->map());
    while (Layer *layer = iterator.next()) {
        if (selectedLayers.contains(layer)) {
            lowestSelectedLayer = layer;
            break;
        }
    }
    Q_ASSERT(lowestSelectedLayer);

    // Darken layers below the lowest selected layer
    const int siblingIndex = lowestSelectedLayer->siblingIndex();
    const auto parentLayer = lowestSelectedLayer->parentLayer();
    QGraphicsItem *parentItem = mLayerItems.value(parentLayer);
    if (!parentItem)
        parentItem = this;

    mDarkRectangle->setParentItem(parentItem);
    mDarkRectangle->setZValue(siblingIndex - 0.5);
    mDarkRectangle->setVisible(true);

    // Set layers above the current layer to reduced opacity
    iterator.toFront();
    bool foundSelected = false;

    while (Layer *layer = iterator.next()) {
        bool isSelected = selectedLayers.contains(layer);
        foundSelected |= isSelected;

        if (!layer->isGroupLayer()) {
            qreal multiplier = (foundSelected && !isSelected) ? opacityFactor : 1;
            mLayerItems.value(layer)->setOpacity(layer->opacity() * multiplier);
        }
    }
}

} // namespace Tiled

#include "mapitem.moc"
#include "moc_mapitem.cpp"
