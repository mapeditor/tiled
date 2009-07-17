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

#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QMap>

class QModelIndex;

namespace Tiled {

class Layer;
class MapObject;
class Tile;

namespace Internal {

class BrushItem;
class MapDocument;
class MapObjectItem;
class ObjectGroupItem;
class TileLayerItem;

/**
 * A graphics scene that represents the contents of a map.
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

    /**
     * Override for handling enter and leave events.
     */
    bool event(QEvent *event);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private slots:
    /**
     * Refreshes the map scene.
     */
    void refreshScene();

    /**
     * Repaints the specified region. The region is in tile coordinates.
     */
    void repaintRegion(const QRegion &region);

    /**
     * Notifies the scene that the current tile has changed. The scene passes
     * this on to the tile brush.
     */
    void currentTileChanged(Tile *tile);

    void currentLayerChanged();

    void layerAdded(int index);
    void layerRemoved(int index);
    void layerChanged(int index);

    void objectsAdded(const QList<MapObject*> &objects);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);

private:
    QGraphicsItem *createLayerItem(Layer *layer);

    void updateInteractionMode();
    void setBrushVisible(bool visible);
    void updateBrushVisibility();

    MapDocument *mMapDocument;
    ObjectGroupItem *mSelectedObjectGroupItem;
    BrushItem *mBrush;
    bool mGridVisible;
    bool mBrushVisible;
    bool mPainting;
    QVector<QGraphicsItem*> mLayerItems;

    typedef QMap<MapObject*, MapObjectItem*> ObjectItems;
    ObjectItems mObjectItems;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPSCENE_H
