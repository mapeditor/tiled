/*
 * Orx Engine Tiled Plugin
 * Copyright 2017, Denis Brachet aka Ainvar <thegwydd@gmail.com>
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

#ifndef ORX_PLUGIN_H
#define ORX_PLUGIN_H

#include "orx_global.h"

#include "mapformat.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "imagelayer.h"
#include "mapobject.h"

#include <QDataStream>
#include <QFileInfo>

#include "orx_objects.h"

#include <QObject>

namespace Orx {

class ORX_SHARED_EXPORT OrxPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    OrxPlugin();

    bool write(const Tiled::Map *map, const QString &fileName) override;
    QString nameFilter() const override;
    QString errorString() const override;

    static QString normalize_name(const QString & name);
    static QString get_tile_name(const Tiled::Tile * tile);
    static Orx::GraphicPtr build_tile(Tiled::Tile * tile);
    static Orx::PrefabPtr build_prefab(const std::string & name, int id, Orx::GraphicPtr graphic);
    static Orx::PrefabPtr get_prefab(Orx::PrefabPtrs & prefabs, int tile_id);
    static Orx::GraphicPtr get_graphic(Orx::GraphicPtrs & graphics, int tile_id);
    static Orx::ObjectPtr build_object(const Tiled::Cell * cell, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics);
    static Orx::ObjectPtr build_object(const Tiled::MapObject * map_object, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics);
    static bool process_tilesets(const Tiled::Map * map, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics);
    static bool process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics);
    static bool process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics);
    static bool process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics);
    static bool process_layers(const Tiled::Map * map, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics);



private:
    QString mError;
};

} // namespace Orx

#endif // ORX_PLUGIN_H
