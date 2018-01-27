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
#include "orx_utility.h"
#include "orx_cell_optimizer.h"

#include "optionsdialog.h"

#include <QApplication>
#include <QProgressDialog>

namespace Orx {

class orxExporter
{
public:
    orxExporter();

    bool Export(const Tiled::Map *map, const QString &fileName);

private:
    // perform the export
    bool do_export(const Tiled::Map *map, const QString &fileName);
    // builds a prefab object from a Tile in the tileset.
    Orx::PrefabPtr build_prefab(Tiled::Tile * tile, Orx::ImagePtr image, int index, int row, int col, int src_x, int src_y);
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
    Orx::ImagePtr process_tileset(const Tiled::SharedTileset tset);
    // processes a tile layer and generates objects
    bool process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent, bool normal_mode);
    // processes a tile layer and generates objects
    bool process_normal_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent);
    // processes a tile layer and generates objects
    bool process_shader_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent);
    // now does nothing...:)
    bool process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent, bool normal_mode);
    // now does nothing...:)
    bool process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent, bool normal_mode);
    // processes all layers to generate map contents
    bool process_layers(const Tiled::Map * map, QVector<OptionsDialog::SelectedLayer> & selected);
    // computes the number of entities that will be processes (for progress dlg)
    int get_num_entities(const Tiled::Map *map, QVector<OptionsDialog::SelectedLayer> & selected);
    // increments the progress indication in progress dlg
    void inc_progress();
    // generates a binary map into a QPixmap
    QImage * generate_binary_map(const Tiled::TileLayer * layer, QVector<Tiled::SharedTileset> & used_tilesets);
    // gets the index of the tileset in the given array
    int get_tileset_index(QVector<Tiled::SharedTileset> & used_tilesets, Tiled::Tileset * tset);
    // get num of tilesets used in one layer
    int count_used_tilesets(const Tiled::TileLayer * layer) { return layer->usedTilesets().size(); }


private:
    // destination filename
    QString                 m_filename;
    // collection of images/textures
    Orx::ImagePtrs          m_images;
    // collection of prefabs (tile model)
    Orx::PrefabPtrs         m_prefabs;
    // collection of game objects (tile instance)
    Orx::ObjectPtrs         m_objects;
    // collection of level layers
    Orx::ShaderLayerPtrs    m_ShaderLayers;
    // the map shader entity
    Orx::MapShaderPtr       m_MapShader;

    QProgressDialog *       m_progress;
    int                     m_progressCounter;
    QString                 m_ImagesFolder;
    QVector<OptionsDialog::SelectedLayer> m_SelectedLayers;
    bool                    m_Optimize;
    bool                    m_OptimizeHV;
    QString                 m_NamingRule;
    bool                    m_GenerateShaderCode;


};

} // namespace Orx


#endif // ORXEXPORTER_H
