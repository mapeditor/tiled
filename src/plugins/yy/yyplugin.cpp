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
#include "maprenderer.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>

#include <vector>

using namespace Tiled;
using namespace Yy;

namespace Yy {

static QString sanitizeName(QString name)
{
    static const QRegularExpression regexp(QLatin1String("[^a-zA-Z0-9]"));
    return name.replace(regexp, QStringLiteral("_"));
}

/**
 * Determine the resource identifier associated with the given filepath.
 * This is achieved by looking for a *.yy meta file and, if found, returning
 * its name without the extension. The function starts searching the current
 * directory and up to two parent directories to account for a typical
 * project directory structure.
 * If no *.yy file can be found, returns the filename without its extension.
 */
static QString determineResourceId(const QString &filePath)
{
    constexpr unsigned int searchDepth = 3;
    QFileInfo fi(filePath);
    QDir dir(fi.path());
    dir.setNameFilters(QStringList("*.yy"));

    for (unsigned int i = 0; i < searchDepth; ++i) {
        if (i > 0 && !dir.cdUp())
            break;

        const QString yyFile = QDirIterator(dir).next();
        if (!yyFile.isEmpty())
            return QFileInfo(yyFile).completeBaseName();
    }

    return sanitizeName(fi.completeBaseName());
}

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
    virtual ~GMResource() = default;

    QString resourceVersion = QStringLiteral("1.0");
    QString name;
    QStringList tags;
    ResourceType resourceType;
};

struct GMRGraphic final : GMResource
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

struct GMOverriddenProperty final : GMResource
{
    GMOverriddenProperty() : GMResource(GMOverriddenPropertyType) {}

    QString propertyId;
    QString objectId;
    QString value;
};

struct GMRInstance final : GMResource
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

struct GMPath final : GMResource
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

struct GMRTileLayer final : GMRLayer
{
    GMRTileLayer() : GMRLayer(GMRTileLayerType) {}

    QString tilesetId;
    int x = 0;
    int y = 0;
    int SerialiseWidth = 0;
    int SerialiseHeight = 0;
    std::vector<unsigned> tiles;
};

struct GMRAssetLayer final : GMRLayer
{
    GMRAssetLayer() : GMRLayer(GMRAssetLayerType) {}

    std::vector<GMRGraphic> assets;
};

struct GMRInstanceLayer final : GMRLayer
{
    GMRInstanceLayer() : GMRLayer(GMRInstanceLayerType) {}

    std::vector<GMRInstance> instances;
};

struct GMRPathLayer final : GMRLayer
{
    GMRPathLayer() : GMRLayer(GMRPathLayerType) {}

    QString pathId;
    QColor colour = Qt::red;
};

struct GMRBackgroundLayer final : GMRLayer
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
    std::vector<GMRView> views;
    std::vector<GMPath> paths;
    std::vector<InstanceCreation> instanceCreationOrder;
    std::unique_ptr<MapRenderer> renderer;
    ExportContext exportContext;

    QString makeUnique(const QString &name)
    {
        int num = 0;
        QString uniqueName = name;
        while (usedNames.contains(uniqueName)) {
            ++num;
            uniqueName = QStringLiteral("%1_%2").arg(name).arg(num);
        }
        usedNames.insert(uniqueName);
        return uniqueName;
    }

    const QString &instanceName(const MapObject *mapObject, const QString &prefix = QString())
    {
        QString &name = instanceNames[mapObject];
        if (name.isEmpty()) {
            // Include object ID in the name when necessary because duplicates are not allowed
            if (mapObject->name().isEmpty())
                name = makeUnique(QStringLiteral("%1_%2").arg(prefix, QString::number(mapObject->id())));
            else
                name = makeUnique(sanitizeName(mapObject->name()));
        }
        return name;
    }

    QString resourceId(const QString &filePath)
    {
        if (filePath.isEmpty())
            return QString();

        QString &resourceId = resourceIds[filePath];
        if (resourceId.isEmpty())
            resourceId = determineResourceId(filePath);

        return resourceId;
    }

private:
    QSet<QString> usedNames;
    QHash<const MapObject*, QString> instanceNames;
    QHash<QString, QString> resourceIds;
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

static QString spriteId(const Object *object, const QUrl &imageUrl, Context &context)
{
    // If the custom property "sprite" exist use it instead of crawling the file system
    const QVariant var = object->resolvedProperty("sprite");
    if (var.isValid())
        return var.value<QString>();
    else
        return context.resourceId(imageUrl.path());
}

static unsigned colorToAbgr(const QColor &color)
{
    const QRgb rgba = color.rgba();
    return ((qAlpha(rgba) & 0xffu) << 24) |
            ((qBlue(rgba) & 0xffu) << 16) |
            ((qGreen(rgba) & 0xffu) << 8) |
            (qRed(rgba) & 0xffu);
}

static QString toOverriddenPropertyValue(const QVariant &value, Context &context)
{
    if (value.userType() == objectRefTypeId()) {
        const auto id = value.value<ObjectRef>().id;
        if (const auto mapObject = context.renderer->map()->findObjectById(id))
            return context.instanceName(mapObject, QStringLiteral("inst"));
        else
            return QString::number(id);
    }

    switch (value.userType()) {
    case QMetaType::Bool:
        return value.toBool() ? QStringLiteral("True") : QStringLiteral("False");

    case QMetaType::QColor: {
        const unsigned abgr = colorToAbgr(value.value<QColor>());
        return QColor(abgr).name(QColor::HexArgb).replace(QLatin1Char('#'), QLatin1Char('$'));
    }

    default: {
        const auto exportValue = context.exportContext.toExportValue(value);
        return exportValue.value.toString();
    }
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

    constexpr unsigned Uninitialized        = 0x80000000;
    constexpr unsigned FlippedHorizontally  = 0x10000000;
    constexpr unsigned FlippedVertically    = 0x20000000;
    constexpr unsigned Rotated90            = 0x40000000;

    for (int y = 0; y < tileLayer->height(); ++y) {
        for (int x = 0; x < tileLayer->width(); ++x) {
            const Cell &cell = tileLayer->cellAt(x, y);
            if (cell.tileset() != tileset) {
                gmrTileLayer.tiles.push_back(Uninitialized);
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

static void initializeTileGraphic(GMRGraphic &g,
                                  QSize size,
                                  const Cell &cell,
                                  const Tile *tile,
                                  Context &context)
{
    const Tileset *tileset = tile->tileset();

    g.spriteId = spriteId(tileset, tileset->imageSource(), context);

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

        GMRGraphic &g = assets.emplace_back(isSprite);

        QSize size = tile->size();
        QPointF origin(optionalProperty(tile, "originX", 0.0),
                       optionalProperty(tile, "originY", 0.0));
        QPointF pos = screenPos + tileset->tileOffset() + layerOffset + origin;

        if (isSprite) {
            g.spriteId = spriteId(tile, tile->imageSource(), context);
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
            initializeTileGraphic(g, size, cell, tile, context);

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
    };

    context.renderer->drawTileLayer(tileRenderFunction, rect);
}

static std::unique_ptr<GMRLayer> processTileLayer(const TileLayer *tileLayer,
                                                  Context &context)
{
    std::unique_ptr<GMRLayer> gmrLayer;

    std::vector<std::unique_ptr<GMRLayer>> gmrLayers;
    std::vector<GMRGraphic> assets;

    // For orthogonal maps we try to use GMRTileLayer, for performance
    // reasons and because it supports tile rotation.
    //
    // GMRTileLayer can't support other orientations, or image
    // collection tilesets or tiles using a different grid size than
    // tile size. Such tiles are exported to a GMRAssetLayer instead.
    //
    if (tileLayer->map()->orientation() == Map::Orthogonal) {
        auto tilesets = tileLayer->usedTilesets().values();
        std::sort(tilesets.begin(), tilesets.end(),
                  [] (const SharedTileset &a, const SharedTileset &b) {
            return a->name() < b->name();
        });

        for (const auto &tileset : std::as_const(tilesets)) {
            if (tileset->isCollection())
                continue;
            if (tileset->tileSize() != tileLayer->map()->tileSize())
                continue;

            auto gmrTileLayer = std::make_unique<GMRTileLayer>();
            gmrTileLayer->name = sanitizeName(QStringLiteral("%1_%2").arg(tileLayer->name(), tileset->name()));
            fillTileLayer(*gmrTileLayer, tileLayer, tileset.data());
            gmrLayers.push_back(std::move(gmrTileLayer));
        }
    }

    createAssetsFromTiles(assets, tileLayer, context);

    if (!assets.empty()) {
        auto gmrAssetLayer = std::make_unique<GMRAssetLayer>();
        gmrAssetLayer->name = sanitizeName(QStringLiteral("%1_Assets").arg(tileLayer->name()));
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

    return gmrLayer;
}

static std::unique_ptr<GMRLayer> processObjectGroup(const ObjectGroup *objectGroup,
                                                    Context &context)
{
    std::unique_ptr<GMRLayer> gmrLayer;

    std::vector<GMRGraphic> assets;
    std::vector<GMRInstance> instances;
    std::vector<GMPath> paths;

    const bool frozen = !objectGroup->isUnlocked();
    auto color = objectGroup->effectiveTintColor();
    color.setAlphaF(color.alphaF() * objectGroup->effectiveOpacity());
    const auto layerOffset = objectGroup->totalOffset().toPoint();

    auto objects = objectGroup->objects();

    // Make sure the objects export in the rendering order
    if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder) {
        std::stable_sort(objects.begin(), objects.end(),
                         [](const MapObject *a, const MapObject *b) { return a->y() < b->y(); });
    }

    for (const MapObject *mapObject : std::as_const(objects)) {
        const QString &className = mapObject->effectiveClassName();

        if (className == QLatin1String("view")) {
            // GM only has 8 views so drop anything more than that
            if (context.views.size() > 7) {
                Tiled::ERROR(QLatin1String("YY plugin: Can't export more than 8 views."),
                             Tiled::JumpToObject { mapObject });
                continue;
            }

            GMRView &view = context.views.emplace_back();

            view.inherit = optionalProperty(mapObject, "inherit", false);
            view.visible = mapObject->isVisible();
            // Note: GM only supports ints for positioning
            // so views could be off if user doesn't align to whole number
            view.xview = qRound(mapObject->x());
            view.yview = qRound(mapObject->y());
            view.wview = qRound(mapObject->width());
            view.hview = qRound(mapObject->height());
            // Round these in case user adds properties as floats and not ints
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
        else if (!className.isEmpty())
        {
            GMRInstance &instance = instances.emplace_back();

            auto props = mapObject->resolvedProperties();

            // The type is used to refer to the name of the object
            instance.isDnd = takeProperty(props, "isDnd", instance.isDnd);
            instance.objectId = sanitizeName(className);

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
                origin -= alignmentOffset(mapObject->size(), mapObject->alignment());
            }

            // Allow overriding the scale using custom properties
            instance.scaleX = takeProperty(props, "scaleX", instance.scaleX);
            instance.scaleY = takeProperty(props, "scaleY", instance.scaleY);

            QPointF pos = context.renderer->pixelToScreenCoords(mapObject->position());

            // Adjust the position based on the origin
            QTransform transform;
            transform.rotate(mapObject->rotation());
            pos += transform.map(origin);

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

            instance.name = context.instanceName(mapObject, QStringLiteral("inst"));
            instance.tags = readTags(mapObject);
            props.remove(QStringLiteral("tags"));

            InstanceCreation &instanceCreation = context.instanceCreationOrder.emplace_back();
            instanceCreation.name = instance.name;
            instanceCreation.creationOrder = takeProperty(props, "creationOrder", 0);

            // Remaining unknown custom properties are assumed to
            // override properties defined on the GameMaker object.
            for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
                GMOverriddenProperty &prop = instance.properties.emplace_back();
                prop.propertyId = it.key();
                prop.objectId = instance.objectId;
                prop.value = toOverriddenPropertyValue(it.value(), context);
            }
        }
        else if (mapObject->isTileObject())
        {
            const Cell &cell = mapObject->cell();
            const Tile *tile = cell.tile();
            if (!tile)
                continue;

            const bool isSprite = !tile->imageSource().isEmpty();

            GMRGraphic &g = assets.emplace_back(isSprite);

            QPointF origin(optionalProperty(mapObject, "originX", 0.0),
                           optionalProperty(mapObject, "originY", 0.0));

            if (isSprite) {
                g.spriteId = spriteId(tile, tile->imageSource(), context);
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
                initializeTileGraphic(g, mapObject->size().toSize(), cell, tile, context);

                if (mapObject->rotation() != 0.0) {
                    Tiled::WARNING(QStringLiteral("YY plugin: Sub-sprite graphics don't support rotation (object %1).").arg(mapObject->id()),
                                   Tiled::JumpToObject { mapObject });
                }
            }

            // Tile objects don't necessarily have top-left origin in Tiled,
            // so the position needs to be translated for top-left origin in
            // GameMaker, taking into account the rotation.
            origin -= alignmentOffset(mapObject->size(), mapObject->alignment());

            QPointF pos = context.renderer->pixelToScreenCoords(mapObject->position());

            // Adjust the position based on the origin
            QTransform transform;
            transform.rotate(mapObject->rotation());
            pos += transform.map(origin);

            g.colour = optionalProperty(mapObject, "colour", color);
            // TODO: g.inheritedItemId
            g.frozen = frozen;
            g.ignore = optionalProperty(mapObject, "ignore", !mapObject->isVisible());
            g.inheritItemSettings = optionalProperty(mapObject, "inheritItemSettings", g.inheritItemSettings);
            g.x = pos.x();
            g.y = pos.y();

            if (isSprite)
                g.name = context.instanceName(mapObject, QStringLiteral("graphic"));
            else
                g.name = context.instanceName(mapObject, QStringLiteral("tile"));

            g.tags = readTags(mapObject);
        }
        else if (mapObject->shape() == MapObject::Polygon || mapObject->shape() == MapObject::Polyline)
        {
            GMPath &p = paths.emplace_back();
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
        gmrAssetLayer->name = sanitizeName(QStringLiteral("%1_Assets").arg(objectGroup->name()));
        gmrAssetLayer->assets = std::move(assets);
        gmrLayers.push_back(std::move(gmrAssetLayer));
    }

    if (!instances.empty()) {
        auto gmrInstancesLayer = std::make_unique<GMRInstanceLayer>();
        gmrInstancesLayer->name = sanitizeName(QStringLiteral("%1_Instances").arg(objectGroup->name()));
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

    return gmrLayer;
}

static std::unique_ptr<GMRLayer> processImageLayer(const ImageLayer *imageLayer, Context &context)
{
    auto gmrBackgroundLayer = std::make_unique<GMRBackgroundLayer>();

    gmrBackgroundLayer->spriteId = spriteId(imageLayer, imageLayer->imageSource(), context);

    auto color = imageLayer->effectiveTintColor();
    color.setAlphaF(color.alphaF() * imageLayer->effectiveOpacity());
    gmrBackgroundLayer->colour = optionalProperty(imageLayer, "colour", color);

    const auto layerOffset = imageLayer->totalOffset().toPoint();
    gmrBackgroundLayer->x = layerOffset.x();
    gmrBackgroundLayer->y = layerOffset.y();

    // Give custom properties priority to ensure backwards compatibility
    gmrBackgroundLayer->htiled = optionalProperty(imageLayer, "htiled", imageLayer->repeatX());
    gmrBackgroundLayer->vtiled = optionalProperty(imageLayer, "vtiled", imageLayer->repeatY());

    gmrBackgroundLayer->hspeed = optionalProperty(imageLayer, "hspeed", gmrBackgroundLayer->hspeed);
    gmrBackgroundLayer->vspeed = optionalProperty(imageLayer, "vspeed", gmrBackgroundLayer->vspeed);
    gmrBackgroundLayer->stretch = optionalProperty(imageLayer, "stretch", gmrBackgroundLayer->stretch);
    gmrBackgroundLayer->animationFPS = optionalProperty(imageLayer, "animationFPS", gmrBackgroundLayer->animationFPS);
    gmrBackgroundLayer->animationSpeedType = optionalProperty(imageLayer, "animationSpeedType", gmrBackgroundLayer->animationSpeedType);
    gmrBackgroundLayer->userdefinedAnimFPS = imageLayer->resolvedProperty(QStringLiteral("animationFPS")).isValid();

    // Workaround compilation issue with mingw49
    return std::unique_ptr<GMRLayer>(std::move(gmrBackgroundLayer));
}

static void processLayers(std::vector<std::unique_ptr<GMRLayer>> &gmrLayers,
                          const QList<Layer *> &layers,
                          Context &context)
{
    for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
        const Layer *layer = *it;

        if (layer->resolvedProperty("noExport").toBool())
            continue;

        std::unique_ptr<GMRLayer> gmrLayer;

        switch (layer->layerType()) {
        case Layer::TileLayerType:
            gmrLayer = processTileLayer(static_cast<const TileLayer*>(layer), context);
            break;
        case Layer::ObjectGroupType:
            gmrLayer = processObjectGroup(static_cast<const ObjectGroup*>(layer), context);
            break;
        case Layer::ImageLayerType: {
            gmrLayer = processImageLayer(static_cast<const ImageLayer*>(layer), context);
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
    // Not using SaveFile here, because GameMaker's reload functionality does
    // not work correctly when the file is replaced.
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    const QString baseName = QFileInfo(fileName).completeBaseName();

    JsonWriter json(&file);

    json.setMinimize(options.testFlag(WriteMinimized));

    json.writeStartObject();

    writeProperty(json, map, "isDnd", false);
    writeProperty(json, map, "volume", 1.0);
    json.writeMember("parentRoom", QJsonValue(QJsonValue::Null));    // TODO: Provide a way to set this?

    Context context;
    context.renderer = MapRenderer::create(map);

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
    for (const GMRView &view : std::as_const(context.views)) {
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

    file.flush();

    if (file.error() != QFileDevice::NoError) {
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
