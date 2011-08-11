/*
 * Flare Tiled Plugin
 * Copyright 2010, Jaderamiso <jaderamiso@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"

#include <QFile>
#include <QTextStream>

using namespace Flare;
using namespace Tiled;

FlarePlugin::FlarePlugin()
{
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
    if (!checkOneLayerWithName(map, QLatin1String("background")))
        return false;
    if (!checkOneLayerWithName(map, QLatin1String("object")))
        return false;
    if (!checkOneLayerWithName(map, QLatin1String("collision")))
        return false;

    QString title = checkProperty(map, QLatin1String("title"));
    QString tileset = checkProperty(map, QLatin1String("tileset"));
    QString music = checkProperty(map, QLatin1String("music"));

    if (title.isEmpty() || tileset.isEmpty() || music.isEmpty())
        return false;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        mError = tr("File cannot be opened");
        return false;
    }

    QTextStream out(&file);
    int mapWidth = map->width();
    int mapHeight = map->height();

    // write [header]
    out << "[header]\n";
    out << "width=" << mapWidth << "\n";
    out << "height=" << mapHeight << "\n";
    out << "music=" << music << "\n";
    out << "tileset=" << tileset << "\n";
    out << "title=" << title << "\n";
    QString spawnpoint = map->property(QLatin1String("spawnpoint"));
    if (!spawnpoint.isEmpty())
        out << "spawnpoint=" << spawnpoint << "\n";

    out << "\n";

    GidMapper g;
    uint firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        g.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }

    // write layers
    foreach (Layer *layer, map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            out << "[layer]\n";
            out << "id=" << layer->name() << "\n";
            out << "format=dec\n";
            out << "data=\n";
            for (int y = 0; y < mapWidth; ++y) {
                for (int x = 0; x < mapHeight; ++x) {
                    Cell t = tileLayer->cellAt(x, y);
                    int id = 0;
                    if (t.tile)
                        id = g.cellToGid(t);
                    out << id;
                    if (x < mapHeight - 1)
                        out << ",";
                }
                if (y < mapWidth - 1)
                    out << ",";
                out << "\n";
            }
            out << "\n";
        }
        if (ObjectGroup *group = layer->asObjectGroup()) {
            foreach (MapObject *o, group->objects()) {
                out << "[" << group->name() << "]\n";
                out << "type=" << o->type() << "\n";
                out << "location=" << o->x() << "," << o->y();
                QString facing = o->property(QLatin1String("facing"));
                if (group->name() == QLatin1String("event") || group->name() == QLatin1String("enemygroup"))
                    out << "," << o->width() << "," << o->height() << "\n";
                else if (group->name() == QLatin1String("enemy") && !facing.isEmpty())
                    out << "," << facing << "\n";
                else
                    out << "\n";
                Properties::const_iterator it = o->properties().constBegin();
                Properties::const_iterator it_end = o->properties().constEnd();
                for (; it != it_end; ++it) {
                    if (QLatin1String(it.key().toUtf8()) != QLatin1String("facing"))
                        out << it.key().toUtf8() << "=" << it.value().toUtf8() << "\n";
                }
                out << "\n";
            }
        }
    }
    file.close();
    return true;
}

bool FlarePlugin::checkOneLayerWithName(const Tiled::Map *map,
                                        const QString &name)
{
    int count = 0;
    foreach (Layer *layer, map->layers())
        if (layer->name() == name)
            count++;

    if (count == 0) {
        mError = tr("No \"%1\" layer found!").arg(name);
        return false;
    } else if (count > 1) {
        mError = tr("Multiple \"%1\" layers found!").arg(name);
        return false;
    }

    return true;
}

QString FlarePlugin::checkProperty(const Tiled::Map *map,
                                   const QString &name)
{
    QString value = map->property(name);
    if (value.isEmpty())
        mError = tr("No map property \"%1\" found!").arg(name);
    return value;
}

Q_EXPORT_PLUGIN2(Flare, FlarePlugin)
