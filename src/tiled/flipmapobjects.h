/*
 * flipmapobjects.h
 * Copyright 2013-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include <QVector>

namespace Tiled {

class MapObject;
class Document;

class FlipMapObjects : public QUndoCommand
{
public:
    FlipMapObjects(Document *document,
                   const QList<MapObject *> &mapObjects,
                   FlipDirection flipDirection,
                   QPointF flipOrigin);

    void undo() override { flip(); }
    void redo() override { flip(); }

private:
    void flip();

    Document *mDocument;
    const QList<MapObject *> mMapObjects;
    FlipDirection mFlipDirection;
    QPointF mFlipOrigin;

    QVector<MapObject::ChangedProperties> mOldChangedProperties;
    QVector<MapObject::ChangedProperties> mNewChangedProperties;
};

} // namespace Tiled
