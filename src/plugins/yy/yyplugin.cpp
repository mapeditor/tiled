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

enum ResourceType
{
    GMPathType,
    GMRAssetLayerType,
    GMRBackgroundLayerType,
    GMRGraphicType,
    GMRInstanceLayerType,
    GMRInstanceType,
    GMRLayerType,
    GMRPathLayerType,
    GMRTileLayerType,
};

static const char *resourceTypeStr(ResourceType type)
{
    switch (type) {
    case GMPathType:                return "GMPath";
    case GMRAssetLayerType:         return "GMRAssetLayer";
    case GMRBackgroundLayerType:    return "GMRBackgroundLayer";
    case GMRGraphicType:            return "GMRGraphic";
    case GMRInstanceLayerType:      return "GMRInstanceLayer";
    case GMRInstanceType:           return "GMRInstance";
    case GMRLayerType:              return "GMRLayer";
    case GMRPathLayerType:          return "GMRPathLayer";
    case GMRTileLayerType:          return "GMRTileLayer";
    }

    return "Unknown";
}

struct GMRView
{
    bool inherit = false;
    bool visible = false;
    int xview = 0;
    int yview = 0;
    int wview = 1366;
    int hview = 768;
    int xport = 0;
    int yport = 0;
    int wport = 1366;
    int hport = 768;
    int hborder = 32;
    int vborder = 32;
    int hspeed = -1;
    int vspeed = -1;
    QJsonValue objectId = QJsonValue(QJsonValue::Null);
};

struct GMResource
{
    GMResource(ResourceType type) : resourceType(type) {}

    QString resourceVersion = QStringLiteral("1.0");
    QString name;
    QStringList tags;
    ResourceType resourceType;
};

struct GMRGraphic : GMResource
{
    GMRGraphic() : GMResource(GMRGraphicType) {}

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
};

struct GMRInstance : GMResource
{
    GMRInstance() : GMResource(GMRInstanceType) {}

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
};

struct GMPath : GMResource
{
    GMPath() : GMResource(GMPathType) {}

    int kind = 0;
    bool closed = false;
    int precision = 4;
    QVector<QPointF> points;
};

struct GMRLayer : GMResource
{
    GMRLayer(ResourceType type = GMRLayerType) : GMResource(type) {}

    bool visible = true;
    int depth = 0;
    bool userdefinedDepth = false;
    bool inheritLayerDepth = false;
    bool inheritLayerSettings = false;
    int gridX = 32;
    int gridY = 32;
    std::vector<std::unique_ptr<GMRLayer>> layers;
    bool hierarchyFrozen = false;
};

struct GMRTileLayer : GMRLayer
{
    GMRTileLayer() : GMRLayer(GMRTileLayerType) {}

    QString tilesetId;
    int x = 0;
    int y = 0;
    int SerialiseWidth = 0;
    int SerialiseHeight = 0;
    std::vector<unsigned> tiles;
};

struct GMRAssetLayer : GMRLayer
{
    GMRAssetLayer() : GMRLayer(GMRAssetLayerType) {}

    std::vector<GMRGraphic> assets;
};

struct GMRInstanceLayer : GMRLayer
{
    GMRInstanceLayer() : GMRLayer(GMRInstanceLayerType) {}

    std::vector<GMRInstance> instances;
};

struct GMRPathLayer : GMRLayer
{
    GMRPathLayer() : GMRLayer(GMRPathLayerType) {}

    QString pathId;
    QColor colour = Qt::red;
};

struct GMRBackgroundLayer : GMRLayer
{
    GMRBackgroundLayer() : GMRLayer(GMRBackgroundLayerType) {}

    QString spriteId;
    QColor colour = Qt::white;
    int x = 0;
    int y = 0;
    bool htiled = false;
    bool vtiled = false;
    double hspeed = 0.0;
    double vspeed = 0.0;
    bool stretch = false;
    int animationFPS = 15;
    int animationSpeedType = 0;
    bool userdefinedAnimFPS = false;
};

struct Context
{
    int depth = 0;
    QSet<QString> usedNames;
    std::vector<GMRView> views;
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

static void writeId(JsonWriter &json, const char *member, const QString &id, const QString &scope)
{
    if (id.isEmpty()) {
        json.writeMember(member, QJsonValue(QJsonValue::Null));
    } else {
        json.writeStartObject(member);
        json.writeMember("name", id);
        json.writeMember("path", QStringLiteral("%1/%2/%2.yy").arg(scope, id));
        json.writeEndObject();
    }
}

static QString sanitizeName(QString name)
{
    static const QRegularExpression regexp(QLatin1String("[^a-zA-Z0-9]"));
    return name.replace(regexp, QStringLiteral("_"));
}

static unsigned convertColor(const QColor &color)
{
    const QRgb rgba = color.rgba();
    return ((qAlpha(rgba) & 0xffu) << 24) |
            ((qBlue(rgba) & 0xffu) << 16) |
            ((qGreen(rgba) & 0xffu) << 8) |
            (qRed(rgba) & 0xffu);
}

static void writeLayers(JsonWriter &json, const std::vector<std::unique_ptr<GMRLayer>> &layers)
{
    json.writeStartArray("layers");

    for (const std::unique_ptr<GMRLayer> &layer : layers) {
        json.prepareNewLine();
        json.writeStartObject();

        switch (layer->resourceType) {
        case GMRAssetLayerType: {
            auto &assetLayer = static_cast<const GMRAssetLayer&>(*layer);

            json.writeStartArray("assets");

            for (const auto &asset : assetLayer.assets) {
                json.prepareNewLine();
                json.writeStartObject();
                const bool wasMinimize = json.minimize();
                json.setMinimize(true);

                writeId(json, "spriteId", asset.spriteId, QStringLiteral("sprites"));

                if (asset.isSprite) {
                    json.writeMember("headPosition", asset.headPosition);
                    json.writeMember("rotation", asset.rotation);
                    json.writeMember("scaleX", asset.scaleX);
                    json.writeMember("scaleY", asset.scaleY);
                    json.writeMember("animationSpeed", asset.animationSpeed);
                } else {
                    json.writeMember("w", asset.w);
                    json.writeMember("h", asset.h);
                    json.writeMember("u0", asset.u0);
                    json.writeMember("v0", asset.v0);
                    json.writeMember("u1", asset.u1);
                    json.writeMember("v1", asset.v1);
                }
                json.writeMember("colour", convertColor(asset.colour));
                if (asset.inheritedItemId.isEmpty()) {
                    json.writeMember("inheritedItemId", QJsonValue(QJsonValue::Null));
                } else {
                    json.writeStartObject("inheritedItemId");
                    json.writeMember("name", asset.inheritedItemId);
                    json.writeMember("path", asset.inheritedItemPath);
                    json.writeEndObject();
                }
                json.writeMember("frozen", asset.frozen);
                json.writeMember("ignore", asset.ignore);
                json.writeMember("inheritItemSettings", asset.inheritItemSettings);
                json.writeMember("x", asset.x);
                json.writeMember("y", asset.y);
                json.writeMember("resourceVersion", asset.resourceVersion);
                json.writeMember("name", asset.name);
                writeTags(json, asset.tags);
                json.writeMember("resourceType", asset.isSprite ? "GMRSpriteGraphic" : "GMRGraphic");

                json.writeEndObject();
                json.setMinimize(wasMinimize);
            }

            json.writeEndArray(); // assets
            break;
        }
        case GMRBackgroundLayerType: {
            auto &backgroundLayer = static_cast<const GMRBackgroundLayer&>(*layer);

            writeId(json, "spriteId", backgroundLayer.spriteId, QStringLiteral("sprites"));

            json.writeMember("colour", convertColor(backgroundLayer.colour));
            json.writeMember("x", backgroundLayer.x);
            json.writeMember("y", backgroundLayer.y);
            json.writeMember("htiled", backgroundLayer.htiled);
            json.writeMember("vtiled", backgroundLayer.vtiled);
            json.writeMember("hspeed", backgroundLayer.hspeed);
            json.writeMember("vspeed", backgroundLayer.vspeed);
            json.writeMember("stretch", backgroundLayer.stretch);
            json.writeMember("animationFPS", backgroundLayer.animationFPS);
            json.writeMember("animationSpeedType", backgroundLayer.animationSpeedType);
            json.writeMember("userdefinedAnimFPS", backgroundLayer.userdefinedAnimFPS);
            break;
        }
        case GMRInstanceLayerType: {
            auto &instanceLayer = static_cast<const GMRInstanceLayer&>(*layer);
            json.writeStartArray("instances");

            for (const auto &instance : instanceLayer.instances) {
                json.prepareNewLine();
                json.writeStartObject();
                const bool wasMinimize = json.minimize();
                json.setMinimize(true);

                json.writeMember("isDnd", instance.isDnd);

                writeId(json, "objectId", instance.objectId, QStringLiteral("objects"));

                json.writeMember("inheritCode", instance.inheritCode);
                json.writeMember("hasCreationCode", instance.hasCreationCode);
                json.writeMember("colour", convertColor(instance.colour));
                json.writeMember("rotation", instance.rotation);
                json.writeMember("scaleX", instance.scaleX);
                json.writeMember("scaleY", instance.scaleY);
                json.writeMember("imageIndex", instance.imageIndex);
                json.writeMember("imageSpeed", instance.imageSpeed);
                if (instance.inheritedItemId.isEmpty()) {
                    json.writeMember("inheritedItemId", QJsonValue(QJsonValue::Null));
                } else {
                    json.writeStartObject("inheritedItemId");
                    json.writeMember("name", instance.inheritedItemId);
                    json.writeMember("path", instance.inheritedItemPath);
                    json.writeEndObject();
                }
                json.writeMember("frozen", instance.frozen);
                json.writeMember("ignore", instance.ignore);
                json.writeMember("inheritItemSettings", instance.inheritItemSettings);
                json.writeMember("x", instance.x);
                json.writeMember("y", instance.y);
                json.writeMember("resourceVersion", instance.resourceVersion);
                json.writeMember("name", instance.name);

                writeTags(json, instance.tags);
                json.writeMember("resourceType", resourceTypeStr(instance.resourceType));

                json.writeEndObject();
                json.setMinimize(wasMinimize);
            }

            json.writeEndArray(); // instances
            break;
        }
        case GMRPathLayerType: {
            auto &pathLayer = static_cast<const GMRPathLayer&>(*layer);

            writeId(json, "pathId", pathLayer.pathId, QStringLiteral("paths"));

            json.writeMember("colour", convertColor(pathLayer.colour));
            break;
        }
        case GMRTileLayerType: {
            auto &tileLayer = static_cast<const GMRTileLayer&>(*layer);

            writeId(json, "tilesetId", tileLayer.tilesetId, QStringLiteral("tilesets"));

            json.writeMember("x", tileLayer.x);
            json.writeMember("y", tileLayer.y);

            json.writeStartObject("tiles");
            json.writeMember("SerialiseWidth", tileLayer.SerialiseWidth);
            json.writeMember("SerialiseHeight", tileLayer.SerialiseHeight);
            json.writeStartArray("TileSerialiseData");

            size_t index = 0;

            for (int y = 0; y < tileLayer.SerialiseHeight; ++y) {
                json.prepareNewLine();

                for (int x = 0; x < tileLayer.SerialiseWidth; ++x) {
                    json.writeValue(tileLayer.tiles.at(index));
                    ++index;
                }
            }

            json.writeEndArray();   // TileSerialiseData
            json.writeEndObject();  // tiles
            break;
        }
        default:
            break;
        }

        json.writeMember("visible", layer->visible);
        json.writeMember("depth", layer->depth);
        json.writeMember("userdefinedDepth", layer->userdefinedDepth);
        json.writeMember("inheritLayerDepth", layer->inheritLayerDepth);
        json.writeMember("inheritLayerSettings", layer->inheritLayerSettings);
        json.writeMember("gridX", layer->gridX);
        json.writeMember("gridY", layer->gridY);

        writeLayers(json, layer->layers);

        json.writeMember("hierarchyFrozen", layer->hierarchyFrozen);
        json.writeMember("resourceVersion", layer->resourceVersion);
        json.writeMember("name", layer->name);

        writeTags(json, layer->tags);
        json.writeMember("resourceType", resourceTypeStr(layer->resourceType));

        json.writeEndObject();
    }

    json.writeEndArray();   // layers
}

static void fillTileLayer(GMRTileLayer &gmrTileLayer, const TileLayer *tileLayer, const Tileset *tileset)
{
    const auto layerOffset = tileLayer->totalOffset().toPoint();

    gmrTileLayer.tilesetId = sanitizeName(tileset->name());
    gmrTileLayer.x = layerOffset.x();
    gmrTileLayer.y = layerOffset.y();
    gmrTileLayer.SerialiseHeight = tileLayer->height();
    gmrTileLayer.SerialiseWidth = tileLayer->width();

    constexpr unsigned Unintialized         = 0x80000000;
    constexpr unsigned FlippedHorizontally  = 0x10000000;
    constexpr unsigned FlippedVertically    = 0x20000000;
    constexpr unsigned Rotated90            = 0x40000000;

    for (int y = 0; y < tileLayer->height(); ++y) {
        for (int x = 0; x < tileLayer->width(); ++x) {
            const Cell &cell = tileLayer->cellAt(x, y);
            if (cell.tileset() != tileset) {
                gmrTileLayer.tiles.push_back(Unintialized);
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

            gmrTileLayer.tiles.push_back(tileId);
        }
    }
}

static void processLayers(std::vector<std::unique_ptr<GMRLayer>> &gmrLayers,
                          const QList<Layer *> &layers,
                          Context &context)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;
        auto layerOffset = layer->totalOffset().toPoint();

        std::unique_ptr<GMRLayer> gmrLayer;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            const auto tileLayer = static_cast<const TileLayer*>(layer);
            const auto tilesets = tileLayer->usedTilesets();

            std::vector<std::unique_ptr<GMRLayer>> gmrLayers;

            for (const auto &tileset : tilesets) {
                if (tileset->isCollection()) {
                    // GMRTileLayer can't support an image collection tileset
                    // TODO: Export these as GMRAssetLayer
                    continue;
                } else {
                    auto gmrTileLayer = std::make_unique<GMRTileLayer>();
                    gmrTileLayer->name = sanitizeName(QStringLiteral("%1_%2").arg(layer->name(), tileset->name()));
                    fillTileLayer(*gmrTileLayer, tileLayer, tileset.data());
                    gmrLayers.push_back(std::move(gmrTileLayer));
                }
            }

            if (gmrLayers.size() == 1) {
                // If one layer was set up, that's the layer we'll use.
                gmrLayer = std::move(gmrLayers.front());
            } else if (gmrLayers.empty()) {
                // When no layers are set up, the tile layer is exported as an
                // empty tile layer.
                gmrLayer = std::make_unique<GMRTileLayer>();
            } else {
                // When multiple layers have been created, they will be exported
                // as children of a group layer.
                gmrLayer = std::make_unique<GMRLayer>();
                gmrLayer->layers = std::move(gmrLayers);
            }
            break;
        }
        case Layer::ObjectGroupType: {
            std::vector<GMRGraphic> assets;
            std::vector<GMRInstance> instances;
            std::vector<GMPath> paths;

            auto objectGroup = static_cast<const ObjectGroup*>(layer);
            const bool frozen = !layer->isUnlocked();
            auto color = layer->effectiveTintColor();
            color.setAlphaF(color.alphaF() * layer->effectiveOpacity());

            for (const MapObject *mapObject : *objectGroup) {
                const QString type = mapObject->effectiveType();

                if (type == QLatin1String("view")) {
                    // GM only has 8 views so drop anything more than that
                    if (context.views.size() > 7) {
                        Tiled::ERROR(QLatin1String("YY plugin: Can't export more than 8 views."),
                                     Tiled::JumpToObject { mapObject });
                        continue;
                    }

                    context.views.emplace_back();
                    GMRView &view = context.views.back();

                    view.inherit = optionalProperty(mapObject, "inherit", false);
                    view.visible = mapObject->isVisible();
                    // Note: GM only supports ints for positioning
                    // so views could be off if user doesn't align to whole number
                    view.xview = qRound(mapObject->x());
                    view.yview = qRound(mapObject->y());
                    view.wview = qRound(mapObject->width());
                    view.hview = qRound(mapObject->height());
                    // Round these incase user adds properties as floats and not ints
                    view.xport = qRound(optionalProperty(mapObject, "xport", 0.0));
                    view.yport = qRound(optionalProperty(mapObject, "yport", 0.0));
                    view.wport = qRound(optionalProperty(mapObject, "wport", 1024.0));
                    view.hport = qRound(optionalProperty(mapObject, "hport", 768.0));
                    view.hborder = qRound(optionalProperty(mapObject, "hborder", 32.0));
                    view.vborder = qRound(optionalProperty(mapObject, "vborder", 32.0));
                    view.hspeed = qRound(optionalProperty(mapObject, "hspeed", -1.0));
                    view.vspeed = qRound(optionalProperty(mapObject, "vspeed", -1.0));
                    view.objectId = QJsonValue(QJsonValue::Null);    // TODO: Provide a way to set this?
                }
                else if (!type.isEmpty())
                {
                    instances.emplace_back();
                    GMRInstance &instance = instances.back();

                    // The type is used to refer to the name of the object
                    instance.isDnd = optionalProperty(mapObject, "isDnd", instance.isDnd);
                    instance.objectId = sanitizeName(type);

                    QPointF origin(optionalProperty(mapObject, "originX", 0.0),
                                   optionalProperty(mapObject, "originY", 0.0));

                    if (!mapObject->cell().isEmpty()) {
                        // For tile objects we can support scaling and flipping, though
                        // flipping in combination with rotation didn't work in GameMaker 1.4 (maybe works in 2?).
                        if (auto tile = mapObject->cell().tile()) {
                            const QSize tileSize = tile->size();
                            instance.scaleX = mapObject->width() / tileSize.width();
                            instance.scaleY = mapObject->height() / tileSize.height();

                            if (mapObject->cell().flippedHorizontally()) {
                                instance.scaleX *= -1;
                                origin += QPointF(mapObject->width() - 2 * origin.x(), 0);
                            }
                            if (mapObject->cell().flippedVertically()) {
                                instance.scaleY *= -1;
                                origin += QPointF(0, mapObject->height() - 2 * origin.y());
                            }
                        }

                        // Tile objects don't necessarily have top-left origin in Tiled,
                        // so the position needs to be translated for top-left origin in
                        // GameMaker, taking into account the rotation.
                        origin -= alignmentOffset(mapObject->bounds(), mapObject->alignment());
                    }

                    // Allow overriding the scale using custom properties
                    instance.scaleX = optionalProperty(mapObject, "scaleX", instance.scaleX);
                    instance.scaleY = optionalProperty(mapObject, "scaleY", instance.scaleY);

                    // Adjust the position based on the origin
                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF pos = mapObject->position() + transform.map(origin);

                    // TODO: Support creation code - optionalProperty(mapObject, "code", QString());
                    instance.colour = color;
                    instance.rotation = -mapObject->rotation();
                    instance.imageIndex = optionalProperty(mapObject, "imageIndex", instance.imageIndex);
                    instance.imageSpeed = optionalProperty(mapObject, "imageSpeed", instance.imageSpeed);
                    // TODO: instance.inheritedItemId
                    instance.frozen = frozen;
                    instance.ignore = optionalProperty(mapObject, "ignore", instance.ignore);
                    instance.inheritItemSettings = optionalProperty(mapObject, "inheritItemSettings", instance.inheritItemSettings);
                    instance.x = qRound(pos.x());
                    instance.y = qRound(pos.y());

                    // Include object ID in the name when necessary because duplicates are not allowed
                    if (mapObject->name().isEmpty()) {
                        instance.name = QStringLiteral("inst_%1").arg(mapObject->id());
                    } else {
                        QString name = sanitizeName(mapObject->name());

                        while (context.usedNames.contains(name))
                            name += QStringLiteral("_%1").arg(mapObject->id());

                        context.usedNames.insert(name);
                        instance.name = name;
                    }

                    instance.tags = readTags(mapObject);
                }
                else if (mapObject->isTileObject())
                {
                    const Cell &cell = mapObject->cell();
                    const Tile *tile = cell.tile();
                    if (!tile)
                        continue;

                    assets.emplace_back();
                    GMRGraphic &g = assets.back();

                    g.isSprite = !tile->imageSource().isEmpty();

                    QPointF origin(optionalProperty(mapObject, "originX", 0.0),
                                   optionalProperty(mapObject, "originY", 0.0));

                    // Tile objects don't necessarily have top-left origin in Tiled,
                    // so the position needs to be translated for top-left origin in
                    // GameMaker, taking into account the rotation.
                    origin -= alignmentOffset(mapObject->bounds(), mapObject->alignment());

                    if (g.isSprite) {
                        g.spriteId = sanitizeName(QFileInfo(tile->imageSource().path()).completeBaseName());
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
                        g.spriteId = sanitizeName(QFileInfo(tileset->imageSource().path()).completeBaseName());
                        g.w = qRound(mapObject->width());
                        g.h = qRound(mapObject->height());

                        const int xInTilesetGrid = tile->id() % tileset->columnCount();
                        const int yInTilesetGrid = static_cast<int>(tile->id() / tileset->columnCount());

                        g.u0 = tileset->margin() + (tileset->tileSpacing() + tileset->tileWidth()) * xInTilesetGrid;
                        g.v0 = tileset->margin() + (tileset->tileSpacing() + tileset->tileHeight()) * yInTilesetGrid;
                        g.u1 = g.u0 + tileset->tileWidth();
                        g.v1 = g.v0 + tileset->tileHeight();

                        if (cell.flippedHorizontally())
                            std::swap(g.u0, g.u1);
                        if (cell.flippedVertically())
                            std::swap(g.v0, g.v1);
                    }

                    // Adjust the position based on the origin
                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF pos = mapObject->position() + transform.map(origin);

                    g.colour = color;
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

            std::vector<std::unique_ptr<GMRLayer>> gmrLayers;

            // TODO: Sub-layers for object layers containing instances, assets and/or paths
            if (!assets.empty()) {
                auto gmrAssetLayer = std::make_unique<GMRAssetLayer>();
                gmrAssetLayer->name = sanitizeName(QStringLiteral("%1_Assets").arg(layer->name()));
                gmrAssetLayer->assets = std::move(assets);
                gmrLayers.push_back(std::move(gmrAssetLayer));
            }

            if (!instances.empty()) {
                auto gmrInstancesLayer = std::make_unique<GMRInstanceLayer>();
                gmrInstancesLayer->name = sanitizeName(QStringLiteral("%1_Instances").arg(layer->name()));
                gmrInstancesLayer->instances = std::move(instances);
                gmrLayers.push_back(std::move(gmrInstancesLayer));
            }

            // TODO: Supporting path layers will require writing out a separate
            // file for each path.
            for (GMPath &path : paths) {
                auto gmrPathLayer = std::make_unique<GMRPathLayer>();
                gmrPathLayer->name = path.name;
                gmrPathLayer->pathId = path.name;
                // TODO: colour of path layer?
                gmrLayers.push_back(std::move(gmrPathLayer));
            }

            if (gmrLayers.size() == 1) {
                // If one layer was set up, that's the layer we'll use.
                gmrLayer = std::move(gmrLayers.front());
            } else if (gmrLayers.empty()) {
                // When no layers are set up, the object layer is exported as
                // an empty instance layer (could also have been an asset
                // layer...).
                gmrLayer = std::make_unique<GMRInstanceLayer>();
            } else {
                // When multiple layers have been created, they will be exported
                // as children of a group layer.
                gmrLayer = std::make_unique<GMRLayer>();
                gmrLayer->layers = std::move(gmrLayers);
            }
            break;
        }
        case Layer::ImageLayerType: {
            auto imageLayer = static_cast<const ImageLayer*>(layer);
            auto gmrBackgroundLayer = std::make_unique<GMRBackgroundLayer>();

            gmrBackgroundLayer->spriteId = sanitizeName(QFileInfo(imageLayer->imageSource().toLocalFile()).completeBaseName());

            auto color = layer->effectiveTintColor();
            color.setAlphaF(color.alphaF() * layer->effectiveOpacity());
            gmrBackgroundLayer->colour = color;

            gmrBackgroundLayer->x = layerOffset.x();
            gmrBackgroundLayer->y = layerOffset.y();

            gmrBackgroundLayer->htiled = optionalProperty(layer, "htiled", false);
            gmrBackgroundLayer->vtiled = optionalProperty(layer, "vtiled", false);
            gmrBackgroundLayer->hspeed = optionalProperty(layer, "hspeed", 0.0);
            gmrBackgroundLayer->vspeed = optionalProperty(layer, "vspeed", 0.0);
            gmrBackgroundLayer->stretch = optionalProperty(layer, "stretch", false);
            gmrBackgroundLayer->animationFPS = optionalProperty(layer, "animationFPS", 15);
            gmrBackgroundLayer->animationSpeedType = optionalProperty(layer, "animationSpeedType", 0);
            gmrBackgroundLayer->userdefinedAnimFPS = layer->resolvedProperty(QStringLiteral("animationFPS")).isValid();

            gmrLayer = std::move(gmrBackgroundLayer);
            break;
        }
        case Layer::GroupLayerType:
            gmrLayer = std::make_unique<GMRLayer>();
            break;
        }

        if (!gmrLayer)
            continue;

        gmrLayer->visible = optionalProperty(layer, "visible", layer->isVisible());
        gmrLayer->depth = optionalProperty(layer, "depth", context.depth);
        gmrLayer->userdefinedDepth = layer->resolvedProperty(QStringLiteral("depth")).isValid();
        gmrLayer->inheritLayerDepth = optionalProperty(layer, "inheritLayerDepth", false);
        gmrLayer->inheritLayerSettings = optionalProperty(layer, "inheritLayerSettings", false);
        gmrLayer->gridX = optionalProperty(layer, "gridX", layer->map()->tileWidth());
        gmrLayer->gridY = optionalProperty(layer, "gridY", layer->map()->tileHeight());
        gmrLayer->hierarchyFrozen = layer->isLocked();
        gmrLayer->name = sanitizeName(layer->name());
        gmrLayer->tags = readTags(layer);

        context.depth = gmrLayer->depth + 100;   // TODO: Better support for overridden depth logic

        if (layer->isGroupLayer()) {
            auto groupLayer = static_cast<const GroupLayer*>(layer);
            processLayers(gmrLayer->layers, groupLayer->layers(), context);
        } else {
            // Copy down certain properties to generated child layers
            for (auto &layer : gmrLayer->layers) {
                layer->depth = gmrLayer->depth;
                layer->userdefinedDepth = gmrLayer->userdefinedDepth;
                layer->inheritLayerDepth = gmrLayer->inheritLayerDepth;
                layer->inheritLayerSettings = gmrLayer->inheritLayerSettings;
                layer->gridX = gmrLayer->gridX;
                layer->gridY = gmrLayer->gridY;
                layer->hierarchyFrozen = gmrLayer->hierarchyFrozen;
                layer->tags = gmrLayer->tags;
            }
        }

        gmrLayers.push_back(std::move(gmrLayer));
    }
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

    Context context;
    std::vector<std::unique_ptr<GMRLayer>> layers;
    processLayers(layers, map->layers(), context);

    const bool enableViews = !context.views.empty();

    // Write out views
    // Last view in Object layer is the first view in the room
    json.writeStartArray("views");
    context.views.resize(8);    // GameMaker always stores 8 views
    for (const GMRView &view : qAsConst(context.views)) {
        json.prepareNewLine();
        json.writeStartObject();
        const bool wasMinimize = json.minimize();
        json.setMinimize(true);

        json.writeMember("inherit", view.inherit);
        json.writeMember("visible", view.visible);
        json.writeMember("xview", view.xview);
        json.writeMember("yview", view.yview);
        json.writeMember("wview", view.wview);
        json.writeMember("hview", view.hview);
        json.writeMember("xport", view.xport);
        json.writeMember("yport", view.yport);
        json.writeMember("wport", view.wport);
        json.writeMember("hport", view.hport);
        json.writeMember("hborder", view.hborder);
        json.writeMember("vborder", view.vborder);
        json.writeMember("hspeed", view.hspeed);
        json.writeMember("vspeed", view.vspeed);
        json.writeMember("objectId", view.objectId);

        json.writeEndObject();
        json.setMinimize(wasMinimize);
    }

    json.writeEndArray();   // views

    writeLayers(json, layers);

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
