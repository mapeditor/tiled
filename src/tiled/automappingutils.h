/*
 * automappingutils.h
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QRegion>

namespace Tiled {

class MapObject;
class MapRenderer;
class ObjectGroup;

QRect objectTileRect(const MapRenderer &renderer,
                     const MapObject &object);

QList<MapObject*> objectsInRegion(const MapRenderer &renderer,
                                  const ObjectGroup *layer,
                                  const QRegion &where);

QRegion tileRegionOfObjectGroup(const MapRenderer &renderer,
                                const ObjectGroup *objectGroup);

} // namespace Tiled
