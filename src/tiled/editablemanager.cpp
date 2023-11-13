/*
 * editablemanager.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editablemanager.h"

#include "editablegrouplayer.h"
#include "editableimagelayer.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "editablewangset.h"
#include "tilesetdocument.h"
#include "wangset.h"

#include <QQmlEngine>

namespace Tiled {

std::unique_ptr<EditableManager> EditableManager::mInstance;

EditableManager::EditableManager(QObject *parent)
    : QObject(parent)
{
}

EditableManager &EditableManager::instance()
{
    if (!mInstance)
        mInstance.reset(new EditableManager);
    return *mInstance;
}

void EditableManager::deleteInstance()
{
    mInstance.reset();
}

EditableTileset *Tiled::EditableManager::find(Tileset *tileset) const
{
    return static_cast<EditableTileset*>(tileset->editable());
}

EditableLayer *EditableManager::find(Layer *layer) const
{
    return static_cast<EditableLayer*>(layer->editable());
}

EditableMapObject *EditableManager::find(MapObject *mapObject) const
{
    return static_cast<EditableMapObject*>(mapObject->editable());
}

EditableTile *EditableManager::find(Tile *tile) const
{
    return static_cast<EditableTile*>(tile->editable());
}

EditableWangSet *EditableManager::find(WangSet *wangSet) const
{
    return static_cast<EditableWangSet*>(wangSet->editable());
}

void EditableManager::release(Layer *layer)
{
    if (EditableLayer *editable = find(layer))
        editable->hold();
    else
        delete layer;
}

void EditableManager::release(MapObject *mapObject)
{
    if (EditableMapObject *editable = find(mapObject))
        editable->hold();
    else
        delete mapObject;
}

/**
 * Releases the WangSet by either finding an EditableWangSet instance to take
 * ownership of it or deleting it.
 */
void EditableManager::release(std::unique_ptr<WangSet> wangSet)
{
    if (EditableWangSet *editable = find(wangSet.get())) {
        editable->hold();
        wangSet.release();
    }
}

EditableLayer *EditableManager::editableLayer(EditableMap *map, Layer *layer)
{
    if (!layer)
        return nullptr;

    Q_ASSERT(!map || layer->map() == map->map());

    auto editable = find(layer);
    if (!editable) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            editable = new EditableTileLayer(map, static_cast<TileLayer*>(layer));
            break;
        case Layer::ObjectGroupType:
            editable = new EditableObjectGroup(map, static_cast<ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            editable = new EditableImageLayer(map, static_cast<ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            editable = new EditableGroupLayer(map, static_cast<GroupLayer*>(layer));
            break;
        }
    }

    return static_cast<EditableLayer*>(editable);
}

EditableObjectGroup *EditableManager::editableObjectGroup(EditableAsset *asset, ObjectGroup *objectGroup)
{
    if (!objectGroup)
        return nullptr;

    auto editable = find(objectGroup);
    if (!editable)
        editable = new EditableObjectGroup(asset, objectGroup);

    return static_cast<EditableObjectGroup*>(editable);
}

EditableMapObject *EditableManager::editableMapObject(EditableAsset *asset, MapObject *mapObject)
{
    if (!mapObject)
        return nullptr;

    Q_ASSERT(mapObject->objectGroup());

    auto editable = find(mapObject);
    if (!editable)
        editable = new EditableMapObject(asset, mapObject);

    return static_cast<EditableMapObject*>(editable);
}

EditableTileset *EditableManager::editableTileset(Tileset *tileset)
{
    if (!tileset)
        return nullptr;

    if (auto document = TilesetDocument::findDocumentForTileset(tileset->sharedFromThis()))
        return document->editable();

    auto editable = find(tileset);
    if (!editable)
        editable = new EditableTileset(tileset);

    return static_cast<EditableTileset*>(editable);
}

EditableTile *EditableManager::editableTile(Tile *tile)
{
    if (!tile)
        return nullptr;

    EditableTileset *tileset = editableTileset(tile->tileset());
    return editableTile(tileset, tile);
}

EditableTile *EditableManager::editableTile(EditableTileset *tileset, Tile *tile)
{
    Q_ASSERT(tile);
    Q_ASSERT(tile->tileset() == tileset->tileset());

    auto editable = find(tile);
    if (!editable)
        editable = new EditableTile(tileset, tile);

    return static_cast<EditableTile*>(editable);
}

EditableWangSet *EditableManager::editableWangSet(WangSet *wangSet)
{
    if (!wangSet)
        return nullptr;

    EditableTileset *tileset = editableTileset(wangSet->tileset());
    return editableWangSet(tileset, wangSet);
}

EditableWangSet *EditableManager::editableWangSet(EditableTileset *tileset, WangSet *wangSet)
{
    Q_ASSERT(wangSet);
    Q_ASSERT(wangSet->tileset() == tileset->tileset());

    auto editable = find(wangSet);
    if (!editable)
        editable = new EditableWangSet(tileset, wangSet);

    return static_cast<EditableWangSet*>(editable);
}

} // namespace Tiled

#include "moc_editablemanager.cpp"
