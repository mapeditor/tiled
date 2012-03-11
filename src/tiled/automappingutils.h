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

#ifndef AUTOMAPPINGUTILS_H
#define AUTOMAPPINGUTILS_H

#include <QRegion>

namespace Tiled {

class MapObject;
class ObjectGroup;

namespace Internal {

class MapDocument;

const QList<MapObject*> objectsInRegion(ObjectGroup *layer,
                                        const QRegion &where);

void eraseRegionObjectGroup(MapDocument *mapDocument,
                            ObjectGroup *layer,
                            const QRegion &where);

QRegion tileRegionOfObjectGroup(ObjectGroup *layer);

} // namespace Internal
} // namespace Tiled

#endif // AUTOMAPPINGUTILS_H
