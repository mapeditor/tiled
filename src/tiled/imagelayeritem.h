/*
 * imagelayeritem.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>

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

#include "layeritem.h"

#include "imagelayer.h"

namespace Tiled {

class MapDocument;

/**
 * A graphics item displaying an image layer in a QGraphicsView.
 */
class ImageLayerItem : public LayerItem
{
public:
    /**
     * Constructor.
     *
     * @param layer       the image layer to be displayed
     * @param mapDocument the map document owning the map of this layer
     */
    ImageLayerItem(ImageLayer *layer, MapDocument *mapDocument, QGraphicsItem *parent = nullptr);

    ImageLayer *imageLayer() const;

    /**
     * Updates the size and position of this item. Should be called when the
     * size of either the image layer or its associated map have changed.
     *
     * Calling this function when the size of the map changes is necessary
     * because in certain map orientations this affects the layer position
     * (when using the IsometricRenderer for example).
     */
    void syncWithImageLayer();

    // QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    MapDocument *mMapDocument;
    QRectF mBoundingRect;
};

inline ImageLayer *ImageLayerItem::imageLayer() const
{
    return static_cast<ImageLayer*>(layer());
}

} // namespace Tiled
