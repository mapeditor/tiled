/*
 * Tiled Map Editor (Qt)
 * Copyright 2010 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef ADDREMOVETILESET_H
#define ADDREMOVETILESET_H

#include <QCoreApplication>
#include <QUndoCommand>

namespace Tiled {

class Tileset;

namespace Internal {

class MapDocument;

/**
 * Abstract base class for AddTileset and RemoveTileset.
 */
class AddRemoveTileset : public QUndoCommand
{
public:
    AddRemoveTileset(MapDocument *mapDocument, int index, Tileset *tileset);
    ~AddRemoveTileset();

protected:
    void addTileset();
    void removeTileset();

private:
    MapDocument *mMapDocument;
    Tileset *mTileset;
    int mIndex;
};

/**
 * Adds a tileset to a map.
 */
class AddTileset : public AddRemoveTileset
{
public:
    AddTileset(MapDocument *mapDocument, Tileset *tileset);

    void undo()
    { removeTileset(); }

    void redo()
    { addTileset(); }
};

/**
 * Removes a tileset from a map.
 */
class RemoveTileset : public AddRemoveTileset
{
public:
    RemoveTileset(MapDocument *mapDocument, int index, Tileset *tileset)
        : AddRemoveTileset(mapDocument, index, tileset)
    {
        setText(QCoreApplication::translate("Undo Commands",
                                            "Remove Tileset"));
    }

    void undo()
    { addTileset(); }

    void redo()
    { removeTileset(); }
};

} // namespace Internal
} // namespace Tiled

#endif // ADDREMOVETILESET_H
