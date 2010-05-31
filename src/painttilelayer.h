/*
 * painttilelayer.h
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

#ifndef PAINTTILELAYER_H
#define PAINTTILELAYER_H

#include "undocommands.h"

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {

class TileLayer;

namespace Internal {

class MapDocument;

/**
 * A command that paints one tile layer on top of another tile layer.
 */
class PaintTileLayer : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * @param mapDocument the map document that's being edited
     * @param target      the target layer to paint on
     * @param x           the x position of the paint location
     * @param y           the y position of the paint location
     * @param source      the source layer to paint on the target layer
     */
    PaintTileLayer(MapDocument *mapDocument,
                   TileLayer *target,
                   int x, int y,
                   const TileLayer *source);

    ~PaintTileLayer();

    /**
     * Sets whether this undo command can be merged with an existing command.
     */
    void setMergeable(bool mergeable)
    { mMergeable = mergeable; }

    void undo();
    void redo();

    int id() const { return Cmd_PaintTileLayer; }
    bool mergeWith(const QUndoCommand *other);

private:
    MapDocument *mMapDocument;
    TileLayer *mTarget;
    TileLayer *mSource;
    TileLayer *mErased;
    int mX, mY;
    QRegion mPaintedRegion;
    bool mMergeable;
};

} // namespace Internal
} // namespace Tiled

#endif // PAINTTILELAYER_H
