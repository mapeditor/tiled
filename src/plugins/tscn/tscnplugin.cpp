/*
 * Godot 4 Scene Tiled Plugin
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

#include "tscnplugin.h"

#include "grouplayer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QMap>
#include <QRegularExpression>

#include <iostream>
#include <stdexcept>

using namespace Tiled;
using namespace Tscn;

TscnPlugin::TscnPlugin()
{
}

// Convenience functions for throwing translated errors
static std::invalid_argument tscnError(const char* error)
{
    return std::invalid_argument(QCoreApplication::translate("TscnPlugin", error).toStdString());
}

template <typename... Types>
static std::invalid_argument tscnError(const char* error, Types... args)
{
    return std::invalid_argument(
        QCoreApplication::translate("TscnPlugin", error).arg(args...).toStdString()
    );
}

/**
 * Determines the res:// root for the Godot project. This is done by searching
 * for a .godot file in the parent paths of the file.
 * If there's no .godot file found in a shared root, it returns a blank string.
 */
static QString determineResRoot(const QString &filePath)
{
    constexpr unsigned int searchDepth = 3;
    QFileInfo fi(filePath);
    QDir dir(fi.path());
    dir.setNameFilters(QStringList("*.godot"));

    for (unsigned int i = 0; i < searchDepth; ++i) {
        if (i > 0 && !dir.cdUp())
            break;

        const QString godotFile = QDirIterator(dir).next();
        if (!godotFile.isEmpty()) {
            return dir.path();
        }
    }

    throw tscnError("Could not find .godot project in file path for file %1", filePath);
}

// Converts a tileset's image source to a res:// path
// If resRoot is null, this will find the nearest .godot project and set that to the resRoot,
// otherwise, it will ensure that the image source is a child of resRoot.
static QString imageSourceToRes(const Tileset* tileset, std::unique_ptr<QString>& resRoot)
{
    auto filePath = tileset->imageSource().toLocalFile();

    if (!resRoot) {
        resRoot = std::make_unique<QString>(determineResRoot(filePath));
    }

    if(!filePath.startsWith(*resRoot)) {
        throw tscnError(
            "All files must share the same project root. File '%1' does not share project root '%2'.",
            filePath,
            *resRoot);
    }

    return "res:/" + filePath.right(filePath.length() - resRoot->length());
}

// Replace any instance of " with \" so you can include it in a quoted string
static QString sanitizeQuotedString(QString str)
{
    static const QRegularExpression regexp(QLatin1String("\""));
    return str.replace(regexp, QStringLiteral("\\\""));
}

// For collecting information about the tilesets we're using
struct TilesetInfo
{
    QString id;             // The id for the Godot texture
    int atlasId = -1;       // The id for the Godot atlas
    QSet<int> usedTiles;    // Tracks which tiles were actually used
    SharedTileset tileset;
};

// All the info we'll collect with collectAssets() before outputting the .tscn file
struct AssetInfo
{
    QMap<QString, TilesetInfo> tilesetInfo;
    QList<const TileLayer*> layers;
    QSet<QString> tilesetIds;
    std::unique_ptr<QString> resRoot;
};

// Search a layer for every tile that was used and store it in assetInfo
static void findUsedTiles(const TileLayer* layer, AssetInfo& assetInfo) {
    auto bounds = layer->bounds();
    for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
        for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
            auto cell = layer->cellAt(x, y);
            if (!cell.isEmpty()) {
                if (cell.tileset()->isCollection())
                    throw tscnError("Cannot export layer '%1' because the Godot exporter does not support collection-type tilesets", layer->name());

                auto resPath = imageSourceToRes(cell.tileset(), assetInfo.resRoot);
                auto& tilesetInfo = assetInfo.tilesetInfo[resPath];
                tilesetInfo.usedTiles.insert(cell.tileId());
                if (!tilesetInfo.tileset)
                    tilesetInfo.tileset = cell.tileset()->sharedPointer();
            }
        }
    }
}

// Used by collectAssets() to search all layers and layer groups
static void collectAssetsRecursive(const QList<Layer*> &layers, AssetInfo& assetInfo)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;

        if (layer->resolvedProperty("noExport").toBool())
            continue;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
                auto tileLayer = static_cast<const TileLayer*>(layer);
                findUsedTiles(tileLayer, assetInfo);
                assetInfo.layers.push_back(tileLayer);
                break;
            }
        case Layer::ObjectGroupType:
            throw tscnError("The Godot exporter does not yet support objects");
            break;
        case Layer::ImageLayerType:
            throw tscnError("The Godot exporter does not yet support image layers");
            break;
        case Layer::GroupLayerType: {
                auto groupLayer = static_cast<const GroupLayer*>(layer);
                collectAssetsRecursive(groupLayer->layers(), assetInfo);
                break;
            }
        }
    }
}

// Search the map for all layers and tiles
// Also finds the .godot project root and assigns IDs to tilesets and atlases
static AssetInfo collectAssets(const Map *map)
{
    AssetInfo assetInfo;
    collectAssetsRecursive(map->layers(), assetInfo);

    // Run through all the assets collected and assign them IDs
    int i = 0;
    for (auto&& itTilesetInfo = assetInfo.tilesetInfo.begin();
        itTilesetInfo != assetInfo.tilesetInfo.end();
        ++itTilesetInfo)
    {
        QString basename = itTilesetInfo->tileset->name();

        // Make sure the tileset ID is unique by adding a number to the end
        QString id = basename;
        int uniqueifier = 1;
        while (assetInfo.tilesetIds.contains(id)) {
            id = basename + "_" + QString::number(uniqueifier);
            uniqueifier++;
        }

        itTilesetInfo->id = id;
        itTilesetInfo->atlasId = i;
        assetInfo.tilesetIds.insert(id);
        ++i;
    }

    return assetInfo;
}

// Export a tile's object groups as Godot physics layers
static bool exportTileCollisions(QFileDevice* device, const Tile* tile, QString tileName)
{
    bool foundCollisions = false;

    auto objectGroup = tile->objectGroup();

    if (objectGroup) {
        int polygonId = 0;

        for (auto&& object : objectGroup->objects()) {
            foundCollisions = true;

            auto centerX = tile->width() / 2 - object->x();
            auto centerY = tile->height() / 2 - object->y();

            device->write(QString("%1/physics_layer_0/polygon_%2/points = PackedVector2Array(")
                .arg(tileName, QString::number(polygonId)).toUtf8());

            switch (object->shape()) {
                case MapObject::Rectangle: {
                    auto x1 = object->x() - centerX;
                    auto y1 = object->y() - centerY;
                    auto x2 = object->width() - centerX;
                    auto y2 = object->height() - centerY;

                    device->write(QString("%1, %2, %3, %2, %3, %4, %1, %4")
                        .arg(QString::number(x1), QString::number(y1), QString::number(x2), QString::number(y2))
                        .toUtf8());

                    break;
                }
                case MapObject::Polygon: {
                    auto polygon = object->polygon().toPolygon();
                    bool first = true;
                    for (auto point : polygon) {
                        if (!first)
                            device->write(", ");
                        device->write(QString::number(point.x() - centerX).toUtf8());
                        device->write(", ");
                        device->write(QString::number(point.y() - centerY).toUtf8());
                        first = false;
                    }
                    break;
                }
                default:
                    throw tscnError("Godot exporter only supports collisions that are rectangles or polygons.");
            }

            device->write(")\n");
            polygonId++;
        }
    }

    return foundCollisions;
}

bool TscnPlugin::write(const Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        mError += QLatin1String("\n");
        mError += fileName;
        return false;
    }

    QFileInfo fi(fileName);
    auto device = file.device();

    try
    {
        AssetInfo assetInfo = collectAssets(map);
        bool foundCollisions = false;

        // One TileSet, one TileMap, plus a Texture2D and TileSetAtlasSource per tileset
        auto loadSteps = assetInfo.tilesetInfo.size() * 2 + 2;

        // gdscene node
        device->write(QString("[gd_scene load_steps=%1 format=3]\n\n").arg(QString::number(loadSteps)).toUtf8());

        std::unique_ptr<QString> resRoot;

        // Texture2D nodes
        for (auto it = assetInfo.tilesetInfo.begin(); it != assetInfo.tilesetInfo.end(); ++it) {
            if (it->usedTiles.size() == 0)
                continue;

            device->write(QString("[ext_resource type=\"Texture2D\" path=\"%1\" id=\"%2\"]\n")
                .arg(sanitizeQuotedString(it.key()), sanitizeQuotedString(it->id)).toUtf8());
        }
        device->write("\n");

        // TileSetAtlasSource nodes
        for (auto itTileset = assetInfo.tilesetInfo.begin(); itTileset != assetInfo.tilesetInfo.end(); ++itTileset) {
            if (itTileset->usedTiles.size() == 0)
                continue;

            device->write(QString("[sub_resource type=\"TileSetAtlasSource\" id=\"TileSetAtlasSource_%1\"]\n")
                .arg(QString::number(itTileset->atlasId)).toUtf8());

            device->write(QString("texture = ExtResource(\"%1\")\n")
                .arg(sanitizeQuotedString(itTileset->id)).toUtf8());

            if (itTileset->tileset->margin() != 0)
                device->write(QString("margins = Vector2i(%1, %1)\n")
                    .arg(QString::number(itTileset->tileset->margin())).toUtf8());

            if (itTileset->tileset->tileSpacing() != 0)
                device->write(QString("separation = Vector2i(%1, %1)\n")
                    .arg(QString::number(itTileset->tileset->tileSpacing())).toUtf8());

            if (itTileset->tileset->tileWidth() != 16 || itTileset->tileset->tileHeight() != 16)
                device->write(QString("texture_region_size = Vector2i(%1, %2)\n").arg(
                    QString::number(itTileset->tileset->tileWidth()),
                    QString::number(itTileset->tileset->tileHeight())).toUtf8());

            // Tile info
            for (auto tile : itTileset->tileset->tiles()) {
                if (itTileset->usedTiles.contains(tile->id()) || tile->objectGroup()) {
                    // Tile existence
                    auto x = tile->id() % itTileset->tileset->columnCount();
                    auto y = tile->id() / itTileset->tileset->columnCount();
                    QString tileName = QString("%1:%2/0").arg(QString::number(x), QString::number(y));
                    device->write(tileName.toUtf8());
                    device->write(" = 0\n");

                    foundCollisions |= exportTileCollisions(device, tile, tileName);
                }
            }

            device->write("\n");
        }

        // TileSet node
        device->write("[sub_resource type=\"TileSet\" id=\"TileSet_0\"]\n");

        if (foundCollisions)
            device->write("physics_layer_0/collision_layer = 1\n");
        
        if (map->tileWidth() != 16 || map->tileHeight() != 16)
            device->write(QString("tile_size = Vector2i(%1, %2)\n").arg(
                QString::number(map->tileWidth()),
                QString::number(map->tileHeight()))
                .toUtf8());

        for (auto it = assetInfo.tilesetInfo.begin(); it != assetInfo.tilesetInfo.end(); ++it)
            device->write(QString("sources/%1 = SubResource(\"TileSetAtlasSource_%2\")\n").arg(
                QString::number(it->atlasId), QString::number(it->atlasId)).toUtf8());

        device->write("\n");

        // TileMap node
        device->write(QString("[node name=\"%1\" type=\"TileMap\"]\n")
            .arg(sanitizeQuotedString(fi.baseName())).toUtf8());

        device->write("tile_set = SubResource(\"TileSet_0\")\n");

        if (map->orientation() != Map::Orthogonal)
            throw tscnError("Godot exporter currently only supports orthogonal maps.");

        device->write("format = 2\n");

        // Tile packing format:
        // DestLocation, SrcX, SrcY
        // Where:
        //   DestLocation = (DestX >= 0 ? DestY : DestY + 1) * 65536 + DestX
        //   SrcX         = SrcX * 65536 + TileSetId
        //   SrcY         = SrcY
        int layerIndex = 0;
        for (auto layer : assetInfo.layers) {
            device->write(QString("layer_%1/name = \"%2\"\n").arg(
                QString::number(layerIndex),
                sanitizeQuotedString(layer->name()))
                .toUtf8());

            if (layer->resolvedProperty("ySortEnabled").isValid())
                device->write(QString("layer_%1/y_sort_enabled = true\n").arg(
                    QString::number(layerIndex))
                    .toUtf8());

            if (layer->resolvedProperty("zIndex").isValid())
                device->write(QString("layer_%1/z_index = %2\n").arg(
                    QString::number(layerIndex),
                    QString::number(layer->resolvedProperty("zIndex").toInt()))
                    .toUtf8());

            device->write(QString("layer_%1/tile_data = PackedInt32Array(")
                .arg(QString::number(layerIndex)).toUtf8());

            bool first = true;
            auto bounds = layer->bounds();
            for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
                for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
                    auto& cell = layer->cellAt(x, y);

                    if (!cell.isEmpty()) {
                        auto resPath = imageSourceToRes(cell.tile()->tileset(), assetInfo.resRoot);
                        auto& tilesetInfo = assetInfo.tilesetInfo[resPath];
                        int destLocation = (x >= 0 ? y : y + 1) * 65536 + x;
                        int srcX = cell.tileId() % cell.tileset()->columnCount();
                        srcX *= 65536;
                        srcX += tilesetInfo.atlasId;
                        int srcY = cell.tileId() / cell.tileset()->columnCount();

                        if (!first) {
                            device->write(", ");
                        }
                        device->write(QString::number(destLocation).toUtf8());
                        device->write(", ");
                        device->write(QString::number(srcX).toUtf8());
                        device->write(", ");
                        device->write(QString::number(srcY).toUtf8());

                        first = false;
                    }
                }
            }

            device->write(")\n");

            layerIndex++;
        }
    }
    catch (std::exception& e) {
        mError = e.what();
        return false;
    }

    if (file.error() != QFileDevice::NoError) {
        mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString TscnPlugin::errorString() const
{
    return mError;
}

QString TscnPlugin::nameFilter() const
{
    return tr("Godot 4 Scene files (*.tscn)");
}

QString TscnPlugin::shortName() const
{
    return QStringLiteral("tscn");
}
