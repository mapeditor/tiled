/*
 * brushitem.h
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
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

#include "map.h"
#include "tilelayer.h"

#include <QGraphicsObject>

namespace Tiled {

class MapDocument;
class MapRenderer;

/**
 * This brush item is used to represent a brush in a map scene before it is
 * used.
 */
class BrushItem : public QGraphicsObject
{
    Q_OBJECT

public:
    BrushItem();

    void setMapDocument(MapDocument *mapDocument);

    void clear();

    void setTileLayer(const SharedTileLayer &tileLayer);
    void setTileLayer(const SharedTileLayer &tileLayer, const QRegion &region);
    const SharedTileLayer &tileLayer() const;

    void setMap(const SharedMap &map);
    void setMap(const SharedMap &map, const QRegion &region);
    const SharedMap &map() const;

    void setTileLayerPosition(QPoint pos);

    void setTileRegion(const QRegion &region);
    const QRegion &tileRegion() const;

    void setLayerOffset(const QPointF &offset);

    void setAnimatedOutline(bool enabled);
    bool animatedOutline() const { return mAnimatedOutline; }

    void setTileOffset(QPoint offset);
    QPoint tileOffset() const { return mTileOffset; }

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

protected:
    MapDocument *mapDocument() const { return mMapDocument; }
    void timerEvent(QTimerEvent *event) override;

private:
    void updateBoundingRect();
    void paintAnimatedOutline(QPainter *painter,
                              const QStyleOptionGraphicsItem *option);
    QPainterPath buildOutlinePath(MapRenderer *renderer,
                                  const QRegion &region);

    MapDocument *mMapDocument;
    SharedTileLayer mTileLayer;
    SharedMap mMap;
    QRegion mRegion;
    QRectF mBoundingRect;

    bool mAnimatedOutline = false;
    QPoint mTileOffset;
    int mAnimationTimer = -1;
    int mDashOffset = 0;
    static constexpr int AnimationInterval = 100;
};

/**
 * Returns the current tile layer.
 */
inline const SharedTileLayer &BrushItem::tileLayer() const
{
    return mTileLayer;
}

/**
 * Returns the current map.
 */
inline const SharedMap &BrushItem::map() const
{
    return mMap;
}

/**
 * Returns the region of the current tile layer or the region that was set
 * using setTileRegion.
 */
inline const QRegion &BrushItem::tileRegion() const
{
    return mRegion;
}

} // namespace Tiled
