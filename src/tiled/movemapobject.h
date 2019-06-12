/*
 * movemapobject.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include <QPointF>

namespace Tiled {

class MapObject;

class Document;

class MoveMapObject : public QUndoCommand
{
public:
    MoveMapObject(Document *document,
                  MapObject *mapObject,
                  const QPointF &oldPos,
                  QUndoCommand *parent = nullptr);

    MoveMapObject(Document *document,
                  MapObject *mapObject,
                  const QPointF &newPos,
                  const QPointF &oldPos,
                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    MapObject *mMapObject;
    QPointF mOldPos;
    QPointF mNewPos;
};

} // namespace Tiled
