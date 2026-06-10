/*
 * TBIN Tiled Plugin
 * Copyright 2017, Casey Warrington <spacechase0.and.cat@gmail.com>
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

#pragma once

#include "tbin_global.h"

#include "mapformat.h"
#include "plugin.h"
#include "properties.h"

#include <QObject>

namespace Tiled
{
    class Object;
    class Cell;
    class Map;
}

namespace tbin
{
    class Map;
}

namespace Tbin {

class TBINSHARED_EXPORT TbinPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;

    static std::unique_ptr< Tiled::Map > fromTbin( const tbin::Map& tmap, const QDir &fileDir );
    static tbin::Map toTbin( const Tiled::Map* map, const QDir &fileDir );
};

} // namespace Tbin
