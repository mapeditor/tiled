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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class MapObject;

class Document;

class RotateMapObject : public QUndoCommand
{
public:
    RotateMapObject(Document *document,
                    MapObject *mapObject,
                    qreal oldRotation);

    RotateMapObject(Document *document,
                    MapObject *mapObject,
                    qreal newRotation,
                    qreal oldRotation);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    MapObject *mMapObject;
    qreal mOldRotation;
    qreal mNewRotation;
    bool mOldChangeState;
};

} // namespace Tiled
