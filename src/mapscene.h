/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>

namespace Tiled {

class Map;

namespace Internal {

/**
 * A graphics scene that displays the contents of a map.
 */
class MapScene : public QGraphicsScene
{
    public:
        /**
         * Constructor.
         */
        MapScene(QObject *parent);

        /**
         * Returns the map this scene is displaying.
         */
        Map *map() const { return mMap; }

        /**
         * Sets the map this scene displays.
         */
        void setMap(Map *map);

    protected:
        /**
         * QGraphicsScene::drawForeground override that draws the tile grid.
         */
        void drawForeground(QPainter *painter, const QRectF &rect);

    private:
        Map *mMap;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPSCENE_H
