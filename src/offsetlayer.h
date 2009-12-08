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

#ifndef OFFSETLAYER_H
#define OFFSETLAYER_H

#include <QRect>
#include <QPoint>
#include <QUndoCommand>

namespace Tiled {

class Layer;

namespace Internal {

class MapDocument;

/**
 * Undo command that offsets a map layer.
 */
class OffsetLayer : public QUndoCommand
{
public:
    /**
     * Creates an undo command that offsets the layer at \a index by \a offset,
     * within \a bounds, and can optionally wrap on the x or y axis.
     */
    OffsetLayer(MapDocument *mapDocument,
                int index,
                const QPoint &offset,
                const QRect &bounds,
                bool xWrap,
                bool yWrap);

    ~OffsetLayer();

    void undo();
    void redo();

private:
    Layer *swapLayer(Layer *layer);

    MapDocument *mMapDocument;
    int mIndex;
    Layer *mOriginalLayer;
    Layer *mOffsetLayer;
};

} // namespace Internal
} // namespace Tiled

#endif // OFFSETLAYER_H
