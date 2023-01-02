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

// Convenience functions for formatting strings.
// This is very similar to the vararg version of Qt's QString::arg function,
// available in Qt 5.14. But as of 2023-01-02, Qt's offline installer is only 5.12.
template <typename T>
static QString FormatString(QString fmt, T&& arg)
{
    return fmt.arg(arg);
}

template <typename T1, typename... Types>
static QString FormatString(QString fmt, T1&& t1, Types&&... args)
{
    return FormatString(fmt.arg(t1), args...);
}

// Same as FormatString, but returns a byte string.
// Used for device->write(), which only accepts byte strings.
template <typename... Types>
static QByteArray FormatByteString(QString fmt, Types&&... args)
{
    return FormatString(fmt, args...).toUtf8();
}

// Convenience functions for throwing translated errors
static std::invalid_argument tscnError(const char *error)
{
    return std::invalid_argument(
        QCoreApplication::translate("TscnPlugin", error).toStdString());
}

template <typename... Types>
static std::invalid_argument tscnError(const char *error, Types&&... args)
{
    return std::invalid_argument(FormatString
        (QCoreApplication::translate("TscnPlugin", error),
        args...).toStdString());
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
// If resRoot is empty, this will find the nearest .godot project and set that to the resRoot,
// otherwise, it will ensure that the image source is a child of resRoot.
static QString imageSourceToRes(const Tileset *tileset, QString &resRoot)
{
    auto filePath = tileset->imageSource().toLocalFile();

    if (resRoot.isEmpty())
        resRoot = determineResRoot(filePath);

    if(!filePath.startsWith(resRoot)) {
        throw tscnError(
            "All files must share the same project root. File '%1' does not share project root '%2'.",
            filePath,
            resRoot);
    }

    return "res:/" + filePath.right(filePath.length() - resRoot.length());
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
    QSet<int> usedTiles;    // Tracks which tiles have data
    QSet<int> reservedAnimationTiles; // Tiles reserved for animation frames
    SharedTileset tileset;
};

// All the info we'll collect with collectAssets() before outputting the .tscn file
struct AssetInfo
{
    QMap<QString, TilesetInfo> tilesetInfo;
    QList<const TileLayer*> layers;
    QSet<QString> tilesetIds;
    QString resRoot;
};

// Adds a tileset to the assetInfo struct
static void addTileset(Tileset *tileset, AssetInfo &assetInfo) {
    if (tileset->isCollection())
        throw tscnError(
            "Cannot export tileset '%1' because the Godot exporter does not "
            "support collection-type tilesets",
            tileset->name());

    auto resPath = imageSourceToRes(tileset, assetInfo.resRoot);
    if (!assetInfo.tilesetInfo.contains(resPath)) {
        auto& tilesetInfo = assetInfo.tilesetInfo[resPath];
        if (!tilesetInfo.tileset)
            tilesetInfo.tileset = tileset->sharedPointer();
        
        // Find the tiles that aren't blank and have no properties
        auto image = tileset->image().toImage();
        for (auto tile : tileset->tiles()) {
            bool blank = true;

            if (!tile->className().isEmpty() || !tile->properties().isEmpty())
                blank = false;

            auto rect = tile->imageRect();
            for (auto y = rect.y(); blank && y < rect.y() + rect.height(); ++y) {
                for (auto x = rect.x(); blank && x < rect.x() + rect.width(); ++x) {
                    if (image.pixelColor(x, y).alpha() != 0) {
                        blank = false;
                    }
                }
            }

            if (!blank)
                tilesetInfo.usedTiles.insert(tile->id());
        }
    }
}

// Search a layer for every tileset that was used and store it in assetInfo
static void findUsedTilesets(const TileLayer *layer, AssetInfo &assetInfo) {
    auto bounds = layer->bounds();
    for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
        for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
            auto cell = layer->cellAt(x, y);
            if (!cell.isEmpty()) {
                addTileset(cell.tileset(), assetInfo);
            }
        }
    }
}

// Used by collectAssets() to search all layers and layer groups
static void collectAssetsRecursive(const QList<Layer*> &layers, AssetInfo &assetInfo)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;

        if (layer->resolvedProperty("noExport").toBool())
            continue;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
                auto tileLayer = static_cast<const TileLayer*>(layer);
                findUsedTilesets(tileLayer, assetInfo);

                if (!layer->resolvedProperty("tilesetOnly").toBool())
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

enum FlippedState {
    FlippedH = 1,
    FlippedV = 2,
    Transposed = 4
};

static void flipState(double& x, double& y, int flippedState) {
    if (flippedState & Transposed)
        std::swap(x, y);
    if (flippedState & FlippedH)
        x *= -1;
    if (flippedState & FlippedV)
        y *= -1;
}

// Export a tile's object groups as Godot physics layers
static bool exportTileCollisions(QFileDevice *device, const Tile *tile, QString tileName, int flippedState)
{
    bool foundCollisions = false;

    auto objectGroup = tile->objectGroup();

    if (objectGroup) {
        int polygonId = 0;

        for (auto&& object : objectGroup->objects()) {
            foundCollisions = true;

            auto centerX = tile->width() / 2 - object->x();
            auto centerY = tile->height() / 2 - object->y();

            device->write(FormatByteString(
                "%1/physics_layer_0/polygon_%2/points = PackedVector2Array(",
                tileName, polygonId));

            switch (object->shape()) {
                case MapObject::Rectangle: {
                    auto x1 = object->x() - centerX;
                    auto y1 = object->y() - centerY;
                    auto x2 = object->width() - centerX;
                    auto y2 = object->height() - centerY;

                    flipState(x1, y1, flippedState);
                    flipState(x2, y2, flippedState);

                    device->write(FormatByteString(
                        "%1, %2, %3, %2, %3, %4, %1, %4",
                        x1, y1, x2, y2));

                    break;
                }
                case MapObject::Polygon: {
                    auto polygon = object->polygon().toPolygon();
                    bool first = true;
                    for (auto point : polygon) {
                        if (!first)
                            device->write(", ");
                        auto x = point.x() - centerX;
                        auto y = point.y() - centerY;
                        flipState(x, y, flippedState);
                        device->write(FormatByteString("%1, %2", x, y));
                        first = false;
                    }
                    break;
                }
                default:
                    throw tscnError(
                        "Godot exporter only supports collisions that are "
                        "rectangles or polygons.");
            }

            device->write(")\n");
            polygonId++;
        }
    }

    return foundCollisions;
}

// Write the tileset
// If you're creating a reusable tileset file, pass in a new file device and
// set isExternal to true, otherwise, reuse the device from the tscn file.
static void writeTileset(const Map *map, QFileDevice *device, bool isExternal, AssetInfo &assetInfo) {
    bool foundCollisions = false;

    // One Texture2D and one TileSetAtlasSource per tileset, plus a resource node
    auto loadSteps = assetInfo.tilesetInfo.size() * 2 + 1;

    if (isExternal) {
        device->write(FormatByteString(
            "[gd_resource type=\"TileSet\" load_steps=%1 format=3]\n\n",
            loadSteps));
    }

    // Texture2D nodes
    for (auto it = assetInfo.tilesetInfo.begin(); it != assetInfo.tilesetInfo.end(); ++it) {
        if (it->usedTiles.isEmpty())
            continue;

        device->write(FormatByteString(
            "[ext_resource type=\"Texture2D\" path=\"%1\" id=\"%2\"]\n",
            sanitizeQuotedString(it.key()),
            sanitizeQuotedString(it->id)));
    }
    device->write("\n");

    // TileSetAtlasSource nodes
    for (auto itTileset = assetInfo.tilesetInfo.begin(); itTileset != assetInfo.tilesetInfo.end(); ++itTileset) {
        if (itTileset->usedTiles.isEmpty())
            continue;

        device->write(FormatByteString(
            "[sub_resource type=\"TileSetAtlasSource\" id=\"TileSetAtlasSource_%1\"]\n",
            itTileset->atlasId));

        device->write(FormatByteString(
            "texture = ExtResource(\"%1\")\n",
            sanitizeQuotedString(itTileset->id)));

        if (itTileset->tileset->margin() != 0)
            device->write(FormatByteString(
                "margins = Vector2i(%1, %1)\n",
                itTileset->tileset->margin()));

        if (itTileset->tileset->tileSpacing() != 0)
            device->write(FormatByteString(
                "separation = Vector2i(%1, %1)\n",
                itTileset->tileset->tileSpacing()));

        if (itTileset->tileset->tileWidth() != 16 || itTileset->tileset->tileHeight() != 16)
            device->write(FormatByteString(
                "texture_region_size = Vector2i(%1, %2)\n",
                itTileset->tileset->tileWidth(), itTileset->tileset->tileHeight()));

        bool hasAlternates = itTileset->tileset->resolvedProperty("exportAlternates").toBool();

        unsigned maxAlternate = hasAlternates ? FlippedH | FlippedV | Transposed : 0;

        // Tile info
        for (auto tile : itTileset->tileset->tiles()) {
            if (itTileset->reservedAnimationTiles.contains(tile->id()))
                continue;

            if (itTileset->usedTiles.contains(tile->id()) || tile->objectGroup()) {
                auto x = tile->id() % itTileset->tileset->columnCount();
                auto y = tile->id() / itTileset->tileset->columnCount();

                if (tile->isAnimated()) {
                    auto lastTileId = tile->id() - 1;
                    int columns = 0;

                    // Check that the frames are in the correct order
                    for (auto& frame : tile->frames()) {
                        if (frame.tileId != lastTileId + 1) {
                            if (columns == 0)
                                columns = lastTileId - tile->id() + 1;

                            if (frame.tileId != lastTileId - columns + 1 + itTileset->tileset->columnCount())
                                throw tscnError("Tile animations must flow left-to-right, "
                                    "top-to-bottom, with no skipped tiles.");   
                        }

                        lastTileId = frame.tileId;
                    }

                    device->write(FormatByteString(
                        "%1:%2/animation_frames = %3\n",
                        x, y, tile->frames().size()));

                    if (columns != 0)
                        device->write(FormatByteString(
                            "%1:%2/animation_columns = %3\n",
                            x, y, columns));

                    int frameNum = 0;
                    for (auto& frame : tile->frames()) {
                        device->write(FormatByteString(
                            "%1:%2/animation_frame_%3/duration = %4\n",
                            x, y, frameNum, frame.duration / 1000.0));

                        if (frame.tileId != tile->id())
                            itTileset->reservedAnimationTiles.insert(frame.tileId);

                        frameNum++;
                    }
                }

                // If we're using alternate tiles, give a hint for the next alt ID
                if (hasAlternates)
                    device->write(FormatByteString(
                        "%1:%2/next_alternative_id = 8\n", x, y));

                for (unsigned alt = 0; alt <= maxAlternate; ++alt) {
                    QString tileName = FormatString("%1:%2/%3", x, y, alt);

                    // Tile presence
                    device->write(FormatByteString("%1 = %2\n", tileName, alt));
                    
                    // Flip/rotate
                    if (alt & FlippedH)
                        device->write(FormatByteString("%1/flip_h = true\n", tileName));
                    if (alt & FlippedV)
                        device->write(FormatByteString("%1/flip_v = true\n", tileName));
                    if (alt & Transposed)
                        device->write(FormatByteString("%1/transpose = true\n", tileName));

                    foundCollisions |= exportTileCollisions(device, tile, tileName, alt);
                }
            }
        }

        device->write("\n");
    }

    // TileSet node
    if (isExternal)
        device->write("[resource]\n");
    else
        device->write("[sub_resource type=\"TileSet\" id=\"TileSet_0\"]\n");

    {
        int shape, layout;
        switch (map->orientation()) {
            case Map::Orthogonal:
                shape = 0;
                layout = 0;
                break;
            case Map::Staggered:
                shape = 1;
                layout = 0;
                break;
            case Map::Isometric:
                shape = 1;
                layout = 5;
                break;
            case Map::Hexagonal:
                shape = 3;
                layout = 0;

                if (map->hexSideLength() != map->tileHeight() / 2)
                    throw tscnError("Godot only supports hexagonal maps "
                        "where the total side length is exactly half its "
                        "height. For a tile height of %1, the Total Side "
                        "Length should be set to %2.",
                        map->tileHeight(),
                        map->tileHeight() / 2);
                break;
            default:
                throw tscnError("Unsupported tile orientation.");
        }

        // We could leave either of these out if they're zero, but as of
        // Godot 4.0 Beta 10, the Godot editor doesn't properly reset these
        // values to their defaults if they're missing.
        device->write(FormatByteString("tile_shape = %1\ntile_layout = %2\n",
            shape, layout));
    }

    if (foundCollisions)
        device->write("physics_layer_0/collision_layer = 1\n");
    
    if (map->tileWidth() != 16 || map->tileHeight() != 16) {
        // When Tiled renders odd-height hex tiles, it rounds down to an
        // even tile height. This is particularly useful for the Godot
        // exporter, because Godot's hex side length is always
        // tileHeight / 2, but our side length is integral.
        int tileHeight = map->tileHeight();
        if (map->orientation() == Map::Hexagonal && tileHeight % 2 != 0)
            --tileHeight;

        device->write(FormatByteString("tile_size = Vector2i(%1, %2)\n",
            map->tileWidth(), tileHeight));
    }

    for (auto it = assetInfo.tilesetInfo.begin(); it != assetInfo.tilesetInfo.end(); ++it)
        device->write(FormatByteString(
            "sources/%1 = SubResource(\"TileSetAtlasSource_%2\")\n",
            it->atlasId, it->atlasId));

    device->write("\n");
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
        auto tilesetResPath = map->propertyAsString("tilesetResPath");

        // One TileSet, one TileMap, plus a Texture2D and TileSetAtlasSource per tileset
        // (unless we're writing the tileset to an external .tres file)
        auto loadSteps = !tilesetResPath.isEmpty() ? 2 : assetInfo.tilesetInfo.size() * 2 + 2;

        // gdscene node
        device->write(FormatByteString("[gd_scene load_steps=%1 format=3]\n\n", loadSteps));

        // tileset, either inline, or as an external file
        if (tilesetResPath.isEmpty())
            writeTileset(map, device, false, assetInfo);
        else {
            QRegularExpressionMatch match;
            if (!tilesetResPath.contains(QRegularExpression("^res://(.*\\.tres)$"), &match))
                throw tscnError("tilesetResPath must be in the form of 'res://<filename>.tres'.");

            device->write(FormatByteString(
                "[ext_resource type=\"TileSet\" path=\"%1\" id=\"TileSet_0\"]\n\n",
                sanitizeQuotedString(tilesetResPath)));

            QString resFileName = assetInfo.resRoot + '/' + match.captured(1);
            SaveFile tilesetFile(resFileName);
            if (!tilesetFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
                mError += QLatin1String("\n");
                mError += resFileName;
                return false;
            }
            auto tilesetDevice = tilesetFile.device();

            writeTileset(map, tilesetDevice, true, assetInfo);

            if (tilesetFile.error() != QFileDevice::NoError) {
                mError = tilesetFile.errorString();
                return false;
            }

            if (!tilesetFile.commit()) {
                mError = tilesetFile.errorString();
                return false;
            }
        }

        // TileMap node
        device->write(FormatByteString("[node name=\"%1\" type=\"TileMap\"]\n",
            sanitizeQuotedString(fi.baseName())));

        if (tilesetResPath.isEmpty())
            device->write("tile_set = SubResource(\"TileSet_0\")\n");
        else
            device->write("tile_set = ExtResource(\"TileSet_0\")\n");

        device->write("format = 2\n");

        // Tile packing format:
        // DestLocation, SrcX, SrcY
        // Where:
        //   DestLocation = (DestX >= 0 ? DestY : DestY + 1) * 65536 + DestX
        //   SrcX         = SrcX * 65536 + TileSetId
        //   SrcY         = SrcY + 65536 * AlternateId
        int layerIndex = 0;
        for (auto layer : assetInfo.layers) {
            device->write(FormatByteString("layer_%1/name = \"%2\"\n",
                layerIndex,
                sanitizeQuotedString(layer->name())));

            if (layer->resolvedProperty("ySortEnabled").isValid())
                device->write(FormatByteString("layer_%1/y_sort_enabled = true\n",
                    layerIndex));

            if (layer->resolvedProperty("zIndex").isValid())
                device->write(FormatByteString("layer_%1/z_index = %2\n",
                    layerIndex,
                    layer->resolvedProperty("zIndex").toInt()));

            device->write(FormatByteString("layer_%1/tile_data = PackedInt32Array(",
                layerIndex));

            bool first = true;
            auto bounds = layer->bounds();
            for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
                for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
                    auto& cell = layer->cellAt(x, y);

                    if (!cell.isEmpty()) {
                        auto resPath = imageSourceToRes(cell.tile()->tileset(), assetInfo.resRoot);
                        auto& tilesetInfo = assetInfo.tilesetInfo[resPath];

                        if (tilesetInfo.reservedAnimationTiles.contains(cell.tileId()))
                            throw tscnError("Cannot use tile %1 from tileset %2 because it is "
                                "reserved as an animation frame.",
                                cell.tileId(),
                                cell.tileset()->name());

                        int alt = 0;
                        if (cell.rotatedHexagonal120())
                            throw tscnError("Cannot export hex tiles that are rotated by 120° degrees.");
                        if (cell.flippedHorizontally())
                            alt |= FlippedH;
                        if (cell.flippedVertically())
                            alt |= FlippedV;
                        if (cell.flippedAntiDiagonally())
                            alt |= Transposed;
                        if (alt && !cell.tileset()->resolvedProperty("exportAlternates").toBool())
                            throw tscnError("Map uses flipped/rotated tiles. The tileset must have "
                                "the custom exportAlternates property enabled to export this map.");

                        int destLocation = (x >= 0 ? y : y + 1) * 65536 + x;
                        int srcX = cell.tileId() % cell.tileset()->columnCount();
                        srcX *= 65536;
                        srcX += tilesetInfo.atlasId;
                        int srcY = cell.tileId() / cell.tileset()->columnCount();
                        srcY += alt * 65536;     

                        if (!first) {
                            device->write(", ");
                        }
                        device->write(FormatByteString("%1, %2, %3",
                            destLocation, srcX, srcY));

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
