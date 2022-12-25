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

    throw std::invalid_argument((
        "Could not find .godot project in file path for file "
        + filePath).toStdString());
}

static QString fileToResPath(const Tileset* tileset, std::unique_ptr<QString>& resRoot)
{
    auto filePath = tileset->imageSource().toLocalFile();

    if (!resRoot) {
        resRoot = std::make_unique<QString>(determineResRoot(filePath));
    }

    if(!filePath.startsWith(*resRoot)) {
        throw std::invalid_argument(QT_TR_NOOP((
            "All files must share the same project root. File '" 
            + filePath 
            + "' does not share project root '" 
            + *resRoot 
            + "'.").toStdString()));
    }

    return "res:/" + filePath.right(filePath.length() - resRoot->length());
}

static QString sanitizeQuotedString(QString str)
{
    static const QRegularExpression regexp(QLatin1String("\""));
    return str.replace(regexp, QStringLiteral("\\\""));
}

struct TilesetInfo
{
    QString id;
    int atlasId = -1;
    QSet<int> usedTiles;
    SharedTileset tileset;
};

struct LayerInfo
{
    const TileLayer* layer;
};

struct AssetLists
{
    QMap<QString, TilesetInfo> tilesetInfo;
    QList<LayerInfo> layerInfo;
};

struct CollectAssetsParams
{
    QSet<QString> tilesetIds;
    std::unique_ptr<QString> resRoot;
};

static void findUsedTiles(const TileLayer* layer, AssetLists& assetLists, CollectAssetsParams& params) {
    auto bounds = layer->bounds();
    for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
        for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
            auto cell = layer->cellAt(x, y);
            if (!cell.isEmpty()) {
                auto resPath = fileToResPath(cell.tileset(), params.resRoot);
                auto& tilesetInfo = assetLists.tilesetInfo[resPath];
                tilesetInfo.usedTiles.insert(cell.tileId());
                if (!tilesetInfo.tileset)
                    tilesetInfo.tileset = cell.tileset()->sharedPointer();
            }
        }
    }
}

static void collectAssetsRecursive(const QList<Layer*> &layers, AssetLists& assetLists, CollectAssetsParams& params)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;

        if (layer->resolvedProperty("noExport").toBool())
            continue;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
                LayerInfo layerInfo;
                layerInfo.layer = static_cast<const TileLayer*>(layer);
                findUsedTiles(layerInfo.layer, assetLists, params);
                assetLists.layerInfo.push_back(std::move(layerInfo));
                break;
            }
        case Layer::ObjectGroupType:
            throw std::invalid_argument("The Godot exporter does not yet support objects");
            break;
        case Layer::ImageLayerType:
            throw std::invalid_argument("The Godot exporter does not yet support image layers");
            break;
        case Layer::GroupLayerType: {
                auto groupLayer = static_cast<const GroupLayer*>(layer);
                collectAssetsRecursive(groupLayer->layers(), assetLists, params);
                break;
            }
        }
    }
}

static AssetLists collectAssets(const Map *map, CollectAssetsParams& params)
{
    AssetLists assetLists;
    collectAssetsRecursive(map->layers(), assetLists, params);

    // Run through all the assets collected and assign them IDs
    int i = 0;
    for (auto&& itTilesetInfo = assetLists.tilesetInfo.begin();
        itTilesetInfo != assetLists.tilesetInfo.end();
        ++itTilesetInfo)
    {
        QString basename = itTilesetInfo->tileset->name();

        if (itTilesetInfo->tileset->isCollection())
            throw std::invalid_argument(("Cannot export tileset "
                + basename
                + " because the Godot exporter does not support tilesets that"
                + " are a collection of images").toStdString());

        QString id = basename;
        int uniqueifier = 1;
        while (params.tilesetIds.contains(id)) {
            id = basename + "_" + QString::number(uniqueifier);
            uniqueifier++;
        }

        itTilesetInfo->id = id;
        itTilesetInfo->atlasId = i;
        params.tilesetIds.insert(id);
        ++i;
    }

    return assetLists;
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
        CollectAssetsParams params;
        auto assetLists = collectAssets(map, params);

        // One TileSet, one TileMap, plus a Texture2D and TileSetAtlasSource per tileset
        auto loadSteps = assetLists.tilesetInfo.size() * 2 + 2;

        // gdscene node
        device->write("[gd_scene load_steps=");
        device->write(QString::number(loadSteps).toUtf8());
        device->write(" format=3]\n\n");

        std::unique_ptr<QString> resRoot;

        // Texture2D nodes
        for (auto it = assetLists.tilesetInfo.begin(); it != assetLists.tilesetInfo.end(); ++it) {
            if (it->usedTiles.size() == 0)
                continue;

            device->write("[ext_resource type=\"Texture2D\" path=\"");
            device->write(sanitizeQuotedString(it.key()).toUtf8());
            device->write("\" id=\"");
            device->write(sanitizeQuotedString(it->id).toUtf8());
            device->write("\"]\n");
        }
        device->write("\n");

        // TileSetAtlasSource nodes
        for (auto itTileset = assetLists.tilesetInfo.begin(); itTileset != assetLists.tilesetInfo.end(); ++itTileset) {
            if (itTileset->usedTiles.size() == 0)
                continue;

            device->write("[sub_resource type=\"TileSetAtlasSource\" id=\"TileSetAtlasSource_");
            device->write(QString::number(itTileset->atlasId).toUtf8());
            device->write("\"]\n");

            // texture
            device->write("texture = ExtResource(\"");
            device->write(sanitizeQuotedString(itTileset->id).toUtf8());
            device->write("\")\n");

            // Margin/Separation/Size
            if (itTileset->tileset->margin() != 0) {
                device->write("margins = Vector2i(");
                device->write(QString::number(itTileset->tileset->margin()).toUtf8());
                device->write(", ");
                device->write(QString::number(itTileset->tileset->margin()).toUtf8());
                device->write(")\n");
            }

            if (itTileset->tileset->tileSpacing() != 0) {
                device->write("separation = Vector2i(");
                device->write(QString::number(itTileset->tileset->tileSpacing()).toUtf8());
                device->write(", ");
                device->write(QString::number(itTileset->tileset->tileSpacing()).toUtf8());
                device->write(")\n");
            }

            if (itTileset->tileset->tileWidth() != 16 || itTileset->tileset->tileHeight() != 16) {
                device->write("texture_region_size = Vector2i(");
                device->write(QString::number(itTileset->tileset->tileWidth()).toUtf8());
                device->write(", ");
                device->write(QString::number(itTileset->tileset->tileHeight()).toUtf8());
                device->write(")\n");
            }

            // Tile info
            for (auto&& tile : itTileset->tileset->tiles()) {
                if (itTileset->usedTiles.contains(tile->id())) {
                    auto x = tile->id() % itTileset->tileset->columnCount();
                    auto y = tile->id() / itTileset->tileset->columnCount();
                    device->write(QString::number(x).toUtf8());
                    device->write(":");
                    device->write(QString::number(y).toUtf8());
                    device->write("/0 = 0\n");
                }
            }

            device->write("\n");
        }

        // TileSet node
        device->write("[sub_resource type=\"TileSet\" id=\"TileSet_0\"]\n");
        if (map->tileWidth() != 16 || map->tileHeight() != 16) {
            device->write("tile_size = Vector2i(");
            device->write(QString::number(map->tileWidth()).toUtf8());
            device->write(", ");
            device->write(QString::number(map->tileHeight()).toUtf8());
            device->write(")\n");
        }

        for (auto it = assetLists.tilesetInfo.begin(); it != assetLists.tilesetInfo.end(); ++it) {
            device->write("sources/");
            device->write(QString::number(it->atlasId).toUtf8());
            device->write(" = SubResource(\"TileSetAtlasSource_");
            device->write(QString::number(it->atlasId).toUtf8());
            device->write("\")\n");
        }

        device->write("\n");

        // TileMap node
        device->write("[node name=\"");
        device->write(sanitizeQuotedString(fi.baseName()).toUtf8());
        device->write("\" type=\"TileMap\"]\n");

        device->write("tile_set = SubResource(\"TileSet_0\")\n");

        if (map->orientation() != Map::Orthogonal)
            throw std::invalid_argument("Godot exporter currently only supports orthogonal maps.");

        device->write("format = 2\n");

        // Tile packing format:
        // DestLocation, SrcX, SrcY
        // Where:
        //   DestLocation = (DestX >= 0 ? DestY : DestY + 1) * 65536 + DestX
        //   SrcX         = SrcX * 65536 + TileSetId
        //   SrcY         = SrcY
        int layerIndex = 0;
        for (auto&& layer : assetLists.layerInfo) {
            device->write("layer_");
            device->write(QString::number(layerIndex).toUtf8());
            device->write("/name = \"");
            device->write(sanitizeQuotedString(layer.layer->name()).toUtf8());
            device->write("\"\n");

            device->write("layer_");
            device->write(QString::number(layerIndex).toUtf8());
            device->write("/tile_data = PackedInt32Array(");

            bool first = true;
            auto bounds = layer.layer->bounds();
            for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
                for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
                    auto& cell = layer.layer->cellAt(x, y);

                    if (!cell.isEmpty()) {
                        auto resPath = fileToResPath(cell.tile()->tileset(), params.resRoot);
                        auto& tilesetInfo = assetLists.tilesetInfo[resPath];
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
        mError = tr(e.what());
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
