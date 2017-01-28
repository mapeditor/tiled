/*
 * Orx Engine Tiled Plugin
 * Copyright 2017, Denis Brachet aka Ainvar <thegwydd@gmail.com>
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

#include "orx_plugin.h"

#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"

#include <QDataStream>
#include "orx_objects.h"

using namespace Orx;

OrxPlugin::OrxPlugin()
{
}

bool OrxPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    using namespace Tiled;

    std::stringstream ss;

    // crate map object
    Orx::ObjectPtr map_object = std::make_shared<Object>();

    std::vector<Orx::ObjectPtr> objects;
    std::vector<Orx::GraphicPtr> graphics;

    for (Layer *layer : map->layers())
    {
        // crate layer object, add to objects object and add as child of map object
        Orx::ObjectPtr layer_object = std::make_shared<Object>(layer->name().toStdString());
        objects.push_back(layer_object);
        map_object->m_Children.push_back(layer_object);
        // get all items of the layer
    }

    for (auto obj : graphics)
        obj->serialize(ss);

    for (auto obj : objects)
        obj->serialize(ss);



    return true;
}

QString OrxPlugin::nameFilter() const
{
    return tr("Orx Engine files (*.ini)");
}

QString OrxPlugin::errorString() const
{
    return mError;
}
