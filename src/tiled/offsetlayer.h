/*
 * offsetlayer.h
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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
