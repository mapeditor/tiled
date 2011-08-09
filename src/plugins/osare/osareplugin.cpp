/*
 * Osare Tiled Plugin
 * Copyright Jaderamiso
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

#include "osareplugin.h"
#include <QDebug>

#include "gidmapper.h"
#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"


#include <QFile>
#include <QTextStream>

using namespace Osare;
using namespace Tiled;

int layerCount(QList<Tiled::Layer*> mapLayers, const QString layerName)
{
    int count = 0;
    foreach (Layer *layer, mapLayers)
        if(layer->name() == layerName)
            count++;
    return count;
}

OsarePlugin::OsarePlugin()
{

}

QString OsarePlugin::nameFilter() const
{
    return tr("Osare map files (*.txt)");
}

QString OsarePlugin::errorString() const
{
    return mError;
}

bool OsarePlugin::write(const Tiled::Map *map, const QString &fileName)
{
    QList<Layer* > mapLayers = map->layers();

    // check background layer
    int count = layerCount(mapLayers, QString("background"));
    if (count == 0) {
        mError = tr("No background layer found!");
        return false;
    } else if (count > 1) {
        mError = tr("Multiple background layers found!");
    }

    // check object layer
    count = layerCount(mapLayers, QString("object"));
    if (count == 0) {
        mError = tr("No object layer found!");
        return false;
    } else if (count > 1) {
        mError = tr("Multiple object layers found!");
        return false;
    }

    // check collision layer
    count = layerCount(mapLayers, QString("collision"));
    if (count == 0) {
        mError = tr("No collision layer found!");
        return false;
    } else if (count > 1) {
        mError = tr("Multiple collision layer found!");
        return false;
    }

    QString title = map->property(QLatin1String("title"));
    if (title.isEmpty()) {
        mError = tr("No map property 'title' found!");
        return false;
    }
    QString tileset = map->property(QLatin1String("tileset"));
    if (tileset.isEmpty()) {
        mError = tr("No map property 'tileset' found!");
        return false;
    }
    QString music = map->property(QLatin1String("music"));
    if (music.isEmpty()) {
        mError = tr("No map property 'music' found!");
        return false;
    }

    // get map properties
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
    g.clear();
    uint firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        g.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }

    // write layers
    foreach(Layer *layer, mapLayers){
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            out << "[layer]\n";
            out << "id=" << layer->name() << "\n";
            out << "format=dec\n";
            out << "data=\n";
            for (int y = 0; y < mapWidth; ++y) {
                for (int x = 0; x < mapHeight; ++x) {
                    Cell t = tileLayer->cellAt(x, y);
                    int id = 0;
                    if(t.tile)
                        id = g.cellToGid(t);
                    out << id;
                    if(x < mapHeight - 1)
                        out << ",";
                }
                if(y < mapWidth - 1)
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

Q_EXPORT_PLUGIN2(Osare, OsarePlugin)
