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
#include "savefile.h"
#include "tile.h"
#include "tiled.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QStringView>
#endif
#include <QTextStream>

#include <memory>

using namespace Tiled;

namespace Flare {

FlarePlugin::FlarePlugin()
{
}

std::unique_ptr<Tiled::Map> FlarePlugin::read(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open (QIODevice::ReadOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    // default to values of the original Flare alpha game.
    Map::Parameters mapParameters;
    mapParameters.orientation = Map::Isometric;
    mapParameters.width = 256;
    mapParameters.height = 256;
    mapParameters.tileWidth = 64;
    mapParameters.tileHeight = 32;

    auto map = std::make_unique<Map>(mapParameters);

    QTextStream stream (&file);
    QString line;
    QString sectionName;
    bool newsection = false;
    QString path = QFileInfo(file).absolutePath();
    int base = 10;
    GidMapper gidMapper;
    int gid = 1;
    TileLayer *tilelayer = nullptr;
    ObjectGroup *objectgroup = nullptr;
    MapObject *mapobject = nullptr;
    bool tilesetsSectionFound = false;
    bool headerSectionFound = false;
    bool tilelayerSectionFound = false; // tile layer or objects
    QColor backgroundColor;

    while (!stream.atEnd()) {
        line = stream.readLine();
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        const QStringView lineView(line);
#else
        const QStringRef lineView(&line);
#endif

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
                const auto key = lineView.left(epos).trimmed();
                const auto value = lineView.mid(epos + 1, -1).trimmed();
                if (key == QLatin1String("width"))
                    map->setWidth(value.toInt());
                else if (key == QLatin1String("height"))
                    map->setHeight(value.toInt());
                else if (key == QLatin1String("tilewidth"))
                    map->setTileWidth(value.toInt());
                else if (key == QLatin1String("tileheight"))
                    map->setTileHeight(value.toInt());
                else if (key == QLatin1String("orientation"))
                    map->setOrientation(orientationFromString(value.toString()));
                else if (key == QLatin1String("background_color")){
                    auto rgbaList = value.split(',');

                    if (!rgbaList.isEmpty())
                        backgroundColor.setRed(rgbaList.takeFirst().toInt());
                    if (!rgbaList.isEmpty())
                        backgroundColor.setGreen(rgbaList.takeFirst().toInt());
                    if (!rgbaList.isEmpty())
                        backgroundColor.setBlue(rgbaList.takeFirst().toInt());
                    if (!rgbaList.isEmpty())
                        backgroundColor.setAlpha(rgbaList.takeFirst().toInt());

                    map->setBackgroundColor(backgroundColor);
                }
                else
                    map->setProperty(key.toString(), value.toString());
            }
        } else if (sectionName == QLatin1String("tilesets")) {
            tilesetsSectionFound = true;
            int epos = line.indexOf(QChar('='));
            const auto key = lineView.left(epos).trimmed();
            const auto value = lineView.mid(epos + 1, -1).trimmed();
            if (key == QLatin1String("tileset")) {
                const auto list = value.split(QChar(','));

                QString absoluteSource(list.first().toString());
                if (QDir::isRelativePath(absoluteSource))
                    absoluteSource = path + QLatin1Char('/') + absoluteSource;

                int tilesetwidth = 0;
                int tilesetheight = 0;
                if (list.size() > 2) {
                    tilesetwidth = list[1].toInt();
                    tilesetheight = list[2].toInt();
                }

                SharedTileset tileset(Tileset::create(QFileInfo(absoluteSource).fileName(),
                                                      tilesetwidth, tilesetheight));
                bool ok = tileset->loadFromImage(absoluteSource);

                if (!ok) {
                    mError = tr("Error loading tileset %1, which expands to %2. Path not found!")
                            .arg(list.first().toString(), absoluteSource);
                    return nullptr;
                } else {
                    if (list.size() > 4)
                        tileset->setTileOffset(QPoint(list[3].toInt(), list[4].toInt()));

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
                return nullptr;
            }
            tilelayerSectionFound = true;
            int epos = line.indexOf(QChar('='));
            if (epos != -1) {
                const auto key = lineView.left(epos).trimmed();
                const auto value = lineView.mid(epos + 1, -1).trimmed();

                if (key == QLatin1String("type")) {
                    tilelayer = new TileLayer(value.toString(), 0, 0,
                                              map->width(),
                                              map->height());
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
                            int tileid = l[x].toInt(nullptr, base);
                            Cell c = gidMapper.gidToCell(tileid, ok);
                            if (!ok) {
                                mError += tr("Error mapping tile id %1.").arg(tileid);
                                return nullptr;
                            }
                            tilelayer->setCell(x, y, c);
                        }
                    }
                } else {
                    tilelayer->setProperty(key.toString(), value.toString());
                }
            }
        } else {
            if (newsection) {
                if (map->indexOfLayer(sectionName) == -1) {
                    objectgroup = new ObjectGroup(sectionName, 0, 0);
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
                QString name = lineView.mid(1).trimmed().toString();
                mapobject->setName(name);
            }

            int epos = line.indexOf(QChar('='));
            if (epos != -1) {
                const auto key = lineView.left(epos).trimmed();
                const auto value = lineView.mid(epos + 1, -1).trimmed();
                if (key == QLatin1String("type")) {
                    mapobject->setType(value.toString());
                } else if (key == QLatin1String("location")) {
                    const auto loc = value.split(QChar(','));
                    qreal x,y;
                    qreal w,h;
                    if (map->orientation() == Map::Orthogonal) {
                        x = loc[0].toDouble() * map->tileWidth();
                        y = loc[1].toDouble() * map->tileHeight();
                        if (loc.size() > 3) {
                            w = loc[2].toInt() * map->tileWidth();
                            h = loc[3].toInt() * map->tileHeight();
                        } else {
                            w = map->tileWidth();
                            h = map->tileHeight();
                        }
                    } else {
                        x = loc[0].toDouble() * map->tileHeight();
                        y = loc[1].toDouble() * map->tileHeight();
                        if (loc.size() > 3) {
                            w = loc[2].toInt() * map->tileHeight();
                            h = loc[3].toInt() * map->tileHeight();
                        } else {
                            w = h = map->tileHeight();
                        }
                    }
                    mapobject->setPosition(QPointF(x, y));
                    mapobject->setSize(w, h);
                } else {
                    mapobject->setProperty(key.toString(), value.toString());
                }
            }
        }
    }

    if (!headerSectionFound || !tilesetsSectionFound || !tilelayerSectionFound) {
        mError = tr("This seems to be no valid flare map. "
                    "A Flare map consists of at least a header "
                    "section, a tileset section and one tile layer.");
        return nullptr;
    }

    return map;
}

bool FlarePlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".txt"), Qt::CaseInsensitive);
}

QString FlarePlugin::nameFilter() const
{
    return tr("Flare map files (*.txt)");
}

QString FlarePlugin::shortName() const
{
    return QStringLiteral("flare");
}

QString FlarePlugin::errorString() const
{
    return mError;
}

bool FlarePlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    SaveFile file(fileName);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    QTextStream out(file.device());
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    out.setCodec("UTF-8");
#endif

    const int mapWidth = map->width();
    const int mapHeight = map->height();
    const QColor backgroundColor = map->backgroundColor();

    // write [header]
    out << "[header]\n";
    out << "width=" << mapWidth << "\n";
    out << "height=" << mapHeight << "\n";
    out << "tilewidth=" << map->tileWidth() << "\n";
    out << "tileheight=" << map->tileHeight() << "\n";
    out << "orientation=" << orientationToString(map->orientation()) << "\n";
    out << "background_color=" << backgroundColor.red() << "," << backgroundColor.green() << "," << backgroundColor.blue() << "," << backgroundColor.alpha() << "\n";

    const QString mapPath = QFileInfo(fileName).absolutePath();

    // write all properties for this map
    writeProperties(out, map->properties(), mapPath);
    out << "\n";

    out << "[tilesets]\n";
    for (const SharedTileset &tileset : map->tilesets()) {
        QString source = toFileReference(tileset->imageSource(), mapPath);
        out << "tileset=" << source
            << "," << tileset->tileWidth()
            << "," << tileset->tileHeight()
            << "," << tileset->tileOffset().x()
            << "," << tileset->tileOffset().y()
            << "\n";
    }
    out << "\n";

    GidMapper gidMapper(map->tilesets());
    // write layers
    for (Layer *layer : map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            out << "[layer]\n";
            out << "type=" << layer->name() << "\n";
            out << "data=\n";
            for (int y = 0; y < mapHeight; ++y) {
                for (int x = 0; x < mapWidth; ++x) {
                    Cell t = tileLayer->cellAt(x, y);
                    int id = gidMapper.cellToGid(t);
                    out << id;
                    if (x < mapWidth - 1)
                        out << ",";
                }
                if (y < mapHeight - 1)
                    out << ",";
                out << "\n";
            }
            //Write all properties for this layer
            writeProperties(out, tileLayer->properties(), mapPath);
            out << "\n";
        }
        if (ObjectGroup *group = layer->asObjectGroup()) {
            for (const MapObject *o : group->objects()) {
                if (!o->type().isEmpty()) {
                    out << "[" << group->name() << "]\n";

                    // display object name as comment
                    if (!o->name().isEmpty())
                        out << "# " << o->name() << "\n";

                    out << "type=" << o->type() << "\n";
                    int x,y,w,h;
                    if (map->orientation() == Map::Orthogonal) {
                        x = o->x()/map->tileWidth();
                        y = o->y()/map->tileHeight();
                        w = o->width()/map->tileWidth();
                        h = o->height()/map->tileHeight();
                    } else {
                        x = o->x()/map->tileHeight();
                        y = o->y()/map->tileHeight();
                        w = o->width()/map->tileHeight();
                        h = o->height()/map->tileHeight();
                    }
                    out << "location=" << x << "," << y;
                    out << "," << w << "," << h << "\n";

                    // write all properties for this object
                    writeProperties(out, o->properties(), mapPath);
                    out << "\n";
                }
            }
        }
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

void FlarePlugin::writeProperties(QTextStream &out, const Properties &properties, const QString &mapPath)
{
    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        const auto exportValue = ExportValue::fromPropertyValue(it.value(), mapPath);
        out << it.key() << "=" << exportValue.value.toString() << "\n";
    }
}

} // namespace Flare
