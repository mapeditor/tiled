#include "orx_exporter.h"
#include <fstream>

#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

#include <QMessageBox>


// TODO:
// Create gui for options with:
// - fix hierarchy for non shader vuoi adding the check of list value length (8kb) and number of children objects (65535)
// - allow user to select proper technique for each layer
// - implement shader export stuff
// ok - add section name generation expression
// - add folder separator radio button



namespace Orx {


    ///////////////////////////////////////////////////////////////////////////////
    orxExporter::orxExporter()
    {
        m_progress = nullptr;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::Export(const Tiled::Map *map, const QString &fileName)
    {
        // edit select options
        OptionsDialog dlg(map);
        int dlg_res = dlg.exec();
        if (dlg_res == QDialog::Accepted)
        {
            m_ImagesFolder = dlg.m_ImagesFolder.replace('\\', '/');
            m_Optimize = dlg.m_Optimize;
            m_OptimizeHV = dlg.m_OptimizeHV;
            m_SelectedLayers = dlg.m_SelectedLayers;
            m_NamingRule = dlg.m_SectionNameExp;

            return do_export(map, fileName);
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::do_export(const Tiled::Map *map, const QString &fileName)
    {
        bool ret = true;

        m_filename = fileName;

        // prepare destination buffer
        QByteArray array;
        QTextStream ss(&array);

        // compute workload for progress bar
        int num_entities = get_num_entities(map, m_SelectedLayers);
        m_progressCounter = 0;
        m_progress = new QProgressDialog("Exporting...", "Cancel", 0, num_entities);
        m_progress->setWindowModality(Qt::WindowModal);

        // get all tilesets and convert to orx object prefabs with graphic
        m_progress->setLabelText("Processing tilesets...");
        ret = process_tilesets(map);

        // get all layers and build the map object
        if (ret)
        {
            m_progress->setLabelText("Processing layers...");
            ret = process_layers(map, m_SelectedLayers);
        }

        if (ret)
        {
            m_progress->setLabelText("Generating textures...");
            QFileInfo dst_finfo(fileName);
            for (auto obj : m_images)
            {
                if (m_progress->wasCanceled())
                    break;

                if (Utility::copy_image(obj, m_ImagesFolder, dst_finfo))
                    obj->serialize(ss);

                inc_progress();
            }

            m_progress->setLabelText("Generating prefabs...");
            for (auto obj : m_prefabs)
            {
                if (m_progress->wasCanceled())
                    break;

                if (obj->m_UseCount)
                    obj->serialize(ss);

                inc_progress();
            }

            m_progress->setLabelText("Generating objects...");
            int map_offset = map->height() * map->tileHeight();
            for (auto obj : m_objects)
            {
                if (m_progress->wasCanceled())
                    break;

                obj->m_Position.m_Y -= map_offset;

                obj->serialize(ss);
                inc_progress();
            }

            if (m_MapShader != nullptr)
                m_MapShader->serialize(ss);

        }

        if (!m_progress->wasCanceled())
        {
            bool saved = false;
            while (!saved)
            {
                QFile outFile(fileName);
                outFile.open(QIODevice::WriteOnly | QIODevice::Text);
                if(outFile.isOpen())
                {
                    outFile.write(array);
                    outFile.close();
                    saved = true;
                    ret = true;
                }
                else
                {
                QMessageBox Msgbox;
                Msgbox.setText("Unable to save file");
                Msgbox.setStandardButtons(QMessageBox::Abort | QMessageBox::Retry);
                if (Msgbox.exec() == QMessageBox::Abort)
                    saved = true;

                ret = false;
                }
            }
        }

        m_progress->setValue(num_entities);

        return ret;
    }


    ///////////////////////////////////////////////////////////////////////////////
    Orx::PrefabPtr orxExporter::build_prefab(Tiled::Tile * tile, Orx::ImagePtr image, int index, int row, int col, int src_x, int src_y)
    {
        QString base_name = Orx::NameGenerator::Generate(m_NamingRule, image->m_ImageName, index, col, row);
        QString prefab_name = base_name + "_" + PREFAB_POSTFIX;
        QString graphic_name = base_name + "_" + GRAPHIC_POSTFIX;

        Orx::PrefabPtr obj = std::make_shared<Orx::Prefab>(prefab_name);
        obj->m_TiledId = tile->id();
        obj->m_BaseName = base_name;

        obj->m_Graphic = std::make_shared<Orx::Graphic>(graphic_name, image->m_Name);
        obj->m_Graphic->m_Origin.m_X = src_x;
        obj->m_Graphic->m_Origin.m_Y = src_y;

        return obj;
    }

    ///////////////////////////////////////////////////////////////////////////////
    Orx::ImagePtr orxExporter::get_image(QString image_name, QString image_file)
    {
        Orx::ImagePtr ptr;

        auto it = std::find_if(m_images.begin(), m_images.end(), [=] (Orx::ImagePtr obj) { return (obj->m_ImageName == image_name); });
        if (it != m_images.end())
            ptr = *it;
        else
        {
            ptr = std::make_shared<Orx::Image>(image_name, image_file);
            m_images.push_back(ptr);
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
    Orx::ObjectPtr orxExporter::build_object(const Tiled::Cell * cell)
    {
        Orx::ObjectPtr item_obj;

        Tiled::Tile * tile = cell->tile();
        if (tile)
        {
            Orx::PrefabPtr prefab_ptr = get_prefab(tile->id());
            if (prefab_ptr)
            {
                prefab_ptr->m_UseCount++;
                // ok, we have the prefab. create object and place in the map
                QString cell_name = prefab_ptr->m_BaseName + "_" + OBJECT_POSTFIX + "_" + QString::number(prefab_ptr->GetNext());
                item_obj = std::make_shared<Orx::Object>(cell_name, prefab_ptr->m_Name);
                item_obj->m_TiledId = tile->id();
                item_obj->m_Cell = cell;
                item_obj->m_FlipH = cell->flippedHorizontally();
                item_obj->m_FlipV = cell->flippedVertically();
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

            if (prefab_ptr)
            {
                prefab_ptr->m_UseCount++;
                // ok, we have the prefab. create object and place in the map
                QString cell_name = prefab_ptr->m_Name + QString("%1").arg(prefab_ptr->GetNext());
                item_obj = std::make_shared<Orx::Object>(cell_name, prefab_ptr->m_Name);
                item_obj->m_TiledId = tile->id();
                item_obj->m_Position.m_X = map_object->position().x();
                item_obj->m_Position.m_Y = map_object->position().y();
                item_obj->m_Position.m_Z = 0;
                item_obj->m_Rotation = map_object->rotation();
            }
        }

        return item_obj;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_tilesets(const Tiled::Map * map)
    {
        bool ret = true;

        // get each tileset in the map
        Q_FOREACH(Tiled::SharedTileset tset, map->tilesets())
        {
            inc_progress();
            // check if the tileset is a collection of images or uses a single image as atlas
            if (tset->isCollection())
                process_collection_tileset(tset);
            else
                process_tileset(tset);
        }

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////
    Orx::ImagePtr orxExporter::process_tileset(const Tiled::SharedTileset tset)
    {
        // generate a single atlas and use it for all tiles
        Orx::ImagePtr image = get_image(tset->name(), tset->imageSource().toLocalFile());
        image->m_UseCount++;

        image->m_Size.m_X = tset->tileWidth();
        image->m_Size.m_Y = tset->tileHeight();

        int colCount = tset->columnCount();
        int margin = tset->margin();
        int tile_w = tset->tileWidth();
        int tile_h = tset->tileHeight();
        int spacing = tset->tileSpacing();

        // for each tile get its position based on the id.
        int index = 0;
        for(auto e : tset->tiles().toStdMap())
        {
            inc_progress();

            int tile_id = e.first;
            Tiled::Tile * tile = e.second;

            int row = tile_id / colCount;
            int col = tile_id % colCount;

            int tile_x = margin + (col * (tile_w + spacing));
            int tile_y = margin + (row * (tile_h + spacing));

            Orx::PrefabPtr tile_obj = build_prefab(tile, image, index++, row, col, tile_x, tile_y);
            m_prefabs.push_back(tile_obj);
        }

        return image;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_collection_tileset(const Tiled::SharedTileset tset)
    {
        bool ret = true;
        /*
                    // generate all tiles
                    for(auto e : tset->tiles().toStdMap())
                    {
                        int tile_id = e.first;
                        Tiled::Tile * tile = e.second;

                        // generate an atlas for every tile
                        Orx::ImagePtr image = get_image(tile->imageSource().toString());

                        Orx::GraphicPtr tile_graphic = build_tile(tile, image);
                        m_graphics.push_back(tile_graphic);

                        Orx::PrefabPtr tile_obj = build_prefab(get_tile_name(tile), tile_id, tile_graphic);
                        m_prefabs.push_back(tile_obj);
                    }
        */

        return ret;
    }


    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent, bool normal_mode)
    {
        if (normal_mode)
            return process_normal_tile_layer(layer, parent);
        else
            return process_shader_tile_layer(layer, parent);
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_normal_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent)
    {
        bool ret = true;

        // crate layer object, add to objects object and add as child of map object
        QString name = OrxObject::normalize_name(layer->name());
        Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(name);

        std::list<Orx::GroupObjectPtr> targets;

        // compute number of needed object containers as ORX can't hold more than 255 child objects per object
        int width = layer->width();
        int height = layer->height();

        int num_objects = width * height;
        if (num_objects > MAX_OBJECT_CHILDREN)
        {
            int num_holders = num_objects / MAX_OBJECT_CHILDREN;
            if (num_objects % MAX_OBJECT_CHILDREN)
                num_holders++;

            for (int a = 0; a < num_holders; a++)
            {
                Orx::GroupObjectPtr target = std::make_shared<Orx::GroupObject>(name + "_" + QString::number(a));
                targets.push_back(target);
                m_objects.push_back(target);
                layer_object->m_Children.push_back(target);
            }
        }
        else
            targets.push_back(layer_object);


        const double offset_x = layer->offset().rx();
        const double offset_y = layer->offset().ry();

        Orx::GroupObjectPtr current_target = targets.front();
        targets.pop_front();

        // build a vector of optimized objects
        Grid2D<OptimizedCell> cell_map(width, height);

        OptimizeMode optimize_mode;

        if (m_Optimize)
        {
            if (m_OptimizeHV)
                optimize_mode = OptimizeMode::HorizontalVertical;
            else
                optimize_mode = OptimizeMode::VerticalHorizontal;
        }
        else
            optimize_mode = OptimizeMode::None;

        CellOptimizer::optimize_cells(optimize_mode, width, height, cell_map, layer);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                inc_progress();

                OptimizedCell & ocell = cell_map.at(x, y);
                if (ocell.m_Valid)
                {
                    Orx::ObjectPtr item_obj = build_object(ocell.m_Cell);
                    if (item_obj)
                    {
                        item_obj->m_Position.m_X = offset_x + (x * layer->map()->tileWidth());
                        item_obj->m_Position.m_Y = offset_y + (y * layer->map()->tileHeight());
                        item_obj->m_Repeat.m_X = ocell.m_RepeatX;
                        item_obj->m_Repeat.m_Y = ocell.m_RepeatY;
                        item_obj->m_Repeat.m_Z = 1;
                        item_obj->m_Scale.m_X = ocell.m_RepeatX;
                        item_obj->m_Scale.m_Y = ocell.m_RepeatY;
                        item_obj->m_Scale.m_Z = 1;

                        m_objects.push_back(item_obj);

                        if (current_target->m_Children.size() >= MAX_OBJECT_CHILDREN)
                        {
                            current_target = targets.front();
                            targets.pop_front();
                        }

                        current_target->m_Children.push_back(item_obj);
                    }
                }
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
    bool orxExporter::process_shader_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent)
    {
        bool ret = true;

        // crate layer object, add to objects object and add as child of map object
        QString name = OrxObject::normalize_name(layer->name());
        Vector3i tile_size = Vector3i(layer->map()->tileWidth(), layer->map()->tileHeight(), 0);
        Vector3i map_size = Vector3i(layer->map()->width(), layer->map()->height(), 0);

        Orx::ShaderLayerPtr layer_object = std::make_shared<Orx::ShaderLayer>(name, tile_size, map_size);

        // detect how many tilesets are used
        int tilesets_count = layer->usedTilesets().size();
        QVector<Tiled::SharedTileset> used_tilesets;
        // process each tileset and generate
        Q_FOREACH(Tiled::SharedTileset tset, layer->usedTilesets())
        {
            used_tilesets.push_back(tset);
            ImagePtr imgptr = process_tileset(tset);
            imgptr->m_Pivot = "top left";
            layer_object->m_Images.push_back(imgptr);
            layer_object->m_SetSizes.push_back(Vector3i(tset->rowCount(), tset->columnCount(), 0));
        }

        // generate binary map in format tileset_index - tile_index
        layer_object->m_BinMap = generate_binary_map(layer, used_tilesets);

        Orx::ObjectPtr objptr = std::static_pointer_cast<Orx::Object>(layer_object);

        m_objects.push_back(objptr);
        parent->m_Children.push_back(objptr);

        if (m_MapShader == nullptr)
            m_MapShader = std::make_shared<Orx::MapShader>();

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent, bool normal_mode)
    {
        bool ret = true;

        // crate layer object, add to objects object and add as child of map object
        QString name = OrxObject::normalize_name(layer->name());
        Orx::GroupObjectPtr layer_object = std::make_shared<Orx::GroupObject>(name);
        Q_FOREACH(Tiled::MapObject * map_object, layer->objects())
        {
            inc_progress();

            Orx::ObjectPtr item_obj = build_object(map_object);
            if (item_obj)
            {
                m_objects.push_back(item_obj);
                layer_object->m_Children.push_back(item_obj);
            }
    //        else
    //        {
    //            ret = false;
    //            break;
    //        }

        }

        if (layer_object->m_Children.size())
        {
            m_objects.push_back(layer_object);
            parent->m_Children.push_back(layer_object);
        }

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent, bool normal_mode)
    {
        inc_progress();
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool orxExporter::process_layers(const Tiled::Map * map, QVector<OptionsDialog::SelectedLayer> & selected)
    {
        bool ret = true;

        inc_progress();
        // crate map object
        Orx::GroupObjectPtr map_object = std::make_shared<Orx::GroupObject>("Map");
        for (OptionsDialog::SelectedLayer & sl : selected)
        {
            inc_progress();

            if (sl.m_layer->isTileLayer())
                ret = process_tile_layer(sl.m_layer->asTileLayer(), map_object, sl.m_normalExport);
            else if (sl.m_layer->isObjectGroup())
                ret = process_object_group(sl.m_layer->asObjectGroup(), map_object, sl.m_normalExport);
            else if (sl.m_layer->isImageLayer())
                ret = process_image_layer(sl.m_layer->asImageLayer(), map_object, sl.m_normalExport);

            if (!ret)
                break;
        }
        m_objects.push_back(map_object);

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////
    int orxExporter::get_num_entities(const Tiled::Map *map, QVector<OptionsDialog::SelectedLayer> & selected)
    {
        int ret = map->tilesets().size() + map->layers().count();
        Q_FOREACH(Tiled::SharedTileset tset, map->tilesets())
        {
            ret += tset->tileCount();
        }

        for (OptionsDialog::SelectedLayer & sl : selected)
        {
            if (sl.m_layer->isTileLayer())
                ret += 2 * (sl.m_layer->asTileLayer()->width() * sl.m_layer->asTileLayer()->height());
            else if (sl.m_layer->isObjectGroup())
                ret += sl.m_layer->asObjectGroup()->objects().size();
            else if (sl.m_layer->isImageLayer())
                ret += 1;
        }

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////
    void orxExporter::inc_progress()
    {
        m_progress->setValue(++m_progressCounter);
    }

    ///////////////////////////////////////////////////////////////////////////////
    QImage * orxExporter::generate_binary_map(const Tiled::TileLayer * layer, QVector<Tiled::SharedTileset> & used_tilesets)
    {
        // we use 16 indexes so 1 nibble for the tileset index (0...15) and the rest for tile index (0..4095 tiles per tileset)

        int width = layer->width();
        int height = layer->height();
        QSet<Tiled::SharedTileset> tilesets = layer->usedTilesets();

        int binary_map_width = width / 2;
        if (width % 2)
            binary_map_width++;

        int binary_map_height = height;

        QImage * image = new QImage(binary_map_width, binary_map_height, QImage::Format_RGBA8888);
        for (int y = 0; y < height; ++y)
        {
            int cnt = 0;
            for (int x = 0; x < width; ++x)
            {
                Tiled::Cell cell = layer->cellAt(x, y);
                Tiled::Tileset * tileset = cell.tileset();
                Tiled::Tile * tile = cell.tile();
                int tile_index = tile->id();
                int tileset_index = get_tileset_index(used_tilesets, tileset);

                uint16_t comp_value = (tileset_index << 12) | tile_index;

                QColor color = image->pixelColor(x / 2, y);
                if (cnt % 0)
                {
                    color.setRed((comp_value >> 8) & 0xFF);
                    color.setGreen(comp_value & 0xFF);
                }
                else
                {
                    color.setBlue((comp_value >> 8) & 0xFF);
                    color.setAlpha(comp_value & 0xFF);
                }
                image->setPixelColor(x / 2, y, color);
                cnt++;
            }
        }
        return image;
    }

    ///////////////////////////////////////////////////////////////////////////////
    int orxExporter::get_tileset_index(QVector<Tiled::SharedTileset> & used_tilesets, Tiled::Tileset * tset)
    {
        int ret = 0;

        for (ret = 0; ret != used_tilesets.size(); ret++)
            if (used_tilesets[ret].data() == tset)
                break;

        return ret;
    }



}
