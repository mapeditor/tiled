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

#include "mapdocument.h"

#include <QGraphicsObject>
#include <QMap>

#include <memory>

namespace Tiled {

class ImageLayer;
class Layer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

class BorderItem;
class LayerItem;
class MapObjectItem;
class ObjectSelectionItem;
class TileGridItem;
class TileSelectionItem;

/**
 * A graphics item that represents the contents of a map.
 *
 * It also adds the functionality for displaying tile and object selection,
 * as well as highlighting of the current layer.
 */
class MapItem : public QGraphicsObject
{
    Q_OBJECT

public:
    enum DisplayMode {
        ReadOnly,
        Editable
    };

    MapItem(const MapDocumentPtr &mapDocument, DisplayMode displayMode,
            QGraphicsItem *parent = nullptr);
    ~MapItem() override;

    MapDocument *mapDocument() const;

    void setDisplayMode(DisplayMode displayMode);
    void setShowTileCollisionShapes(bool enabled);

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *, const QStyleOptionGraphicsItem *,
               QWidget *widget = nullptr) override;

signals:
    void boundingRectChanged();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /**
     * Repaints the specified \a region of the given \a tileLayer. The region
     * is in tile coordinates.
     */
    void repaintRegion(const QRegion &region, TileLayer *tileLayer);

    void documentChanged(const ChangeEvent &change);
    void mapChanged();
    void tileLayerChanged(TileLayer *tileLayer, MapDocument::TileLayerChangeFlags flags);

    void layerAdded(Layer *layer);
    void layerRemoved(Layer *layer);
    void layerChanged(Layer *layer);

    void imageLayerChanged(ImageLayer *imageLayer);

    void adaptToTilesetTileSizeChanges(Tileset *tileset);
    void adaptToTileSizeChanges(Tile *tile);
    void tileObjectGroupChanged(Tile *tile);

    void tilesetReplaced(int index, Tileset *tileset);

    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void deleteObjectItems(const QList<MapObject*> &objects);
    void syncObjectItems(const QList<MapObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    void syncAllObjectItems();

    void setObjectLineWidth(qreal lineWidth);
    void setShowTileObjectOutlines(bool enabled);

    void createLayerItems(const QList<Layer *> &layers);
    LayerItem *createLayerItem(Layer *layer);
    void deleteLayerItems(Layer *layer);

    void updateBoundingRect();
    void updateSelectedLayersHighlight();

    MapDocumentPtr mMapDocument;
    QGraphicsRectItem *mDarkRectangle;
    QGraphicsRectItem *mBorderRectangle;
    std::unique_ptr<TileSelectionItem> mTileSelectionItem;
    std::unique_ptr<TileGridItem> mTileGridItem;
    std::unique_ptr<ObjectSelectionItem> mObjectSelectionItem;
    QMap<Layer*, LayerItem*> mLayerItems;
    QMap<MapObject*, MapObjectItem*> mObjectItems;
    DisplayMode mDisplayMode;
    QRectF mBoundingRect;
    bool mIsHovered = false;
};

inline MapDocument *MapItem::mapDocument() const
{
    return mMapDocument.data();
}

} // namespace Tiled
