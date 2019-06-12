/*
 * flipmapobjects.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mapobject.h"

#include <QList>
#include <QUndoCommand>

namespace Tiled {

class Document;

class FlipMapObjects : public QUndoCommand
{
public:
    FlipMapObjects(Document *document,
                   const QList<MapObject *> &mapObjects,
                   FlipDirection flipDirection);

    void undo() override { flip(); }
    void redo() override { flip(); }

private:
    void flip();

    Document *mDocument;
    const QList<MapObject *> mMapObjects;
    QPointF mObjectsCenter;
    FlipDirection mFlipDirection;

    QVector<bool> mOldRotationStates;
    QVector<bool> mNewRotationStates;
    QVector<bool> mOldCellStates;
    QVector<bool> mNewCellStates;
};

} // namespace Tiled
