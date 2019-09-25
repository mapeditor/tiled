/*
 * tilelayeritem.h
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled Quick.
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

#include <QQuickItem>

#include "tilelayer.h"

namespace Tiled {
class MapRenderer;
}

namespace TiledQuick {

class MapItem;

/**
 * A graphical item displaying a tile layer in a Qt Quick scene.
 */
class TileLayerItem : public QQuickItem
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param layer    the tile layer to be displayed
     * @param renderer the map renderer to use to render the layer
     */
    TileLayerItem(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer,
                  MapItem *parent);

    /**
     * Updates the size and position of this item. Should be called when the
     * size of either the tile layer or its associated map have changed.
     *
     * Calling this function when the size of the map changes is necessary
     * because in certain map orientations this affects the layer position
     * (when using the IsometricRenderer for example).
     */
    void syncWithTileLayer();

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *);

public slots:
    void updateVisibleTiles();

private:
    void layerVisibilityChanged();

    Tiled::TileLayer *mLayer;
    Tiled::MapRenderer *mRenderer;
    QRect mVisibleTiles;
};

/**
 * A graphical item displaying a single tile in a Qt Quick scene.
 */
class TileItem : public QQuickItem
{
    Q_OBJECT

public:
    TileItem(const Tiled::Cell &cell, QPoint position, MapItem *parent);

    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *);

    QPoint position() const;

private:
    Tiled::Cell mCell;
    QPoint mPosition;
};

inline QPoint TileItem::position() const
{
    return mPosition;
}

} // namespace TiledQuick
