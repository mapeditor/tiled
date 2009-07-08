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

#ifndef DELETELAYER_H
#define DELETELAYER_H

#include <QUndoCommand>

namespace Tiled {

class Layer;

namespace Internal {

class MapDocument;

class DeleteLayer : public QUndoCommand
{
public:
    /**
     * Creates an undo command that deletes the layer at the given index.
     */
    DeleteLayer(MapDocument *mapDocument, int index);

    ~DeleteLayer();

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    Layer *mLayer;
    int mIndex;
};

} // namespace Internal
} // namespace Tiled

#endif // DELETELAYER_H
