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

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include <cmath>

namespace DefoldCollection {

static const char cellTemplate[] =
R"(  cell {
    x: {{x}}
    y: {{y}}
    tile: {{tile}}
    h_flip: {{h_flip}}
    v_flip: {{v_flip}}
  }
)";

static const char layerTemplate[] =
R"(layers {
  id: "{{id}}"
  z: {{z}}
  is_visible: {{is_visible}}
{{cells}}}
)";

static const char tileMapTemplate[] =
R"(tile_set: "{{tile_set}}"
{{layers}}
material: "{{material}}"
blend_mode: {{blend_mode}}
)";

static const char collectionTemplate[] =
R"(name: "default"
scale_along_z: 0
{{embedded-instances}}

)";

static const char componentTemplate[] =
R"(  "components {\n"
  "  id: \"{{tilemap_name}}\"\n"
  "  component: \"{{tilemap_rel_path}}\"\n"
  "  position {\n"
  "    x: 0.0\n"
  "    y: 0.0\n"
  "    z: 0.0\n"
  "  }\n"
  "  rotation {\n"
  "    x: 0.0\n"
  "    y: 0.0\n"
  "    z: 0.0\n"
  "    w: 1.0\n"
  "  }\n"
  "}\n"
)";

static const char childTemplate[] =
R"(  children: "{{child-name}}"
)";

static const char emdeddedInstanceTemplate[] =
R"(embedded_instances {
  id: "{{instance-name}}"
{{children}}  data: {{components}}
  ""
  position {
    x: {{pos-x}}
    y: {{pos-y}}
    z: 0.0
  }
  rotation {
    x: 0.0
    y: 0.0
    z: 0.0
    w: 1.0
  }
  scale3 {
    x: 1.0
    y: 1.0
    z: 1.0
  }
}
)";

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
static QString tilesetRelativePath(const QString &filePath)
{
    QString gameproject = "game.project";
    QFileInfo fi(filePath);
    QDir dir = fi.dir();

    while (dir.exists() && !dir.isRoot()) {
        if (dir.exists(gameproject)) {
            // return relative path
            return filePath.right(filePath.length() - dir.path().length());
        } else if (!dir.cdUp()) {
            break;
        }
    }
    return fi.fileName();
}

/*
 * Returns z-Index for a layer, depending on its order in the map
 */
static float zIndexForLayer(const Tiled::Map &map, const Tiled::Layer &inLayer, bool isTopLayer)
{
    if (isTopLayer) {
        int topLayerOrder = 0;
        for (auto layer : map.layers()) {
            if (layer->layerType() != Tiled::Layer::GroupLayerType && layer->layerType() != Tiled::Layer::TileLayerType)
                continue;
            if (&inLayer == layer)
                return qBound(0, topLayerOrder, 9999) * 0.0001f;
            topLayerOrder++;
        }
    } else if (inLayer.parentLayer()) {
        float zIndex = zIndexForLayer(map, *inLayer.parentLayer(), true);
        int subLayerOrder = 0;
        for (auto subLayer : inLayer.parentLayer()->layers()) {
            if (subLayer == &inLayer) {
                zIndex += qBound(0, subLayerOrder, 9999) * 0.00000001f;
                return zIndex;
            }
            subLayerOrder++;
        }
    }
    return 0;
}

/*
 * Writes a .collection file, as well as multiple .tilemap files required by this collection
 */
bool DefoldCollectionPlugin::write(const Tiled::Map *map, const QString &collectionFile, Options options)
{
    Q_UNUSED(options)

    QFileInfo fi(collectionFile);
    QString outputFilePath = fi.filePath();
    QString outputFileName = fi.fileName();
    QString mapName = fi.completeBaseName();

    QVariantHash collectionHash;
    QString embeddedInstances;

    QVariantHash mainEmbeddedInstanceHash;
    mainEmbeddedInstanceHash["instance-name"] = "tilemaps";
    mainEmbeddedInstanceHash["pos-x"] = map->property(QLatin1String("x-offset")).toInt();
    mainEmbeddedInstanceHash["pos-y"] = map->property(QLatin1String("y-offset")).toInt();
    QString topLevelChildren;
    QString topLevelComponents;

    QString tilesetFileDir = outputFilePath;
    tilesetFileDir.chop(outputFileName.length());

    // dealing with top-level tile layers here only
    // create a tilemap file for each tileset this map uses, and for each of them create a "component" in the main embedded instance
    for (auto &tileset : map->tilesets()) {
        QString tilemapFilePath = tilesetFileDir;
        tilemapFilePath.append(mapName + "-" + tileset->name() + ".tilemap");

        QVariantHash componentHash;
        componentHash["tilemap_name"] = mapName + "-" + tileset->name();
        componentHash["tilemap_rel_path"] = tilesetRelativePath(tilemapFilePath);

        QVariantHash tileMapHash;

        bool tilemapHasCells = false;

        int componentCells = 0;
        QString layers;
        for (auto layer : map->layers()) {
            if (layer->layerType() != Tiled::Layer::TileLayerType)
                continue;
            auto tileLayer = static_cast<Tiled::TileLayer*>(layer);

            QVariantHash layerHash;
            layerHash["id"] = tileLayer->name();
            layerHash["z"] = zIndexForLayer(*map, *tileLayer, true);
            layerHash["is_visible"] = tileLayer->isVisible() ? 1 : 0;
            QString cells;

            for (int x = 0; x < tileLayer->width(); ++x) {
                for (int y = 0; y < tileLayer->height(); ++y) {
                    const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                    if (cell.isEmpty())
                        continue;
                    if (cell.tileset() != tileset) // skip cell if it doesn't belong to current tileset
                        continue;

                    tilemapHasCells = true;
                    componentCells++;
                    QVariantHash cellHash;
                    cellHash["x"] = x;
                    cellHash["y"] = tileLayer->height() - y - 1;
                    cellHash["tile"] = cell.tileId();
                    cellHash["h_flip"] = cell.flippedHorizontally() ? 1 : 0;
                    cellHash["v_flip"] = cell.flippedVertically() ? 1 : 0;
                    cells.append(replaceTags(QLatin1String(cellTemplate), cellHash));

                    // Create a component for this embedded instance only when the first cell of this component is found.
                    // If 0 cells are found, this component is not necessary.
                    // If more than 1 cells are found, recreating it would be redundant.
                    if (componentCells == 1)
                        topLevelComponents.append(replaceTags(QLatin1String(componentTemplate), componentHash));
                }
            }
            layerHash["cells"] = cells;

            // only add this layer to the .tilemap if it has any cells
            if (!cells.isEmpty())
                layers.append(replaceTags(QLatin1String(layerTemplate), layerHash));
        }
        // make a check that this tilemap has cells at all, or no .tilesource file is necessary
        if (tilemapHasCells) {
            tileMapHash["layers"] = layers;
            tileMapHash["material"] = "/builtins/materials/tile_map.material";
            tileMapHash["blend_mode"] = "BLEND_MODE_ALPHA";
            // Below, we input a value that's not necessarily correct in Defold, but it lets the user know what tilesource to link this tilemap with manually.
            // However, if the user keeps all tilesources in /tilesources/ and the name of the tilesource corresponds with the name of the tileset in Defold,
            // the value will be automatically correct.
            tileMapHash["tile_set"] = "/tilesources/" + tileset->name() + ".tilesource";

            QString result = replaceTags(QLatin1String(tileMapTemplate), tileMapHash);
            Tiled::SaveFile mapFile(tilemapFilePath);
            if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
                return false;
            }
            QTextStream stream(mapFile.device());
            stream << result;

            if (mapFile.error() != QFileDevice::NoError) {
                mError = mapFile.errorString();
                return false;
            }

            if (!mapFile.commit()) {
                mError = mapFile.errorString();
                return false;
            }
        }
    }

    // For each Group Layer, create a "GameObject" parented to the "tilemaps" GO
    // and create tilemaps for layers (as components of this GO)
    for (auto layer : map->layers()) {
        if (layer->layerType() != Tiled::Layer::GroupLayerType)
            continue;
        auto groupLayer = static_cast<Tiled::GroupLayer*>(layer);

        QVariantHash childHash;
        childHash["child-name"] = layer->name();
        topLevelChildren.append(replaceTags(QLatin1String(childTemplate), childHash));

        QVariantHash emdeddedInstanceHash;
        emdeddedInstanceHash["instance-name"] = layer->name();
        emdeddedInstanceHash["pos-x"] = 0;
        emdeddedInstanceHash["pos-y"] = 0;
        emdeddedInstanceHash["children"] = "";

        QString components;

        // write as many tilemaps as there are tilesets per group layer
        for (auto &tileset : map->tilesets()) {
            QString tilemapFilePath = tilesetFileDir;
            tilemapFilePath.append(mapName + "-" + layer->name() + "-" + tileset->name() + ".tilemap");

            QVariantHash tileMapHash;
            QString layers;

            int componentCells = 0;
            for (auto subLayer : groupLayer->layers()) {
                auto tileLayer = subLayer->asTileLayer();
                if (!tileLayer)
                    continue;

                QVariantHash layerHash;
                layerHash["id"] = tileLayer->name();
                layerHash["z"] = zIndexForLayer(*map, *subLayer, false);

                layerHash["is_visible"] = layer->isVisible() ? 1 : 0;
                QString cells;

                for (int x = 0; x < tileLayer->width(); ++x) {
                    for (int y = 0; y < tileLayer->height(); ++y) {
                        const Tiled::Cell &cell = tileLayer->cellAt(x, y);
                        if (cell.isEmpty() || cell.tileset() != tileset) // skip cell if it doesn't belong to current tileset
                            continue;

                        QVariantHash cellHash;
                        cellHash["x"] = x;
                        cellHash["y"] = tileLayer->height() - y - 1;
                        cellHash["tile"] = cell.tileId();
                        cellHash["h_flip"] = cell.flippedHorizontally() ? 1 : 0;
                        cellHash["v_flip"] = cell.flippedVertically() ? 1 : 0;
                        cells.append(replaceTags(QLatin1String(cellTemplate), cellHash));
                        componentCells++;

                        // Create a component for this embedded instance only when the first cell of this component is found.
                        // If 0 cells are found, this component is not necessary.
                        // If more than 1 cells are found, recreating it would be redundant.
                        if (componentCells == 1) {
                            QVariantHash componentHash;
                            componentHash["tilemap_name"] = mapName + "-" + layer->name() + "-" + tileset->name();
                            componentHash["tilemap_rel_path"] = tilesetRelativePath(tilemapFilePath);
                            components.append(replaceTags(QLatin1String(componentTemplate), componentHash));
                        }
                    }
                }

                if (!cells.isEmpty()) {
                    layerHash["cells"] = cells;
                    layers.append(replaceTags(QLatin1String(layerTemplate), layerHash));
                }
            }

            // no need to save a tilemap with 0 cells
            if (layers.isEmpty())
                continue;

            tileMapHash["layers"] = layers;
            tileMapHash["material"] = "/builtins/materials/tile_map.material";
            tileMapHash["blend_mode"] = "BLEND_MODE_ALPHA";
            // Below, we input a value that's not necessarily correct in Defold, but it lets the user know what tilesource to link this tilemap with manually.
            // However, if the user keeps all tilesources in /tilesources/ and the name of the tilesource corresponds with the name of the tileset in Defold,
            // the value will be automatically correct.
            tileMapHash["tile_set"] = "/tilesources/" + tileset->name() + ".tilesource";

            QString result = replaceTags(QLatin1String(tileMapTemplate), tileMapHash);
            Tiled::SaveFile mapFile(tilemapFilePath);
            if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
                return false;
            }

            QTextStream stream(mapFile.device());
            stream << result;

            if (mapFile.error() != QFileDevice::NoError) {
                mError = mapFile.errorString();
                return false;
            }

            if (!mapFile.commit()) {
                mError = mapFile.errorString();
                return false;
            }
        }
        emdeddedInstanceHash["components"] = components;
        embeddedInstances.append(replaceTags(QLatin1String(emdeddedInstanceTemplate), emdeddedInstanceHash));
    }

    mainEmbeddedInstanceHash["components"] = topLevelComponents;
    mainEmbeddedInstanceHash["children"] = topLevelChildren;
    embeddedInstances.prepend(replaceTags(QLatin1String(emdeddedInstanceTemplate), mainEmbeddedInstanceHash));
    collectionHash["embedded-instances"] = embeddedInstances;

    QString result = replaceTags(QLatin1String(collectionTemplate), collectionHash);
    Tiled::SaveFile mapFile(collectionFile);
    if (!mapFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }
    QTextStream stream(mapFile.device());
    stream << result;

    if (mapFile.error() != QFileDevice::NoError) {
        mError = mapFile.errorString();
        return false;
    }

    if (!mapFile.commit()) {
        mError = mapFile.errorString();
        return false;
    }

    return true;
}

} // namespace DefoldCollection
