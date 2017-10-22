#include "orx_exporter.h"
#include <fstream>

#include <QDataStream>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>

#include <QMessageBox>


// TODO:
// Create gui for options with:
// - image destination folder
// - select layers to export
// - add map objects to export as
//          .name
//          .type
//          .points
// - toggle cell repeat/scale optimization
//   - toggle optimization order (x or y preference - now is x and y is missing)



namespace Orx {


///////////////////////////////////////////////////////////////////////////////
orxExporter::orxExporter()
{
    m_progress = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::Export(const Tiled::Map *map, const QString &fileName)
{
    bool ret = true;

    m_filename = fileName;

    SerializationContext context;

    QByteArray array;
    QTextStream ss(&array);

    //QApplication::setOverrideCursor(Qt::WaitCursor);

    int num_entities = get_num_entities(map);

    m_progressCounter = 0;
    m_progress = new QProgressDialog("Exporting...", "Cancel", 0, num_entities);
    m_progress->setWindowModality(Qt::WindowModal);

    QFileInfo dst_finfo(fileName);

    m_progress->setLabelText("Processing tilesets...");

    // get all tilesets and convert to orx object prefabs with graphic
    ret = process_tilesets(map);

    m_progress->setLabelText("Processing layers...");

    // get all layers and build the map object
    if (ret)
        ret = process_layers(map);

    if (ret)
    {
        m_progress->setLabelText("Generating textures...");

        for (auto obj : m_images)
        {
            if (m_progress->wasCanceled())
                break;

            if (obj->m_UseCount)
            {
                QFileInfo src_finfo(obj->m_Texture);
                QString fn = src_finfo.fileName();
                QFile::copy(obj->m_Texture, dst_finfo.dir().absolutePath() + QDir::separator() + fn);
                obj->m_Texture = fn;

                obj->serialize(context, ss);
            }
            inc_progress();
        }

        m_progress->setLabelText("Generating prefabs...");

        for (auto obj : m_prefabs)
        {
            if (m_progress->wasCanceled())
                break;

            if (obj->m_UseCount)
                obj->serialize(context, ss);

            inc_progress();
        }

        m_progress->setLabelText("Generating objects...");

        int map_offset = map->height() * map->tileHeight();

        for (auto obj : m_objects)
        {
            if (m_progress->wasCanceled())
                break;

            obj->m_Position.m_Y -= map_offset;

            obj->serialize(context, ss);
            inc_progress();
        }
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
    QString tile_filename;
    if (tile->imageSource().isEmpty() == false)
        tile_filename = tile->imageSource().toString();
    else
        tile_filename = tile->tileset()->imageSource().toString();

    QFileInfo fi(tile_filename);
    tile_filename = fi.baseName();
    tile_filename = OrxObject::normalize_name(tile_filename);
    return tile_filename;
}

///////////////////////////////////////////////////////////////////////////////
Orx::PrefabPtr orxExporter::build_prefab(Tiled::Tile * tile, Orx::ImagePtr image, int row, int col, int src_x, int src_y)
{
    QString base_name = QString("%1_c%2r%3")
                            .arg(image->m_ImageName)
                            .arg(QString::number(col))
                            .arg(QString::number(row));
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
bool orxExporter::process_tileset(const Tiled::SharedTileset tset)
{
    bool ret = true;

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
    for(auto e : tset->tiles().toStdMap())
    {
        inc_progress();

        int tile_id = e.first;
        Tiled::Tile * tile = e.second;

        int row = tile_id / colCount;
        int col = tile_id % colCount;

        int tile_x = margin + (col * (tile_w + spacing));
        int tile_y = margin + (row * (tile_h + spacing));

        Orx::PrefabPtr tile_obj = build_prefab(tile, image, row, col, tile_x, tile_y);
        m_prefabs.push_back(tile_obj);
    }

    return ret;
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
int orxExporter::get_h_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_x)
{
    int count = 0;
    for (; x <= max_x; ++x)
        {
        const Tiled::Cell * cell = &layer->cellAt(x, y);
        if (*cell == *value)
            count++;
        else
            break;
        }

    return count;
}

///////////////////////////////////////////////////////////////////////////////
int orxExporter::get_v_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_y)
{
    int count = 0;
    for (; y <= max_y; ++y)
        {
        const Tiled::Cell * cell = &layer->cellAt(x, y);
        if (*cell == *value)
            count++;
        else
            break;
        }

    return count;
}


///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_tile_layer(const Tiled::TileLayer * layer, Orx::ObjectPtr parent)
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

    // perform horizontal optimization
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; /*++x*/)
        {
            const Tiled::Cell * cell = &layer->cellAt(x, y);
            int rep = get_h_repetitions(layer, x, y, cell, width);
            OptimizedCell & ocell = cell_map.at(x, y);
            ocell.m_Valid = true;
            ocell.m_RepeatX = rep;
            ocell.m_RepeatY = 1;
            ocell.m_Cell = cell;
            x += rep;
        }
    }

    // perform vertical optimization
    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; /*++y*/)
        {
            const Tiled::Cell * cell = &layer->cellAt(x, y);
            OptimizedCell & ocell = cell_map.at(x, y);
            if (ocell.m_Valid && (cell_map.at(x, y).m_RepeatX == 1))
            {
                int rep = get_v_repetitions(layer, x, y, cell, width);
                ocell.m_Valid = true;
                ocell.m_RepeatY = rep;
                ocell.m_Cell = cell;
                y += rep;
            }
            else
                y++;
        }
    }

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
bool orxExporter::process_object_group(const Tiled::ObjectGroup * layer, Orx::ObjectPtr parent)
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
bool orxExporter::process_image_layer(const Tiled::ImageLayer * layer, Orx::ObjectPtr parent)
{
    inc_progress();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool orxExporter::process_layers(const Tiled::Map * map)
{
    bool ret = true;

    inc_progress();
    // crate map object
    Orx::GroupObjectPtr map_object = std::make_shared<Orx::GroupObject>("Map");
    for (Tiled::Layer *layer : map->layers())
    {
        inc_progress();

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

///////////////////////////////////////////////////////////////////////////////
int orxExporter::get_num_entities(const Tiled::Map *map)
{
    int ret = map->tilesets().size() + map->layers().count();
    Q_FOREACH(Tiled::SharedTileset tset, map->tilesets())
    {
        ret += tset->tileCount();
    }

    for (Tiled::Layer *layer : map->layers())
    {
        if (layer->isTileLayer())
            ret += 2 * (layer->asTileLayer()->width() * layer->asTileLayer()->height());
        else if (layer->isObjectGroup())
            ret += layer->asObjectGroup()->objects().size();
        else if (layer->isImageLayer())
            ret += 1;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
void orxExporter::inc_progress()
{
    m_progress->setValue(++m_progressCounter);
}





}
