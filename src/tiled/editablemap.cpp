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
#include "changeevents.h"
#include "changelayer.h"
#include "changemapproperty.h"
#include "changeselectedarea.h"
#include "editablegrouplayer.h"
#include "editableimagelayer.h"
#include "editablelayer.h"
#include "editablemanager.h"
#include "editablemapobject.h"
#include "editableobjectgroup.h"
#include "editableselectedarea.h"
#include "editabletilelayer.h"
#include "grouplayer.h"
#include "movemapobject.h"
#include "replacetileset.h"
#include "resizemap.h"
#include "resizetilelayer.h"
#include "scriptmanager.h"
#include "tileset.h"
#include "tilesetdocument.h"

#include <imagelayer.h>
#include <mapobject.h>
#include <maprenderer.h>
#include <objectgroup.h>
#include <tilelayer.h>

#include <QUndoStack>

#include "qtcompat_p.h"

namespace Tiled {

EditableMap::EditableMap(QObject *parent)
    : EditableAsset(nullptr, new Map(), parent)
    , mReadOnly(false)
    , mSelectedArea(nullptr)
{
    connect(map(), &Map::sizeChanged, this, &EditableMap::sizeChanged);
    connect(map(), &Map::tileWidthChanged, this, &EditableMap::tileWidthChanged);
    connect(map(), &Map::tileHeightChanged, this, &EditableMap::tileHeightChanged);
}

EditableMap::EditableMap(MapDocument *mapDocument, QObject *parent)
    : EditableAsset(mapDocument, mapDocument->map(), parent)
    , mReadOnly(false)
    , mSelectedArea(new EditableSelectedArea(mapDocument, this))
{
    connect(map(), &Map::sizeChanged, this, &EditableMap::sizeChanged);
    connect(map(), &Map::tileWidthChanged, this, &EditableMap::tileWidthChanged);
    connect(map(), &Map::tileHeightChanged, this, &EditableMap::tileHeightChanged);

    connect(mapDocument, &Document::fileNameChanged, this, &EditableAsset::fileNameChanged);
    connect(mapDocument, &Document::changed, this, &EditableMap::documentChanged);
    connect(mapDocument, &MapDocument::layerAdded, this, &EditableMap::attachLayer);
    connect(mapDocument, &MapDocument::layerRemoved, this, &EditableMap::detachLayer);

    connect(mapDocument, &MapDocument::currentLayerChanged, this, &EditableMap::onCurrentLayerChanged);
    connect(mapDocument, &MapDocument::selectedLayersChanged, this, &EditableMap::selectedLayersChanged);
    connect(mapDocument, &MapDocument::selectedObjectsChanged, this, &EditableMap::selectedObjectsChanged);
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

EditableMap::~EditableMap()
{
    for (Layer *layer : map()->layers())
        detachLayer(layer);
}

QList<QObject *> EditableMap::tilesets() const
{
    QList<QObject *> editableTilesets;
    for (const SharedTileset &tileset : map()->tilesets())
        if (auto document = TilesetDocument::findDocumentForTileset(tileset))
            editableTilesets.append(document->editable());
    return editableTilesets;
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
    for (Layer *layer : mapDocument()->selectedLayers())
        selectedLayers.append(editableManager.editableLayer(this, layer));

    return selectedLayers;
}

QList<QObject *> EditableMap::selectedObjects()
{
    if (!mapDocument())
        return QList<QObject*>();

    QList<QObject*> selectedObjects;

    auto &editableManager = EditableManager::instance();
    for (MapObject *object : mapDocument()->selectedObjects())
        selectedObjects.append(editableManager.editableMapObject(this, object));

    return selectedObjects;
}

EditableLayer *EditableMap::layerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return nullptr;
    }

    Layer *layer = map()->layerAt(index);
    return EditableManager::instance().editableLayer(this, layer);
}

void EditableMap::removeLayerAt(int index)
{
    if (index < 0 || index >= layerCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return;
    }

    push(new RemoveLayer(mapDocument(), index, nullptr));
}

void EditableMap::removeLayer(EditableLayer *editableLayer)
{
    if (!editableLayer) {
        ScriptManager::instance().throwError(tr("Invalid argument"));
        return;
    }

    int index = map()->layers().indexOf(editableLayer->layer());
    if (index == -1) {
        ScriptManager::instance().throwError(tr("Layer not found"));
        return;
    }

    removeLayerAt(index);
}

void EditableMap::insertLayerAt(int index, EditableLayer *editableLayer)
{
    if (index < 0 || index > layerCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return;
    }

    if (!editableLayer) {
        ScriptManager::instance().throwError(tr("Invalid argument"));
        return;
    }

    if (editableLayer->map()) {
        ScriptManager::instance().throwError(tr("Layer already part of a map"));
        return;
    }

    push(new AddLayer(mapDocument(), index, editableLayer->layer(), nullptr));
}

void EditableMap::addLayer(EditableLayer *editableLayer)
{
    insertLayerAt(layerCount(), editableLayer);
}

bool EditableMap::addTileset(EditableTileset *editableTileset)
{
    const auto &tileset = editableTileset->tileset()->sharedPointer();
    if (map()->indexOfTileset(tileset) != -1)
        return false;   // can't add existing tileset

    push(new AddTileset(mapDocument(), tileset));
    return true;
}

bool EditableMap::replaceTileset(EditableTileset *oldEditableTileset,
                                 EditableTileset *newEditableTileset)
{
    if (oldEditableTileset == newEditableTileset) {
        ScriptManager::instance().throwError(tr("Invalid argument"));
        return false;
    }

    SharedTileset oldTileset = oldEditableTileset->tileset()->sharedPointer();
    int indexOfOldTileset = map()->indexOfTileset(oldTileset);
    if (indexOfOldTileset == -1)
        return false;   // can't replace non-existing tileset

    SharedTileset newTileset = newEditableTileset->tileset()->sharedPointer();
    int indexOfNewTileset = map()->indexOfTileset(newTileset);
    if (indexOfNewTileset != -1)
        return false;   // can't replace with tileset that is already part of the map (undo broken)

    push(new ReplaceTileset(mapDocument(), indexOfOldTileset, newTileset));
    return true;
}

bool EditableMap::removeTileset(EditableTileset *editableTileset)
{
    Tileset *tileset = editableTileset->tileset();
    int index = map()->indexOfTileset(tileset->sharedPointer());
    if (index == -1)
        return false;   // can't remove non-existing tileset

    if (map()->isTilesetUsed(tileset))
        return false;   // not allowed to remove a tileset that's in use

    push(new RemoveTileset(mapDocument(), index));
    return true;
}

QList<QObject *> EditableMap::usedTilesets() const
{
    const auto tilesets = map()->usedTilesets();

    QList<QObject *> editableTilesets;
    for (const SharedTileset &tileset : tilesets)
        if (auto document = TilesetDocument::findDocumentForTileset(tileset))
            editableTilesets.append(document->editable());
    return editableTilesets;
}

void EditableMap::setTileWidth(int value)
{
    push(new ChangeMapProperty(mapDocument(), ChangeMapProperty::TileWidth, value));
}

void EditableMap::setTileHeight(int value)
{
    push(new ChangeMapProperty(mapDocument(), ChangeMapProperty::TileHeight, value));
}

void EditableMap::setInfinite(bool value)
{
    push(new ChangeMapProperty(mapDocument(), ChangeMapProperty::Infinite, value));
}

void EditableMap::setHexSideLength(int value)
{
    push(new ChangeMapProperty(mapDocument(), ChangeMapProperty::HexSideLength, value));
}

void EditableMap::setStaggerAxis(StaggerAxis value)
{
    push(new ChangeMapProperty(mapDocument(), static_cast<Map::StaggerAxis>(value)));
}

void EditableMap::setStaggerIndex(StaggerIndex value)
{
    push(new ChangeMapProperty(mapDocument(), static_cast<Map::StaggerIndex>(value)));
}

void EditableMap::setOrientation(Orientation value)
{
    push(new ChangeMapProperty(mapDocument(), static_cast<Map::Orientation>(value)));
}

void EditableMap::setRenderOrder(RenderOrder value)
{
    push(new ChangeMapProperty(mapDocument(), static_cast<Map::RenderOrder>(value)));
}

void EditableMap::setBackgroundColor(const QColor &value)
{
    push(new ChangeMapProperty(mapDocument(), value));
}

void EditableMap::setLayerDataFormat(LayerDataFormat value)
{
    push(new ChangeMapProperty(mapDocument(), static_cast<Map::LayerDataFormat>(value)));
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
            ScriptManager::instance().throwError(tr("Not a layer"));
            return;
        }
        if (editableLayer->map() != this) {
            ScriptManager::instance().throwError(tr("Layer not from this map"));
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
            ScriptManager::instance().throwError(tr("Not an object"));
            return;
        }
        if (editableMapObject->map() != this) {
            ScriptManager::instance().throwError(tr("Object not from this map"));
            return;
        }

        plainObjects.append(editableMapObject->mapObject());
    }

    document->setSelectedObjects(plainObjects);
}

/**
 * Custom intersects check necessary because QRectF::intersects wants a
 * non-empty area of overlap, but we should also consider overlap with empty
 * area as intersection.
 *
 * Results for rectangles with negative size are undefined.
 */
static bool intersects(const QRectF &a, const QRectF &b)
{
    return a.right() >= b.left() &&
            a.bottom() >= b.top() &&
            a.left() <= b.right() &&
            a.top() <= b.bottom();
}

static bool visibleIn(const QRectF &area, MapObject *object,
                      const MapRenderer &renderer)
{
    QRectF boundingRect = renderer.boundingRect(object);

    if (object->rotation() != 0) {
        // Rotate around object position
        QPointF pos = renderer.pixelToScreenCoords(object->position());
        boundingRect.translate(-pos);

        QTransform transform;
        transform.rotate(object->rotation());
        boundingRect = transform.mapRect(boundingRect);

        boundingRect.translate(pos);
    }

    return intersects(area, boundingRect);
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
    if (size.isEmpty()) {
        ScriptManager::instance().throwError(tr("Invalid size"));
        return;
    }

    const QRegion movedSelection = mapDocument()->selectedArea().translated(offset);
    const QRect newArea = QRect(-offset, size);
    const QRectF visibleArea = renderer()->boundingRect(newArea);

    const QPointF origin = renderer()->tileToPixelCoords(QPointF());
    const QPointF newOrigin = renderer()->tileToPixelCoords(-offset);
    const QPointF pixelOffset = origin - newOrigin;

    // Resize the map and each layer
    QUndoCommand *command = new QUndoCommand(tr("Resize Map"));

    QList<MapObject *> objectsToRemove;

    LayerIterator iterator(map());
    while (Layer *layer = iterator.next()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            TileLayer *tileLayer = static_cast<TileLayer*>(layer);
            new ResizeTileLayer(mapDocument(), tileLayer, size, offset, command);
            break;
        }
        case Layer::ObjectGroupType: {
            ObjectGroup *objectGroup = static_cast<ObjectGroup*>(layer);

            for (MapObject *o : objectGroup->objects()) {
                if (removeObjects && !visibleIn(visibleArea, o, *renderer())) {
                    // Remove objects that will fall outside of the map
                    objectsToRemove.append(o);
                } else {
                    QPointF oldPos = o->position();
                    QPointF newPos = oldPos + pixelOffset;
                    new MoveMapObject(mapDocument(), o, newPos, oldPos, command);
                }
            }
            break;
        }
        case Layer::ImageLayerType: {
            // Adjust image layer by changing its offset
            auto imageLayer = static_cast<ImageLayer*>(layer);
            new SetLayerOffset(mapDocument(), layer,
                               imageLayer->offset() + pixelOffset,
                               command);
            break;
        }
        case Layer::GroupLayerType: {
            // Recursion handled by LayerIterator
            break;
        }
        }
    }

    if (!objectsToRemove.isEmpty())
        new RemoveMapObjects(mapDocument(), objectsToRemove, command);

    new ResizeMap(mapDocument(), size, command);
    new ChangeSelectedArea(mapDocument(), movedSelection, command);

    push(command);

    // TODO: Handle layers that don't match the map size correctly
}

void EditableMap::documentChanged(const ChangeEvent &change)
{
    switch (change.type) {
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

void EditableMap::onCurrentLayerChanged(Layer *)
{
    emit currentLayerChanged();
}

} // namespace Tiled
