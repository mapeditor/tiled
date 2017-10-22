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

#include <QApplication>
#include <QProgressDialog>

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
    // builds a prefab object from a Tile in the tileset.
    Orx::PrefabPtr build_prefab(Tiled::Tile * tile, Orx::ImagePtr image, int row, int col, int src_x, int src_y);
    // returns an new or existing image shared pointer (a Texture object) from the file name
    Orx::ImagePtr get_image(QString image_name, QString image_file);
    // gets a prefab by its tile id
    Orx::PrefabPtr get_prefab(int tile_id);
    // gets a graphic by its tile id
    Orx::GraphicPtr get_graphic(int tile_id);
    // builds an object for a cell (it discovers the prefab to use)
    Orx::ObjectPtr build_object(const Tiled::Cell * cell);
    // builds a special map object (a shape)
    Orx::ObjectPtr build_object(const Tiled::MapObject * map_object);
    // processes all tilesets and generates prefabs
    bool process_tilesets(const Tiled::Map * map);
    // now does nothing...:)
    bool process_collection_tileset(const Tiled::SharedTileset tset);
    // generated prefabs from a tilesed
    bool process_tileset(const Tiled::SharedTileset tset);
    // processes a tile layer and generates objects
    bool process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent);
    // now does nothing...:)
    bool process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent);
    // now does nothing...:)
    bool process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent);
    // processes all layers to generate map contents
    bool process_layers(const Tiled::Map * map);
    // computes the number of entities that will be processes (for progress dlg)
    int get_num_entities(const Tiled::Map *map);
    // increments the progress indication in progress dlg
    void inc_progress();
    // computes the number of times a cell is repeated horizontally starting from given coords
    int get_h_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_x);
    // computes the number of times a cell is repeated vertically starting from given coords
    int get_v_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_y);

private:
    QString             m_filename;
    Orx::ImagePtrs      m_images;
    Orx::PrefabPtrs     m_prefabs;
    Orx::ObjectPtrs     m_objects;
    QProgressDialog *   m_progress;
    int                 m_progressCounter;
};

} // namespace Orx


#endif // ORXEXPORTER_H
