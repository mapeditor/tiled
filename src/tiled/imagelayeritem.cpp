/*
 * imagelayeritem.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>
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

#include "imagelayeritem.h"

#include "imagelayer.h"
#include "maprenderer.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

ImageLayerItem::ImageLayerItem(ImageLayer *layer, MapRenderer *renderer)
    : mLayer(layer)
    , mRenderer(renderer)
{
    setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);

    syncWithImageLayer();
    setOpacity(mLayer->opacity());
}

void ImageLayerItem::syncWithImageLayer()
{
    prepareGeometryChange();
    mBoundingRect = mRenderer->boundingRect(mLayer->bounds());
}

QRectF ImageLayerItem::boundingRect() const
{
    return mBoundingRect;
}

void ImageLayerItem::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           QWidget *)
{
    // TODO: Display a border around the layer when selected
    mRenderer->drawImageLayer(painter, mLayer, option->exposedRect);
}
