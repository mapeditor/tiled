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
#include "hexagonalrenderer.h"
#include "imagelayer.h"
#include "isometricrenderer.h"
#include "jsonwriter.h"
#include "logginginterface.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "savefile.h"
#include "staggeredrenderer.h"
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
    GMOverriddenPropertyType,
    GMPathType,
    GMRAssetLayerType,
    GMRBackgroundLayerType,
    GMRGraphicType,
    GMRInstanceLayerType,
    GMRInstanceType,
    GMRLayerType,
    GMRPathLayerType,
    GMRSpriteGraphicType,
    GMRTileLayerType,
};

static const char *resourceTypeStr(ResourceType type)
{
    switch (type) {
    case GMOverriddenPropertyType:  return "GMOverriddenProperty";
    case GMPathType:                return "GMPath";
    case GMRAssetLayerType:         return "GMRAssetLayer";
    case GMRBackgroundLayerType:    return "GMRBackgroundLayer";
    case GMRGraphicType:            return "GMRGraphic";
    case GMRInstanceLayerType:      return "GMRInstanceLayer";
    case GMRInstanceType:           return "GMRInstance";
    case GMRLayerType:              return "GMRLayer";
    case GMRPathLayerType:          return "GMRPathLayer";
    case GMRSpriteGraphicType:      return "GMRSpriteGraphic";
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
    QString objectId;
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
    GMRGraphic(bool isSprite)
        : GMResource(isSprite ? GMRSpriteGraphicType : GMRGraphicType)
    {}

    QString spriteId;

    union {
        // part of a bigger sprite (GMRGraphic)
        struct {
            int w;
            int h;
            int u0;
            int v0;
            int u1;
            int v1;
        };

        // for whole sprites (GMRSpriteGraphic)
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

struct GMOverriddenProperty : GMResource
{
    GMOverriddenProperty() : GMResource(GMOverriddenPropertyType) {}

    QString propertyId;
    QString objectId;
    QString value;
};

struct GMRInstance : GMResource
{
    GMRInstance() : GMResource(GMRInstanceType) {}

    std::vector<GMOverriddenProperty> properties;
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
    double animationFPS = 15.0;
    int animationSpeedType = 0;
    bool userdefinedAnimFPS = false;
};

struct InstanceCreation
{
    QString name;
    int creationOrder = 0;

    bool operator<(const InstanceCreation &other) const
    { return creationOrder < other.creationOrder; }
};

struct Context
{
    QSet<QString> usedNames;
    std::vector<GMRView> views;
    std::vector<GMPath> paths;
    std::vector<InstanceCreation> instanceCreationOrder;
    std::unique_ptr<MapRenderer> renderer;

    QString makeUnique(const QString &name) const
    {
        int num = 0;
        QString uniqueName = name;
        while (usedNames.contains(uniqueName)) {
            ++num;
            uniqueName = QStringLiteral("%1_%2").arg(name).arg(num);
        }
        return uniqueName;
    }
};

} // namespace Yy

template <typename T>
static T optionalProperty(const Object *object, const QString &name, const T &def)
{
    const QVariant var = object->resolvedProperty(name);
    return var.isValid() ? var.value<T>() : def;
}

template <typename T>
static T takeProperty(Properties &properties, const QString &name, const T &def)
{
    const QVariant var = properties.take(name);
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

static void writeResourceProperties(JsonWriter &json, const GMResource &resource)
{
    json.writeMember("resourceVersion", resource.resourceVersion);
    json.writeMember("name", resource.name);
    writeTags(json, resource.tags);
    json.writeMember("resourceType", resourceTypeStr(resource.resourceType));
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

static QString spriteId(const Object *object, const QUrl &imageUrl)
{
    return optionalProperty(object, "sprite", sanitizeName(QFileInfo(imageUrl.fileName()).completeBaseName()));
}

static unsigned colorToAbgr(const QColor &color)
{
    const QRgb rgba = color.rgba();
    return ((qAlpha(rgba) & 0xffu) << 24) |
            ((qBlue(rgba) & 0xffu) << 16) |
            ((qGreen(rgba) & 0xffu) << 8) |
            (qRed(rgba) & 0xffu);
}

static QString toOverriddenPropertyValue(const QVariant &value)
{
    switch (value.type()) {
    case QVariant::Bool:
        return value.toBool() ? QStringLiteral("True") : QStringLiteral("False");

    case QVariant::Color: {
        const unsigned abgr = colorToAbgr(value.value<QColor>());
        return QColor(abgr).name(QColor::HexArgb).replace(QLatin1Char('#'), QLatin1Char('$'));
    }

    default:
        return toExportValue(value).toString();
    }
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

                if (asset.resourceType == GMRSpriteGraphicType) {
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
                json.writeMember("colour", colorToAbgr(asset.colour));
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

                writeResourceProperties(json, asset);

                json.writeEndObject();
                json.setMinimize(wasMinimize);
            }

            json.writeEndArray(); // assets
            break;
        }
        case GMRBackgroundLayerType: {
            auto &backgroundLayer = static_cast<const GMRBackgroundLayer&>(*layer);

            writeId(json, "spriteId", backgroundLayer.spriteId, QStringLiteral("sprites"));

            json.writeMember("colour", colorToAbgr(backgroundLayer.colour));
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

                json.writeStartArray("properties");
                for (const GMOverriddenProperty &prop : instance.properties) {
                    json.writeStartObject();

                    json.writeStartObject("propertyId");
                    json.writeMember("name", prop.propertyId);
                    json.writeMember("path", QStringLiteral("%1/%2/%2.yy").arg(QStringLiteral("objects"), prop.objectId));
                    json.writeEndObject();  // propertyId

                    writeId(json, "objectId", prop.objectId, "objects");

                    json.writeMember("value", prop.value);

                    writeResourceProperties(json, prop);

                    json.writeEndObject();
                }
                json.writeEndArray();   // properties

                json.writeMember("isDnd", instance.isDnd);

                writeId(json, "objectId", instance.objectId, QStringLiteral("objects"));

                json.writeMember("inheritCode", instance.inheritCode);
                json.writeMember("hasCreationCode", instance.hasCreationCode);
                json.writeMember("colour", colorToAbgr(instance.colour));
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

                writeResourceProperties(json, instance);

                json.writeEndObject();
                json.setMinimize(wasMinimize);
            }

            json.writeEndArray(); // instances
            break;
        }
        case GMRPathLayerType: {
            auto &pathLayer = static_cast<const GMRPathLayer&>(*layer);

            writeId(json, "pathId", pathLayer.pathId, QStringLiteral("paths"));

            json.writeMember("colour", colorToAbgr(pathLayer.colour));
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

        writeResourceProperties(json, *layer);

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

            if (tileId == 0) {
                Tiled::WARNING(QStringLiteral("YY plugin: First tile in tileset used, which will appear invisible in GameMaker"),
                               Tiled::JumpToTile { tileLayer->map(), QPoint(x, y), tileLayer });
            }

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

static void initializeTileGraphic(GMRGraphic &g, QSize size, const Cell &cell, const Tile *tile)
{
    const Tileset *tileset = tile->tileset();

    g.spriteId = spriteId(tileset, tileset->imageSource());

    g.w = size.width();
    g.h = size.height();

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

static void createAssetsFromTiles(std::vector<GMRGraphic> &assets,
                                  const TileLayer *tileLayer,
                                  Context &context)
{
    const auto layerOffset = tileLayer->totalOffset().toPoint();
    const QRect rect = context.renderer->boundingRect(tileLayer->bounds());

    const bool frozen = !tileLayer->isUnlocked();
    auto color = tileLayer->effectiveTintColor();
    color.setAlphaF(color.alphaF() * tileLayer->effectiveOpacity());

    auto tileRenderFunction = [&](QPoint tilePos, const QPointF &screenPos) {
        const Cell &cell = tileLayer->cellAt(tilePos);
        const Tileset *tileset = cell.tileset();

        // Skip tiles that should be already covered by a GMRTileLayer
        if (!tileset)
            return;
        if (!tileset->isCollection() &&
                tileset->tileSize() == tileLayer->map()->tileSize() &&
                tileLayer->map()->orientation() == Map::Orthogonal)
            return;

        const auto tile = tileset->findTile(cell.tileId());
        if (!tile || tile->image().isNull())
            return;

        const bool isSprite = !tile->imageSource().isEmpty();

        assets.emplace_back(isSprite);
        GMRGraphic &g = assets.back();

        QSize size = tile->size();
        QPointF origin(optionalProperty(tile, "originX", 0.0),
                       optionalProperty(tile, "originY", 0.0));
        QPointF pos = screenPos + tileset->tileOffset() + layerOffset + origin;

        if (isSprite) {
            g.spriteId = spriteId(tile, tile->imageSource());
            g.headPosition = 0.0;
            g.rotation = 0.0;
            g.scaleX = 1.0;
            g.scaleY = 1.0;
            g.animationSpeed = 1.0;

            if (cell.flippedAntiDiagonally()) {
                g.rotation = -90.0;
                g.scaleY = -1.0;

                pos.ry() -= size.width() - size.height();

                if (cell.flippedVertically()) {
                    g.scaleX = -1.0;
                    pos.ry() += size.width() - 2 * origin.x();
                }
                if (!cell.flippedHorizontally()) {
                    g.scaleY = 1.0;
                    pos.rx() += size.height() - 2 * origin.y();
                }
            } else {
                if (cell.flippedHorizontally()) {
                    g.scaleX = -1.0;
                    pos.rx() += size.width() - 2 * origin.x();
                }
                if (cell.flippedVertically()) {
                    g.scaleY = -1.0;
                    pos.ry() += size.height() - 2 * origin.y();
                }
            }
        } else {
            initializeTileGraphic(g, size, cell, tile);

            if (cell.flippedAntiDiagonally()) {
                Tiled::WARNING(QStringLiteral("YY plugin: Sub-sprite graphics don't support rotated tiles."),
                               Tiled::JumpToTile { tileLayer->map(), tilePos, tileLayer });
            }
        }

        g.colour = color;
        g.frozen = frozen;
        g.ignore = optionalProperty(tileLayer, "ignore", g.ignore);
        g.x = pos.x();
        g.y = pos.y() - size.height();

        if (isSprite)
            g.name = context.makeUnique(QStringLiteral("graphic_%1").arg(tile->id()));
        else
            g.name = context.makeUnique(QStringLiteral("tile_%1").arg(tile->id()));

        context.usedNames.insert(g.name);
    };

    context.renderer->drawTileLayer(tileRenderFunction, rect);
}

static void processLayers(std::vector<std::unique_ptr<GMRLayer>> &gmrLayers,
                          const QList<Layer *> &layers,
                          Context &context)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;

        if (layer->resolvedProperty("noExport").toBool())
            continue;

        const auto layerOffset = layer->totalOffset().toPoint();

        std::unique_ptr<GMRLayer> gmrLayer;

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            const auto tileLayer = static_cast<const TileLayer*>(layer);

            std::vector<std::unique_ptr<GMRLayer>> gmrLayers;
            std::vector<GMRGraphic> assets;

            // For orthogonal maps we try to use GMRTileLayer, for performance
            // reasons and because it supports tile rotation.
            //
            // GMRTileLayer can't support other orientations, or image
            // collection tilesets or tiles using a different grid size than
            // tile size. Such tiles are exported to a GMRAssetLayer instead.
            //
            if (layer->map()->orientation() == Map::Orthogonal) {
                auto tilesets = tileLayer->usedTilesets().values();
                std::sort(tilesets.begin(), tilesets.end(),
                          [] (const SharedTileset &a, const SharedTileset &b) {
                    return a->name() < b->name();
                });

                for (const auto &tileset : qAsConst(tilesets)) {
                    if (tileset->isCollection())
                        continue;
                    if (tileset->tileSize() != layer->map()->tileSize())
                        continue;

                    auto gmrTileLayer = std::make_unique<GMRTileLayer>();
                    gmrTileLayer->name = sanitizeName(QStringLiteral("%1_%2").arg(layer->name(), tileset->name()));
                    fillTileLayer(*gmrTileLayer, tileLayer, tileset.data());
                    gmrLayers.push_back(std::move(gmrTileLayer));
                }
            }

            createAssetsFromTiles(assets, tileLayer, context);

            if (!assets.empty()) {
                auto gmrAssetLayer = std::make_unique<GMRAssetLayer>();
                gmrAssetLayer->name = sanitizeName(QStringLiteral("%1_Assets").arg(layer->name()));
                gmrAssetLayer->assets = std::move(assets);
                gmrLayers.push_back(std::move(gmrAssetLayer));
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

            auto objects = objectGroup->objects();

            // Make sure the objects export in the rendering order
            if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder) {
                std::stable_sort(objects.begin(), objects.end(),
                                 [](const MapObject *a, const MapObject *b) { return a->y() < b->y(); });
            }

            for (const MapObject *mapObject : qAsConst(objects)) {
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
                    view.objectId = optionalProperty(mapObject, "objectId", QString());
                }
                else if (!type.isEmpty())
                {
                    instances.emplace_back();
                    GMRInstance &instance = instances.back();

                    auto props = mapObject->resolvedProperties();

                    // The type is used to refer to the name of the object
                    instance.isDnd = takeProperty(props, "isDnd", instance.isDnd);
                    instance.objectId = sanitizeName(type);

                    QPointF origin(takeProperty(props, "originX", 0.0),
                                   takeProperty(props, "originY", 0.0));

                    if (!mapObject->cell().isEmpty()) {
                        props.remove(QStringLiteral("sprite")); // ignore this, probably inherited from tile

                        // For tile objects we can support scaling and flipping, though
                        // flipping in combination with rotation didn't work in GameMaker 1.4 (maybe works in 2?).
                        if (auto tile = mapObject->cell().tile()) {
                            const QSize tileSize = tile->size();
                            instance.scaleX = mapObject->width() / tileSize.width();
                            instance.scaleY = mapObject->height() / tileSize.height();

                            origin = QPointF(origin.x() * instance.scaleX,
                                             origin.y() * instance.scaleY);

                            if (mapObject->cell().flippedHorizontally()) {
                                instance.scaleX *= -1.0;
                                origin.rx() += mapObject->width() - 2 * origin.x();
                            }
                            if (mapObject->cell().flippedVertically()) {
                                instance.scaleY *= -1.0;
                                origin.ry() += mapObject->height() - 2 * origin.y();
                            }
                        }

                        // Tile objects don't necessarily have top-left origin in Tiled,
                        // so the position needs to be translated for top-left origin in
                        // GameMaker, taking into account the rotation.
                        origin -= alignmentOffset(mapObject->bounds(), mapObject->alignment());
                    }

                    // Allow overriding the scale using custom properties
                    instance.scaleX = takeProperty(props, "scaleX", instance.scaleX);
                    instance.scaleY = takeProperty(props, "scaleY", instance.scaleY);

                    // Adjust the position based on the origin
                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF pos = mapObject->position() + transform.map(origin);

                    // TODO: Support creation code - takeProperty(props, "code", QString());
                    // Would need to be written out as a separate file
                    instance.hasCreationCode = takeProperty(props, "hasCreationCode", instance.hasCreationCode);
                    instance.colour = takeProperty(props, "colour", color);
                    instance.rotation = -mapObject->rotation();
                    instance.imageIndex = takeProperty(props, "imageIndex", instance.imageIndex);
                    instance.imageSpeed = takeProperty(props, "imageSpeed", instance.imageSpeed);
                    // TODO: instance.inheritedItemId
                    instance.frozen = frozen;
                    instance.ignore = takeProperty(props, "ignore", !mapObject->isVisible());
                    instance.inheritItemSettings = takeProperty(props, "inheritItemSettings", instance.inheritItemSettings);
                    instance.x = qRound(pos.x());
                    instance.y = qRound(pos.y());

                    // Include object ID in the name when necessary because duplicates are not allowed
                    if (mapObject->name().isEmpty()) {
                        instance.name = QStringLiteral("inst_%1").arg(mapObject->id());
                    } else {
                        instance.name = context.makeUnique(sanitizeName(mapObject->name()));
                        context.usedNames.insert(instance.name);
                    }

                    instance.tags = readTags(mapObject);
                    props.remove(QStringLiteral("tags"));

                    context.instanceCreationOrder.emplace_back();
                    InstanceCreation &instanceCreation = context.instanceCreationOrder.back();
                    instanceCreation.name = instance.name;
                    instanceCreation.creationOrder = takeProperty(props, "creationOrder", 0);

                    // Remaining unknown custom properties are assumed to
                    // override properties defined on the GameMaker object.
                    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
                        instance.properties.emplace_back();
                        GMOverriddenProperty &prop = instance.properties.back();
                        prop.propertyId = it.key();
                        prop.objectId = instance.objectId;
                        prop.value = toOverriddenPropertyValue(it.value());
                    }
                }
                else if (mapObject->isTileObject())
                {
                    const Cell &cell = mapObject->cell();
                    const Tile *tile = cell.tile();
                    if (!tile)
                        continue;

                    const bool isSprite = !tile->imageSource().isEmpty();

                    assets.emplace_back(isSprite);
                    GMRGraphic &g = assets.back();

                    QPointF origin(optionalProperty(mapObject, "originX", 0.0),
                                   optionalProperty(mapObject, "originY", 0.0));

                    if (isSprite) {
                        g.spriteId = spriteId(tile, tile->imageSource());
                        g.headPosition = optionalProperty(mapObject, "headPosition", 0.0);
                        g.rotation = -mapObject->rotation();

                        const QSize tileSize = tile->size();
                        g.scaleX = mapObject->width() / tileSize.width();
                        g.scaleY = mapObject->height() / tileSize.height();

                        origin = QPointF(origin.x() * g.scaleX,
                                         origin.y() * g.scaleY);

                        if (cell.flippedHorizontally()) {
                            g.scaleX *= -1;
                            origin.rx() += mapObject->width() - 2 * origin.x();
                        }
                        if (cell.flippedVertically()) {
                            g.scaleY *= -1;
                            origin.ry() += mapObject->height() - 2 * origin.y();
                        }

                        // Allow overriding the scale using custom properties
                        g.scaleX = optionalProperty(mapObject, "scaleX", g.scaleX);
                        g.scaleY = optionalProperty(mapObject, "scaleY", g.scaleY);

                        g.animationSpeed = optionalProperty(mapObject, "animationSpeed", 1.0);
                    } else {
                        initializeTileGraphic(g, mapObject->size().toSize(), cell, tile);

                        if (mapObject->rotation() != 0.0) {
                            Tiled::WARNING(QStringLiteral("YY plugin: Sub-sprite graphics don't support rotation (object %1).").arg(mapObject->id()),
                                           Tiled::JumpToObject { mapObject });
                        }
                    }

                    // Tile objects don't necessarily have top-left origin in Tiled,
                    // so the position needs to be translated for top-left origin in
                    // GameMaker, taking into account the rotation.
                    origin -= alignmentOffset(mapObject->bounds(), mapObject->alignment());

                    // Adjust the position based on the origin
                    QTransform transform;
                    transform.rotate(mapObject->rotation());
                    const QPointF pos = mapObject->position() + transform.map(origin);

                    g.colour = optionalProperty(mapObject, "colour", color);
                    // TODO: g.inheritedItemId
                    g.frozen = frozen;
                    g.ignore = optionalProperty(mapObject, "ignore", !mapObject->isVisible());
                    g.inheritItemSettings = optionalProperty(mapObject, "inheritItemSettings", g.inheritItemSettings);
                    g.x = pos.x();
                    g.y = pos.y();

                    // Include object ID in the name when necessary because duplicates are not allowed
                    if (mapObject->name().isEmpty()) {
                        if (isSprite)
                            g.name = QStringLiteral("graphic_%1").arg(mapObject->id());
                        else
                            g.name = QStringLiteral("tile_%1").arg(mapObject->id());
                    } else {
                        g.name = context.makeUnique(sanitizeName(mapObject->name()));
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

                    Tiled::WARNING(QStringLiteral("YY plugin: Exporting of paths is not supported yet"),
                                   Tiled::JumpToObject { mapObject });
                }
                else
                {
                    Tiled::WARNING(QStringLiteral("YY plugin: Ignoring non-tile object %1 without type.").arg(mapObject->id()),
                                   Tiled::JumpToObject { mapObject });

                }
            }

            std::vector<std::unique_ptr<GMRLayer>> gmrLayers;

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

            gmrBackgroundLayer->spriteId = spriteId(imageLayer, imageLayer->imageSource());

            auto color = layer->effectiveTintColor();
            color.setAlphaF(color.alphaF() * layer->effectiveOpacity());
            gmrBackgroundLayer->colour = optionalProperty(layer, "colour", color);

            gmrBackgroundLayer->x = layerOffset.x();
            gmrBackgroundLayer->y = layerOffset.y();

            gmrBackgroundLayer->htiled = optionalProperty(layer, "htiled", gmrBackgroundLayer->htiled);
            gmrBackgroundLayer->vtiled = optionalProperty(layer, "vtiled", gmrBackgroundLayer->vtiled);
            gmrBackgroundLayer->hspeed = optionalProperty(layer, "hspeed", gmrBackgroundLayer->hspeed);
            gmrBackgroundLayer->vspeed = optionalProperty(layer, "vspeed", gmrBackgroundLayer->vspeed);
            gmrBackgroundLayer->stretch = optionalProperty(layer, "stretch", gmrBackgroundLayer->stretch);
            gmrBackgroundLayer->animationFPS = optionalProperty(layer, "animationFPS", gmrBackgroundLayer->animationFPS);
            gmrBackgroundLayer->animationSpeedType = optionalProperty(layer, "animationSpeedType", gmrBackgroundLayer->animationSpeedType);
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
        gmrLayer->depth = optionalProperty(layer, "depth", 0);
        gmrLayer->userdefinedDepth = layer->resolvedProperty(QStringLiteral("depth")).isValid();
        gmrLayer->inheritLayerDepth = optionalProperty(layer, "inheritLayerDepth", false);
        gmrLayer->inheritLayerSettings = optionalProperty(layer, "inheritLayerSettings", false);
        gmrLayer->gridX = optionalProperty(layer, "gridX", layer->map()->tileWidth());
        gmrLayer->gridY = optionalProperty(layer, "gridY", layer->map()->tileHeight());
        gmrLayer->hierarchyFrozen = optionalProperty(layer, "hierarchyFrozen", layer->isLocked());
        gmrLayer->name = sanitizeName(layer->name());
        gmrLayer->tags = readTags(layer);

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

static void collectLayers(const std::vector<std::unique_ptr<GMRLayer>> &layers, std::vector<GMRLayer*> &flattenedLayers)
{
    for (const auto &layer : layers) {
        flattenedLayers.push_back(layer.get());
        collectLayers(layer->layers, flattenedLayers);
    }
}

static void autoAssignDepth(const std::vector<std::unique_ptr<GMRLayer>> &layers)
{
    std::vector<GMRLayer*> flattenedLayers;
    collectLayers(layers, flattenedLayers);

    const auto end = flattenedLayers.cend();
    auto current = flattenedLayers.cbegin();

    auto findNext = [&] (std::vector<GMRLayer*>::const_iterator start) {
        return std::find_if(start, end,
                            [] (GMRLayer *layer) { return layer->userdefinedDepth; });
    };

    auto next = findNext(current);
    int depth = 0;
    int depthIncrement = 100;

    if (next != end)
        depth = (*next)->depth - std::distance(current, next) * depthIncrement;

    for (; current != end; ++current) {
        if (current == next) {
            next = findNext(current + 1);
            depth = (*current)->depth;

            if (next == end) {
                depthIncrement = 100;
            } else {
                if ((*next)->depth < depth)
                    Tiled::WARNING(QStringLiteral("YY plugin: User defined layer depths are not adequately spaced, results in game are undefined."));

                const int diff = (*next)->depth - (*current)->depth;
                const int dist = std::distance(current, next);
                depthIncrement = diff / dist;
            }
        } else {
            (*current)->depth = depth;
        }

        depth += depthIncrement;
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

    const QString baseName = QFileInfo(fileName).completeBaseName();

    JsonWriter json(file.device());

    json.setMinimize(options.testFlag(WriteMinimized));

    json.writeStartObject();

    writeProperty(json, map, "isDnd", false);
    writeProperty(json, map, "volume", 1.0);
    json.writeMember("parentRoom", QJsonValue(QJsonValue::Null));    // TODO: Provide a way to set this?

    Context context;

    switch (map->orientation()) {
    case Map::Isometric:
        context.renderer = std::make_unique<IsometricRenderer>(map);
        break;
    case Map::Staggered:
        context.renderer = std::make_unique<StaggeredRenderer>(map);
        break;
    case Map::Hexagonal:
        context.renderer = std::make_unique<HexagonalRenderer>(map);
        break;
    case Map::Orthogonal:
    case Map::Unknown:
        context.renderer = std::make_unique<OrthogonalRenderer>(map);
        break;
    }

    std::vector<std::unique_ptr<GMRLayer>> layers;
    processLayers(layers, map->layers(), context);

    // If a valid background color is set, create a background layer with this color.
    if (map->backgroundColor().isValid()) {
        auto gmrBackgroundLayer = std::make_unique<GMRBackgroundLayer>();
        gmrBackgroundLayer->name = QStringLiteral("Background_Color");
        gmrBackgroundLayer->colour = map->backgroundColor();
        layers.push_back(std::move(gmrBackgroundLayer));
    }

    autoAssignDepth(layers);

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

        writeId(json, "objectId", view.objectId, "objects");

        json.writeEndObject();
        json.setMinimize(wasMinimize);
    }

    json.writeEndArray();   // views

    writeLayers(json, layers);

    writeProperty(json, map, "inheritLayers", false);
    writeProperty(json, map, "creationCodeFile", QString());
    writeProperty(json, map, "inheritCode", false);

    const QString currentRoomPath = QStringLiteral("rooms/%1/%1.yy").arg(baseName);

    std::stable_sort(context.instanceCreationOrder.begin(),
                     context.instanceCreationOrder.end());

    json.writeStartArray("instanceCreationOrder");
    for (const auto &creation : context.instanceCreationOrder) {
        json.prepareNewLine();
        json.writeStartObject();
        const bool wasMinimize = json.minimize();
        json.setMinimize(true);
        json.writeMember("name", creation.name);
        json.writeMember("path", currentRoomPath);
        json.writeEndObject();
        json.setMinimize(wasMinimize);
    }
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
    const QString parent = optionalProperty(map, "parent", QStringLiteral("Rooms"));
    json.writeMember("name", QFileInfo(parent).fileName());
    json.writeMember("path", QStringLiteral("folders/%1.yy").arg(parent));
    json.writeEndObject();

    writeProperty(json, map, "resourceVersion", QString("1.0"));
    writeProperty(json, map, "name", baseName);
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
