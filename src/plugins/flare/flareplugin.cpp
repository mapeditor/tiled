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

    // write all properties for this map
    Properties::const_iterator it = map->properties().constBegin();
    Properties::const_iterator it_end = map->properties().constEnd();
    for (; it != it_end; ++it) {
        out << it.key() << "=" << it.value() << "\n";
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

bool FlarePlugin::checkOneLayerWithName(const Tiled::Map *map,
                                        const QString &name)
{
    int count = 0;
    foreach (const Layer *layer, map->layers())
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

Q_EXPORT_PLUGIN2(Flare, FlarePlugin)
