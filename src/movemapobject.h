/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MOVEMAPOBJECT_H
#define MOVEMAPOBJECT_H

#include <QUndoCommand>
#include <QPoint>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

class MoveMapObject : public QUndoCommand
{
public:
    MoveMapObject(MapDocument *mapDocument,
                  MapObject *mapObject,
                  const QPoint &oldPos);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;
    QPoint mOldPos;
    QPoint mNewPos;
};

} // namespace Internal
} // namespace Tiled

#endif // MOVEMAPOBJECT_H
