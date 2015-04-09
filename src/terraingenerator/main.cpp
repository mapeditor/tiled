/*
 * main.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#if QT_VERSION >= 0x050000
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QStringList>

using namespace Tiled;

namespace {

struct CommandLineOptions {
    CommandLineOptions()
        : showHelp(false)
        , showVersion(false)
        , overwrite(false)
    {}

    bool showHelp;
    bool showVersion;
    bool overwrite;
    QString target;
    QStringList sources;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qWarning() <<
            "Usage: terraingenerator [options] [target] [sources...]\n\n"
            "Options:\n"
            "  -h --help      : Display this help\n"
            "  -v --version   : Display the version\n"
            "     --overwrite : Target is overwritten rather than extended";
}

static void showVersion()
{
    qWarning() << "Terrain Generator"
               << qPrintable(QCoreApplication::applicationVersion());
}

static void parseCommandLineArguments(CommandLineOptions &options)
{
    const QStringList arguments = QCoreApplication::arguments();

    for (int i = 1; i < arguments.size(); ++i) {
        const QString &arg = arguments.at(i);
        if (arg == QLatin1String("--help") || arg == QLatin1String("-h")) {
            options.showHelp = true;
        } else if (arg == QLatin1String("--version")
                || arg == QLatin1String("-v")) {
            options.showVersion = true;
        } else if (arg == QLatin1String("--overwrite")) {
            options.overwrite = true;
        } else if (arg.at(0) == QLatin1Char('-')) {
            qWarning() << "Unknown option" << arg;
            options.showHelp = true;
        } else if (options.target.isEmpty()) {
            options.target = arg;
        } else {
            options.sources.append(arg);
        }
    }
}

static bool hasTerrain(Tileset *tileset, const QString &name)
{
    foreach (Terrain *terrain, tileset->terrains())
        if (terrain->name() == name)
            return true;

    return false;
}

static unsigned short terrainId(const QString &name, Tileset *tileset)
{
    foreach (Terrain *terrain, tileset->terrains())
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

    unsigned toTerrain(Tileset *tileset) const
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
    const QRgb * const last = rgb + image.byteCount() / 4;

    for (; rgb != last; ++rgb)
        if (qAlpha(*rgb) > 0)
            return false;

    return true;
}


int main(int argc, char *argv[])
{
#if QT_VERSION >= 0x050000
    QGuiApplication a(argc, argv);
#else
    QApplication a(argc, argv);
#endif

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("TerrainGenerator"));
    a.setApplicationVersion(QLatin1String("1.0"));

    CommandLineOptions options;
    parseCommandLineArguments(options);

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

    MapReader reader;
    Tileset *targetTileset = 0;
    QList<Tileset*> sources;

    if (!options.overwrite && QFile::exists(options.target)) {
        targetTileset = reader.readTileset(options.target);
        if (!targetTileset) {
            qFatal("Error reading target tileset:\n%s",
                   qPrintable(reader.errorString()));
        }

        // Remove empty tiles from the end of the tileset
        for (int i = targetTileset->tileCount() - 1; i >= 0; --i) {
            Tile *tile = targetTileset->tileAt(i);
            if (!isEmpty(tile->image().toImage()))
                break;

            targetTileset->removeLastTile();
        }

        // If the target tileset already exists, it is also a source tileset
        sources.append(targetTileset);
    }

    foreach (const QString &sourceFileName, options.sources) {
        Tileset *source = reader.readTileset(sourceFileName);
        if (!source) {
            qFatal("Error reading source tileset '%s':\n%s",
                   qPrintable(sourceFileName),
                   qPrintable(reader.errorString()));
        }
        sources.append(source);
    }

    // If the target tileset does not exist yet, create it
    if (!targetTileset) {
        QString name = QFileInfo(options.target).completeBaseName();
        Tileset *firstSource = sources.first();
        int tileWidth = firstSource->tileWidth();
        int tileHeight = firstSource->tileHeight();

        targetTileset = new Tileset(name, tileWidth, tileHeight, 0, 0);
    }

    // Set up a mapping from terrain to tile, for quick lookup
    QMap<TileTerrainNames, Tile*> terrainToTile;
    foreach (Tileset *tileset, sources)
        foreach (Tile *tile, tileset->tiles())
            if (tile->terrain() != 0xFFFFFFFF)
                if (!terrainToTile.contains(TileTerrainNames(tile))) // TODO: Optimize
                    terrainToTile.insert(TileTerrainNames(tile), tile);

    // Set up the list of all terrains, mapped by name.
    QMap<QString, Terrain*> terrains;
    foreach (Tileset *tileset, sources)
        foreach (Terrain *terrain, tileset->terrains())
            if (!terrains.contains(terrain->name()))
                terrains.insert(terrain->name(), terrain);

    // Assign terrain priority based on the order in which we encounter
    // them (this will determine drawing order).
    QStringList terrainOrder;
    terrainOrder << "LavaRock";
    terrainOrder << "Dirt2";
    terrainOrder << "Dirt";
    terrainOrder << "Sand";
    terrainOrder << "Water";
    terrainOrder << "Hole";
    terrainOrder << "Grass";
    terrainOrder << "PlowedSoil";
    terrainOrder << "Lava";
    TerrainLessThan lessThan;
    int priority = 0;
    foreach (const QString &terrainName, terrainOrder) {
        lessThan.terrainPriority.insert(terrainName, priority);
        ++priority;
    }

    qDebug() << "Terrains found:" << terrains.keys();

    // Add terrains that are not defined in the target tileset yet
    // TODO: This step should be more configurable
    foreach (Terrain *terrain, terrains) {
        if (!hasTerrain(targetTileset, terrain->name())) {
            Tile *terrainTile = terrain->imageTile();
            QPixmap terrainImage = terrainTile->image();

            Tile *newTerrainTile = targetTileset->addTile(terrainImage);

            Terrain *newTerrain =  targetTileset->addTerrain(terrain->name(),
                                                             newTerrainTile->id());

            // WARNING: This assumes the terrain tile has this terrain on all
            // its corners.
            newTerrainTile->setTerrain(makeTerrain(newTerrain->id()));
            terrainToTile.insert(TileTerrainNames(newTerrainTile),
                                 newTerrainTile);
        }
    }

    QList<Terrain*> terrainList;// = targetTileset->terrains();
    terrainList.append(terrains["Dirt"]);
    terrainList.append(terrains["Darkdirt"]);
    terrainList.append(terrains["Water"]);
    terrainList.append(terrains["Grass"]);

    // Construct a vector with all terrain combinations to process
    // TODO: This step should be more configurable
    QVector<TileTerrainNames> process;
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }

    terrainList.clear();
    terrainList.append(terrains["Darkdirt"]);
    terrainList.append(terrains["Grass"]);
    terrainList.append(terrains["Lavarock"]);
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }

    terrainList.clear();
    terrainList.append(terrains["Lava"]);
    terrainList.append(terrains["Lavarock"]);
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }



    terrainList.clear();
    terrainList.append(terrains["PlowedSoil"]);
    terrainList.append(terrains["Grass"]);
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }

    terrainList.clear();
    terrainList.append(terrains["Dirt"]);
    terrainList.append(terrains["Hole"]);
    terrainList.append(terrains["Grass"]);
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }

    terrainList.clear();
    terrainList.append(terrains["Sand"]);
    terrainList.append(terrains["Water"]);
    foreach (Terrain *topLeft, terrainList) {
        foreach (Terrain *topRight, terrainList) {
            foreach (Terrain *bottomLeft, terrainList) {
                foreach (Terrain *bottomRight, terrainList) {
                    process.append(TileTerrainNames(topLeft->name(),
                                                    topRight->name(),
                                                    bottomLeft->name(),
                                                    bottomRight->name()));
                }
            }
        }
    }

    // Go through each combination of terrains and add the tile to the target
    // tileset if it's not in there yet.
    foreach (TileTerrainNames terrainNames, process) {
        Tile *tile = terrainToTile.value(terrainNames);

        if (tile && tile->tileset() == targetTileset)
            continue;

        QPixmap image;

        if (!tile) {
            qWarning() << "Generating" << terrainNames;

            // Start a new image
            QImage tileImage = QImage(targetTileset->tileWidth(),
                                      targetTileset->tileHeight(),
                                      QImage::Format_ARGB32);
            tileImage.fill(Qt::transparent);
            QPainter painter(&tileImage);

            QStringList terrainList = terrainNames.terrainList();
            qSort(terrainList.begin(), terrainList.end(), lessThan);

            // Draw the lowest terrain to avoid pixel gaps
            QString baseTerrain = terrainList.first();
            QPixmap baseImage = terrains[baseTerrain]->imageTile()->image();
            painter.drawPixmap(0, 0, baseImage);

            foreach (const QString &terrainName, terrainList) {
                TileTerrainNames filtered = terrainNames.filter(terrainName);
                Tile *tile = terrainToTile.value(filtered);
                if (!tile) {
                    qWarning() << "Missing" << filtered;
                    continue;
                }

                painter.drawPixmap(0, 0, tile->image());
            }

            image = QPixmap::fromImage(tileImage);
        } else {
            qWarning() << "Copying" << terrainNames << "from"
                       << QFileInfo(tile->tileset()->fileName()).fileName();

            image = tile->image();
        }

        Tile *newTile = targetTileset->addTile(image);
        newTile->setTerrain(terrainNames.toTerrain(targetTileset));
        terrainToTile.insert(terrainNames, newTile);
    }

    if (targetTileset->tileCount() == 0)
        qFatal("Target tileset is empty");

    int columns = qMin(16, targetTileset->tileCount());
    int rows = targetTileset->tileCount() / 16;
    if (targetTileset->tileCount() % 16 > 0)
        ++rows;

    // Save the target tileset image
    QImage image(targetTileset->tileWidth() * columns,
                 targetTileset->tileHeight() * rows,
                 QImage::Format_ARGB32);

    image.fill(Qt::transparent);
    QPainter painter(&image);

    foreach (Tile *tile, targetTileset->tiles()) {
        int x = (tile->id() % 16) * targetTileset->tileWidth();
        int y = (tile->id() / 16) * targetTileset->tileHeight();
        painter.drawPixmap(x, y, tile->image());
    }

    QString imageFileName = QFileInfo(options.target).completeBaseName();
    imageFileName += ".png";
    image.save(imageFileName);

    targetTileset->setImageSource(imageFileName);

    // Save the target tileset
    MapWriter writer;
    targetTileset->setFileName(QString());
    writer.writeTileset(targetTileset, options.target);

    qDeleteAll(sources);
    return 0;
}
