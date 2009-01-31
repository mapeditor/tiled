/*
 * Tiled Map Editor (Qt)
 * Copyright 2008-2009 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef BRUSHITEM_H
#define BRUSHITEM_H

#include <QGraphicsItem>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * This brush item is used to represent a brush in a map scene before it is
 * used.
 *
 * It currently also implements a simple tile brush.
 * TODO: Separate Brush out of BrushItem
 */
class BrushItem : public QGraphicsItem
{
public:
    /**
     * Constructor.
     */
    BrushItem();

    /**
     * Sets the map document this brush is operating on.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Updates the position in tiles. Paints while painting is active.
     */
    void setTilePos(int x, int y);

    /**
     * Start painting.
     */
    void beginPaint();

    /**
     * Stop painting.
     */
    void endPaint();

    // QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    /**
     * Performs the actual painting.
     */
    void doPaint();

    int mTileX, mTileY;
    MapDocument *mMapDocument;
    bool mPainting;
};

} // namespace Internal
} // namespace Tiled

#endif // BRUSHITEM_H
