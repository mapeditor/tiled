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

#include "orx_plugin.h"
#include <fstream>


namespace Orx {

///////////////////////////////////////////////////////////////////////////////
OrxPlugin::OrxPlugin()
{
}

///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::normalize_name(const QString & name)
{
    QString ret = name;
    ret.replace(QString(" "), QString("_"));
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::get_tile_name(const Tiled::Tile * tile)
{
    QString tile_filename = tile->imageSource();
    QFileInfo fi(tile_filename);
    return normalize_name(fi.baseName());
}

///////////////////////////////////////////////////////////////////////////////
Orx::GraphicPtr OrxPlugin::build_tile(Tiled::Tile * tile)
{
    QString name = get_tile_name(tile);

    Orx::GraphicPtr tile_graphic = std::make_shared<Orx::Graphic>(name.toStdString() + "Graphic");
    tile_graphic->m_Texture = tile->imageSource().toStdString();
    tile_graphic->m_Origin.m_X = tile->offset().x();
    tile_graphic->m_Origin.m_Y = tile->offset().y();
    tile_graphic->m_Size.m_X = tile->width();
    tile_graphic->m_Size.m_Y = tile->height();
    tile_graphic->m_TiledId = tile->id();

    return tile_graphic;
}

///////////////////////////////////////////////////////////////////////////////
Orx::PrefabPtr OrxPlugin::build_prefab(const std::string & name, int id, GraphicPtr graphic)
{
    Orx::PrefabPtr obj = std::make_shared<Orx::Prefab>(name);
    obj->m_TiledId = id;
    obj->m_Graphic = graphic;
    obj->m_UseCount = 0;
    return obj;
}

///////////////////////////////////////////////////////////////////////////////
Orx::PrefabPtr OrxPlugin::get_prefab(Orx::PrefabPtrs & prefabs, int tile_id)
{
    Orx::PrefabPtr ptr;

    auto it = std::find_if(prefabs.begin(), prefabs.end(), [=] (Orx::PrefabPtr obj) { return (obj->m_TiledId == tile_id); });
    if (it != prefabs.end())
        ptr = *it;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////
Orx::GraphicPtr OrxPlugin::get_graphic(Orx::GraphicPtrs & graphics, int tile_id)
{
    Orx::GraphicPtr ptr;

    auto it = std::find_if(graphics.begin(), graphics.end(), [=] (Orx::GraphicPtr obj) { return (obj->m_TiledId == tile_id); });
    if (it != graphics.end())
        ptr = *it;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////
Orx::ObjectPtr OrxPlugin::build_object(const Tiled::Cell * cell, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics)
{
    Orx::ObjectPtr item_obj;

    Tiled::Tile * tile = cell->tile();
    if (tile)
    {
        Orx::PrefabPtr prefab_ptr = get_prefab(prefabs, tile->id());
        Orx::GraphicPtr graphic_ptr = get_graphic(graphics, tile->id());

        if (prefab_ptr)
        {
            // ok, we have the prefab. create object and place in the map
            std::string cell_name = prefab_ptr->m_name + string_converter<int>::to_string(++prefab_ptr->m_UseCount);
            item_obj = std::make_shared<Orx::Object>(cell_name);
            item_obj->m_parent = prefab_ptr->m_name;
            item_obj->m_TiledId = tile->id();

            if (graphic_ptr)
                item_obj->m_Graphic = graphic_ptr;
        }
    }

    return item_obj;
}

///////////////////////////////////////////////////////////////////////////////
Orx::ObjectPtr OrxPlugin::build_object(const Tiled::MapObject * map_object, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics)
{
    Orx::ObjectPtr item_obj;

    const Tiled::Cell & cell = map_object->cell();
    Tiled::Tile * tile = cell.tile();
    if (tile)
    {
        Orx::PrefabPtr prefab_ptr = get_prefab(prefabs, tile->id());
        Orx::GraphicPtr graphic_ptr = get_graphic(graphics, tile->id());

        if (prefab_ptr)
        {
            // ok, we have the prefab. create object and place in the map
            std::string cell_name = prefab_ptr->m_name + "_" + string_converter<int>::to_string(map_object->id());
            item_obj = std::make_shared<Orx::Object>(cell_name);
            item_obj->m_parent = prefab_ptr->m_name;
            item_obj->m_TiledId = tile->id();
            item_obj->m_Position.m_X = map_object->position().x();
            item_obj->m_Position.m_Y = map_object->position().y();
            item_obj->m_Position.m_Z = 0;
            item_obj->m_Rotation = map_object->rotation();

            if (graphic_ptr)
                item_obj->m_Graphic = graphic_ptr;
        }
    }

    return item_obj;


    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::process_tilesets(const Tiled::Map * map, Orx::PrefabPtrs & prefabs, Orx::GraphicPtrs & graphics)
{
    bool ret = true;

    Q_FOREACH(Tiled::SharedTileset tset, map->tilesets())
    {
        for(auto e : tset->tiles().toStdMap())
        {
            int tile_id = e.first;
            Tiled::Tile * tile = e.second;
            Orx::GraphicPtr tile_graphic = build_tile(tile);
            graphics.push_back(tile_graphic);

            Orx::PrefabPtr tile_obj = build_prefab(get_tile_name(tile).toStdString(), tile_id, tile_graphic);
            prefabs.push_back(tile_obj);
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics)
{
    bool ret = true;

    // crate layer object, add to objects object and add as child of map object
    Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(normalize_name(layer->name()).toStdString());

    // get all items of the layer
    const int width = layer->width();
    const int height = layer->height();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Orx::ObjectPtr item_obj = build_object(&layer->cellAt(x, y), prefabs, graphics);
            if (item_obj)
            {
                objects.push_back(item_obj);
                layer_object->m_Children.push_back(item_obj);
            }
//            else
//            {
//                ret = false;
//                y = height;
//                x = width;
//            }
        }
    }

    if (layer_object->m_Children.size())
    {
        objects.push_back(layer_object);
        map_object->m_Children.push_back(layer_object);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics)
{
    bool ret = true;

    // crate layer object, add to objects object and add as child of map object
    Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(normalize_name(layer->name()).toStdString());
    Q_FOREACH(Tiled::MapObject * map_object, layer->objects())
    {
        Orx::ObjectPtr item_obj = build_object(map_object, prefabs, graphics);
        if (item_obj)
        {
            objects.push_back(item_obj);
            layer_object->m_Children.push_back(item_obj);
        }
        else
        {
            ret = false;
            break;
        }

    }

    if (layer_object->m_Children.size())
    {
        objects.push_back(layer_object);
        map_object->m_Children.push_back(layer_object);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr map_object, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::process_layers(const Tiled::Map * map, Orx::PrefabPtrs & prefabs, Orx::ObjectPtrs & objects, Orx::GraphicPtrs & graphics)
{
    bool ret = true;

    // crate map object
    Orx::GroupObjectPtr map_object = std::make_shared<Orx::GroupObject>("Map");
    for (Tiled::Layer *layer : map->layers())
    {
        if (layer->isTileLayer())
            ret = process_tile_layer(layer->asTileLayer(), map_object, prefabs, objects, graphics);
        else if (layer->isObjectGroup())
            ret = process_object_group(layer->asObjectGroup(), map_object, prefabs, objects, graphics);
        else if (layer->isImageLayer())
            ret = process_image_layer(layer->asImageLayer(), map_object, prefabs, objects, graphics);

        if (!ret)
            break;
    }
    objects.push_back(map_object);

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    bool ret = true;

    std::stringstream ss;
    Orx::PrefabPtrs prefabs;
    Orx::ObjectPtrs objects;
    Orx::GraphicPtrs graphics;

    // get all tilesets and convert to orx object prefabs with graphic
    ret = process_tilesets(map, prefabs, graphics);

    // get all layers and build the map object
    if (ret)
        ret = process_layers(map, prefabs, objects, graphics);

    if (ret)
    {
        for (auto obj : graphics)
            obj->serialize(ss);

        for (auto obj : prefabs)
            obj->serialize(ss);

        for (auto obj : objects)
            obj->serialize(ss);

        std::ofstream outFile;
        outFile.open(fileName.toStdString());
        if (outFile.is_open())
        {
            outFile << ss.rdbuf();
            outFile.close();
        }
        else
            ret = false;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::nameFilter() const
{
    return tr("Orx Engine files (*.ini)");
}

///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::errorString() const
{
    return mError;
}


} //namespace Orx
