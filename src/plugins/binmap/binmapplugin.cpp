/*
 * BinMap Tiled Plugin
 * Copyright 2025, bzt <bztsrc@gitlab>
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
 *
 * To install, copy this directory under `tiled/src/plugins/binmap` directory,
 * and add `"binmap",` line to `tiled/src/plugins/plugins.qbs` file.
 *
 * Simplest format possible. Consist of a 16 bytes long header and then data.
 * See FORMAT.md for details.
 */

#include "binmapplugin.h"

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
#include <QStringView>
#include <QTextStream>

#include <memory>
#include <fstream>

#include "binmap.h"

using namespace Tiled;

namespace Binmap {

BinmapPlugin::BinmapPlugin()
{
}

QString BinmapPlugin::nameFilter() const
{
    return tr("Binary map files (*.map)");
}

QString BinmapPlugin::shortName() const
{
    return QStringLiteral("binmap");
}

QString BinmapPlugin::errorString() const
{
    return mError;
}

bool BinmapPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)
    binmap_header_t header;

    // construct header
    memset( &header, 0, sizeof(header) );
    header.magic[0] = 'M';
    header.magic[1] = 'A';
    header.magic[2] = 'P';
    header.version = 0;
    header.mapWidth = map->width();
    header.mapHeight = map->height();
    header.tileWidth = map->tileWidth();
    header.tileHeight = map->tileHeight();
    switch( map->orientation() ) {
        case Map::Orthogonal: header.orientation = O_ORTHO; break;
        case Map::Isometric:  header.orientation = O_ISO; break;
        case Map::Hexagonal:  header.orientation = map->staggerAxis() == Map::StaggerX ? O_HEXH : O_HEXV; break;
        default:
            mError = QCoreApplication::translate("File Errors", "Unsupported map format.");
            return false;
        break;
    }
    // count layers
    for (Layer *layer : map->layers()) {
        if (layer->asTileLayer()) header.numLayers++;
    }

    // open file
    std::ofstream file(fileName.toStdString(), std::ios::trunc | std::ios::binary);
    if (!file) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    GidMapper gidMapper(map->tilesets());

    // write header
    file.write( reinterpret_cast< const char* >( &header ), sizeof(header) );

    // write layers
    for (Layer *layer : map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            for (int y = 0; y < map->height(); ++y) {
                for (int x = 0; x < map->width(); ++x) {
                    Cell t = tileLayer->cellAt(x, y);
                    unsigned short id = gidMapper.cellToGid(t);
                    file.write( reinterpret_cast< const char* >( &id ), sizeof(id) );
                }
            }
        }
    }

    file.close();

    return true;
}

} // namespace Flare
