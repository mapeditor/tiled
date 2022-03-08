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

#include "documentmanager.h"
#include "editablegrouplayer.h"
#include "editableimagelayer.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "editablewangset.h"
#include "scriptmanager.h"
#include "wangset.h"

#include <QQmlEngine>

#include <QtQml/private/qqmldata_p.h>

namespace Tiled {

template <typename T>
static T *checkNull(T *object)
{
    return QQmlData::wasDeleted(object) ? nullptr : object;
}

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
    return static_cast<EditableTileset*>(checkNull(mEditables.value(tileset)));
}

EditableLayer *EditableManager::find(Layer *layer) const
{
    return static_cast<EditableLayer*>(checkNull(mEditables.value(layer)));
}

EditableMapObject *EditableManager::find(MapObject *mapObject) const
{
    return static_cast<EditableMapObject*>(checkNull(mEditables.value(mapObject)));
}

EditableTile *EditableManager::find(Tile *tile) const
{
    return static_cast<EditableTile*>(checkNull(mEditables.value(tile)));
}

EditableWangSet *EditableManager::find(WangSet *wangSet) const
{
    return static_cast<EditableWangSet*>(checkNull(mEditables.value(wangSet)));
}

void EditableManager::remove(EditableObject *editable)
{
    auto it = mEditables.find(editable->object());
    if (it != mEditables.end() && *it == editable)
        mEditables.erase(it);
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

    EditableObject* &editable = mEditables[layer];
    if (QQmlData::wasDeleted(editable)) {
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
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

    return static_cast<EditableLayer*>(editable);
}

EditableObjectGroup *EditableManager::editableObjectGroup(EditableAsset *asset, ObjectGroup *objectGroup)
{
    if (!objectGroup)
        return nullptr;

    EditableObject* &editable = mEditables[objectGroup];
    if (QQmlData::wasDeleted(editable)) {
        editable = new EditableObjectGroup(asset, objectGroup);
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

    return static_cast<EditableObjectGroup*>(editable);
}

EditableMapObject *EditableManager::editableMapObject(EditableAsset *asset, MapObject *mapObject)
{
    if (!mapObject)
        return nullptr;

    Q_ASSERT(mapObject->objectGroup());

    EditableObject* &editable = mEditables[mapObject];
    if (QQmlData::wasDeleted(editable)) {
        editable = new EditableMapObject(asset, mapObject);
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

    return static_cast<EditableMapObject*>(editable);
}

EditableTileset *EditableManager::editableTileset(Tileset *tileset)
{
    if (!tileset)
        return nullptr;

    if (auto document = TilesetDocument::findDocumentForTileset(tileset->sharedFromThis()))
        return document->editable();

    EditableObject* &editable = mEditables[tileset];
    if (QQmlData::wasDeleted(editable)) {
        editable = new EditableTileset(tileset);
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

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

    EditableObject* &editable = mEditables[tile];
    if (QQmlData::wasDeleted(editable)) {
        editable = new EditableTile(tileset, tile);
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

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

    EditableObject* &editable = mEditables[wangSet];
    if (QQmlData::wasDeleted(editable)) {
        editable = new EditableWangSet(tileset, wangSet);
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
    }

    return static_cast<EditableWangSet*>(editable);
}

} // namespace Tiled

#include "moc_editablemanager.cpp"
