/*
 * movetileset.h
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

#ifndef MOVETILESET_H
#define MOVETILESET_H

#include "undocommands.h"

#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * An undo command for moving a tileset from one place to another.
 */
class MoveTileset : public QUndoCommand
{
public:
    MoveTileset(MapDocument *mapDocument, int from, int to);

    void undo();
    void redo();

    int id() const { return Cmd_MoveTileset; }
    bool mergeWith(const QUndoCommand *other);

private:
    MapDocument *mMapDocument;
    int mFrom;
    int mTo;
};

} // namespace Internal
} // namespace Tiled

#endif // MOVETILESET_H
