/*
 * changemapobjectsorder.h
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

#ifndef CHANGEMAPOBJECTSORDER_H
#define CHANGEMAPOBJECTSORDER_H

#include <QUndoCommand>

namespace Tiled {

class ObjectGroup;

namespace Internal {

class MapDocument;

class ChangeMapObjectsOrder : public QUndoCommand
{
public:
    ChangeMapObjectsOrder(MapDocument *mapDocument,
                          ObjectGroup *objectGroup,
                          int from,
                          int to,
                          int count);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    ObjectGroup *mObjectGroup;
    int mFrom;
    int mTo;
    int mCount;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEMAPOBJECTSORDER_H
