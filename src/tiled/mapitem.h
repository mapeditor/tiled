/*
 * mapitem.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include <QGraphicsObject>
#include <QMap>
#include <QSet>

namespace Tiled {

class ImageLayer;
class Layer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

namespace Internal {

class LayerItem;
class MapDocument;
class MapObjectItem;

/**
 * A graphics item that represents the contents of a map.
 *
 * It also adds the functionality for displaying tile and object selection,
 * as well as highlighting of the current layer.
 */
class MapItem : public QGraphicsObject
{
public:
    MapItem(MapDocument *mapDocument, QGraphicsItem *parent = nullptr);

    MapDocument *mapDocument() const;

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *,
               QWidget *widget = nullptr) override;

private:
    /**
     * Repaints the specified \a region of the given \a tileLayer. The region
     * is in tile coordinates.
     */
    void repaintRegion(const QRegion &region, TileLayer *tileLayer);

    void currentLayerChanged();

    void mapChanged();
    void tileLayerChanged(TileLayer *tileLayer);

    void layerAdded(Layer *layer);
    void layerRemoved(Layer *layer);
    void layerChanged(Layer *layer);

    void objectGroupChanged(ObjectGroup *objectGroup);
    void imageLayerChanged(ImageLayer *imageLayer);

    void adaptToTilesetTileSizeChanges(Tileset *tileset);
    void adaptToTileSizeChanges(Tile *tile);

    void tilesetReplaced(int index, Tileset *tileset);

    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    void syncAllObjectItems();

    void setObjectLineWidth(qreal lineWidth);
    void setShowTileObjectOutlines(bool enabled);

    void createLayerItems(const QList<Layer *> &layers);
    LayerItem *createLayerItem(Layer *layer);

    void updateCurrentLayerHighlight();

    MapDocument *mMapDocument;
    QGraphicsRectItem *mDarkRectangle;
    QMap<Layer*, LayerItem*> mLayerItems;
    QMap<MapObject*, MapObjectItem*> mObjectItems;
};

inline MapDocument *MapItem::mapDocument() const
{
    return mMapDocument;
}

} // namespace Internal
} // namespace Tiled
