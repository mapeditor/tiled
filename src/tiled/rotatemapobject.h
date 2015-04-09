/*
 * rotatemapobject.h
 * Copyright 2012, Przemysław Grzywacz <nexather@gmail.com>
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

#ifndef ROTATEMAPOBJECT_H
#define ROTATEMAPOBJECT_H

#include <QUndoCommand>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

class RotateMapObject : public QUndoCommand
{
public:
    RotateMapObject(MapDocument *mapDocument,
                    MapObject *mapObject,
                    qreal oldRotation);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;
    qreal mOldRotation;
    qreal mNewRotation;
};

} // namespace Internal
} // namespace Tiled

#endif // ROTATEMAPOBJECT_H
