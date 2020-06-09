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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class Layer;
class Map;

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
     * @param layer       the layer to move
     * @param direction   the direction in which to move the layer
     */
    MoveLayer(MapDocument *mapDocument, Layer *layer, Direction direction);

    void undo() override { moveLayer(); }
    void redo() override { moveLayer(); }

    static bool canMoveUp(const Layer &layer);
    static bool canMoveDown(const Layer &layer);
    static bool canMoveUp(const QList<Layer *> &layers);
    static bool canMoveDown(const QList<Layer *> &layers);

private:
    void moveLayer();

    MapDocument *mMapDocument;
    Layer *mLayer;
    Direction mDirection;
};

} // namespace Tiled
