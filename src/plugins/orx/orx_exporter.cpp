#include "orx_exporter.h"
#include <fstream>

#include <QDataStream>
#include <QFileInfo>
#include <QDir>

namespace Orx {


///////////////////////////////////////////////////////////////////////////////
orxExporter::orxExporter()
{
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::Export(const Tiled::Map *map, const QString &fileName)
{
    bool ret = true;

    m_filename = fileName;

    std::stringstream ss;
    // get all tilesets and convert to orx object prefabs with graphic
    ret = process_tilesets(map);

    // get all layers and build the map object
    if (ret)
        ret = process_layers(map);

    if (ret)
    {
        for (auto obj : m_images)
            obj->serialize(ss);

        for (auto obj : m_graphics)
            obj->serialize(ss);

        for (auto obj : m_prefabs)
            obj->serialize(ss);

        for (auto obj : m_objects)
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
QString orxExporter::get_relative_filename(const QString & filename)
{
    QFileInfo fi(m_filename);
    QDir dir(fi.dir());
    QString s = dir.relativeFilePath(filename);
    return s;
}

///////////////////////////////////////////////////////////////////////////////
QString orxExporter::get_tile_name(const Tiled::Tile * tile)
{
    QString tile_filename = tile->imageSource();
    QFileInfo fi(tile_filename);
    return QString(normalize_name(fi.baseName().toStdString()).c_str());
}

///////////////////////////////////////////////////////////////////////////////
Orx::GraphicPtr orxExporter::build_tile(Tiled::Tile * tile, Orx::ImagePtr image)
{
    QString name = get_tile_name(tile);

    Orx::GraphicPtr tile_graphic = std::make_shared<Orx::Graphic>(image);
    tile_graphic->m_Origin.m_X = tile->offset().x();
    tile_graphic->m_Origin.m_Y = tile->offset().y();
    tile_graphic->m_Size.m_X = tile->width();
    tile_graphic->m_Size.m_Y = tile->height();
    tile_graphic->m_TiledId = tile->id();

    return tile_graphic;
}

///////////////////////////////////////////////////////////////////////////////
Orx::PrefabPtr orxExporter::build_prefab(const std::string & name, int id, GraphicPtr graphic)
{
    Orx::PrefabPtr obj = std::make_shared<Orx::Prefab>(name);
    obj->m_TiledId = id;
    obj->m_Graphic = graphic;
    obj->m_UseCount = 0;
    return obj;
}

///////////////////////////////////////////////////////////////////////////////
Orx::ImagePtr orxExporter::get_image(std::string image_file)
{
    Orx::ImagePtr ptr;

    auto it = std::find_if(m_images.begin(), m_images.end(), [=] (Orx::ImagePtr obj) { return (obj->m_Texture == image_file); });
    if (it != m_images.end())
        ptr = *it;
    else
    {
        ptr = std::make_shared<Orx::Image>(image_file);
    }

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////
Orx::PrefabPtr orxExporter::get_prefab(int tile_id)
{
    Orx::PrefabPtr ptr;

    auto it = std::find_if(m_prefabs.begin(), m_prefabs.end(), [=] (Orx::PrefabPtr obj) { return (obj->m_TiledId == tile_id); });
    if (it != m_prefabs.end())
        ptr = *it;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////
Orx::GraphicPtr orxExporter::get_graphic(int tile_id)
{
    Orx::GraphicPtr ptr;

    auto it = std::find_if(m_graphics.begin(), m_graphics.end(), [=] (Orx::GraphicPtr obj) { return (obj->m_TiledId == tile_id); });
    if (it != m_graphics.end())
        ptr = *it;

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////
Orx::ObjectPtr orxExporter::build_object(const Tiled::Cell * cell)
{
    Orx::ObjectPtr item_obj;

    Tiled::Tile * tile = cell->tile();
    if (tile)
    {
        Orx::PrefabPtr prefab_ptr = get_prefab(tile->id());
        Orx::GraphicPtr graphic_ptr = get_graphic(tile->id());

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
Orx::ObjectPtr orxExporter::build_object(const Tiled::MapObject * map_object)
{
    Orx::ObjectPtr item_obj;

    const Tiled::Cell & cell = map_object->cell();
    Tiled::Tile * tile = cell.tile();
    if (tile)
    {
        Orx::PrefabPtr prefab_ptr = get_prefab(tile->id());
        Orx::GraphicPtr graphic_ptr = get_graphic(tile->id());

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
bool orxExporter::process_tilesets(const Tiled::Map * map)
{
    bool ret = true;

    Q_FOREACH(Tiled::SharedTileset tset, map->tilesets())
    {
        for(auto e : tset->tiles().toStdMap())
        {

            get the name of the tile
            build the name of the image
            build the name of the graphic@image
            build the name of the prefab

            int tile_id = e.first;
            Tiled::Tile * tile = e.second;

            Orx::ImagePtr image = get_image(tset->imageSource().toStdString());

            Orx::GraphicPtr tile_graphic = build_tile(tile, image);
            m_graphics.push_back(tile_graphic);

            Orx::PrefabPtr tile_obj = build_prefab(get_tile_name(tile).toStdString(), tile_id, tile_graphic);
            m_prefabs.push_back(tile_obj);
        }
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent)
{
    bool ret = true;

    // crate layer object, add to objects object and add as child of map object
    Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(normalize_name(layer->name().toStdString()));

    // get all items of the layer
    const int width = layer->width();
    const int height = layer->height();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Orx::ObjectPtr item_obj = build_object(&layer->cellAt(x, y));
            if (item_obj)
            {
                m_objects.push_back(item_obj);
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
        m_objects.push_back(layer_object);
        parent->m_Children.push_back(layer_object);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent)
{
    bool ret = true;

    // crate layer object, add to objects object and add as child of map object
    Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(normalize_name(layer->name().toStdString()));
    Q_FOREACH(Tiled::MapObject * map_object, layer->objects())
    {
        Orx::ObjectPtr item_obj = build_object(map_object);
        if (item_obj)
        {
            m_objects.push_back(item_obj);
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
        m_objects.push_back(layer_object);
        parent->m_Children.push_back(layer_object);
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_layers(const Tiled::Map * map)
{
    bool ret = true;

    // crate map object
    Orx::GroupObjectPtr map_object = std::make_shared<Orx::GroupObject>("Map");
    for (Tiled::Layer *layer : map->layers())
    {
        if (layer->isTileLayer())
            ret = process_tile_layer(layer->asTileLayer(), map_object);
        else if (layer->isObjectGroup())
            ret = process_object_group(layer->asObjectGroup(), map_object);
        else if (layer->isImageLayer())
            ret = process_image_layer(layer->asImageLayer(), map_object);

        if (!ret)
            break;
    }
    m_objects.push_back(map_object);

    return ret;
}


}
