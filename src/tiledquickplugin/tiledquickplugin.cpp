/*
 * tiledquickplugin.cpp
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled Quick.
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

#include "tiledquickplugin.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"

#include "mapitem.h"
#include "maploader.h"

#include <qqml.h>

using namespace TiledQuick;

void TiledQuickPlugin::registerTypes(const char *uri)
{
    // @uri org.mapeditor.Tiled

    qmlRegisterType<Tiled::Map>(uri, 1, 0, "Map");
    qmlRegisterType<Tiled::MapObject>(uri, 1, 0, "MapObject");
    qmlRegisterType<Tiled::ObjectGroup>(uri, 1, 0, "ObjectGroup");

    qmlRegisterType<TiledQuick::MapLoader>(uri, 1, 0, "MapLoader");
    qmlRegisterType<TiledQuick::MapItem>(uri, 1, 0, "MapItem");
}
