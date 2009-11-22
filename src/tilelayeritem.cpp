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

#include "tilelayeritem.h"

#include "tile.h"
#include "tilelayer.h"
#include "map.h"
#include "maprenderer.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

TileLayerItem::TileLayerItem(TileLayer *layer, MapRenderer *renderer)
    : mLayer(layer)
    , mRenderer(renderer)
{
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
#endif

    syncWithTileLayer();
    setOpacity(mLayer->opacity());
}

void TileLayerItem::syncWithTileLayer()
{
    prepareGeometryChange();
    mBoundingRect = mRenderer->boundingRect(mLayer->bounds());
}

QRectF TileLayerItem::boundingRect() const
{
    return mBoundingRect;
}

void TileLayerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *)
{
    // TODO: Display a border around the layer when selected
    mRenderer->drawTileLayer(painter, mLayer, option->exposedRect);
}
