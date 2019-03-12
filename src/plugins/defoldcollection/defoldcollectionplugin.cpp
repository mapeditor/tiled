/*
 * DefoldCollection Tiled Plugin
 * Copyright 2019, CodeSpartan
 * Based on Defold Tiled Plugin by Nikita Razdobreev and Thorbjørn Lindeijer
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

#include "defoldcollectionplugin.h"

#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "grouplayer.h"

#include <QTextStream>
#include <QFileInfo>
#include <QDir>

#include <cmath>

namespace DefoldCollection {

static const char cell_t[] =
"  cell {\n\
    x: {{x}}\n\
    y: {{y}}\n\
    tile: {{tile}}\n\
    h_flip: {{h_flip}}\n\
    v_flip: {{v_flip}}\n\
  }\n";

static const char layer_t[] =
"layers {\n\
  id: \"{{id}}\"\n\
  z: {{z}}\n\
  is_visible: {{is_visible}}\n\
{{cells}}\
}\n";

static const char map_t[] =
"tile_set: \"{{tile_set}}\"\n\
{{layers}}\n\
material: \"{{material}}\"\n\
blend_mode: {{blend_mode}}\n";

static const char collection_t[] =
"name: \"default\"\n\
scale_along_z: 0\n\
embedded_instances {\n\
  id: \"tilemaps\"\n\
{{children}}\
  data: {{components}}\n\
  \"\"\n\
  position {\n\
    x: {{pos-x}}\n\
    y: {{pos-y}}\n\
    z: 0.0\n\
  }\n\
  rotation {\n\
    x: 0.0\n\
    y: 0.0\n\
    z: 0.0\n\
    w: 1.0\n\
  }\n\
  scale3 {\n\
    x: 1.0\n\
    y: 1.0\n\
    z: 1.0\n\
  }\n\
}\n\
{{embedded-instances}}\n\
\n";

static const char component_t[] =
"  \"components {\\n\"\n\
  \"  id: \\\"{{tilemap_name}}\\\"\\n\"\n\
  \"  component: \\\"{{tilemap_rel_path}}\\\"\\n\"\n\
  \"  position {\\n\"\n\
  \"    x: 0.0\\n\"\n\
  \"    y: 0.0\\n\"\n\
  \"    z: 0.0\\n\"\n\
  \"  }\\n\"\n\
  \"  rotation {\\n\"\n\
  \"    x: 0.0\\n\"\n\
  \"    y: 0.0\\n\"\n\
  \"    z: 0.0\\n\"\n\
  \"    w: 1.0\\n\"\n\
  \"  }\\n\"\n\
  \"}\\n\"\n\
";

static const char child_t[] =
"  children: \"{{child-name}}\"\n\
";

static const char emdedded_instance_t[] =
"embedded_instances {\n\
  id: \"{{instance-name}}\"\n\
  data: {{components}}\n\
  \"\"\n\
  position {\n\
    x: 0.0\n\
    y: 0.0\n\
    z: 0.0\n\
  }\n\
  rotation {\n\
    x: 0.0\n\
    y: 0.0\n\
    z: 0.0\n\
    w: 1.0\n\
  }\n\
  scale3 {\n\
    x: 1.0\n\
    y: 1.0\n\
    z: 1.0\n\
  }\n\
}\n\
";

static QString replaceTags(QString context, const QVariantHash &map)
{
    QHashIterator<QString,QVariant> it{map};
    while (it.hasNext()) {
        it.next();
        context.replace(QLatin1String("{{") + it.key() + QLatin1String("}}"),
                        it.value().toString());
    }
    return context;
}

DefoldCollectionPlugin::DefoldCollectionPlugin()
{
}

QString DefoldCollectionPlugin::nameFilter() const
{
    return tr("Defold collection (*.collection)");
}

QString DefoldCollectionPlugin::shortName() const
{
    return QLatin1String("defoldcollection");
}

QString DefoldCollectionPlugin::errorString() const
{
    return mError;
}

/*
 * Returns a new filepath relative to the root of the Defold project if we're in one.
 * Determines the root of the project by looking for a file called "game.project".
 * If no such file is found by going up the hierarchy, return filename from the \a filePath.
*/
QString DefoldCollectionPlugin::TilesetRelativePath(QString filePath)
{
    QString gameproject = "/game.project";
    QFileInfo fi(filePath);
    QDir Qd = fi.dir();

    while (Qd.exists() && !Qd.isRoot())
    {
        if (QFileInfo::exists(Qd.path() + gameproject))
        {
            // return relative path
            filePath.replace(Qd.path(), "");
            return filePath;
        }
        else if (!Qd.cdUp())
        {
            break;
        }
    }
    return fi.fileName();
}

bool DefoldCollectionPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    QFileInfo fi(fileName);
    QString outputFilePath = fi.filePath();
    QString outputFileName = fi.fileName();

    QString mapName = outputFileName;
    mapName.replace(".collection", "");

    QString tilesetFileDir = outputFilePath;
    tilesetFileDir.chop(outputFileName.length());

    // if there are no group layers, all tilemaps are components of the top gameobject called "tilemaps"
    if (map->groupLayerCount() == 0)
    {
        QString components;

        // create a tilemap for each tileset this map uses
        for (auto& tileset_current : map->tilesets())
        {
            QString tilesetFilePath = tilesetFileDir;
            tilesetFilePath.append(mapName + "-" + tileset_current->name() + ".tilemap");

            QVariantHash component_h;
            component_h["tilemap_name"] = mapName + "-" + tileset_current->name();
            component_h["tilemap_rel_path"] = TilesetRelativePath(tilesetFilePath);
            components.append(replaceTags(QLatin1String(component_t), component_h));

            QVariantHash map_h;

            QString layers;            
            Tiled::LayerIterator it(map, Tiled::Layer::TileLayerType);
            int layerOrder = 0;
            while (auto tileLayer = static_cast<Tiled::TileLayer*>(it.next()))
            {
                QVariantHash layer_h;
                layer_h["id"] = tileLayer->name();
                float zIndex = qBound(0, layerOrder, 1000) * 0.00000001f;
                layer_h["z"] = zIndex;
                layer_h["is_visible"] = tileLayer->isVisible() ? 1 : 0;
                QString cells;

                for (int x = 0; x < tileLayer->width(); ++x)
                {
                    for (int y = 0; y < tileLayer->height(); ++y)
                    {
                        const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                        if (cell.isEmpty() || cell.tileset() != tileset_current) // skip cell if it doesn't belong to current tileset
                            continue;
                        QVariantHash cell_h;
                        cell_h["x"] = x;
                        cell_h["y"] = tileLayer->height() - y - 1;
                        cell_h["tile"] = cell.tileId();
                        cell_h["h_flip"] = cell.flippedHorizontally() ? 1 : 0;
                        cell_h["v_flip"] = cell.flippedVertically() ? 1 : 0;
                        cells.append(replaceTags(QLatin1String(cell_t), cell_h));
                    }
                }
                layer_h["cells"] = cells;
                layers.append(replaceTags(QLatin1String(layer_t), layer_h));
                layerOrder++;
            }
            map_h["layers"] = layers;
            map_h["material"] = "/builtins/materials/tile_map.material";
            map_h["blend_mode"] = "BLEND_MODE_ALPHA";
            // Below, we input a value that's not necessarily correct in Defold, but it lets the user know what tilesource to link this tilemap with manually.
            // However, if the user keeps all tilesources in /tilesources/ and the name of the tilesource corresponds with the name of the tileset in Defold,
            // the value will be automatically correct.
            map_h["tile_set"] = "/tilesources/" + tileset_current->name() + ".tilesource";

            QString result = replaceTags(QLatin1String(map_t), map_h);
            Tiled::SaveFile mapFile(tilesetFilePath);
            if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                mError = tr("Could not open file for writing.");
                return false;
            }
            QTextStream stream(mapFile.device());
            stream << result;

            if (mapFile.error() != QFileDevice::NoError)
            {
                mError = mapFile.errorString();
                return false;
            }

            if (!mapFile.commit())
            {
                mError = mapFile.errorString();
                return false;
            }
        }

        QVariantHash collection_h;
        collection_h["children"] = QString("");
        collection_h["embedded-instances"] = QString("");
        // optional custom properties that allow exporting a map with an offset
        collection_h["pos-x"] = map->property(QLatin1String("x-offset")).toInt();
        collection_h["pos-y"] = map->property(QLatin1String("y-offset")).toInt();
        collection_h["components"] = components;        

        QString result = replaceTags(QLatin1String(collection_t), collection_h);
        Tiled::SaveFile mapFile(fileName);
        if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            mError = tr("Could not open file for writing.");
            return false;
        }
        QTextStream stream(mapFile.device());
        stream << result;

        if (mapFile.error() != QFileDevice::NoError)
        {
            mError = mapFile.errorString();
            return false;
        }

        if (!mapFile.commit()) {
            mError = mapFile.errorString();
            return false;
        }
    }
    // if there are group layers, each group layer is a gameobject, parented to the top gameobject, while being a parent to multiple tilemap components
    else
    {
        QString children;
        QString embeddedInstances;

        // foreach Group Layer
        int groupLayerOrder = 0;
        for (auto& layer : map->layers())
        {
            if (layer->layerType() != Tiled::Layer::GroupLayerType)
                continue;

            QVariantHash child_h;
            child_h["child-name"] = layer->name();
            children.append(replaceTags(QLatin1String(child_t), child_h));

            QVariantHash emdedded_instance_h;
            emdedded_instance_h["instance-name"] = layer->name();

            QString components;

            // write as many tilemaps as there are tilesets per group layer
            for (auto& tileset_current : map->tilesets())
            {
                QString tilesetFilePath = tilesetFileDir;
                tilesetFilePath.append(mapName + "-" + layer->name() + "-" + tileset_current->name() + ".tilemap");

                QVariantHash map_h;

                QString layers;
                int cells_on_this_tilemap = 0;
                int layerOrder = 0;
                for (auto& subLayer : layer->asGroupLayer()->layers())
                {
                    auto tileLayer = subLayer->asTileLayer();
                    if (tileLayer == nullptr)
                        continue;

                    QVariantHash layer_h;
                    layer_h["id"] = tileLayer->name();

                    float zIndex = qBound(0, groupLayerOrder, 1000) * 0.0001f;
                    zIndex += qBound(0, layerOrder, 1000) * 0.00000001f;
                    layer_h["z"] = zIndex;

                    layer_h["is_visible"] = layer->isVisible() ? 1 : 0;
                    QString cells;

                    for (int x = 0; x < tileLayer->width(); ++x)
                    {
                        for (int y = 0; y < tileLayer->height(); ++y)
                        {
                            const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                            if (cell.isEmpty() || cell.tileset() != tileset_current) // skip cell if it doesn't belong to current tileset
                                continue;
                            QVariantHash cell_h;
                            cell_h["x"] = x;
                            cell_h["y"] = tileLayer->height() - y - 1;
                            cell_h["tile"] = cell.tileId();
                            cell_h["h_flip"] = cell.flippedHorizontally() ? 1 : 0;
                            cell_h["v_flip"] = cell.flippedVertically() ? 1 : 0;
                            cells.append(replaceTags(QLatin1String(cell_t), cell_h));
                            cells_on_this_tilemap++;

                            // Create a component for this embedded instance only when the first cell is found.
                            // If 0 cells are found, this component is not necessary.
                            // If more than 1 cells are found, recreating it would be redundant.
                            // Hence, only when encountering the first cell do we create it.
                            if (cells_on_this_tilemap == 1)
                            {
                                QVariantHash component_h;
                                component_h["tilemap_name"] = mapName + "-" + layer->name() + "-" + tileset_current->name();
                                component_h["tilemap_rel_path"] = TilesetRelativePath(tilesetFilePath);
                                components.append(replaceTags(QLatin1String(component_t), component_h));
                            }
                        }
                    }
                    layer_h["cells"] = cells;
                    layers.append(replaceTags(QLatin1String(layer_t), layer_h));
                    layerOrder++;
                }
                map_h["layers"] = layers;
                map_h["material"] = "/builtins/materials/tile_map.material";
                map_h["blend_mode"] = "BLEND_MODE_ALPHA";
                // Below, we input a value that's not necessarily correct in Defold, but it lets the user know what tilesource to link this tilemap with manually.
                // However, if the user keeps all tilesources in /tilesources/ and the name of the tilesource corresponds with the name of the tileset in Defold,
                // the value will be automatically correct.
                map_h["tile_set"] = "/tilesources/" + tileset_current->name() + ".tilesource";

                // avoid saving tilemaps with 0 cells
                if (cells_on_this_tilemap == 0)
                    continue;

                QString result = replaceTags(QLatin1String(map_t), map_h);
                Tiled::SaveFile mapFile(tilesetFilePath);
                if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    mError = tr("Could not open file for writing.");
                    return false;
                }

                QTextStream stream(mapFile.device());
                stream << result;

                if (mapFile.error() != QFileDevice::NoError)
                {
                    mError = mapFile.errorString();
                    return false;
                }

                if (!mapFile.commit())
                {
                    mError = mapFile.errorString();
                    return false;
                }
            }
            emdedded_instance_h["components"] = components;
            embeddedInstances.append(replaceTags(QLatin1String(emdedded_instance_t), emdedded_instance_h));

            groupLayerOrder++;
        }
        QVariantHash collection_h;
        // optional custom properties that allow exporting a map with an offset
        collection_h["pos-x"] = map->property(QLatin1String("x-offset")).toInt();
        collection_h["pos-y"] = map->property(QLatin1String("y-offset")).toInt();
        collection_h["components"] = QString("");
        collection_h["children"] = children;
        collection_h["embedded-instances"] = embeddedInstances;

        QString result = replaceTags(QLatin1String(collection_t), collection_h);
        Tiled::SaveFile mapFile(fileName);
        if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            mError = tr("Could not open file for writing.");
            return false;
        }
        QTextStream stream(mapFile.device());
        stream << result;

        if (mapFile.error() != QFileDevice::NoError)
        {
            mError = mapFile.errorString();
            return false;
        }

        if (!mapFile.commit())
        {
            mError = mapFile.errorString();
            return false;
        }
    }

    return true;
}

} // namespace DefoldCollection
