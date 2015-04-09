/*
 * The Mana World Tiled Plugin
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tmwplugin.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"

#include <QDataStream>
#include <QFile>

using namespace Tmw;

TmwPlugin::TmwPlugin()
{
}

bool TmwPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    using namespace Tiled;

    TileLayer *collisionLayer = 0;

    foreach (Layer *layer, map->layers()) {
        if (layer->name().compare(QLatin1String("collision"),
                                  Qt::CaseInsensitive) == 0) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                if (collisionLayer) {
                    mError = tr("Multiple collision layers found!");
                    return false;
                }
                collisionLayer = tileLayer;
            }
        }
    }

    if (!collisionLayer) {
        mError = tr("No collision layer found!");
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    const int width = collisionLayer->width();
    const int height = collisionLayer->height();

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << (qint16) width;
    stream << (qint16) height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Tile *tile = collisionLayer->cellAt(x, y).tile;
            stream << (qint8) (tile && tile->id() > 0);
        }
    }

    return true;
}

QString TmwPlugin::nameFilter() const
{
    return tr("TMW-eAthena collision files (*.wlk)");
}

QString TmwPlugin::errorString() const
{
    return mError;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Tmw, TmwPlugin)
#endif
