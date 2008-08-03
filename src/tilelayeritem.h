/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef TILELAYERITEM_H
#define TILELAYERITEM_H

#include <QGraphicsItem>

namespace Tiled {

class TileLayer;

namespace Internal {

/**
 * A graphics item displaying a tile layer in a QGraphicsView.
 */
class TileLayerItem : public QGraphicsItem
{
    public:
        /**
         * Constructor.
         *
         * @param layer the tile layer to be displayed
         */
        TileLayerItem(TileLayer *layer);

        // QGraphicsView
        QRectF boundingRect() const;
        void paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget = 0);

    private:
        TileLayer *mLayer;
};

} // namespace Internal
} // namespace Tiled

#endif // TILELAYERITEM_H
