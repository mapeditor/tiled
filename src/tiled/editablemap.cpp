/*
 * editablemap.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editablemap.h"

#include "addremovelayer.h"
#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "automappingmanager.h"
#include "changeevents.h"
#include "changemapproperty.h"
#include "editablelayer.h"
#include "editablemanager.h"
#include "editablemapobject.h"
#include "editableselectedarea.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "grouplayer.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "minimaprenderer.h"
#include "objectgroup.h"
#include "replacetileset.h"
#include "resizemap.h"
#include "scriptmanager.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QCoreApplication>
#include <QQmlEngine>
#include <QUndoStack>

namespace Tiled {

EditableMap::EditableMap(QObject *parent)
    : EditableAsset(nullptr, new Map(), parent)
{
    mDetachedMap.reset(map());
}

EditableMap::EditableMap(MapDocument *mapDocument, QObject *parent)
    : EditableAsset(mapDocument, mapDocument->map(), parent)
    , mSelectedArea(new EditableSelectedArea(mapDocument, this))
{
    connect(mapDocument, &Document::fileNameChanged, this, &EditableAsset::fileNameChanged);
    connect(mapDocument, &Document::changed, this, &EditableMap::documentChanged);
    connect(mapDocument, &MapDocument::layerAdded, this, &EditableMap::attachLayer);
    connect(mapDocument, &MapDocument::layerRemoved, this, &EditableMap::detachLayer);

    connect(mapDocument, &MapDocument::currentLayerChanged, this, &EditableMap::currentLayerChanged);
    connect(mapDocument, &MapDocument::selectedLayersChanged, this, &EditableMap::selectedLayersChanged);
    connect(mapDocument, &MapDocument::selectedObjectsChanged, this, &EditableMap::selectedObjectsChanged);

    connect(mapDocument, &MapDocument::regionEdited, this, &EditableMap::onRegionEdited);
}

/**
 * Creates a read-only instance of EditableMap that works on the given \a map.
 *
 * The map's lifetime must exceed that of the EditableMap instance.
 */
EditableMap::EditableMap(const Map *map, QObject *parent)
    : EditableAsset(nullptr, const_cast<Map*>(map), parent)
    , mReadOnly(true)
    , mSelectedArea(nullptr)
{
}

EditableMap::EditableMap(std::unique_ptr<Map> map, QObject *parent)
    : EditableAsset(nullptr, map.get(), parent)
    , mDetachedMap(std::move(map))
{
}

EditableMap::~EditableMap()
{
    for (Layer *layer : map()->layers())
        detachLayer(layer);
}

QList<QObject *> EditableMap::tilesets() const
{
    QList<QObject *> editableTilesets;
    auto &editableManager = EditableManager::instance();

    for (const SharedTileset &tileset : map()->tilesets())
        editableTilesets.append(editableManager.editableTileset(tileset.data()));

    return editableTilesets;
}

QList<QObject *> EditableMap::layers()
{
    QList<QObject *> editables;
    auto &editableManager = EditableManager::instance();

    for (const auto layer : map()->layers())
        editables.append(editableManager.editableLayer(this, layer));

    return editables;
}

EditableLayer *EditableMap::currentLayer()
{
    if (auto document = mapDocument())
        return EditableManager::instance().editableLayer(this, document->currentLayer());
    return nullptr;
}

QList<QObject *> EditableMap::selectedLayers()
{
    if (!mapDocument())
        return QList<QObject*>();

    QList<QObject*> selectedLayers;

    auto &editableManager = EditableManager::instance();
    const auto selectedLayersOrdered = mapDocument()->selectedLayersOrdered();
    for (Layer *layer : selectedLayersOrdered)
        selectedLayers.append(editableManager.editableLayer(this, layer));

    return selectedLayers;
}

QList<QObject *> EditableMap::selectedObjects()
{
    if (!mapDocument())
        return QList<QObject*>();

    QList<QObject*> selectedObjects;

    auto &editableManager = EditableManager::instance();
    const auto selectedObjectsOrdered = mapDocument()->selectedObjectsOrdered();
    for (MapObject *object : selectedObjectsOrdered)
        selectedObjects.append(editableManager.editableMapObject(this, object));

    return selectedObjects;
}

EditableLayer *EditableMap::layerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return nullptr;
    }

    Layer *layer = map()->layerAt(index);
    return EditableManager::instance().editableLayer(this, layer);
}

void EditableMap::removeLayerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (auto doc = mapDocument()) {
        push(new RemoveLayer(doc, index, nullptr));
    } else if (!checkReadOnly()) {
        auto layer = map()->takeLayerAt(index);
        EditableManager::instance().release(layer);
    }
}

void EditableMap::removeLayer(EditableLayer *editableLayer)
{
    if (!editableLayer) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    int index = map()->layers().indexOf(editableLayer->layer());
    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer not found"));
        return;
    }

    removeLayerAt(index);
}

static void initializeSize(Layer *layer, QSize size)
{
    if (layer->isTileLayer()) {
        auto tileLayer = static_cast<TileLayer*>(layer);
        if (tileLayer->size().isNull())
            tileLayer->setSize(size);
    } else if (layer->isGroupLayer()) {
        for (Layer *childLayer : static_cast<GroupLayer*>(layer)->layers())
            initializeSize(childLayer, size);
    }
}

void EditableMap::insertLayerAt(int index, EditableLayer *editableLayer)
{
    if (index < 0 || index > layerCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (!editableLayer) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    if (editableLayer->map()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer already part of a map"));
        return;
    }

    // If this map has a valid size but the tile layer that's getting added
    // doesn't, default the layer's size to the map size.
    if (!size().isNull())
        initializeSize(editableLayer->layer(), size());

    const auto tilesets = editableLayer->layer()->usedTilesets();

    if (auto doc = mapDocument()) {
        auto command = new AddLayer(doc, index, editableLayer->layer(), nullptr);

        for (const auto &tileset : tilesets)
            if (!map()->tilesets().contains(tileset))
                new AddTileset(doc, tileset, command);

        push(command);
    } else if (!checkReadOnly()) {
        map()->addTilesets(tilesets);

        // ownership moves to the map
        map()->insertLayer(index, editableLayer->release());
    }
}

void EditableMap::addLayer(EditableLayer *editableLayer)
{
    insertLayerAt(layerCount(), editableLayer);
}

bool EditableMap::addTileset(EditableTileset *editableTileset)
{
    if (!editableTileset) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }
    const auto &tileset = editableTileset->tileset()->sharedFromThis();
    if (map()->indexOfTileset(tileset) != -1)
        return false;   // can't add existing tileset

    if (auto doc = mapDocument())
        push(new AddTileset(doc, tileset));
    else if (!checkReadOnly())
        map()->addTileset(tileset);

    return true;
}

bool EditableMap::replaceTileset(EditableTileset *oldEditableTileset,
                                 EditableTileset *newEditableTileset)
{
    if (!oldEditableTileset) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }
    if (!newEditableTileset) {
        ScriptManager::instance().throwNullArgError(1);
        return false;
    }
    if (oldEditableTileset == newEditableTileset) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return false;
    }

    SharedTileset oldTileset = oldEditableTileset->tileset()->sharedFromThis();
    int indexOfOldTileset = map()->indexOfTileset(oldTileset);
    if (indexOfOldTileset == -1)
        return false;   // can't replace non-existing tileset

    SharedTileset newTileset = newEditableTileset->tileset()->sharedFromThis();
    int indexOfNewTileset = map()->indexOfTileset(newTileset);
    if (indexOfNewTileset != -1)
        return false;   // can't replace with tileset that is already part of the map (undo broken)

    if (auto doc = mapDocument())
        push(new ReplaceTileset(doc, indexOfOldTileset, newTileset));
    else if (!checkReadOnly())
        map()->replaceTileset(oldTileset, newTileset);

    return true;
}

bool EditableMap::removeTileset(EditableTileset *editableTileset)
{
    if (!editableTileset) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }
    Tileset *tileset = editableTileset->tileset();
    int index = map()->indexOfTileset(tileset->sharedFromThis());
    if (index == -1)
        return false;   // can't remove non-existing tileset

    if (map()->isTilesetUsed(tileset))
        return false;   // not allowed to remove a tileset that's in use

    if (auto doc = mapDocument())
        push(new RemoveTileset(doc, index));
    else if (!checkReadOnly())
        map()->removeTilesetAt(index);

    return true;
}

QList<QObject *> EditableMap::usedTilesets() const
{
    const auto tilesets = map()->usedTilesets();

    QList<QObject *> editableTilesets;
    for (const SharedTileset &tileset : tilesets)
        editableTilesets.append(EditableManager::instance().editableTileset(tileset.data()));
    return editableTilesets;
}

void EditableMap::removeObjects(const QList<QObject*> &objects)
{
    QList<MapObject *> mapObjects;
    mapObjects.reserve(objects.size());

    for (QObject *object : objects) {
        auto editableMapObject = qobject_cast<EditableMapObject*>(object);
        if (!editableMapObject) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not an object"));
            return;
        }
        if (editableMapObject->map() != this) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object not from this map"));
            return;
        }

        auto mapObject = editableMapObject->mapObject();
        if (!mapObjects.contains(mapObject))
            mapObjects.append(mapObject);
    }

    if (auto doc = mapDocument()) {
        asset()->push(new RemoveMapObjects(doc, mapObjects));
    } else {
        for (MapObject *mapObject : mapObjects) {
            mapObject->objectGroup()->removeObject(mapObject);
            EditableManager::instance().release(mapObject);
        }
    }
}

/**
 * Merges the given map with this map. Automatically adds any tilesets that are
 * used by the merged map which are not yet part of this map.
 *
 * Might replace tilesets in the given \a editableMap, if it is detached.
 *
 * Pass \a canJoin as 'true' if the operation is allowed to join with the
 * previous one on the undo stack.
 *
 * @warning Currently only supports tile layers!
 */
void EditableMap::merge(EditableMap *editableMap, bool canJoin)
{
    if (!editableMap) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    if (!mapDocument()) {   // todo: support this outside of the undo stack
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Merge is currently not supported for detached maps"));
        return;
    }

    // unifyTilesets might modify the given map, so need to clone if it has a document.
    Map *map = editableMap->map();
    std::unique_ptr<Map> copy;      // manages lifetime
    if (editableMap->document()) {
        copy = map->clone();
        map = copy.get();
    }

    QVector<SharedTileset> missingTilesets;
    mapDocument()->unifyTilesets(*map, missingTilesets);
    mapDocument()->paintTileLayers(*map, canJoin, &missingTilesets);
}

/**
 * Resize this map to the given \a size, while at the same time shifting
 * the contents by \a offset. If \a removeObjects is true then all objects
 * which are outside the map will be removed.
 */
void EditableMap::resize(QSize size, QPoint offset, bool removeObjects)
{
    if (checkReadOnly())
        return;
    if (!mapDocument()) {   // todo: should be able to resize still
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Resize is currently not supported for detached maps"));
        return;
    }
    if (size.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid size"));
        return;
    }

    mapDocument()->resizeMap(size, offset, removeObjects);
}

void EditableMap::autoMap(const RegionValueType &region, const QString &rulesFile)
{
    if (checkReadOnly())
        return;
    if (!mapDocument()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "AutoMapping is currently not supported for detached maps"));
        return;
    }

    if (!mAutomappingManager)
        mAutomappingManager = new AutomappingManager(this);

    AutomappingManager &manager = *mAutomappingManager;
    manager.setMapDocument(mapDocument(), rulesFile);

    if (region.region().isEmpty())
        manager.autoMap();
    else
        manager.autoMapRegion(region.region());
}

Tiled::ScriptImage *EditableMap::toImage(QSize size)
{
    const MiniMapRenderer miniMapRenderer(map());
    const QSize imageSize = size.isValid() ? size : miniMapRenderer.mapSize();
    const MiniMapRenderer::RenderFlags renderFlags(MiniMapRenderer::DrawTileLayers |
                                                   MiniMapRenderer::DrawMapObjects |
                                                   MiniMapRenderer::DrawImageLayers |
                                                   MiniMapRenderer::IgnoreInvisibleLayer |
                                                   MiniMapRenderer::DrawBackground);

    return new ScriptImage(miniMapRenderer.render(imageSize, renderFlags));
}

QPointF EditableMap::screenToTile(qreal x, qreal y) const
{
    return renderer()->screenToTileCoords(x, y);
}

QPointF EditableMap::tileToScreen(qreal x, qreal y) const
{
    return renderer()->tileToScreenCoords(x, y);
}

QPointF EditableMap::screenToPixel(qreal x, qreal y) const
{
    return renderer()->screenToPixelCoords(x, y);
}

QPointF EditableMap::pixelToScreen(qreal x, qreal y) const
{
    return renderer()->pixelToScreenCoords(x, y);
}

QPointF EditableMap::pixelToTile(qreal x, qreal y) const
{
    return renderer()->pixelToTileCoords(x, y);
}

QPointF EditableMap::tileToPixel(qreal x, qreal y) const
{
    return renderer()->tileToPixelCoords(x, y);
}

void EditableMap::setSize(int width, int height)
{
    if (auto doc = mapDocument()) {
        push(new ResizeMap(doc, QSize(width, height)));
    } else if (!checkReadOnly()) {
        map()->setWidth(width);
        map()->setHeight(height);
    }
}

void EditableMap::setTileWidth(int value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, Map::TileWidthProperty, value));
    else if (!checkReadOnly())
        map()->setTileWidth(value);
}

void EditableMap::setTileHeight(int value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, Map::TileHeightProperty, value));
    else if (!checkReadOnly())
        map()->setTileHeight(value);
}

void EditableMap::setTileSize(int width, int height)
{
    if (checkReadOnly())
        return;

    if (auto doc = mapDocument()) {
        doc->undoStack()->beginMacro(QCoreApplication::translate("Undo Commands",
                                                                 "Change Tile Size"));
        setTileWidth(width);
        setTileHeight(height);
        doc->undoStack()->endMacro();
    } else {
        map()->setTileWidth(width);
        map()->setTileHeight(height);
    }
}

void EditableMap::setInfinite(bool value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, Map::InfiniteProperty, value));
    else if (!checkReadOnly())
        map()->setInfinite(value);
}

void EditableMap::setHexSideLength(int value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, Map::HexSideLengthProperty, value));
    else if (!checkReadOnly())
        map()->setHexSideLength(value);
}

void EditableMap::setStaggerAxis(StaggerAxis value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, static_cast<Map::StaggerAxis>(value)));
    else if (!checkReadOnly())
        map()->setStaggerAxis(static_cast<Map::StaggerAxis>(value));
}

void EditableMap::setStaggerIndex(StaggerIndex value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, static_cast<Map::StaggerIndex>(value)));
    else if (!checkReadOnly())
        map()->setStaggerIndex(static_cast<Map::StaggerIndex>(value));
}

void EditableMap::setParallaxOrigin(const QPointF &parallaxOrigin)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, parallaxOrigin));
    else if (!checkReadOnly())
        map()->setParallaxOrigin(parallaxOrigin);
}

void EditableMap::setOrientation(Orientation value)
{
    if (auto doc = mapDocument()) {
        push(new ChangeMapProperty(doc, static_cast<Map::Orientation>(value)));
    } else if (!checkReadOnly()) {
        map()->setOrientation(static_cast<Map::Orientation>(value));
        mRenderer.reset();
    }
}

void EditableMap::setRenderOrder(RenderOrder value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, static_cast<Map::RenderOrder>(value)));
    else if (!checkReadOnly())
        map()->setRenderOrder(static_cast<Map::RenderOrder>(value));
}

void EditableMap::setBackgroundColor(const QColor &value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, value));
    else if (!checkReadOnly())
        map()->setBackgroundColor(value);
}

void EditableMap::setLayerDataFormat(LayerDataFormat value)
{
    if (auto doc = mapDocument())
        push(new ChangeMapProperty(doc, static_cast<Map::LayerDataFormat>(value)));
    else if (!checkReadOnly())
        map()->setLayerDataFormat(static_cast<Map::LayerDataFormat>(value));
}

void EditableMap::setCurrentLayer(EditableLayer *layer)
{
    QList<QObject*> layers;
    if (layer)
        layers.append(layer);

    setSelectedLayers(layers);
}

void EditableMap::setSelectedLayers(const QList<QObject *> &layers)
{
    auto document = mapDocument();
    if (!document)
        return;

    QList<Layer*> plainLayers;

    for (QObject *layerObject : layers) {
        auto editableLayer = qobject_cast<EditableLayer*>(layerObject);
        if (!editableLayer) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not a layer"));
            return;
        }
        if (editableLayer->map() != this) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer not from this map"));
            return;
        }

        plainLayers.append(editableLayer->layer());
    }

    document->switchSelectedLayers(plainLayers);
}

void EditableMap::setSelectedObjects(const QList<QObject *> &objects)
{
    auto document = mapDocument();
    if (!document)
        return;

    QList<MapObject*> plainObjects;

    for (QObject *objectObject : objects) {
        auto editableMapObject = qobject_cast<EditableMapObject*>(objectObject);
        if (!editableMapObject) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not an object"));
            return;
        }
        if (editableMapObject->map() != this) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object not from this map"));
            return;
        }

        plainObjects.append(editableMapObject->mapObject());
    }

    document->setSelectedObjects(plainObjects);
}

QSharedPointer<Document> EditableMap::createDocument()
{
    Q_ASSERT(mDetachedMap);

    auto document = MapDocumentPtr::create(std::move(mDetachedMap));
    document->setEditable(std::unique_ptr<EditableAsset>(this));

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    return document;
}

void EditableMap::documentChanged(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::MapChanged:
        if (static_cast<const MapChangeEvent&>(change).property == Map::OrientationProperty)
            mRenderer.reset();
        break;
    case ChangeEvent::MapObjectsAdded:
        attachMapObjects(static_cast<const MapObjectsEvent&>(change).mapObjects);
        break;
    case ChangeEvent::MapObjectsAboutToBeRemoved:
        detachMapObjects(static_cast<const MapObjectsEvent&>(change).mapObjects);
        break;
    default:
        break;
    }
}

void EditableMap::attachLayer(Layer *layer)
{
    if (EditableLayer *editable = EditableManager::instance().find(layer))
        editable->attach(this);

    if (GroupLayer *groupLayer = layer->asGroupLayer()) {
        for (Layer *childLayer : groupLayer->layers())
            attachLayer(childLayer);
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        attachMapObjects(objectGroup->objects());
    }
}

void EditableMap::detachLayer(Layer *layer)
{
    auto editableLayer = EditableManager::instance().find(layer);
    if (editableLayer && editableLayer->map() == this)
        editableLayer->detach();

    if (GroupLayer *groupLayer = layer->asGroupLayer()) {
        for (Layer *childLayer : groupLayer->layers())
            detachLayer(childLayer);
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        detachMapObjects(objectGroup->objects());
    }
}

void EditableMap::attachMapObjects(const QList<MapObject *> &mapObjects)
{
    const auto &editableManager = EditableManager::instance();
    for (MapObject *mapObject : mapObjects) {
        if (EditableMapObject *editable = editableManager.find(mapObject))
            editable->attach(this);
    }
}

void EditableMap::detachMapObjects(const QList<MapObject *> &mapObjects)
{
    const auto &editableManager = EditableManager::instance();
    for (MapObject *mapObject : mapObjects) {
        if (EditableMapObject *editable = editableManager.find(mapObject)) {
            Q_ASSERT(editable->map() == this);
            editable->detach();
        }
    }
}

void EditableMap::onRegionEdited(const QRegion &region, TileLayer *layer)
{
    auto &editableManager = EditableManager::instance();
    const auto editableLayer = static_cast<EditableTileLayer*>(editableManager.editableLayer(this, layer));
    emit regionEdited(RegionValueType(region), editableLayer);
}

MapRenderer *EditableMap::renderer() const
{
    if (!mRenderer)
        mRenderer = MapRenderer::create(map());

    return mRenderer.get();
}

} // namespace Tiled

#include "moc_editablemap.cpp"
