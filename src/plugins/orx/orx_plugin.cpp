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
#include "orx_exporter.h"


namespace Orx {

///////////////////////////////////////////////////////////////////////////////
OrxPlugin::OrxPlugin()
{
}

///////////////////////////////////////////////////////////////////////////////
bool OrxPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    std::shared_ptr<orxExporter> exporter = std::make_shared<orxExporter>();
    return exporter->Export(map, fileName);
}

///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::nameFilter() const
{
    return tr("Orx Engine files (*.ini)");
}

///////////////////////////////////////////////////////////////////////////////
QString OrxPlugin::errorString() const
{
    return mError;
}


} //namespace Orx
