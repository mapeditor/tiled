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

#ifndef PAINTTILELAYER_H
#define PAINTTILELAYER_H

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

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    TileLayer *mTarget;
    TileLayer *mSource;
    TileLayer *mErased;
    int mX, mY;
};

} // namespace Internal
} // namespace Tiled

#endif // PAINTTILELAYER_H
