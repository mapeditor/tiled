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

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <QPainter>

namespace Tiled {

class Layer;
class Map;
class TileLayer;

namespace Internal {

/**
 * This class is responsible for rendering tile layers. Currently, only
 * orthogonal maps are supported.
 *
 * TODO: The functionality in this class depends on the orientation of the map.
 *       Hence, this class should really be an interface, implemented by the
 *       different supported orientations.
 */
class MapRenderer
{
public:
    MapRenderer(Map *map);

    /**
     * Returns the bounding rectangle of the given \a layer in pixels.
     */
    QRect layerBoundingRect(Layer *layer) const;

    /**
     * Draws the given \a layer using the given \a painter.
     *
     * Optionally, you can pass in the \a exposed rect (of pixels), so that
     * only tiles that can be visible in this area will be drawn.
     */
    void drawTileLayer(QPainter *painter, TileLayer *layer,
                       const QRectF &exposed = QRectF());

private:
    Map *mMap;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPRENDERER_H
