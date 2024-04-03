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
#include "logginginterface.h"
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
#include <map>
#include <stdexcept>

using namespace Tiled;
using namespace Tscn;

TscnPlugin::TscnPlugin()
{
}

// Convenience functions for formatting strings. This is very similar to
// QString::arg function, but it also supports multiple numbers.
template <typename T>
static QString formatString(const QString &fmt, T&& arg)
{
    return fmt.arg(arg);
}

template <typename T1, typename... Types>
static QString formatString(const QString &fmt, T1&& t1, Types&&... args)
{
    return formatString(fmt.arg(t1), args...);
}

// Same as formatString, but returns a byte string.
// Used for device->write(), which only accepts byte strings.
template <typename... Types>
static QByteArray formatByteString(const QString &fmt, Types&&... args)
{
    return formatString(fmt, args...).toUtf8();
}

// Convenience functions for throwing translated errors
static std::invalid_argument tscnError(const QString &error)
{
    return std::invalid_argument(error.toStdString());
}

/**
 * Determines the res:// root for the Godot project. This is done by searching
 * for a .godot file in the parent paths of the file.
 *
 * Throws std::invalid_argument if there's no .godot file found.
 */
static QString determineResRoot(const QString &filePath)
{
    QDir dir(QFileInfo(filePath).path());
    dir.setNameFilters(QStringList("*.godot"));

    do {
        const QString godotFile = QDirIterator(dir).next();
        if (!godotFile.isEmpty())
            return dir.path();
    } while (dir.cdUp());

    throw tscnError(TscnPlugin::tr("Could not find .godot project in file path for file %1").arg(filePath));
}

// Converts a tileset's image source to a res:// path
// If resRoot is empty, this will find the nearest .godot project and set that to the resRoot,
// otherwise, it will ensure that the image source is a child of resRoot.
static QString imageSourceToRes(const Tileset *tileset, QString &resRoot)
{
    auto filePath = tileset->imageSource().toLocalFile();

    if (resRoot.isEmpty())
        resRoot = determineResRoot(filePath);

    if (!filePath.startsWith(resRoot))
        throw tscnError(TscnPlugin::tr("All files must share the same project root. File '%1' does not share project root '%2'.").arg(filePath, resRoot));

    return "res:/" + filePath.right(filePath.length() - resRoot.length());
}

// Replace any instance of " with \" so you can include it in a quoted string
static QString sanitizeQuotedString(QString str)
{
    return str.replace(QLatin1Char('"'), QStringLiteral("\\\""));
}

// https://docs.godotengine.org/en/latest/classes/class_tileset.html#enum-tileset-tileshape
enum TileShape {
    TILE_SHAPE_SQUARE = 0,
    TILE_SHAPE_ISOMETRIC = 1,
    TILE_SHAPE_HALF_OFFSET_SQUARE = 2,
    TILE_SHAPE_HEXAGON = 3,
};

// https://docs.godotengine.org/en/latest/classes/class_tileset.html#enum-tileset-tilelayout
enum TileLayout {
    TILE_LAYOUT_STACKED = 0,
    TILE_LAYOUT_STACKED_OFFSET = 1,
    TILE_LAYOUT_STAIRS_RIGHT = 2,
    TILE_LAYOUT_STAIRS_DOWN = 3,
    TILE_LAYOUT_DIAMOND_RIGHT = 4,
    TILE_LAYOUT_DIAMOND_DOWN = 5,
};

// https://docs.godotengine.org/en/latest/classes/class_%40globalscope.html#enum-globalscope-variant-type
enum VariantType {
    TYPE_NIL = 0,
    TYPE_BOOL = 1,
    TYPE_INT = 2,
    TYPE_FLOAT = 3,
    TYPE_STRING = 4,
    TYPE_VECTOR2 = 5,
    TYPE_VECTOR2I = 6,
    TYPE_RECT2 = 7,
    TYPE_RECT2I = 8,
    TYPE_VECTOR3 = 9,
    TYPE_VECTOR3I = 10,
    TYPE_TRANSFORM2D = 11,
    TYPE_VECTOR4 = 12,
    TYPE_VECTOR4I = 13,
    TYPE_PLANE = 14,
    TYPE_QUATERNION = 15,
    TYPE_AABB = 16,
    TYPE_BASIS = 17,
    TYPE_TRANSFORM3D = 18,
    TYPE_PROJECTION = 19,
    TYPE_COLOR = 20,
    TYPE_STRING_NAME = 21,
    TYPE_NODE_PATH = 22,
    TYPE_RID = 23,
    TYPE_OBJECT = 24,
    TYPE_CALLABLE = 25,
    TYPE_SIGNAL = 26,
    TYPE_DICTIONARY = 27,
    TYPE_ARRAY = 28,
    TYPE_PACKED_BYTE_ARRAY = 29,
    TYPE_PACKED_INT32_ARRAY = 30,
    TYPE_PACKED_INT64_ARRAY = 31,
    TYPE_PACKED_FLOAT32_ARRAY = 32,
    TYPE_PACKED_FLOAT64_ARRAY = 33,
    TYPE_PACKED_STRING_ARRAY = 34,
    TYPE_PACKED_VECTOR2_ARRAY = 35,
    TYPE_PACKED_VECTOR3_ARRAY = 36,
    TYPE_PACKED_COLOR_ARRAY = 37,
    TYPE_MAX = 38,
};

static VariantType variantType(const QVariant &value)
{
    switch (value.userType()) {
    case QMetaType::Bool:
        return TYPE_BOOL;

    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::SChar:
    case QMetaType::Short:
    case QMetaType::UChar:
    case QMetaType::UInt:
    case QMetaType::UShort:
        return TYPE_INT;

    case QMetaType::Float:
    case QMetaType::Double:
        return TYPE_FLOAT;

    case QMetaType::QString:
        return TYPE_STRING;

    case QMetaType::QQuaternion:
        return TYPE_QUATERNION;

    case QMetaType::QColor:
        return TYPE_COLOR;

    case QMetaType::QVariantMap:
    case QMetaType::QVariantHash:
        return TYPE_DICTIONARY;

    case QMetaType::QVariantList:
    case QMetaType::QStringList:
        return TYPE_ARRAY;

    default:
        if (value.userType() == filePathTypeId()) {
            return TYPE_STRING;
        } else if (value.userType() == objectRefTypeId()) {
            return TYPE_INT;
        } else if (value.userType() == propertyValueId()) {
            const auto propertyValue = value.value<PropertyValue>();
            if (propertyValue.type()->isClass()) {
                return TYPE_DICTIONARY;
            } else if (propertyValue.type()->isEnum()) {
                return TYPE_INT;
            }
        }
        return TYPE_NIL;
    }
}

struct CustomDataLayer
{
    VariantType type = TYPE_NIL;
    int index = 0;
};

// Remove any special chars from a string
static QString sanitizeSpecialChars(QString str)
{
    static QRegularExpression sanitizer("[^a-zA-Z0-9]");
    return str.replace(sanitizer, "");
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
    QMap<QString, QString> objectIds;           // Map resPaths to unique IDs
    QList<const MapObject*> objects;
    QString resRoot;
    std::map<QString, CustomDataLayer> customDataLayers;
};

// Adds a tileset to the assetInfo struct
static void addTileset(Tileset *tileset, AssetInfo &assetInfo)
{
    if (tileset->isCollection())
        throw tscnError(TscnPlugin::tr("Cannot export tileset '%1' because the Godot exporter does not support collection-type tilesets.").arg(tileset->name()));

    auto resPath = imageSourceToRes(tileset, assetInfo.resRoot);
    auto& tilesetInfo = assetInfo.tilesetInfo[resPath];
    if (!tilesetInfo.tileset) {
        tilesetInfo.tileset = tileset->sharedPointer();

        // Find the tiles that aren't blank or have properties. Also determine
        // the Custom Data Layers.
        auto image = tileset->image().toImage();
        for (const auto tile : tileset->tiles()) {
            bool blank = true;

            const auto properties = tile->resolvedProperties();

            if (!tile->className().isEmpty() || !properties.isEmpty()) {
                // Always export tiles with class name or custom properties, since
                // this data might be relevant.
                blank = false;
            } else {
                // Otherwise, export the tile if it isn't entirely transparent
                const auto rect = tile->imageRect();
                for (auto y = rect.top(); blank && y <= rect.bottom(); ++y)
                    for (auto x = rect.left(); blank && x <= rect.right(); ++x)
                        blank &= image.pixelColor(x, y).alpha() == 0;
            }

            if (blank)
                continue;

            tilesetInfo.usedTiles.insert(tile->id());

            // Set up custom data layers to cover all used tile properties
            Properties::const_iterator it = properties.begin();
            const Properties::const_iterator it_end = properties.end();
            for (; it != it_end; ++it) {
                const QString &name = it.key();
                const QVariant &value = it.value();

                const auto type = variantType(value);
                if (type == TYPE_NIL) {
                    Tiled::WARNING(TscnPlugin::tr("Godot exporter does not support property type of '%1'").arg(name));
                    continue;
                }

                auto &customDataLayer = assetInfo.customDataLayers[name];
                if (customDataLayer.type == TYPE_NIL)
                    customDataLayer.type = type;
                else if (customDataLayer.type != type)
                    Tiled::WARNING(TscnPlugin::tr("Inconsistent type for property '%1'").arg(name));
            }
        }
    }
}

// Search a layer for every tileset that was used and store it in assetInfo
static void findUsedTilesets(const TileLayer *layer, AssetInfo &assetInfo)
{
    auto bounds = layer->localBounds();
    for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
        for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
            auto cell = layer->cellAt(x, y);
            if (!cell.isEmpty())
                addTileset(cell.tileset(), assetInfo);
        }
    }
}

// Search an object layer for all object resources and save them in assetInfo
static void findUsedObjects(const ObjectGroup *objectLayer, AssetInfo &assetInfo)
{
    static QRegularExpression resPathValidator("^res://(.*)\\.tscn$");

    for (const MapObject *object : objectLayer->objects()) {
        const auto resPath = object->resolvedProperty("resPath").toString();

        if (resPath.isEmpty()) {
            Tiled::WARNING(TscnPlugin::tr("Only objects with the resPath property will be exported"),
                           Tiled::JumpToObject { object });
            continue;
        }

        QRegularExpressionMatch match;
        if (!resPath.contains(resPathValidator, &match)) {
            Tiled::ERROR(TscnPlugin::tr("resPath must be in the form of 'res://<filename>.tscn'."),
                         Tiled::JumpToObject { object });
            continue;
        }

        const QString baseName = sanitizeSpecialChars(match.captured(1));
        int uniqueifier = 1;
        QString id = baseName;

        // Create the objectId map such that every resPath has a unique ID.
        while (true) {
            // keys() is slow. If this becomes a problem, we can create a reverse map.
            auto keys = assetInfo.objectIds.keys(id);

            if (keys.empty()) {
                assetInfo.objectIds[resPath] = id;
                break;
            }

            // The key already exists with the right value
            if (keys[0] == resPath)
                break;

            // The baseName is based off of a file path, which is unique by definition,
            // but because we santized it, paths like res://ab/c.tscn and res://a/bc.tscn
            // would both get santitized into the non-unique name of abc, so we need to
            // add a uniqueifier and try again.
            ++uniqueifier;
            id = baseName + QString::number(uniqueifier);
        }

        assetInfo.objects.append(object);
    }
}

// Used by collectAssets() to search all layers and layer groups
static void collectAssetsRecursive(const QList<Layer*> &layers, AssetInfo &assetInfo)
{
    for (const Layer *layer : layers) {
        if (layer->resolvedProperty("noExport").toBool())
            continue;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            auto tileLayer = static_cast<const TileLayer*>(layer);
            findUsedTilesets(tileLayer, assetInfo);

            if (!layer->resolvedProperty("tilesetOnly").toBool())
                assetInfo.layers.append(tileLayer);

            break;
        }
        case Layer::ObjectGroupType: {
            auto objectLayer = static_cast<const ObjectGroup*>(layer);
            findUsedObjects(objectLayer, assetInfo);
            break;
        }
        case Layer::ImageLayerType:
            Tiled::WARNING(TscnPlugin::tr("The Godot exporter does not yet support image layers"),
                           Tiled::SelectLayer { layer });
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
    for (TilesetInfo &tilesetInfo : assetInfo.tilesetInfo) {
        const QString &basename = tilesetInfo.tileset->name();

        // Make sure the tileset ID is unique by adding a number to the end
        QString id = basename;
        int uniqueifier = 1;
        while (assetInfo.tilesetIds.contains(id)) {
            id = basename + "_" + QString::number(uniqueifier);
            uniqueifier++;
        }

        tilesetInfo.id = id;
        tilesetInfo.atlasId = i;
        assetInfo.tilesetIds.insert(id);
        ++i;
    }

    // Assign indexes to the custom data layers
    i = 0;
    for (auto& [name, layer] : assetInfo.customDataLayers) {
        layer.index = i;
        ++i;
    }

    return assetInfo;
}

enum FlippedState {
    // exportAlternate Deprecation Note: Change to 1<<12, 1<<13, 1<<14 when exportAlternates is removed
    FlippedH = 1,
    FlippedV = 2,
    Transposed = 4
};

static void flipState(double& x, double& y, int flippedState)
{
    if (flippedState & Transposed)
        std::swap(x, y);
    if (flippedState & FlippedH)
        x *= -1;
    if (flippedState & FlippedV)
        y *= -1;
}

// Export a tile's object groups as Godot physics layers
static bool exportTileCollisions(QFileDevice *device, const Tile *tile,
                                 const QString &tileName, int flippedState)
{
    bool foundCollisions = false;

    if (const auto objectGroup = tile->objectGroup()) {
        int polygonId = 0;

        const auto centerX = tile->width() / 2;
        const auto centerY = tile->height() / 2;

        for (const auto object : objectGroup->objects()) {
            const auto shape = object->shape();

            if (shape != MapObject::Rectangle && shape != MapObject::Polygon) {
                Tiled::WARNING(TscnPlugin::tr("Godot exporter only supports collisions that are rectangles or polygons."),
                               Tiled::SelectTile { tile });
                continue;
            }

            foundCollisions = true;

            device->write(formatByteString(
                "%1/physics_layer_0/polygon_%2/points = PackedVector2Array(",
                tileName, polygonId));

            switch (shape) {
            case MapObject::Rectangle: {
                auto x1 = object->x() - centerX;
                auto y1 = object->y() - centerY;
                auto x2 = object->x() + object->width() - centerX;
                auto y2 = object->y() + object->height() - centerY;

                flipState(x1, y1, flippedState);
                flipState(x2, y2, flippedState);

                device->write(formatByteString("%1, %2, %3, %2, %3, %4, %1, %4",
                                               x1, y1, x2, y2));

                break;
            }
            case MapObject::Polygon: {
                bool first = true;
                for (auto point : object->polygon()) {
                    if (!first)
                        device->write(", ");
                    auto x = object->x() + point.x() - centerX;
                    auto y = object->y() + point.y() - centerY;
                    flipState(x, y, flippedState);
                    device->write(formatByteString("%1, %2", x, y));
                    first = false;
                }
                break;
            }
            default:
                break;
            }

            device->write(")\n");
            polygonId++;
        }
    }

    return foundCollisions;
}

static bool writeProperties(QFileDevice *device, const QVariantMap &properties, const QString &resRoot);

// Write supported property values to output device
static void writePropertyValue(QFileDevice *device, const QVariant &value, const QString &resRoot)
{
    const int metaType = value.userType();

    switch (metaType) {
    case QMetaType::QString:
        device->write(formatByteString("\"%3\"", sanitizeQuotedString(value.toString())));
        break;
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::Double:
        device->write(value.toString().toUtf8());
        break;
    case QMetaType::QColor: {
        const QColor color = value.value<QColor>();
        device->write(formatByteString("Color(%1, %2, %3, %4)",
            color.redF(), color.greenF(), color.blueF(), color.alphaF()));
        break;
    }
    default:
        if (metaType == propertyValueId()) {
            const auto propertyValue = value.value<PropertyValue>();
            if (propertyValue.type()->isClass()) {
                device->write("{\n");
                bool empty = writeProperties(device, propertyValue.value.toMap(), resRoot);
                device->write(empty ? "}" : "\n}");
            } else if (propertyValue.type()->isEnum()) {
                device->write(QByteArray::number(propertyValue.value.toInt()));
            }
        } else if (metaType == filePathTypeId()) {
            const auto filePath = value.value<FilePath>();
            device->write(formatByteString("\"%1\"", sanitizeQuotedString(Tiled::toFileReference(filePath.url, resRoot))));
        } else if (metaType == objectRefTypeId()) {
            const auto objectRef = value.value<ObjectRef>();
            device->write(QByteArray::number(objectRef.id));
        } else {
            Tiled::WARNING(TscnPlugin::tr("Godot exporter does not support property of type '%1'").arg(metaType));
            device->write("0");
        }
        break;
    }
}

static bool writeProperties(QFileDevice *device, const QVariantMap &properties, const QString &resRoot)
{
    bool first = true;

    QMapIterator<QString,QVariant> it(properties);
    while (it.hasNext()) {
        it.next();

        if (!first)
            device->write(",\n");

        device->write(formatByteString("\"%2\": ", sanitizeQuotedString(it.key())));
        writePropertyValue(device, it.value(), resRoot);

        first = false;
    }

    return first;
}

// Write the ext_resource lines for any objects exported
static void writeExtObjects(QFileDevice *device, const AssetInfo &assetInfo)
{
    for (auto it = assetInfo.objectIds.begin(); it != assetInfo.objectIds.end(); ++it) {
        device->write(formatByteString(
            "[ext_resource type=\"PackedScene\" path=\"%1\" id=\"%2\"]\n",
            sanitizeQuotedString(it.key()),
            it.value()));
    }

    device->write("\n");
}

// Write the tileset
// If you're creating a reusable tileset file, pass in a new file device and
// set isExternal to true, otherwise, reuse the device from the tscn file.
static void writeTileset(const Map *map, QFileDevice *device, bool isExternal, AssetInfo &assetInfo)
{
    bool foundCollisions = false;

    if (isExternal) {
        // One Texture2D and one TileSetAtlasSource per tileset, plus a resource node
        auto loadSteps = assetInfo.tilesetInfo.size() * 2 + 1;

        device->write(formatByteString(
            "[gd_resource type=\"TileSet\" load_steps=%1 format=3]\n\n",
            loadSteps));
    }

    // Texture2D nodes
    for (auto it = assetInfo.tilesetInfo.begin(); it != assetInfo.tilesetInfo.end(); ++it) {
        if (it->usedTiles.isEmpty())
            continue;

        device->write(formatByteString(
            "[ext_resource type=\"Texture2D\" path=\"%1\" id=\"%2\"]\n",
            sanitizeQuotedString(it.key()),
            sanitizeQuotedString(it->id)));
    }

    device->write("\n");

    // TileSetAtlasSource nodes
    for (TilesetInfo &tilesetInfo : assetInfo.tilesetInfo) {
        if (tilesetInfo.usedTiles.isEmpty())
            continue;

        const Tileset &tileset = *tilesetInfo.tileset;

        device->write(formatByteString("[sub_resource type=\"TileSetAtlasSource\" id=\"TileSetAtlasSource_%1\"]\n",
                                       tilesetInfo.atlasId));

        device->write(formatByteString("texture = ExtResource(\"%1\")\n",
                                       sanitizeQuotedString(tilesetInfo.id)));

        if (tileset.margin() != 0)
            device->write(formatByteString("margins = Vector2i(%1, %1)\n",
                                           tileset.margin()));

        if (tileset.tileSpacing() != 0)
            device->write(formatByteString("separation = Vector2i(%1, %1)\n",
                                           tileset.tileSpacing()));

        if (tileset.tileWidth() != 16 || tileset.tileHeight() != 16)
            device->write(formatByteString("texture_region_size = Vector2i(%1, %2)\n",
                                           tileset.tileWidth(), tileset.tileHeight()));

        // exportAlternate Deprecation Note: Remove this block
        bool hasAlternates = tileset.resolvedProperty("exportAlternates").toBool();
        if (hasAlternates)
            Tiled::WARNING(TscnPlugin::tr("exportAlternates is deprecated. Godot 4.2 supports native tile rotation. Tileset: %1").arg(tileset.name()));
        unsigned maxAlternate = hasAlternates ? FlippedH | FlippedV | Transposed : 0;

        // Tile info
        for (const auto tile : tileset.tiles()) {
            if (tilesetInfo.reservedAnimationTiles.contains(tile->id()))
                continue;

            if (tilesetInfo.usedTiles.contains(tile->id()) || tile->objectGroup()) {
                auto x = tile->id() % tileset.columnCount();
                auto y = tile->id() / tileset.columnCount();

                if (tile->isAnimated()) {
                    auto lastTileId = tile->id() - 1;
                    int columns = 0;

                    // Check that the frames are in the correct order
                    for (auto& frame : tile->frames()) {
                        if (frame.tileId != lastTileId + 1) {
                            if (columns == 0)
                                columns = lastTileId - tile->id() + 1;

                            if (frame.tileId != lastTileId - columns + 1 + tileset.columnCount()) {
                                Tiled::ERROR(TscnPlugin::tr("Tile animations must flow left-to-right, top-to-bottom, with no skipped tiles."),
                                             Tiled::SelectTile { tile });
                                break;
                            }
                        }

                        lastTileId = frame.tileId;
                    }

                    device->write(formatByteString("%1:%2/animation_frames = %3\n",
                                                   x, y, tile->frames().size()));

                    if (columns != 0) {
                        device->write(formatByteString("%1:%2/animation_columns = %3\n",
                                                       x, y, columns));
                    }

                    int frameNum = 0;
                    for (auto& frame : tile->frames()) {
                        device->write(formatByteString(
                            "%1:%2/animation_frame_%3/duration = %4\n",
                            x, y, frameNum, frame.duration / 1000.0));

                        if (frame.tileId != tile->id())
                            tilesetInfo.reservedAnimationTiles.insert(frame.tileId);

                        frameNum++;
                    }
                }

                // If we're using alternate tiles, give a hint for the next alt ID
                // exportAlternate Deprecation Note: Remove the entire if and for blocks following
                if (hasAlternates) {
                    device->write(formatByteString("%1:%2/next_alternative_id = 8\n",
                                                   x, y));
                }

                for (unsigned alt = 0; alt <= maxAlternate; ++alt) {
                    const QString tileName = QStringLiteral("%1:%2/%3").arg(QString::number(x),
                                                                            QString::number(y),
                                                                            QString::number(alt));

                    // Tile presence
                    device->write(formatByteString("%1 = %2\n", tileName, alt));

                    // Flip/rotate
                    if (alt & FlippedH)
                        device->write(formatByteString("%1/flip_h = true\n", tileName));
                    if (alt & FlippedV)
                        device->write(formatByteString("%1/flip_v = true\n", tileName));
                    if (alt & Transposed)
                        device->write(formatByteString("%1/transpose = true\n", tileName));

                    foundCollisions |= exportTileCollisions(device, tile, tileName, alt);

                    // Custom properties
                    QMapIterator<QString,QVariant> it(tile->resolvedProperties());
                    while (it.hasNext()) {
                        it.next();
                        const QString &name = it.key();
                        const QVariant &value = it.value();

                        auto dataLayerIt = assetInfo.customDataLayers.find(name);
                        if (dataLayerIt != assetInfo.customDataLayers.end()) {
                            device->write(formatByteString("%1/custom_data_%2 = ", tileName, dataLayerIt->second.index));
                            writePropertyValue(device, value, assetInfo.resRoot);
                            device->write("\n");
                        }
                    }
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
            shape = TILE_SHAPE_SQUARE;
            layout = TILE_LAYOUT_STACKED;
            break;
        case Map::Staggered:
            shape = TILE_SHAPE_ISOMETRIC;
            layout = TILE_LAYOUT_STACKED;
            break;
        case Map::Isometric:
            shape = TILE_SHAPE_ISOMETRIC;
            layout = TILE_LAYOUT_DIAMOND_DOWN;
            break;
        case Map::Hexagonal:
            shape = TILE_SHAPE_HEXAGON;
            layout = TILE_LAYOUT_STACKED;

            if (map->hexSideLength() != map->tileHeight() / 2) {
                throw tscnError(TscnPlugin::tr("Godot only supports hexagonal maps "
                                               "where the Hex Side Length is exactly half its "
                                               "height. For a tile height of %1, the Hex Side "
                                               "Length should be set to %2.")
                                .arg(QString::number(map->tileHeight()),
                                     QString::number(map->tileHeight() / 2)));
            }
            break;
        default:
            throw tscnError(TscnPlugin::tr("Unsupported tile orientation."));
        }

        // We could leave either of these out if they're zero, but as of
        // Godot 4.0 Beta 10, the Godot editor doesn't properly reset these
        // values to their defaults if they're missing.
        device->write(formatByteString("tile_shape = %1\ntile_layout = %2\n",
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

        device->write(formatByteString("tile_size = Vector2i(%1, %2)\n",
            map->tileWidth(), tileHeight));
    }

    for (const auto& [name, layer] : assetInfo.customDataLayers) {
        device->write(formatByteString("custom_data_layer_%1/name = \"%2\"\n", layer.index, sanitizeQuotedString(name)));
        device->write(formatByteString("custom_data_layer_%1/type = %2\n", layer.index, layer.type));
    }

    for (const TilesetInfo &tilesetInfo : std::as_const(assetInfo.tilesetInfo)) {
        device->write(formatByteString(
            "sources/%1 = SubResource(\"TileSetAtlasSource_%2\")\n",
            tilesetInfo.atlasId, tilesetInfo.atlasId));
    }

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

    try {
        AssetInfo assetInfo = collectAssets(map);
        auto tilesetResPath = map->propertyAsString("tilesetResPath");

        // One TileSet, one TileMap, plus a Texture2D and TileSetAtlasSource per tileset
        // (unless we're writing the tileset to an external .tres file)
        auto loadSteps = !tilesetResPath.isEmpty() ? 2 : assetInfo.tilesetInfo.size() * 2 + 2;

        // And an extra load step per object resource
        loadSteps += assetInfo.objectIds.size();

        // gdscene node
        device->write(formatByteString("[gd_scene load_steps=%1 format=3]\n\n", loadSteps));

        writeExtObjects(device, assetInfo);

        // tileset, either inline, or as an external file
        if (tilesetResPath.isEmpty()) {
            writeTileset(map, device, false, assetInfo);
        } else {
            QRegularExpressionMatch match;
            if (!tilesetResPath.contains(QRegularExpression("^res://(.*\\.tres)$"), &match))
                throw tscnError(tr("tilesetResPath must be in the form of 'res://<filename>.tres'."));

            device->write(formatByteString(
                "[ext_resource type=\"TileSet\" path=\"%1\" id=\"TileSet_0\"]\n",
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

        // Root node
        device->write(formatByteString("[node name=\"%1\" type=\"Node2D\"]\n\n",
            sanitizeQuotedString(fi.baseName())));

        // TileMap node
        device->write("[node name=\"TileMap\" type=\"TileMap\" parent=\".\"]\n");

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
        //   SrcY         = SrcY + 65536 * (AlternateId | FLIP_H | FLIP_V | TRANSPOSE)
        int layerIndex = 0;
        for (const auto layer : std::as_const(assetInfo.layers)) {
            device->write(formatByteString("layer_%1/name = \"%2\"\n",
                                           layerIndex,
                                           sanitizeQuotedString(layer->name())));

            if (layer->resolvedProperty("ySortEnabled").isValid()) {
                device->write(formatByteString("layer_%1/y_sort_enabled = true\n",
                                               layerIndex));
            }

            if (layer->resolvedProperty("zIndex").isValid()) {
                device->write(formatByteString("layer_%1/z_index = %2\n",
                                               layerIndex,
                                               layer->resolvedProperty("zIndex").toInt()));
            }

            device->write(formatByteString("layer_%1/tile_data = PackedInt32Array(",
                                           layerIndex));

            bool first = true;
            auto bounds = layer->bounds();
            for (int y = bounds.y(); y < bounds.y() + bounds.height(); ++y) {
                for (int x = bounds.x(); x < bounds.x() + bounds.width(); ++x) {
                    auto& cell = layer->cellAt(x, y);

                    if (!cell.isEmpty()) {
                        auto resPath = imageSourceToRes(cell.tile()->tileset(), assetInfo.resRoot);
                        auto& tilesetInfo = assetInfo.tilesetInfo[resPath];

                        if (tilesetInfo.reservedAnimationTiles.contains(cell.tileId())) {
                            Tiled::ERROR(TscnPlugin::tr("Cannot use tile %1 from tileset %2 because it is "
                                                        "reserved as an animation frame.")
                                                     .arg(cell.tileId())
                                                     .arg(cell.tileset()->name()),
                                         Tiled::SelectTile { cell.tile() });
                        }

                        int alt = 0;
                        if (cell.rotatedHexagonal120()) {
                            Tiled::ERROR(TscnPlugin::tr("Hex tiles that are rotated by 120° degrees are not supported."),
                                         Tiled::JumpToTile { map, QPoint(x, y), layer });
                        }
                        if (cell.flippedHorizontally())
                            alt |= FlippedH;
                        if (cell.flippedVertically())
                            alt |= FlippedV;
                        if (cell.flippedAntiDiagonally())
                            alt |= Transposed;
                        // exportAlternate Deprecation Note: Remove this if block
                        if (alt && !cell.tileset()->resolvedProperty("exportAlternates").toBool()) {
                            alt <<= 12;
                        }

                        int destLocation = (x >= 0 ? y : y + 1) * 65536 + x;
                        int srcX = cell.tileId() % cell.tileset()->columnCount();
                        srcX *= 65536;
                        srcX += tilesetInfo.atlasId;
                        int srcY = cell.tileId() / cell.tileset()->columnCount();
                        srcY += alt * 65536;     

                        if (!first)
                            device->write(", ");

                        device->write(formatByteString("%1, %2, %3",
                                                       destLocation, srcX, srcY));

                        first = false;
                    }
                }
            }

            device->write(")\n");

            layerIndex++;
        }

        device->write("\n");
                    
        // Object scene nodes
        for (const MapObject *object : assetInfo.objects) {
            device->write("\n");

            auto name = object->name();
            if (name.isEmpty())
                name = "Object" + QString::number(object->id());

            const auto resPath = object->resolvedProperty("resPath").toString();

            device->write(formatByteString(
                "[node name=\"%1\" parent=\".\" instance=ExtResource(\"%2\")]\n",
                sanitizeQuotedString(name),
                sanitizeQuotedString(assetInfo.objectIds[resPath]))
            );

            // Convert Tiled's alignment position to Godot's centre-aligned position.
            QPointF pos =
                object->position() -
                Tiled::alignmentOffset(object->size(), object->alignment()) +
                QPointF(object->width()/2, object->height()/2);

            device->write(formatByteString(
                "position = Vector2(%1, %2)\n",
                pos.x(),
                pos.y()
            ));
        }
    } catch (std::exception& e) {
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
