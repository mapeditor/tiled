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

#include <QObject>

#include <memory>

namespace Tiled {

class Layer;
class MapObject;
class Object;
class ObjectGroup;
class Tile;
class Tileset;
class WangSet;

class EditableAsset;
class EditableLayer;
class EditableMap;
class EditableMapObject;
class EditableObject;
class EditableObjectGroup;
class EditableTile;
class EditableTileset;
class EditableWangSet;

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
    EditableTileset *find(Tileset *tileset) const;
    EditableWangSet *find(WangSet *wangSet) const;

    void release(Layer *layer);
    void release(MapObject *mapObject);
    void release(std::unique_ptr<WangSet> wangSet);

    EditableLayer *editableLayer(EditableMap *map, Layer *layer);
    EditableObjectGroup *editableObjectGroup(EditableAsset *asset, ObjectGroup *objectGroup);
    EditableMapObject *editableMapObject(EditableAsset *asset, MapObject *mapObject);
    EditableTileset *editableTileset(Tileset *tileset);
    EditableTile *editableTile(Tile *tile);
    EditableTile *editableTile(EditableTileset *tileset, Tile *tile);
    EditableWangSet *editableWangSet(WangSet *wangSet);
    EditableWangSet *editableWangSet(EditableTileset *tileset, WangSet *wangSet);

private:
    friend class EditableLayer;
    friend class EditableMapObject;
    friend class EditableTileset;
    friend class EditableTile;
    friend class EditableWangSet;

    static std::unique_ptr<EditableManager> mInstance;
};

} // namespace Tiled
