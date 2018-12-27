/*
 * addremovetileset.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileset.h"
#include "undocommands.h"

#include <QCoreApplication>
#include <QUndoCommand>

namespace Tiled {

class MapDocument;

/**
 * Abstract base class for AddTileset and RemoveTileset.
 */
class AddRemoveTileset : public QUndoCommand
{
public:
    AddRemoveTileset(MapDocument *mapDocument,
                     int index,
                     const SharedTileset &tileset,
                     QUndoCommand *parent = nullptr);

    ~AddRemoveTileset();

protected:
    void addTileset();
    void removeTileset();

    MapDocument *mMapDocument;
    SharedTileset mTileset;
    int mIndex;
};

/**
 * Adds a tileset to a map.
 */
class AddTileset : public AddRemoveTileset, public ClonableUndoCommand
{
public:
    AddTileset(MapDocument *mapDocument, const SharedTileset &tileset,
               QUndoCommand *parent = nullptr);

    void undo() override
    { removeTileset(); }

    void redo() override
    { addTileset(); }

    AddTileset *clone(QUndoCommand *parent = nullptr) const override;
};

/**
 * Removes a tileset from a map.
 */
class RemoveTileset : public AddRemoveTileset
{
public:
    RemoveTileset(MapDocument *mapDocument, int index);

    void undo() override
    { addTileset(); }

    void redo() override
    { removeTileset(); }
};

} // namespace Tiled
