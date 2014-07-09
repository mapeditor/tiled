/*
 * Flare Tiled Plugin
 * Copyright 2010, Jaderamiso <jaderamiso@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2011, Clint Bellanger <clintbellanger@gmail.com>
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

#include "flareplugin.h"

#include "gidmapper.h"
#include "map.h"
#include "mapobject.h"
#include "mapreader.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QStringList>
#include <QTextStream>

using namespace Flare;
using namespace Tiled;

FlarePlugin::FlarePlugin()
{
}

Tiled::Map *FlarePlugin::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open (QIODevice::ReadOnly)) {
        mError = tr("Could not open file for reading.");
        return 0;
    }

    // default to values of the original flare alpha game.
    Map *map = new Map(Map::Isometric, 256, 256, 64, 32);

    QTextStream stream (&file);
    QString line;
    QString sectionName;
    bool newsection = false;
    QString path = QFileInfo(file).absolutePath();
    int base = 10;
    GidMapper gidMapper;
    int gid = 1;
    TileLayer *tilelayer = 0;
    ObjectGroup *objectgroup = 0;
    MapObject *mapobject = 0;
    bool tilesetsSectionFound = false;
    bool headerSectionFound = false;
    bool tilelayerSectionFound = false; // tile layer or objects

    while (!stream.atEnd()) {
        line = stream.readLine();

        if (!line.length())
            continue;

        QChar startsWith = line.at(0);

        if (startsWith == QChar('[')) {
            sectionName = line.mid(1, line.indexOf(QChar(']')) - 1);
            newsection = true;
            continue;
        }
        if (sectionName == QLatin1String("header")) {
            headerSectionFound = true;
            //get map properties
            int epos = line.indexOf(QChar('='));
            if (epos != -1) {
                QString key = line.left(epos).trimmed();
                QString value = line.mid(epos + 1, -1).trimmed();
                if (key == QLatin1String("width"))
                    map->setWidth(value.toInt());
                else if (key == QLatin1String("height"))
                    map->setHeight(value.toInt());
                else if (key == QLatin1String("tilewidth"))
                    map->setTileWidth(value.toInt());
                else if (key == QLatin1String("tileheight"))
                    map->setTileHeight(value.toInt());
                else
                    map->setProperty(key, value);
            }
        } else if (sectionName == QLatin1String("tilesets")) {
            tilesetsSectionFound = true;
            int epos = line.indexOf(QChar('='));
            QString key = line.left(epos).trimmed();
            QString value = line.mid(epos + 1, -1).trimmed();
            if (key == QLatin1String("tileset")) {
                QStringList list = value.split(QChar(','));
                QString absoluteSource = path + QLatin1Char('/') + list[0];
                int tilesetwidth = 0;
                int tilesetheight = 0;
                if (list.size() > 2) {
                    tilesetwidth = list[1].toInt();
                    tilesetheight = list[2].toInt();
                }

                Tileset *tileset = new Tileset(QFileInfo(absoluteSource).fileName(),
                                               tilesetwidth, tilesetheight);
                bool ok = tileset->loadFromImage(absoluteSource);

                if (!ok) {
                    mError = tr("Error loading tileset %1, which expands to %2. Path not found!")
                            .arg(list[0], absoluteSource);
                    delete map;
                    return 0;
                } else {
                    if (list.size() > 4)
                        tileset->setTileOffset(QPoint(list[3].toInt(),list[4].toInt()));

                    gidMapper.insert(gid, tileset);
                    if (list.size() > 5) {
                        gid += list[5].toInt();
                    } else {
                        gid += tileset->tileCount();
                    }
                    map->addTileset(tileset);
                }
            }
        } else if (sectionName == QLatin1String("layer")) {
            if (!tilesetsSectionFound) {
                mError = tr("No tilesets section found before layer section.");
                delete map;
                return 0;
            }
            tilelayerSectionFound = true;
            int epos = line.indexOf(QChar('='));
            if (epos != -1) {
                QString key = line.left(epos).trimmed();
                QString value = line.mid(epos + 1, -1).trimmed();

                if (key == QLatin1String("type")) {
                    tilelayer = new TileLayer(value, 0, 0,
                                              map->width(),map->height());
                    map->addLayer(tilelayer);
                } else if (key == QLatin1String("format")) {
                    if (value == QLatin1String("dec")) {
                        base = 10;
                    } else if (value == QLatin1String("hex")) {
                        base = 16;
                    }
                } else if (key == QLatin1String("data")) {
                    for (int y=0; y < map->height(); y++) {
                        line = stream.readLine();
                        QStringList l = line.split(QChar(','));
                        for (int x=0; x < qMin(map->width(), l.size()); x++) {
                            bool ok;
                            int tileid = l[x].toInt(0, base);
                            Cell c = gidMapper.gidToCell(tileid, ok);
                            if (!ok) {
                                mError += tr("Error mapping tile id %1.").arg(tileid);
                                delete map;
                                return 0;
                            }
                            tilelayer->setCell(x, y, c);
                        }
                    }
                } else {
                    tilelayer->setProperty(key, value);
                }
            }
        } else {
            if (newsection) {
                if (map->indexOfLayer(sectionName) == -1) {
                    objectgroup = new ObjectGroup(sectionName, 0,0,map->width(), map->height());
                    map->addLayer(objectgroup);
                } else {
                    objectgroup = dynamic_cast<ObjectGroup*>(map->layerAt(map->indexOfLayer(sectionName)));
                }
                mapobject = new MapObject();
                objectgroup->addObject(mapobject);
                newsection = false;
            }
            if (!mapobject)
                continue;

            if (startsWith == QChar('#')) {
                QString name = line.mid(1).trimmed();
                mapobject->setName(name);
            }

            int epos = line.indexOf(QChar('='));
            if (epos != -1) {
                QString key = line.left(epos).trimmed();
                QString value = line.mid(epos + 1, -1).trimmed();
                if (key == QLatin1String("type")) {
                    mapobject->setType(value);
                } else if (key == QLatin1String("location")) {
                    QStringList loc = value.split(QChar(','));
                    mapobject->setPosition(QPointF(loc[0].toFloat(), loc[1].toFloat()));
                    if (loc.size() > 3)
                        mapobject->setSize(loc[2].toInt(), loc[3].toInt());
                    else
                        mapobject->setSize(1, 1);
                } else {
                    mapobject->setProperty(key, value);
                }
            }
        }
    }

    if (!headerSectionFound || !tilesetsSectionFound || !tilelayerSectionFound) {
        mError = tr("This seems to be no valid flare map. "
                    "A Flare map consists of at least a header "
                    "section, a tileset section and one tile layer.");
        delete map;
        return 0;
    }

    return map;
}

bool FlarePlugin::supportsFile(const QString &fileName) const
{
    return QFileInfo(fileName).suffix() == QLatin1String("txt");
}

QString FlarePlugin::nameFilter() const
{
    return tr("Flare map files (*.txt)");
}

QString FlarePlugin::errorString() const
{
    return mError;
}

bool FlarePlugin::write(const Tiled::Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    const int mapWidth = map->width();
    const int mapHeight = map->height();

    // write [header]
    out << "[header]\n";
    out << "width=" << mapWidth << "\n";
    out << "height=" << mapHeight << "\n";
    out << "tilewidth=" << map->tileWidth() << "\n";
    out << "tileheight=" << map->tileHeight() << "\n";

    // write all properties for this map
    Properties::const_iterator it = map->properties().constBegin();
    Properties::const_iterator it_end = map->properties().constEnd();
    for (; it != it_end; ++it) {
        out << it.key() << "=" << it.value() << "\n";
    }
    out << "\n";

    QDir mapDir = QFileInfo(fileName).absoluteDir();

    out << "[tilesets]\n";
    foreach (Tileset *ts, map->tilesets()) {
        const QString &imageSource = ts->imageSource();
        QString source = mapDir.relativeFilePath(imageSource);
        out << "tileset=" << source
            << "," << ts->tileWidth()
            << "," << ts->tileHeight()
            << "," << ts->tileOffset().x()
            << "," << ts->tileOffset().y()
            << "\n";
    }
    out << "\n";

    GidMapper gidMapper(map->tilesets());
    // write layers
    foreach (Layer *layer, map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            out << "[layer]\n";
            out << "type=" << layer->name() << "\n";
            out << "data=\n";
            for (int y = 0; y < mapHeight; ++y) {
                for (int x = 0; x < mapWidth; ++x) {
                    Cell t = tileLayer->cellAt(x, y);
                    int id = 0;
                    if (t.tile)
                        id = gidMapper.cellToGid(t);
                    out << id;
                    if (x < mapWidth - 1)
                        out << ",";
                }
                if (y < mapHeight - 1)
                    out << ",";
                out << "\n";
            }
            out << "\n";
        }
        if (ObjectGroup *group = layer->asObjectGroup()) {
            foreach (const MapObject *o, group->objects()) {
                if ((!o->type().isEmpty())) {
                    out << "[" << group->name() << "]\n";

                    // display object name as comment
                    if (!(o->name().isEmpty())) {
                        out << "# " << o->name() << "\n";
                    }

                    out << "type=" << o->type() << "\n";
                    out << "location=" << o->x() << "," << o->y();
                    out << "," << o->width() << "," << o->height() << "\n";

                    // write all properties for this object
                    Properties::const_iterator it = o->properties().constBegin();
                    Properties::const_iterator it_end = o->properties().constEnd();
                    for (; it != it_end; ++it) {
                        out << it.key() << "=" << it.value() << "\n";
                    }
                    out << "\n";
                }
            }
        }
    }
    file.close();
    return true;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Flare, FlarePlugin)
#endif
