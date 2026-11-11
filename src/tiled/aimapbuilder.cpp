/*
 * aimapbuilder.cpp
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#include "aimapbuilder.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>

namespace Tiled {

static const QString symbolPool = QStringLiteral(
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#@%&*+=~^!$:;<>-_/\\|");

static QString normalizedName(const QString &name)
{
    return name.trimmed().toLower().replace(QLatin1Char(' '), QLatin1Char('_'));
}

// ---------------------------------------------------------------------------
// AiTileCatalog
// ---------------------------------------------------------------------------

AiTileCatalog::AiTileCatalog(const QVector<SharedTileset> &tilesets)
{
    int nextSymbol = 0;

    for (const SharedTileset &tileset : tilesets) {
        for (Tile *tile : tileset->tiles()) {
            const QString name = normalizedName(tile->className());
            if (name.isEmpty())
                continue;

            if (mTileForName.contains(name)) {
                warnings.append(QStringLiteral("Duplicate tile name '%1' ignored").arg(name));
                continue;
            }
            if (nextSymbol >= symbolPool.size()) {
                warnings.append(QStringLiteral("Too many named tiles, '%1' left out").arg(name));
                continue;
            }

            const QChar symbol = symbolPool.at(nextSymbol++);
            mTileForName.insert(name, tile);
            mTileForSymbol.insert(symbol, tile);
            mSymbolForTile.insert(tile, symbol);
            mLegendLines.append(QStringLiteral("%1 = %2").arg(QString(symbol), name));
        }

        const auto composites = loadComposites(*tileset);
        for (const AiComposite &composite : composites)
            mComposites.append(composite);
    }
}

QChar AiTileCatalog::symbolForTile(const Tile *tile) const
{
    return mSymbolForTile.value(tile, QLatin1Char('?'));
}

const AiComposite *AiTileCatalog::composite(const QString &name) const
{
    const QString normalized = normalizedName(name);
    for (const AiComposite &composite : mComposites) {
        if (composite.name == normalized)
            return &composite;
    }
    return nullptr;
}

QString AiTileCatalog::describe() const
{
    QString text = QStringLiteral("TILE LEGEND (one symbol per tile; '.' means empty):\n");
    text += mLegendLines.join(QLatin1Char('\n'));
    text += QStringLiteral("\n. = empty\n");

    if (!mComposites.isEmpty()) {
        text += QStringLiteral("\nCOMPOSITES (multi-tile objects, placed with \"stamps\"; x,y is the top-left tile):\n");
        for (const AiComposite &composite : mComposites) {
            text += QStringLiteral("%1: %2x%3 tiles\n")
                    .arg(composite.name)
                    .arg(composite.cols)
                    .arg(composite.rows);
        }
    }
    return text;
}

QVector<AiComposite> AiTileCatalog::loadComposites(const Tileset &tileset)
{
    QVector<AiComposite> result;

    const QString json = tileset.property(QLatin1String(CompositesProperty)).toString();
    if (json.isEmpty())
        return result;

    const QJsonArray array = QJsonDocument::fromJson(json.toUtf8()).array();
    for (const QJsonValue &value : array) {
        const QJsonObject object = value.toObject();

        AiComposite composite;
        composite.name = normalizedName(object.value(QLatin1String("name")).toString());
        composite.cols = object.value(QLatin1String("cols")).toInt(1);
        composite.rows = object.value(QLatin1String("rows")).toInt(1);

        const QJsonArray ids = object.value(QLatin1String("ids")).toArray();
        for (const QJsonValue &id : ids)
            composite.tiles.append(id.toInt(-1) >= 0 ? tileset.findTile(id.toInt()) : nullptr);

        if (composite.name.isEmpty() || composite.cols < 1 || composite.rows < 1
                || composite.tiles.size() != composite.cols * composite.rows)
            continue;

        result.append(composite);
    }
    return result;
}

QString AiTileCatalog::compositesToJson(const QVector<AiComposite> &composites)
{
    QJsonArray array;
    for (const AiComposite &composite : composites) {
        QJsonArray ids;
        for (const Tile *tile : composite.tiles)
            ids.append(tile ? tile->id() : -1);

        array.append(QJsonObject {
            { QLatin1String("name"), composite.name },
            { QLatin1String("cols"), composite.cols },
            { QLatin1String("rows"), composite.rows },
            { QLatin1String("ids"), ids }
        });
    }
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

// ---------------------------------------------------------------------------
// Prompts
// ---------------------------------------------------------------------------

QString AiMapBuilder::systemPrompt()
{
    return QStringLiteral(
        "You are a level designer who builds tile maps for the Tiled map editor. "
        "You only use the tiles and composites you are told about. "
        "Reply with ONE JSON object and nothing else.\n\n"
        "JSON format:\n"
        "{\"layers\":[{\"name\":\"ground\",\"rows\":[\"aaabb\",\"aabbb\"]}],"
        "\"stamps\":[{\"layer\":\"buildings\",\"composite\":\"house\",\"x\":3,\"y\":2}]}\n\n"
        "Rules:\n"
        "- Each layer has exactly one string per map row and exactly one character per map column, "
        "using the legend symbols; '.' is an empty cell.\n"
        "- Order layers bottom to top: ground/terrain first, then paths and water edges, "
        "then objects such as trees and buildings, then decoration.\n"
        "- Make the map coherent: terrain transitions use edge/corner tiles when they exist, "
        "paths connect, buildings sit on valid ground and face a path, nothing floats.\n"
        "- Use composites through \"stamps\" so that multi-tile objects stay intact.\n"
        "- Never invent symbols or composite names.");
}

QString AiMapBuilder::generationPrompt(const AiTileCatalog &catalog,
                                       const QString &description,
                                       QSize size)
{
    return QStringLiteral("%1\nMAP SIZE: %2 columns x %3 rows (every layer must have %3 rows of %2 characters).\n\n"
                          "MAP DESCRIPTION: %4")
            .arg(catalog.describe())
            .arg(size.width())
            .arg(size.height())
            .arg(description);
}

QString AiMapBuilder::variantsPrompt(const AiTileCatalog &catalog,
                                     const Map &map,
                                     const QRect &region,
                                     const QString &description,
                                     int count)
{
    constexpr int margin = 3;
    const QRect context = region.adjusted(-margin, -margin, margin, margin)
            .intersected(QRect(0, 0, map.width(), map.height()));

    QString text = catalog.describe();
    text += QStringLiteral("\nThe existing map is %1x%2. I want to redo the REGION x=%3 y=%4 width=%5 height=%6.\n")
            .arg(map.width()).arg(map.height())
            .arg(region.x()).arg(region.y()).arg(region.width()).arg(region.height());
    text += QStringLiteral("Below is the current content of the region plus %1 tiles of surroundings "
                           "(context rows start at x=%2 y=%3). Your new region must blend with the surroundings.\n")
            .arg(margin).arg(context.x()).arg(context.y());

    for (const Layer *layer : map.tileLayers()) {
        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
        text += QStringLiteral("\nLAYER \"%1\":\n").arg(layer->name());
        for (int y = context.top(); y <= context.bottom(); ++y) {
            QString row;
            for (int x = context.left(); x <= context.right(); ++x) {
                const Tile *tile = tileLayer->cellAt(x, y).tile();
                row += tile ? catalog.symbolForTile(tile) : QChar(QLatin1Char('.'));
            }
            text += row + QLatin1Char('\n');
        }
    }

    text += QStringLiteral("\nCreate %1 clearly different alternatives for the REGION only. "
                           "Reply with {\"variants\":[{\"label\":\"short name\",\"layers\":[...],\"stamps\":[...]}, ...]}. "
                           "Inside a variant, layers must be exactly %2 rows of %3 characters (the region only, "
                           "not the surroundings) and stamp coordinates are relative to the region's top-left. "
                           "Use the same layer names as above.\n")
            .arg(count).arg(region.height()).arg(region.width());

    if (!description.trimmed().isEmpty())
        text += QStringLiteral("\nWHAT I WANT IN THE REGION: %1\n").arg(description);

    return text;
}

// ---------------------------------------------------------------------------
// Building maps
// ---------------------------------------------------------------------------

void AiMapBuilder::fillLayers(Map &map, const QJsonObject &reply, QStringList *warnings) const
{
    const int width = map.width();
    const int height = map.height();

    auto layerNamed = [&](const QString &name) -> TileLayer * {
        for (Layer *layer : map.tileLayers()) {
            if (layer->name() == name)
                return static_cast<TileLayer*>(layer);
        }
        auto layer = std::make_unique<TileLayer>(name, 0, 0, width, height);
        TileLayer *raw = layer.get();
        map.addLayer(std::move(layer));
        return raw;
    };

    const QJsonArray layers = reply.value(QLatin1String("layers")).toArray();
    for (const QJsonValue &layerValue : layers) {
        const QJsonObject layerObject = layerValue.toObject();
        QString name = layerObject.value(QLatin1String("name")).toString();
        if (name.isEmpty())
            name = QStringLiteral("Layer %1").arg(map.layerCount() + 1);

        TileLayer *layer = layerNamed(name);
        const QJsonArray rows = layerObject.value(QLatin1String("rows")).toArray();

        if (rows.size() != height && warnings)
            warnings->append(QStringLiteral("Layer '%1' has %2 rows instead of %3").arg(name).arg(rows.size()).arg(height));

        for (int y = 0; y < height && y < rows.size(); ++y) {
            const QString row = rows.at(y).toString();
            if (row.size() != width && warnings)
                warnings->append(QStringLiteral("Layer '%1' row %2 has %3 columns instead of %4")
                                 .arg(name).arg(y).arg(row.size()).arg(width));

            for (int x = 0; x < width && x < row.size(); ++x) {
                const QChar symbol = row.at(x);
                if (symbol == QLatin1Char('.') || symbol == QLatin1Char(' ') || symbol == QLatin1Char('?'))
                    continue;

                if (Tile *tile = mCatalog.tileForSymbol(symbol))
                    layer->setCell(x, y, Cell(tile));
                else if (warnings)
                    warnings->append(QStringLiteral("Unknown symbol '%1' at %2,%3").arg(symbol).arg(x).arg(y));
            }
        }
    }

    const QJsonArray stamps = reply.value(QLatin1String("stamps")).toArray();
    for (const QJsonValue &stampValue : stamps) {
        const QJsonObject stamp = stampValue.toObject();
        const QString compositeName = stamp.value(QLatin1String("composite")).toString();
        const AiComposite *composite = mCatalog.composite(compositeName);
        if (!composite) {
            if (warnings)
                warnings->append(QStringLiteral("Unknown composite '%1'").arg(compositeName));
            continue;
        }

        QString layerName = stamp.value(QLatin1String("layer")).toString();
        if (layerName.isEmpty())
            layerName = QStringLiteral("objects");

        TileLayer *layer = layerNamed(layerName);
        const int originX = stamp.value(QLatin1String("x")).toInt();
        const int originY = stamp.value(QLatin1String("y")).toInt();

        for (int cy = 0; cy < composite->rows; ++cy) {
            for (int cx = 0; cx < composite->cols; ++cx) {
                Tile *tile = composite->tiles.at(cy * composite->cols + cx);
                const int x = originX + cx;
                const int y = originY + cy;
                if (tile && x >= 0 && y >= 0 && x < width && y < height)
                    layer->setCell(x, y, Cell(tile));
            }
        }
    }
}

std::unique_ptr<Map> AiMapBuilder::buildMap(const QJsonObject &reply,
                                            const Map &templateMap,
                                            QSize size,
                                            QStringList *warnings) const
{
    Map::Parameters parameters = templateMap.parameters();
    parameters.width = size.width();
    parameters.height = size.height();
    parameters.infinite = false;

    auto map = std::make_unique<Map>(parameters);
    for (const SharedTileset &tileset : templateMap.tilesets())
        map->addTileset(tileset);

    fillLayers(*map, reply, warnings);
    return map;
}

std::vector<AiVariant> AiMapBuilder::buildVariants(const QJsonObject &reply,
                                               const Map &sourceMap,
                                               QSize regionSize,
                                               QStringList *warnings) const
{
    std::vector<AiVariant> variants;

    const QJsonArray array = reply.value(QLatin1String("variants")).toArray();
    for (const QJsonValue &value : array) {
        const QJsonObject object = value.toObject();

        Map::Parameters parameters = sourceMap.parameters();
        parameters.width = regionSize.width();
        parameters.height = regionSize.height();
        parameters.infinite = false;

        AiVariant variant;
        variant.label = object.value(QLatin1String("label")).toString();
        if (variant.label.isEmpty())
            variant.label = QStringLiteral("Option %1").arg(variants.size() + 1);

        variant.map = std::make_unique<Map>(parameters);
        for (const SharedTileset &tileset : sourceMap.tilesets())
            variant.map->addTileset(tileset);

        fillLayers(*variant.map, object, warnings);
        variant.preview = renderPreview(*variant.map);
        variants.push_back(std::move(variant));
    }
    return variants;
}

QImage AiMapBuilder::renderPreview(const Map &map, int maxEdge)
{
    const int tw = map.tileWidth();
    const int th = map.tileHeight();
    QImage image(qMax(1, map.width() * tw), qMax(1, map.height() * th), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    {
        QPainter painter(&image);
        for (const Layer *layer : map.tileLayers()) {
            const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
            for (int y = 0; y < map.height(); ++y) {
                for (int x = 0; x < map.width(); ++x) {
                    const Tile *tile = tileLayer->cellAt(x, y).tile();
                    if (!tile)
                        continue;
                    const QPixmap &pixmap = tile->image();
                    painter.drawPixmap(x * tw, (y + 1) * th - pixmap.height(), pixmap);
                }
            }
        }
    }

    return image.scaled(maxEdge, maxEdge, Qt::KeepAspectRatio, Qt::FastTransformation);
}

} // namespace Tiled
