/*
 * tilelayeritem.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilelayeritem.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapview.h"
#include "tile.h"
#include "zoomable.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;

TileLayerItem::TileLayerItem(TileLayer *layer, MapDocument *mapDocument, QGraphicsItem *parent)
    : LayerItem(layer, parent)
    , mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    syncWithTileLayer();
}

void TileLayerItem::syncWithTileLayer()
{
    prepareGeometryChange();

    QRect layerBounds = tileLayer()->bounds();
    if (!mMapDocument->map()->infinite())
        layerBounds &= tileLayer()->rect();

    const MapRenderer *renderer = mMapDocument->renderer();
    QRectF boundingRect = renderer->boundingRect(layerBounds);

    QMargins margins = tileLayer()->drawMargins();
    if (const Map *map = tileLayer()->map()) {
        margins.setTop(margins.top() - map->tileHeight());
        margins.setRight(margins.right() - map->tileWidth());
    }

    mBoundingRect = boundingRect.adjusted(-margins.left(),
                                          -margins.top(),
                                          margins.right(),
                                          margins.bottom());
}

QRectF TileLayerItem::boundingRect() const
{
    return mBoundingRect;
}

void TileLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    const qreal scale = static_cast<MapView*>(widget->parent())->zoomable()->scale();

    MapRenderer *renderer = mMapDocument->renderer();
    renderer->setPainterScale(scale);
    // TODO: Display a border around the layer when selected
    renderer->drawTileLayer(painter, tileLayer(), option->exposedRect);
}
