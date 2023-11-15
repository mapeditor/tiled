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

    static EditableLayer *find(Layer *layer);
    static EditableMapObject *find(MapObject *mapObject);
    static EditableTile *find(Tile *tile);
    static EditableTileset *find(Tileset *tileset);
    static EditableWangSet *find(WangSet *wangSet);

    static void release(Layer *layer);
    static void release(MapObject *mapObject);
    static void release(std::unique_ptr<WangSet> wangSet);

    static EditableLayer *editableLayer(EditableMap *map, Layer *layer);
    static EditableObjectGroup *editableObjectGroup(EditableAsset *asset, ObjectGroup *objectGroup);
    static EditableMapObject *editableMapObject(EditableAsset *asset, MapObject *mapObject);
    static EditableTileset *editableTileset(Tileset *tileset);
    static EditableTile *editableTile(Tile *tile);
    static EditableTile *editableTile(EditableTileset *tileset, Tile *tile);
    static EditableWangSet *editableWangSet(WangSet *wangSet);
    static EditableWangSet *editableWangSet(EditableTileset *tileset, WangSet *wangSet);

private:
    static std::unique_ptr<EditableManager> mInstance;
};

} // namespace Tiled
