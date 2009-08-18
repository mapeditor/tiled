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

#ifndef ADDREMOVEMAPOBJECT_H
#define ADDREMOVEMAPOBJECT_H

#include <QUndoCommand>

namespace Tiled {

class MapObject;
class ObjectGroup;

namespace Internal {

class MapDocument;

/**
 * Abstract base class for AddMapObject and RemoveMapObject.
 */
class AddRemoveMapObject : public QUndoCommand
{
public:
    AddRemoveMapObject(MapDocument *mapDocument,
                       ObjectGroup *objectGroup,
                       MapObject *mapObject,
                       bool ownObject);
    ~AddRemoveMapObject();

protected:
    void addObject();
    void removeObject();

private:
    MapDocument *mMapDocument;
    MapObject *mMapObject;
    ObjectGroup *mObjectGroup;
    int mIndex;
    bool mOwnsObject;
};

/**
 * Undo command that adds an object to a map.
 */
class AddMapObject : public AddRemoveMapObject
{
public:
    AddMapObject(MapDocument *mapDocument, ObjectGroup *objectGroup,
                 MapObject *mapObject)
        : AddRemoveMapObject(mapDocument,
                             objectGroup,
                             mapObject,
                             true)
    {
        setText(QObject::tr("Add Object"));
    }

    void undo()
    { removeObject(); }

    void redo()
    { addObject(); }
};

/**
 * Undo command that removes an object from a map.
 */
class RemoveMapObject : public AddRemoveMapObject
{
public:
    RemoveMapObject(MapDocument *mapDocument, MapObject *mapObject);

    void undo()
    { addObject(); }

    void redo()
    { removeObject(); }
};

} // namespace Internal
} // namespace Tiled

#endif // ADDREMOVEMAPOBJECT_H
