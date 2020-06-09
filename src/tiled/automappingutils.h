/*
 * automappingutils.h
 * Copyright 2012, Stefan Beller, stefanbeller@googlemail.com
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
class ObjectGroup;

class MapDocument;

const QList<MapObject*> objectsInRegion(const ObjectGroup *layer,
                                        const QRegion &where);

void eraseRegionObjectGroup(MapDocument *mapDocument,
                            ObjectGroup *layer,
                            const QRegion &where);

QRegion tileRegionOfObjectGroup(const ObjectGroup *layer);

} // namespace Tiled
