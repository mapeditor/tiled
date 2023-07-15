/*
 * main.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2015, Przemysław Grzywacz <nexather@gmail.com>
 *
 * This file is part of the Terrain Generator tool.
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

#include "mapreader.h"
#include "mapwriter.h"
#include "wangset.h"
#include "tile.h"
#include "tileset.h"

#include <QGuiApplication>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QStringList>

#include <algorithm>

using namespace Tiled;

namespace {

struct CommandLineOptions {
    CommandLineOptions()
    {}

    bool showHelp = false;
    bool showVersion = false;
    bool overwrite = false;
    bool embedImage = false;
    int columns = 16;
    QString target;
    QStringList sources;
    QStringList terrainPriority;
    QList<QStringList> combineList;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qInfo() <<
            "Usage: terraingenerator [options]\n\n"
            "Options:\n"
            "  -h --help        : Display this help.\n"
            "  -v --version     : Display the version.\n"
            "     --overwrite   : Target is overwritten rather than extended.\n"
            "     --columns N   : Amount of columns in the target tileset image (default: 16).\n"
            "  -e --embed-image : Tile images will be embedded in the TSX file instead\n"
            "                     of being saved as a separated PNG file.\n"
            "  -c --combine T1[ T2 [Tn ...]]\n"
            "                   : Specify the terrains to combine together (all combinations).\n"
            "  -s --source TS1[ TS2 [TSn ...]]\n"
            "                   : Add source tilesets, order is not important.\n"
            "  -o --output OUT  : Specify output tileset filename.\n"
            "  -p --priority T1[ T2 [Tn ...]]\n"
            "                   : Add terrain names to priority list (T1 < T2 < Tn).\n"
    ;
}

static void showVersion()
{
    qInfo().noquote() << "Terrain Generator"
                      << QCoreApplication::applicationVersion();
}

static bool parseCommandLineArguments(CommandLineOptions &options)
{
    const QStringList arguments = QCoreApplication::arguments();
    bool inPriority = false;
    bool inSource = false;
    bool inCombine = false;
    int argCount = 0;

    for (int i = 1; i < arguments.size(); ++i) {
        const QString &arg = arguments.at(i);

        // Process source option.
        if (inSource) {
            if (arg.at(0) == QLatin1Char('-')) {
                // Some other option is starting here.
                if (argCount == 0) {
                    qWarning() << "Missing arguments for --source option.";
                }
                inSource = false;
            } else {
                // Append the source file.
                argCount++;
                options.sources.append(arg);
                continue;
            }
        }

        // Process priority option.
        if (inPriority) {
            if (arg.at(0) == QLatin1Char('-')) {
                // Some other option is starting here.
                if (argCount == 0) {
                    qWarning() << "Missing arguments for --priority option.";
                }
                inPriority = false;
            } else {
                // Append the terrain name.
                argCount++;
                options.terrainPriority.append(arg);
                continue;
            }
        }

        // Process combine option.
        if (inCombine) {
            if (arg.at(0) == QLatin1Char('-')) {
                // Some other option is starting here.
                if (argCount == 0) {
                    qWarning() << "Missing arguments for --combine option.";
                }
                inCombine = false;
            } else {
                // Append the terrain name.
                argCount++;
                options.combineList.last().append(arg);
                continue;
            }
        }

        // Process other options.
        if (arg == QLatin1String("--help") || arg == QLatin1String("-h")) {
            options.showHelp = true;
        } else if (arg == QLatin1String("--version")
                || arg == QLatin1String("-v")) {
            options.showVersion = true;
        } else if (arg == QLatin1String("-o")
                || arg == QLatin1String("--output")) {
            i++;
            if (i >= arguments.size()) {
                qWarning() << "Missing argument to" << arg << "option";
                return false;
            }
            const QString& arg2 = arguments.at(i);
            if (arg2.at(0) == QLatin1Char('-')) {
                i--;
                qWarning() << "Missing argument to" << arg << "option";
                return false;
            } else {
                options.target = arg2;
            }
        } else if (arg == QLatin1String("--overwrite")) {
            options.overwrite = true;
        } else if (arg == QLatin1String("-e")
                || arg == QLatin1String("--embed-image")) {
            options.embedImage = true;
        } else if (arg == QLatin1String("-c")
                || arg == QLatin1String("--combine")) {
            // What follows is a list of terrain names to combine together.
            inCombine = true;
            options.combineList.append(QStringList());
            argCount = 0;
        } else if (arg == QLatin1String("-p")
                || arg == QLatin1String("--priority")) {
            // What follows is a list of terrain names.
            inPriority = true;
            argCount = 0;
        } else if (arg == QLatin1String("-s")
                || arg == QLatin1String("--source")) {
            // What follows is a list of input tilesets.
            inSource = true;
            argCount = 0;
        } else if (arg == QLatin1String("--columns")) {
            i++;
            if (i >= arguments.size()) {
                qWarning() << "Missing argument to" << arg << "option";
                return false;
            }
            const QString& arg2 = arguments.at(i);
            bool ok = false;
            options.columns = arg2.toInt(&ok);
            if (!ok || options.columns <= 0) {
                qWarning() << "Invalid or missing argument to" << arg << "option";
                return false;
            }
        } else if (arg.at(0) == QLatin1Char('-')) {
            qWarning() << "Unknown option" << arg;
            options.showHelp = true;
        } else {
            qWarning() << "Unknown argument" << arg;
        }
    }

    return true;
}

static QString nameOf(int wangColor, const WangSet &wangSet)
{
    return wangColor ? wangSet.colorAt(wangColor)->name() : QString();
}

struct TileTerrainNames
{
    explicit TileTerrainNames(const QString &terrain = QString())
        : topLeft(terrain)
        , topRight(terrain)
        , bottomLeft(terrain)
        , bottomRight(terrain)
    {}

    TileTerrainNames(const QString &topLeft,
                     const QString &topRight,
                     const QString &bottomLeft,
                     const QString &bottomRight)
        : topLeft(topLeft)
        , topRight(topRight)
        , bottomLeft(bottomLeft)
        , bottomRight(bottomRight)
    {}

    TileTerrainNames filter(const QString &terrainName) const
    {
        return TileTerrainNames(topLeft == terrainName ? topLeft : QString(),
                                topRight == terrainName ? topRight : QString(),
                                bottomLeft == terrainName ? bottomLeft : QString(),
                                bottomRight == terrainName ? bottomRight : QString());
    }

    QStringList terrainList() const
    {
        QStringList list;
        list << topLeft;
        if (!list.contains(topRight))
            list << topRight;
        if (!list.contains(bottomLeft))
            list << bottomLeft;
        if (!list.contains(bottomRight))
            list << bottomRight;
        return list;
    }

    bool operator < (const TileTerrainNames &other) const
    {
        if (topLeft != other.topLeft)
            return topLeft < other.topLeft;
        if (topRight != other.topRight)
            return topRight < other.topRight;
        if (bottomLeft != other.bottomLeft)
            return bottomLeft < other.bottomLeft;

        return bottomRight < other.bottomRight;
    }

    QString topLeft;
    QString topRight;
    QString bottomLeft;
    QString bottomRight;
};

static TileTerrainNames terrainNames(const Tile *tile, const WangSet &wangSet)
{
    const WangId wangId = wangSet.wangIdOfTile(tile);
    return TileTerrainNames(nameOf(wangId.indexColor(WangId::TopLeft), wangSet),
                            nameOf(wangId.indexColor(WangId::TopRight), wangSet),
                            nameOf(wangId.indexColor(WangId::BottomLeft), wangSet),
                            nameOf(wangId.indexColor(WangId::BottomRight), wangSet));
}

struct TerrainSetBuilder
{
    TerrainSetBuilder(WangSet &wangSet)
        : mWangSet(wangSet)
    {}

    bool hasTerrain(const QString &name) const
    {
        for (auto &color : mWangSet.colors())
            if (color->name() == name)
                return true;

        return false;
    }

    WangColor *addTerrain(const WangColor *terrain)
    {
        auto newTerrain = QSharedPointer<WangColor>::create(0,
                                                            terrain->name(),
                                                            terrain->color(),
                                                            -1,
                                                            terrain->probability());
        newTerrain->setProperties(terrain->properties());
        mWangSet.addWangColor(newTerrain);
        return newTerrain.data();
    }

    void setWangId(const Tile *tile, WangId wangId)
    {
        mWangSet.setWangId(tile->id(), wangId);
    }

    TileTerrainNames terrainNames(const Tile *tile) const
    {
        return ::terrainNames(tile, mWangSet);
    }

    WangId toWangId(const TileTerrainNames &names) const
    {
        WangId wangId;
        wangId.setIndexColor(WangId::TopLeft, terrainId(names.topLeft));
        wangId.setIndexColor(WangId::TopRight, terrainId(names.topRight));
        wangId.setIndexColor(WangId::BottomLeft, terrainId(names.bottomLeft));
        wangId.setIndexColor(WangId::BottomRight, terrainId(names.bottomRight));
        return wangId;
    }

private:
    unsigned terrainId(const QString &name) const
    {
        for (auto &color : mWangSet.colors())
            if (color->name() == name)
                return color->colorIndex();

        return 0;
    }

    WangSet &mWangSet;
};

struct TerrainLessThan
{
    bool operator () (const QString &terrainA, const QString &terrainB) const
    {
        return terrainPriority[terrainA] < terrainPriority[terrainB];
    }

    QMap<QString, int> terrainPriority;
};

static QDebug operator<<(QDebug dbg, const TileTerrainNames &t)
{
    dbg.nospace() << '[' << t.topLeft
                  << ", " << t.topRight
                  << ", " << t.bottomLeft
                  << ", " << t.bottomRight << ']';
    return dbg.space();
}

static bool isEmpty(const QImage &image)
{
    if (image.format() == QImage::Format_RGB32)
        return false;

    Q_ASSERT(image.format() == QImage::Format_ARGB32 ||
             image.format() == QImage::Format_ARGB32_Premultiplied);

    const QRgb *rgb = reinterpret_cast<const QRgb*>(image.constBits());
    const QRgb * const last = rgb + image.sizeInBytes() / 4;

    for (; rgb != last; ++rgb)
        if (qAlpha(*rgb) > 0)
            return false;

    return true;
}


int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    a.setOrganizationDomain(QStringLiteral("mapeditor.org"));
    a.setApplicationName(QStringLiteral("TerrainGenerator"));
    a.setApplicationVersion(QStringLiteral("1.0"));

    CommandLineOptions options;

    if (!parseCommandLineArguments(options)) {
        // Something went wrong, abort.
        return 1;
    }

    if (options.showVersion)
        showVersion();
    if (options.showHelp)
        showHelp();
    if (options.showVersion || options.showHelp)
        return 0;

    if (options.target.isEmpty()) {
        qWarning() << "Error: No target tileset provided";
        showHelp();
        return 1;
    }
    if (options.sources.isEmpty()) {
        qWarning() << "Error: No source tilesets provided";
        showHelp();
        return 1;
    }

    // Check terrain priorities.
    if (options.terrainPriority.isEmpty()) {
        qWarning("Error: No terrain priorities set (option -p).");
        showHelp();
        return 1;
    }

    MapReader reader;
    SharedTileset targetTileset;
    QList<SharedTileset> sources;

    if (!options.overwrite && QFile::exists(options.target)) {
        targetTileset = reader.readTileset(options.target);
        if (!targetTileset) {
            qCritical("Error reading target tileset:\n%s",
                      qUtf8Printable(reader.errorString()));
        }

        // Remove empty tiles from the end of the tileset
        int nextTileId = targetTileset->nextTileId();
        for (int id = nextTileId - 1; id >= 0; --id) {
            if (Tile *tile = targetTileset->findTile(id)) {
                if (isEmpty(tile->image().toImage())) {
                    targetTileset->deleteTile(id);
                    nextTileId = id;
                    continue;
                }
            }

            break;
        }
        targetTileset->setNextTileId(nextTileId);

        // If the target tileset already exists, it is also a source tileset
        sources.append(targetTileset);
    }

    // Read source tilesets.
    for (const QString &sourceFileName : std::as_const(options.sources)) {
        SharedTileset source = reader.readTileset(sourceFileName);
        if (!source) {
            qCritical("Error reading source tileset '%s':\n%s",
                      qUtf8Printable(sourceFileName),
                      qUtf8Printable(reader.errorString()));
            continue;
        }
        source->setFileName(sourceFileName);
        sources.append(source);
    }

    // If the target tileset does not exist yet, create it
    if (!targetTileset) {
        QString name = QFileInfo(options.target).completeBaseName();
        const SharedTileset &firstSource = sources.first();
        int tileWidth = firstSource->tileWidth();
        int tileHeight = firstSource->tileHeight();

        targetTileset = Tileset::create(name, tileWidth, tileHeight);
    }

    // Create a WangSet if the target tileset doesn't have one yet
    if (targetTileset->wangSetCount() == 0) {
        targetTileset->addWangSet(std::make_unique<WangSet>(targetTileset.data(),
                                                            QStringLiteral("Terrains"),
                                                            WangSet::Corner));
    }

    // A mapping from terrain to tile, for quick lookup
    QMap<TileTerrainNames, Tile*> terrainToTile;

    // A list of all terrains, mapped by name.
    QMap<QString, WangColor*> terrains;

    for (const SharedTileset &tileset : sources) {
        for (const auto &wangSet : tileset->wangSets()) {
            for (Tile *tile : tileset->tiles()) {
                if (WangId id = wangSet->wangIdOfTile(tile)) {
                    TileTerrainNames names = ::terrainNames(tile, *wangSet);
                    if (!terrainToTile.contains(names))
                        terrainToTile.insert(names, tile);
                }
            }

            for (const auto &color : wangSet->colors())
                if (!terrains.contains(color->name()))
                    terrains.insert(color->name(), color.data());
        }
    }

    // Check if there is anything to combine.
    if (options.combineList.isEmpty()) {
        qWarning() << "No terrain specified to combine (-c option).";
    } else {
        // Dump the combine lists.
        qInfo() << "Terrains to combine:";
        for (const QStringList &combine : std::as_const(options.combineList)) {
            if (combine.isEmpty()) {
                qCritical("Empty combine set");
            }
            qInfo() << combine;

            // Make sure every terrain from this set was defined.
            for (const QString &terrainName : combine)
                if (!terrains.contains(terrainName))
                    qCritical("Terrain %s is in combine list, however it wasn't defined by any tileset.",
                              qUtf8Printable(terrainName));
        }
    }

    // Setup terrain priorities.
    TerrainLessThan lessThan;
    int priority = 0;
    for (const QString &terrainName : std::as_const(options.terrainPriority)) {
        lessThan.terrainPriority.insert(terrainName, priority);
        ++priority;
    }

    const auto terrainNames = terrains.keys();
    qInfo() << "Terrains found:" << terrainNames;

    // Check if all terrains from priority list were found and loaded.
    const auto terrainsWithPriority = lessThan.terrainPriority.keys();
    for (const QString &terrainName : terrainsWithPriority)
        if (!terrains.contains(terrainName))
            qWarning() << "Terrain" << terrainName << "from priority list not found.";

    // Add terrain names not specified from command line.
    for (const QString &terrainName : terrainNames) {
        if (!lessThan.terrainPriority.contains(terrainName)) {
            qWarning() << "No priority set for" << terrainName;
            lessThan.terrainPriority.insert(terrainName, priority);
            ++priority;
        }
    }

    // Currently we always choose the first terrain set
    // TODO: Make this configurable
    TerrainSetBuilder builder(*targetTileset->wangSet(0));

    // Add terrains that are not defined in the target tileset yet
    // TODO: This step should be more configurable
    QList<WangColor*> newTerrains;
    for (WangColor *terrain : terrains)
        if (!builder.hasTerrain(terrain->name()))
            newTerrains.append(builder.addTerrain(terrain));

    // Copy of and/or assign the tile images to each terrain
    for (WangColor *newTerrain : newTerrains) {
        const WangColor *sourceTerrain = terrains.value(newTerrain->name());
        const WangSet *sourceWangSet = sourceTerrain->wangSet();
        const Tileset *sourceTileset = sourceWangSet->tileset();

        if (const Tile *sourceImageTile = sourceTileset->findTile(sourceTerrain->imageId())) {
            qInfo() << "Copying terrain image for" << newTerrain->name();
            Tile *newImageTile = targetTileset->addTile(sourceImageTile->image());
            newImageTile->setProperties(sourceImageTile->properties());
            newTerrain->setImageId(newImageTile->id());

            const TileTerrainNames names = ::terrainNames(sourceImageTile, *sourceWangSet);
            builder.setWangId(newImageTile, builder.toWangId(names));
            if (!terrainToTile.contains(names))
                terrainToTile.insert(names, newImageTile);
        }
    }

    // Prepare a list of terrain combinations.
    QVector<TileTerrainNames> process;
    for (const QStringList &combine : std::as_const(options.combineList)) {
        QList<WangColor*> terrainList;
        // Get the terrains to combine
        for (const QString &terrainName : combine)
            terrainList.append(terrains[terrainName]);

        // Construct a vector with all terrain combinations to process
        for (WangColor *topLeft : terrainList) {
            for (WangColor *topRight : terrainList) {
                for (WangColor *bottomLeft : terrainList) {
                    for (WangColor *bottomRight : terrainList) {
                        process.append(TileTerrainNames(topLeft->name(),
                                                        topRight->name(),
                                                        bottomLeft->name(),
                                                        bottomRight->name()));
                    }
                }
            }
        }
    }

    // Go through each combination of terrains and add the tile to the target
    // terrain set if it's not in there yet.
    for (const TileTerrainNames &terrainNames : process) {
        Tile *tile = terrainToTile.value(terrainNames);

        if (tile && tile->tileset() == targetTileset)
            continue;

        QPixmap image;
        Properties properties;

        if (!tile) {
            qInfo() << "Generating" << terrainNames;

            // Start a new image
            QImage tileImage = QImage(targetTileset->tileWidth(),
                                      targetTileset->tileHeight(),
                                      QImage::Format_ARGB32);
            tileImage.fill(Qt::transparent);
            QPainter painter(&tileImage);

            QStringList terrainList = terrainNames.terrainList();
            std::sort(terrainList.begin(), terrainList.end(), lessThan);

            // Draw the lowest terrain to avoid pixel gaps
            const TileTerrainNames baseTerrain(terrainList.first());
            Tile *baseTile = terrainToTile.value(baseTerrain);
            if (baseTile)
                painter.drawPixmap(0, 0, baseTile->image());

            for (const QString &terrainName : std::as_const(terrainList)) {
                TileTerrainNames filtered = terrainNames.filter(terrainName);
                Tile *tile = terrainToTile.value(filtered);
                if (!tile) {
                    qWarning() << "Missing" << filtered;
                    continue;
                }

                painter.drawPixmap(0, 0, tile->image());
                mergeProperties(properties, tile->properties());
            }

            image = QPixmap::fromImage(tileImage);
        } else {
            qInfo() << "Copying" << terrainNames << "from"
                    << QFileInfo(tile->tileset()->fileName()).fileName();

            image = tile->image();
            properties = tile->properties();
        }

        Tile *newTile = targetTileset->addTile(image);
        builder.setWangId(newTile, builder.toWangId(terrainNames));
        newTile->setProperties(properties);
        terrainToTile.insert(terrainNames, newTile);
    }

    if (targetTileset->tileCount() == 0)
        qCritical("Target tileset is empty");

    if (options.embedImage) {
        // Make sure there is no source name, this way the image will be saved in the TSX file.
        targetTileset->setImageSource(QString());
    } else {
        // Save the target tileset image as separate file.
        int columns = qMin(options.columns, targetTileset->tileCount());
        int rows = targetTileset->tileCount() / options.columns;
        if (targetTileset->tileCount() % options.columns > 0)
            ++rows;

        qInfo() << "Writing external tileset image.";
        // Save the target tileset image
        QImage image(targetTileset->tileWidth() * columns,
                     targetTileset->tileHeight() * rows,
                     QImage::Format_ARGB32);

        image.fill(Qt::transparent);
        QPainter painter(&image);

        for (Tile *tile : targetTileset->tiles()) {
            int x = (tile->id() % options.columns) * targetTileset->tileWidth();
            int y = (tile->id() / options.columns) * targetTileset->tileHeight();
            painter.drawPixmap(x, y, tile->image());
        }

        QString imageFileName = QFileInfo(options.target).completeBaseName();
        imageFileName += QStringLiteral(".png");
        image.save(imageFileName);

        targetTileset->setImageSource(imageFileName);
        targetTileset->setColumnCount(options.columns);
    }

    // Save the target tileset
    MapWriter writer;
    writer.writeTileset(*targetTileset, options.target);

    return 0;
}
