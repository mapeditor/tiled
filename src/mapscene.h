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

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QString>

namespace Tiled {

namespace Internal {

class MapDocument;
class MapObjectItem;
class TileLayerItem;

/**
 * A graphics scene that displays the contents of a map.
 */
class MapScene : public QGraphicsScene
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    MapScene(QObject *parent);

    /**
     * Returns the map document this scene is displaying.
     */
    MapDocument *mapDocument() const { return mMapDocument; }

    /**
     * Sets the map this scene displays.
     */
    void setMapDocument(MapDocument *map);

    /**
     * Returns whether the tile grid is visible.
     */
    bool isGridVisible() const { return mGridVisible; }

    /**
     * Returns the TileLayerItem for the layer with the given name.
     */
    TileLayerItem *layer(const QString& layer);

public slots:
    /**
     * Sets whether the tile grid is visible.
     */
    void setGridVisible(bool visible);

protected:
    /**
     * QGraphicsScene::drawForeground override that draws the tile grid.
     */
    void drawForeground(QPainter *painter, const QRectF &rect);

public slots:
    /**
     * Refreshes the map scene.
     *
     * TODO: Only temporarily public until there is a nicer way for the
     *       scene to be notified about changes to the map.
     */
    void refreshScene();

private:
    MapDocument *mMapDocument;
    bool mGridVisible;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPSCENE_H
