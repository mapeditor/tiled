/*
 * movelayer.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MOVELAYER_H
#define MOVELAYER_H

#include <QUndoCommand>

namespace Tiled {

class Map;

namespace Internal {

class MapDocument;

/**
 * A command that moves a map layer up or down in the layer stack.
 */
class MoveLayer : public QUndoCommand
{
public:
    enum Direction { Up, Down };

    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being changed
     * @param index       the index of the layer to move
     * @param direction   the direction in which to move the layer
     */
    MoveLayer(MapDocument *mapDocument, int index, Direction direction);

    void undo();
    void redo();

private:
    void moveLayer();

    MapDocument *mMapDocument;
    int mIndex;
    Direction mDirection;
};

} // namespace Internal
} // namespace Tiled

#endif // MOVELAYER_H
