#ifndef ORXEXPORTER_H
#define ORXEXPORTER_H

#include "mapformat.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "imagelayer.h"
#include "imagereference.h"
#include "mapobject.h"

#include "orx_objects.h"

namespace Orx {

class orxExporter
{
public:
    orxExporter();

    bool Export(const Tiled::Map *map, const QString &fileName);


private:
    // converts the given absolute filename to a filename relative to destination save directory
    QString get_relative_filename(const QString & filename);
    // returns the name of the given tile
    QString get_tile_name(const Tiled::Tile * tile);
    // builds a tile.
    Orx::GraphicPtr build_tile(Tiled::Tile * tile, Orx::ImagePtr image);
    Orx::PrefabPtr build_prefab(const QString & name, int id, Orx::GraphicPtr graphic);
    Orx::ImagePtr get_image(QString image_file);
    Orx::PrefabPtr get_prefab(int tile_id);
    Orx::GraphicPtr get_graphic(int tile_id);
    Orx::ObjectPtr build_object(const Tiled::Cell * cell);
    Orx::ObjectPtr build_object(const Tiled::MapObject * map_object);
    bool process_tilesets(const Tiled::Map * map);
    bool process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent);
    bool process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent);
    bool process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent);
    bool process_layers(const Tiled::Map * map);

private:
    QString             m_filename;
    Orx::ImagePtrs      m_images;
    Orx::PrefabPtrs     m_prefabs;
    Orx::ObjectPtrs     m_objects;
    Orx::GraphicPtrs    m_graphics;

};

} // namespace Orx


#endif // ORXEXPORTER_H
