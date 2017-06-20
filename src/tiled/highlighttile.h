/*
 * highlighttile.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2017 Leon Moctezuma <leon.moctezuma@gmail.com>
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

#include "tilelayer.h"

#include <QGraphicsItem>
#include <Qtimer>
#include <QPropertyAnimation>
#include <QColor>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * This brush item is used to represent a brush in a map scene before it is
 * used.
 */
class HighlightTile : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(QColor insideMapColor READ insideMapColor WRITE setInsideMapColor)

public:


    HighlightTile();

    void setMapDocument(MapDocument *mapDocument);

    void clear();

    void setTileLayer(const SharedTileLayer &tileLayer);
    void setTileLayer(const SharedTileLayer &tileLayer, const QRegion &region);
    const SharedTileLayer &tileLayer() const;

    void setTileLayerPosition(const QPoint &pos);

    void setTileRegion(const QRegion &region);
    QRegion tileRegion() const;

    void setLayerOffset(const QPointF &offset);

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void setInsideMapColor(const QColor &color);
    QColor insideMapColor() const { return mInsideMapColor; }

private:
    void updateBoundingRect();

    MapDocument *mMapDocument;
    SharedTileLayer mTileLayer;
    QRegion mRegion;
    QRectF mBoundingRect;

    QColor mInsideMapColor;
    QPropertyAnimation mAnimation;

public slots:

    void updateHighlight();
};

/**
 * Clears the tile layer and region set on this item.
 */
inline void HighlightTile::clear()
{
    setTileLayer(SharedTileLayer());
}

/**
 * Returns the current tile layer.
 */
inline const SharedTileLayer &HighlightTile::tileLayer() const
{
    return mTileLayer;
}

/**
 * Returns the region of the current tile layer or the region that was set
 * using setTileRegion.
 */
inline QRegion HighlightTile::tileRegion() const
{
    return mRegion;
}

} // namespace Internal
} // namespace Tiled
