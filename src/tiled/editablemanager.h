/*
 * editablemanager.h
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

#pragma once

#include <QHash>
#include <QObject>

#include <memory>

namespace Tiled {

class Layer;
class MapObject;
class ObjectGroup;
class Terrain;
class Tile;

class EditableAsset;
class EditableLayer;
class EditableMap;
class EditableMapObject;
class EditableObjectGroup;
class EditableTerrain;
class EditableTile;
class EditableTileset;

/**
 * Manages editable wrappers that are used to expose properties to scripts.
 */
class EditableManager : public QObject
{
    Q_OBJECT

    explicit EditableManager(QObject *parent = nullptr);

public:
    static EditableManager &instance();
    static void deleteInstance();

    EditableLayer *find(Layer *layer) const;
    EditableMapObject *find(MapObject *mapObject) const;
    EditableTile *find(Tile *tile) const;
    EditableTerrain *find(Terrain *terrain) const;

    void release(Layer *layer);
    void release(MapObject *mapObject);

    EditableLayer *editableLayer(EditableMap *map, Layer *layer);
    EditableObjectGroup *editableObjectGroup(EditableAsset *asset, ObjectGroup *objectGroup);
    EditableMapObject *editableMapObject(EditableAsset *asset, MapObject *mapObject);
    EditableTile *editableTile(EditableTileset *tileset, Tile *tile);
    EditableTerrain *editableTerrain(EditableTileset *tileset, Terrain *terrain);

private:
    friend class EditableLayer;
    friend class EditableMapObject;
    friend class EditableTile;
    friend class EditableTerrain;

    QHash<Layer*, EditableLayer*> mEditableLayers;
    QHash<MapObject*, EditableMapObject*> mEditableMapObjects;
    QHash<Tile*, EditableTile*> mEditableTiles;
    QHash<Terrain*, EditableTerrain*> mEditableTerrains;

    static std::unique_ptr<EditableManager> mInstance;
};


inline EditableLayer *EditableManager::find(Layer *layer) const
{
    return mEditableLayers.value(layer);
}

inline EditableMapObject *EditableManager::find(MapObject *mapObject) const
{
    return mEditableMapObjects.value(mapObject);
}

inline EditableTile *EditableManager::find(Tile *tile) const
{
    return mEditableTiles.value(tile);
}

inline EditableTerrain *EditableManager::find(Terrain *terrain) const
{
    return mEditableTerrains.value(terrain);
}

} // namespace Tiled
