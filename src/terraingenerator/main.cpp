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
#include "terrain.h"
#include "tile.h"
#include "tileset.h"

#include <QGuiApplication>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QStringList>

#include "qtcompat_p.h"

#include <algorithm>

using namespace Tiled;

namespace {

struct CommandLineOptions {
    CommandLineOptions()
        : showHelp(false)
        , showVersion(false)
        , overwrite(false)
        , embedImage(false)
        , columns(16)
    {}

    bool showHelp;
    bool showVersion;
    bool overwrite;
    bool embedImage;
    int columns;
    QString target;
    QStringList sources;
    QStringList terrainPriority;
    QList<QStringList> combineList;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qWarning() <<
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
    qWarning().noquote() << "Terrain Generator"
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

static bool hasTerrain(const Tileset &tileset, const QString &name)
{
    for (Terrain *terrain : tileset.terrains())
        if (terrain->name() == name)
            return true;

    return false;
}

static unsigned short terrainId(const QString &name, const Tileset &tileset)
{
    for (Terrain *terrain : tileset.terrains())
        if (terrain->name() == name)
            return terrain->id();

    return 0xFF;
}

static QString nameOf(Terrain *terrain)
{
    return terrain ? terrain->name() : QString();
}

struct TileTerrainNames
{
    TileTerrainNames() {}

    explicit TileTerrainNames(Tile *tile)
        : topLeft(nameOf(tile->terrainAtCorner(0)))
        , topRight(nameOf(tile->terrainAtCorner(1)))
        , bottomLeft(nameOf(tile->terrainAtCorner(2)))
        , bottomRight(nameOf(tile->terrainAtCorner(3)))
    {
    }

    TileTerrainNames(QString topLeft,
                     QString topRight,
                     QString bottomLeft,
                     QString bottomRight)
        : topLeft(topLeft)
        , topRight(topRight)
        , bottomLeft(bottomLeft)
        , bottomRight(bottomRight)
    {}

    unsigned toTerrain(const Tileset &tileset) const
    {
        return makeTerrain(terrainId(topLeft, tileset),
                           terrainId(topRight, tileset),
                           terrainId(bottomLeft, tileset),
                           terrainId(bottomRight, tileset));
    }

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
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    const QRgb * const last = rgb + image.byteCount() / 4;
#else
    const QRgb * const last = rgb + image.sizeInBytes() / 4;
#endif

    for (; rgb != last; ++rgb)
        if (qAlpha(*rgb) > 0)
            return false;

    return true;
}


int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("TerrainGenerator"));
    a.setApplicationVersion(QLatin1String("1.0"));

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
    for (const QString &sourceFileName : qAsConst(options.sources)) {
        SharedTileset source = reader.readTileset(sourceFileName);
        if (!source) {
            qCritical("Error reading source tileset '%s':\n%s",
                      qUtf8Printable(sourceFileName),
                      qUtf8Printable(reader.errorString()));
        }
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

    // Set up a mapping from terrain to tile, for quick lookup
    QMap<TileTerrainNames, Tile*> terrainToTile;
    for (const SharedTileset &tileset : sources)
        for (Tile *tile : tileset->tiles())
            if (tile->terrain() != 0xFFFFFFFF)
                if (!terrainToTile.contains(TileTerrainNames(tile))) // TODO: Optimize
                    terrainToTile.insert(TileTerrainNames(tile), tile);

    // Set up the list of all terrains, mapped by name.
    QMap<QString, Terrain*> terrains;
    for (const SharedTileset &tileset : sources)
        for (Terrain *terrain : tileset->terrains())
            if (!terrains.contains(terrain->name()))
                terrains.insert(terrain->name(), terrain);

    // Check if there is anything to combine.
    if (options.combineList.size() == 0) {
        qWarning() << "No terrain specified to combine (-c option).";
    } else {
        // Dump the combine lists.
        qWarning() << "Terrains to combine:";
        for (const QStringList &combine : qAsConst(options.combineList)) {
            if (combine.isEmpty()) {
                qCritical("Empty combine set");
            }
            qWarning() << combine;

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
    for (const QString &terrainName : qAsConst(options.terrainPriority)) {
        lessThan.terrainPriority.insert(terrainName, priority);
        ++priority;
    }

    const auto terrainNames = terrains.keys();
    qDebug() << "Terrains found:" << terrainNames;

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

    // Add terrains that are not defined in the target tileset yet
    // TODO: This step should be more configurable
    for (Terrain *terrain : terrains) {
        if (!hasTerrain(*targetTileset, terrain->name())) {
            Tile *terrainTile = terrain->imageTile();
            QPixmap terrainImage = terrainTile->image();

            Tile *newTerrainTile = targetTileset->addTile(terrainImage);
            newTerrainTile->setProperties(terrainTile->properties());

            Terrain *newTerrain =  targetTileset->addTerrain(terrain->name(),
                                                             newTerrainTile->id());

            // WARNING: This assumes the terrain tile has this terrain on all
            // its corners.
            newTerrainTile->setTerrain(makeTerrain(newTerrain->id()));
            terrainToTile.insert(TileTerrainNames(newTerrainTile),
                                 newTerrainTile);
        }
    }

    // Prepare a list of terrain combinations.
    QVector<TileTerrainNames> process;
    for (const QStringList &combine : qAsConst(options.combineList)) {
        QList<Terrain*> terrainList;
        // Get the terrains to combine
        for (const QString &terrainName : combine)
            terrainList.append(terrains[terrainName]);

        // Construct a vector with all terrain combinations to process
        for (Terrain *topLeft : terrainList) {
            for (Terrain *topRight : terrainList) {
                for (Terrain *bottomLeft : terrainList) {
                    for (Terrain *bottomRight : terrainList) {
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
    // tileset if it's not in there yet.
    for (const TileTerrainNames &terrainNames : process) {
        Tile *tile = terrainToTile.value(terrainNames);

        if (tile && tile->tileset() == targetTileset)
            continue;

        QPixmap image;
        Properties properties;

        if (!tile) {
            qWarning() << "Generating" << terrainNames;

            // Start a new image
            QImage tileImage = QImage(targetTileset->tileWidth(),
                                      targetTileset->tileHeight(),
                                      QImage::Format_ARGB32);
            tileImage.fill(Qt::transparent);
            QPainter painter(&tileImage);

            QStringList terrainList = terrainNames.terrainList();
            std::sort(terrainList.begin(), terrainList.end(), lessThan);

            // Draw the lowest terrain to avoid pixel gaps
            QString baseTerrain = terrainList.first();
            QPixmap baseImage = terrains[baseTerrain]->imageTile()->image();
            painter.drawPixmap(0, 0, baseImage);

            for (const QString &terrainName : terrainList) {
                TileTerrainNames filtered = terrainNames.filter(terrainName);
                Tile *tile = terrainToTile.value(filtered);
                if (!tile) {
                    qWarning() << "Missing" << filtered;
                    continue;
                }

                painter.drawPixmap(0, 0, tile->image());
                properties.merge(tile->properties());
            }

            image = QPixmap::fromImage(tileImage);
        } else {
            qWarning() << "Copying" << terrainNames << "from"
                       << QFileInfo(tile->tileset()->fileName()).fileName();

            image = tile->image();
            properties = tile->properties();
        }

        Tile *newTile = targetTileset->addTile(image);
        newTile->setTerrain(terrainNames.toTerrain(*targetTileset));
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

        qWarning() << "Writing external tileset image.";
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
        imageFileName += ".png";
        image.save(imageFileName);

        targetTileset->setImageSource(imageFileName);
        targetTileset->setColumnCount(options.columns);
    }

    // Save the target tileset
    MapWriter writer;
    writer.writeTileset(*targetTileset, options.target);

    return 0;
}
