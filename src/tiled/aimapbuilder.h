/*
 * aimapbuilder.h
 *
 * Turns Claude's compact JSON answers into Tiled tile layers, and describes
 * the named tiles of a map's tilesets to Claude.
 *
 * This file is part of Tiled.
 * Licensed under the GNU General Public License v2 or later.
 */

#pragma once

#include "tileset.h"

#include <QHash>
#include <QImage>
#include <QJsonObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QVector>

#include <memory>
#include <vector>

namespace Tiled {

class Map;
class Tile;
class TileLayer;

/**
 * A group of tiles that belongs together, for example a 3x3 house. Stored in
 * the tileset custom property "ai_composites" as a JSON array of
 * {"name","cols","rows","ids":[tile ids row-major, -1 = empty]}.
 */
struct AiComposite
{
    QString name;
    int cols = 1;
    int rows = 1;
    QVector<Tile*> tiles;   // row-major, nullptr = empty
};

/**
 * Everything Claude is allowed to use: named tiles and composites, with a
 * one-character symbol per named tile so maps can be sent as compact rows.
 */
class AiTileCatalog
{
public:
    static constexpr const char *CompositesProperty = "ai_composites";

    explicit AiTileCatalog(const QVector<SharedTileset> &tilesets);

    bool isEmpty() const { return mTileForName.isEmpty(); }

    Tile *tileForSymbol(QChar symbol) const { return mTileForSymbol.value(symbol); }
    QChar symbolForTile(const Tile *tile) const;
    const AiComposite *composite(const QString &name) const;

    // Text description for the prompt (legend and composites).
    QString describe() const;

    static QVector<AiComposite> loadComposites(const Tileset &tileset);
    static QString compositesToJson(const QVector<AiComposite> &composites);

    QStringList warnings;

private:
    QHash<QChar, Tile*> mTileForSymbol;
    QHash<const Tile*, QChar> mSymbolForTile;
    QHash<QString, Tile*> mTileForName;
    QVector<AiComposite> mComposites;
    QStringList mLegendLines;
};

struct AiVariant
{
    QString label;
    std::unique_ptr<Map> map;       // region-sized map holding the tile layers
    QImage preview;
};

class AiMapBuilder
{
public:
    explicit AiMapBuilder(const AiTileCatalog &catalog) : mCatalog(catalog) {}

    static QString systemPrompt();

    static QString generationPrompt(const AiTileCatalog &catalog,
                                    const QString &description,
                                    QSize size);

    static QString variantsPrompt(const AiTileCatalog &catalog,
                                  const Map &map,
                                  const QRect &region,
                                  const QString &description,
                                  int count);

    /** Builds a new map (sharing the given tilesets) from a generation reply. */
    std::unique_ptr<Map> buildMap(const QJsonObject &reply,
                                  const Map &templateMap,
                                  QSize size,
                                  QStringList *warnings) const;

    /** Builds region-sized variants from a reply to variantsPrompt(). */
    std::vector<AiVariant> buildVariants(const QJsonObject &reply,
                                     const Map &sourceMap,
                                     QSize regionSize,
                                     QStringList *warnings) const;

    /** Renders all tile layers of a map into a small image. */
    static QImage renderPreview(const Map &map, int maxEdge = 160);

private:
    void fillLayers(Map &map,
                    const QJsonObject &reply,
                    QStringList *warnings) const;

    const AiTileCatalog &mCatalog;
};

} // namespace Tiled
