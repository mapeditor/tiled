/*
 * resizemapobject.h
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

#ifndef RESIZEMAPOBJECT_H
#define RESIZEMAPOBJECT_H

#include <QUndoCommand>
#include <QSizeF>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

class ResizeMapObject : public QUndoCommand
{
public:
    ResizeMapObject(MapDocument *mapDocument,
                    MapObject *mapObject,
                    const QSizeF &oldSize);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;
    QSizeF mOldSize;
    QSizeF mNewSize;
};

} // namespace Internal
} // namespace Tiled

#endif // RESIZEMAPOBJECT_H
