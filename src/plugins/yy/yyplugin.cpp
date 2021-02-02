/*
 * YY Tiled Plugin
 * Copyright 2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "yyplugin.h"

#include "grouplayer.h"
#include "imagelayer.h"
#include "jsonwriter.h"
#include "logginginterface.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <vector>

#include "qtcompat_p.h"

using namespace Tiled;
using namespace Yy;

namespace Yy {

struct GMRLayer
{
    bool visible = true;
    int depth = 0;
    bool userdefinedDepth = false;
    bool inheritLayerDepth = false;
    bool inheritLayerSettings = false;
    int gridX = 32;
    int gridY = 32;
    bool hierarchyFrozen = false;
    QString resourceVersion = QStringLiteral("1.0");
    QString name;
    QStringList tags;
};

struct GMRGraphic
{
    QString spriteId;
    bool isSprite = false;

    union {
        // part of a bigger sprite (isSprite == false)
        struct {
            int w;
            int h;
            int u0;
            int v0;
            int u1;
            int v1;
        };

        // for whole sprites (isSprite == true)
        struct {
            double headPosition;
            double rotation;
            double scaleX;
            double scaleY;
            double animationSpeed;
        };
    };

    QColor colour = Qt::white;
    QString inheritedItemId;
    QString inheritedItemPath;
    bool frozen = false;
    bool ignore = false;
    bool inheritItemSettings = false;
    double x = 0.0;
    double y = 0.0;
    QString resourceVersion = QStringLiteral("1.0");
    QString name;
    QStringList tags;
};

struct GMRInstance
{
    bool isDnd = false;
    QString objectId;
    bool inheritCode = false;
    bool hasCreationCode = false;
    QColor colour = Qt::white;
    double rotation = 0.0;
    double scaleX = 1.0;
    double scaleY = 1.0;
    int imageIndex = 0;
    double imageSpeed = 1.0;
    QString inheritedItemId;
    QString inheritedItemPath;
    bool frozen = false;
    bool ignore = false;
    bool inheritItemSettings = false;
    double x = 0.0;
    double y = 0.0;
    QString resourceVersion = QStringLiteral("1.0");
    QString name;
    QStringList tags;
};

struct GMPath
{
    int kind = 0;
    bool closed = false;
    int precision = 4;
    QVector<QPointF> points;
    QString name;
    QStringList tags;
};

struct Context
{
    int depth = 0;
    QSet<QString> usedNames;
    std::vector<GMPath> paths;
};

} // namespace Yy

template <typename T>
static T optionalProperty(const Object *object, const QString &name, const T &def)
{
    const QVariant var = object->resolvedProperty(name);
    return var.isValid() ? var.value<T>() : def;
}

template <typename T>
static void writeProperty(JsonWriter &json,
                          const Object *object,
                          const QString &propertyName,
                          const char *memberName,
                          const T &def)
{
    const T value = optionalProperty(object, propertyName, def);
    json.writeMember(memberName, value);
}

template <typename T>
static void writeProperty(JsonWriter &json,
                          const Object *object,
                          const char *name,
                          const T &def)
{
    writeProperty(json, object, QString::fromLatin1(name), name, def);
}

static QStringList readTags(const Object *object)
{
    const QString tags = optionalProperty(object, "tags", QString());
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QStringList tagList = tags.split(QLatin1Char(','), QString::SkipEmptyParts);
#else
    const QStringList tagList = tags.split(QLatin1Char(','), Qt::SkipEmptyParts);
#endif
    return tagList;
}

static void writeTags(JsonWriter &json, const QStringList &tags)
{
    json.writeMember("tags", QJsonArray::fromStringList(tags));
}

static void writeTags(JsonWriter &json, const Object *object)
{
    writeTags(json, readTags(object));
}

static QString sanitizeName(QString name)
{
    static const QRegularExpression regexp(QLatin1String("[^a-zA-Z0-9]"));
    return name.replace(regexp, QStringLiteral("_"));
}

static void writeLayers(JsonWriter &json, const QList<Layer *> &layers, Context &context)
{
    json.writeStartArray("layers");

    for (const Layer *layer : layers) {
        json.prepareNewLine();
        json.writeStartObject();

        auto layerOffset = layer->totalOffset().toPoint();

        GMRLayer gmrLayer;

        gmrLayer.visible = optionalProperty(layer, "visible", true);
        gmrLayer.depth = optionalProperty(layer, "depth", context.depth);
        gmrLayer.userdefinedDepth = layer->resolvedProperty(QStringLiteral("depth")).isValid();
        gmrLayer.inheritLayerDepth = optionalProperty(layer, "inheritLayerDepth", false);
        gmrLayer.inheritLayerSettings = optionalProperty(layer, "inheritLayerSettings", false);
        gmrLayer.gridX = optionalProperty(layer, "gridX", layer->map()->tileWidth());
        gmrLayer.gridY = optionalProperty(layer, "gridY", layer->map()->tileHeight());
        gmrLayer.hierarchyFrozen = layer->isLocked();
        gmrLayer.name = layer->name();
        gmrLayer.tags = readTags(layer);

        context.depth = gmrLayer.depth + 100;   // TODO: Better support for overridden depth logic

        std::vector<GMRGraphic> graphics;
        std::vector<GMRInstance> instances;
        std::vector<GMPath> paths;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            auto tileLayer = static_cast<const TileLayer*>(layer);
            auto tilesets = tileLayer->usedTilesets().values();

            if (!tilesets.isEmpty()) {
                const auto tileset = tilesets.takeFirst().data();

                json.writeStartObject("tilesetId");
                json.writeMember("name", tileset->name());
                writeProperty(json, tileset, "path", QStringLiteral("tilesets/%1/%1.yy").arg(tileset->name()));
                json.writeEndObject();

                json.writeMember("x", layerOffset.x());
                json.writeMember("y", layerOffset.y());

                json.writeStartObject("tiles");
                json.writeMember("SerialiseWidth", tileLayer->width());
                json.writeMember("SerialiseHeight", tileLayer->height());
                json.writeStartArray("TileSerialiseData");

                constexpr unsigned Unintialized         = 0x80000000;
                constexpr unsigned FlippedHorizontally  = 0x10000000;
                constexpr unsigned FlippedVertically    = 0x20000000;
                constexpr unsigned Rotated90            = 0x40000000;

                for (int y = 0; y < tileLayer->height(); ++y) {
                    json.prepareNewLine();

                    for (int x = 0; x < tileLayer->width(); ++x) {
                        const Cell &cell = tileLayer->cellAt(x, y);
                        if (cell.tileset() != tileset) {
                            json.writeValue(Unintialized);
                            continue;
                        }

                        unsigned tileId = cell.tileId();

                        if (cell.flippedAntiDiagonally()) {
                            tileId |= Rotated90;
                            if (cell.flippedVertically())
                                tileId |= FlippedHorizontally;
                            if (!cell.flippedHorizontally())
                                tileId |= FlippedVertically;
                        } else {
                            if (cell.flippedHorizontally())
                                tileId |= FlippedHorizontally;
                            if (cell.flippedVertically())
                                tileId |= FlippedVertically;
                        }

                        json.writeValue(tileId);
                    }
                }

                json.writeEndArray();   // TileSerialiseData
                json.writeEndObject();  // tiles
            }
            break;
        }
        case Layer::ObjectGroupType: {
            auto objectGroup = static_cast<const ObjectGroup*>(layer);
            const bool frozen = !layer->isUnlocked();
            auto color = layer->effectiveTintColor();
            color.setAlphaF(color.alphaF() * layer->effectiveOpacity());

            for (const MapObject *mapObject : *objectGroup) {
                if (!mapObject->type().isEmpty())
                {
                    // TODO: Export as instance
                }
                else if (mapObject->isTileObject())
                {
                    const Cell &cell = mapObject->cell();
                    const Tile *tile = cell.tile();
                    if (!tile)
                        continue;

                    graphics.emplace_back();
                    GMRGraphic &g = graphics.back();

                    g.isSprite = !tile->imageSource().isEmpty();

                    QPointF origin(optionalProperty(mapObject, "originX", 0.0),
                                   optionalProperty(mapObject, "originY", 0.0));

                    // Tile objects don't necessarily have top-left origin in Tiled,
                    // so the position needs to be translated for top-left origin in
                    // GameMaker, taking into account the rotation.
                    origin -= alignmentOffset(mapObject->bounds(), mapObject->alignment());

                    if (g.isSprite) {
                        g.spriteId = QFileInfo(tile->imageSource().path()).completeBaseName();
                        g.headPosition = optionalProperty(mapObject, "headPosition", 0.0);
                        g.rotation = -mapObject->rotation();

                        const QSize tileSize = tile->size();
                        g.scaleX = mapObject->width() / tileSize.width();
                        g.scaleY = mapObject->height() / tileSize.height();

                        if (cell.flippedHorizontally()) {
                            g.scaleX *= -1;
                            origin += QPointF(mapObject->width() - 2 * origin.x(), 0);
                        }
                        if (cell.flippedVertically()) {
                            g.scaleY *= -1;
                            origin += QPointF(0, mapObject->height() - 2 * origin.y());
                        }

                        // Allow overriding the scale using custom properties
                        g.scaleX = optionalProperty(mapObject, "scaleX", g.scaleX);
                        g.scaleY = optionalProperty(mapObject, "scaleY", g.scaleY);

                        g.animationSpeed = optionalProperty(mapObject, "animationSpeed", 1.0);
                    } else {
                        const Tileset *tileset = tile->tileset();
                        g.spriteId = QFileInfo(tileset->imageSource().path()).completeBaseName();
                        g.w = qRound(mapObject->width());
                        g.h = qRound(mapObject->height());

                        const int xInTilesetGrid = tile->id() % tileset->columnCount();
                        const int yInTilesetGrid = static_cast<int>(tile->id() / tileset->columnCount());

                        g.u0 = tileset->margin() + (tileset->tileSpacing() + tileset->tileWidth()) * xInTilesetGrid;
                        g.v0 = tileset->margin() + (tileset->tileSpacing() + tileset->tileHeight()) * yInTilesetGrid;
                        g.u1 = g.u0 + tileset->tileWidth();
                        g.v1 = g.v0 + tileset->tileHeight();
                    }

                    // Adjust the position based on the origin
                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF pos = mapObject->position() + transform.map(origin);

                    g.colour = color.rgba();
                    // TODO: g.inheritedItemId
                    g.frozen = frozen;
                    g.ignore = optionalProperty(mapObject, "ignore", g.ignore);
                    g.inheritItemSettings = optionalProperty(mapObject, "inheritItemSettings", g.inheritItemSettings);
                    g.x = pos.x();
                    g.y = pos.y();

                    // Include object ID in the name when necessary because duplicates are not allowed
                    if (mapObject->name().isEmpty()) {
                        if (g.isSprite)
                            g.name = QStringLiteral("graphic_%1").arg(mapObject->id());
                        else
                            g.name = QStringLiteral("tile_%1").arg(mapObject->id());
                    } else {
                        g.name = sanitizeName(mapObject->name());

                        while (context.usedNames.contains(g.name))
                            g.name += QString("_%1").arg(mapObject->id());

                        context.usedNames.insert(g.name);
                    }

                    g.tags = readTags(mapObject);
                }
                else if (mapObject->shape() == MapObject::Polygon || mapObject->shape() == MapObject::Polyline)
                {
                    paths.emplace_back();
                    GMPath &p = paths.back();

                    p.kind = optionalProperty(mapObject, "smooth", false);
                    p.closed = mapObject->shape() == MapObject::Polygon;
                    p.precision = optionalProperty(mapObject, "precision", 4);

                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF offset = mapObject->position() + layerOffset;
                    transform.translate(offset.x(), offset.y());

                    p.points = transform.map(mapObject->polygon());

                    p.name = sanitizeName(mapObject->name());
                    if (p.name.isEmpty())
                        p.name = QStringLiteral("Path%1").arg(mapObject->id());

                    p.tags = readTags(mapObject);

                    context.paths.push_back(p);
                }
            }

            if (!graphics.empty()) {
                json.writeStartArray("assets");
                for (const auto &g : graphics) {
                    json.prepareNewLine();
                    json.writeStartObject();
                    const bool wasMinimize = json.minimize();
                    json.setMinimize(true);

                    json.writeStartObject("spriteId");
                    json.writeMember("name", g.spriteId);
                    json.writeMember("path", QStringLiteral("sprites/%1/%1.yy").arg(g.spriteId));
                    json.writeEndObject();  // spriteId

                    if (g.isSprite) {
                        json.writeMember("headPosition", g.headPosition);
                        json.writeMember("rotation", g.rotation);
                        json.writeMember("scaleX", g.scaleX);
                        json.writeMember("scaleY", g.scaleY);
                        json.writeMember("animationSpeed", g.animationSpeed);
                    } else {
                        json.writeMember("w", g.w);
                        json.writeMember("h", g.h);
                        json.writeMember("u0", g.u0);
                        json.writeMember("v0", g.v0);
                        json.writeMember("u1", g.u1);
                        json.writeMember("v1", g.v1);
                    }
                    json.writeMember("colour", g.colour.rgba());
                    if (g.inheritedItemId.isEmpty()) {
                        json.writeMember("inheritedItemId", QJsonValue(QJsonValue::Null));
                    } else {
                        json.writeStartObject("inheritedItemId");
                        json.writeMember("name", g.inheritedItemId);
                        json.writeMember("path", g.inheritedItemPath);
                        json.writeEndObject();
                    }
                    json.writeMember("frozen", g.frozen);
                    json.writeMember("ignore", g.ignore);
                    json.writeMember("inheritItemSettings", g.inheritItemSettings);
                    json.writeMember("x", g.x);
                    json.writeMember("y", g.y);
                    json.writeMember("resourceVersion", g.resourceVersion);
                    json.writeMember("name", g.name);
                    writeTags(json, g.tags);
                    json.writeMember("resourceType", g.isSprite ? "GMRSpriteGraphic" : "GMRGraphic");

                    json.writeEndObject();
                    json.setMinimize(wasMinimize);
                }
                json.writeEndArray();
            }

            // TODO: instances

            // TODO: pathId

            break;
        }
        case Layer::ImageLayerType: {
            auto imageLayer = static_cast<const ImageLayer*>(layer);

            json.writeStartObject("spriteId");
            const QString spriteName = QFileInfo(imageLayer->imageSource().toLocalFile()).completeBaseName();
            json.writeMember("name", spriteName);
            json.writeMember("path", QStringLiteral("sprites/%1/%1.yy").arg(spriteName));
            json.writeEndObject();

            auto color = layer->effectiveTintColor();
            color.setAlphaF(color.alphaF() * layer->effectiveOpacity());
            json.writeMember("colour", color.rgba());

            json.writeMember("x", layerOffset.x());
            json.writeMember("y", layerOffset.y());

            writeProperty(json, layer, "htiled", false);
            writeProperty(json, layer, "vtiled", false);
            writeProperty(json, layer, "hspeed", 0.0);
            writeProperty(json, layer, "vspeed", 0.0);
            writeProperty(json, layer, "stretch", false);
            writeProperty(json, layer, "animationFPS", 15);
            writeProperty(json, layer, "animationSpeedType", 0);
            json.writeMember("userdefinedAnimFPS", layer->resolvedProperty(QStringLiteral("animationFPS")).isValid());
            break;
        }
        case Layer::GroupLayerType:
            break;
        }

        json.writeMember("visible", gmrLayer.visible);
        json.writeMember("depth", gmrLayer.depth);
        json.writeMember("userdefinedDepth", gmrLayer.userdefinedDepth);
        json.writeMember("inheritLayerDepth", gmrLayer.inheritLayerDepth);
        json.writeMember("inheritLayerSettings", gmrLayer.inheritLayerSettings);
        json.writeMember("gridX", gmrLayer.gridX);
        json.writeMember("gridY", gmrLayer.gridY);

        if (layer->isGroupLayer()) {
            auto groupLayer = static_cast<const GroupLayer*>(layer);
            writeLayers(json, groupLayer->layers(), context);
        } else {
            // TODO: Sub-layers for tile layers using multiple tilesets
            // TODO: Sub-layers for object layers containing instances, assets and/or paths
            writeLayers(json, {}, context);


            std::vector<GMRGraphic> graphics;
            std::vector<GMRInstance> instances;
            std::vector<GMPath> paths;
        }

        json.writeMember("hierarchyFrozen", gmrLayer.hierarchyFrozen);
        json.writeMember("resourceVersion", gmrLayer.resourceVersion);
        json.writeMember("name", gmrLayer.name);
        writeTags(json, gmrLayer.tags);

        switch (layer->layerType()) {
        case Layer::TileLayerType:
            json.writeMember("resourceType", "GMRTileLayer");
            break;
        case Layer::ObjectGroupType:
            json.writeMember("resourceType", "GMRAssetLayer");   // and/or GMRInstanceLayer
            // TODO: Convert polygons to GMPath on GMRPathLayer
            break;
        case Layer::ImageLayerType:
            json.writeMember("resourceType", "GMRBackgroundLayer");
            break;
        case Layer::GroupLayerType:
            json.writeMember("resourceType", "GMRLayer");
            break;
        }

        json.writeEndObject();
    }

    json.writeEndArray();   // layers
}

static bool checkIfViewsDefined(const Map *map)
{
    LayerIterator iterator(map);
    while (const Layer *layer = iterator.next()) {

        if (layer->layerType() != Layer::ObjectGroupType)
            continue;

        const ObjectGroup *objectLayer = static_cast<const ObjectGroup*>(layer);

        for (const MapObject *object : objectLayer->objects()) {
            const QString type = object->effectiveType();
            if (type == "view")
                return true;
        }
    }

    return false;
}

YyPlugin::YyPlugin()
{
}

bool YyPlugin::write(const Map *map, const QString &fileName, Options options)
{
    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    JsonWriter json(file.device());

    json.setMinimize(options.testFlag(WriteMinimized));

    json.writeStartObject();

    writeProperty(json, map, "isDnd", false);
    writeProperty(json, map, "volume", 1.0);
    json.writeMember("parentRoom", QJsonValue(QJsonValue::Null));    // TODO: Provide a way to set this?

    // Check if views are defined
    const bool enableViews = checkIfViewsDefined(map);

    // Write out views
    // Last view in Object layer is the first view in the room
    if (enableViews) {
        json.writeStartArray("views");
        int viewCount = 0;
        for (const Layer *layer : map->objectGroups()) {
            const ObjectGroup *objectLayer = static_cast<const ObjectGroup*>(layer);

            for (const MapObject *object : objectLayer->objects()) {
                const QString type = object->effectiveType();
                if (type != "view")
                    continue;

                // GM only has 8 views so drop anything more than that
                if (viewCount > 7) {
                    Tiled::ERROR(QLatin1String("YY plugin: Can't export more than 8 views."),
                                 Tiled::JumpToObject { object });
                    break;
                }

                viewCount++;
                json.prepareNewLine();
                json.writeStartObject();
                const bool wasMinimize = json.minimize();
                json.setMinimize(true);

                writeProperty(json, object, "inherit", false);
                json.writeMember("visible", object->isVisible());
                // Note: GM only supports ints for positioning
                // so views could be off if user doesn't align to whole number
                json.writeMember("xview", qRound(object->x()));
                json.writeMember("yview", qRound(object->y()));
                json.writeMember("wview", qRound(object->width()));
                json.writeMember("hview", qRound(object->height()));
                // Round these incase user adds properties as floats and not ints
                json.writeMember("xport", qRound(optionalProperty(object, "xport", 0.0)));
                json.writeMember("yport", qRound(optionalProperty(object, "yport", 0.0)));
                json.writeMember("wport", qRound(optionalProperty(object, "wport", 1024.0)));
                json.writeMember("hport", qRound(optionalProperty(object, "hport", 768.0)));
                json.writeMember("hborder", qRound(optionalProperty(object, "hborder", 32.0)));
                json.writeMember("vborder", qRound(optionalProperty(object, "vborder", 32.0)));
                json.writeMember("hspeed", qRound(optionalProperty(object, "hspeed", -1.0)));
                json.writeMember("vspeed", qRound(optionalProperty(object, "vspeed", -1.0)));
                json.writeMember("objectId", QJsonValue(QJsonValue::Null));    // TODO: Provide a way to set this?

                json.writeEndObject();
                json.setMinimize(wasMinimize);
            }
        }

        json.writeEndArray();
    }

    Context context;
    writeLayers(json, map->layers(), context);

    writeProperty(json, map, "inheritLayers", false);
    writeProperty(json, map, "creationCodeFile", QString());
    writeProperty(json, map, "inheritCode", false);

    json.writeStartArray("instanceCreationOrder");
    // TODO
    //      {"name":"inst_967BF0","path":"rooms/Room1/Room1.yy",},
    //      {"name":"inst_1D2061C1","path":"rooms/Room1/Room1.yy",},
    //      {"name":"inst_6C3C7802","path":"rooms/Room1/Room1.yy",},
    json.writeEndArray();

    writeProperty(json, map, "inheritCreationOrder", false);
    json.writeMember("sequenceId", QJsonValue(QJsonValue::Null));

    const int mapPixelWidth = map->tileWidth() * map->width();
    const int mapPixelHeight = map->tileHeight() * map->height();

    json.writeStartObject("roomSettings");
    writeProperty(json, map, "inheritRoomSettings", false);
    json.writeMember("Width", mapPixelWidth);
    json.writeMember("Height", mapPixelHeight);
    writeProperty(json, map, "persistent", false);
    json.writeEndObject();

    json.writeStartObject("viewSettings");
    writeProperty(json, map, "inheritViewSettings", false);
    writeProperty(json, map, "enableViews", enableViews);
    writeProperty(json, map, "clearViewBackground", false);
    writeProperty(json, map, "clearDisplayBuffer", true);
    json.writeEndObject();

    json.writeStartObject("physicsSettings");
    writeProperty(json, map, "inheritPhysicsSettings", false);
    writeProperty(json, map, "PhysicsWorld", false);
    writeProperty(json, map, "PhysicsWorldGravityX", 0.0);
    writeProperty(json, map, "PhysicsWorldGravityY", 10.0);
    writeProperty(json, map, "PhysicsWorldPixToMetres", 0.1);
    json.writeEndObject();

    json.writeStartObject("parent");
    const QString path = optionalProperty(map, "path", QStringLiteral("folders/Rooms.yy"));
    json.writeMember("name", QFileInfo(path).completeBaseName());
    json.writeMember("path", path);
    json.writeEndObject();

    writeProperty(json, map, "resourceVersion", QString("1.0"));
    writeProperty(json, map, "name", QFileInfo(fileName).completeBaseName());
    writeTags(json, map);
    json.writeMember("resourceType", "GMRoom");

    json.writeEndObject();
    json.writeEndDocument();

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString YyPlugin::errorString() const
{
    return mError;
}

QString YyPlugin::nameFilter() const
{
    return tr("GameMaker Studio 2 files (*.yy)");
}

QString YyPlugin::shortName() const
{
    return QStringLiteral("yy");
}
