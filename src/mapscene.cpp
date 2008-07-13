/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#include "mapscene.h"

#include "map.h"
#include "layer.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "tilelayeritem.h"

#include <QPainter>

using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMap(0),
    mGridVisible(true)
{
}

void MapScene::setMap(Map *map)
{
    // Clear any existing items
    clear();

    mMap = map;

    if (mMap) {
        // The +1 is to allow space for the right and bottom grid lines
        setSceneRect(0, 0,
                     mMap->width() * map->tileWidth() + 1,
                     mMap->height() * map->tileHeight() + 1);

        int z = 0;
        foreach (Layer *layer, mMap->layers()) {
            TileLayerItem *item = new TileLayerItem(layer);
            item->setZValue(z++);
            mLayers.insert(layer->name(), item);
            addItem(item);
        }
        foreach (ObjectGroup * objectgroup, mMap->objectGroups()) {
            foreach (MapObject * object, objectgroup->objects()) {
                MapObjectItem *item = new MapObjectItem(object);
                item->setZValue(z + 1);
                mObjects.insert(object->name(), item);
                addItem(item);
            }
        }
    } else {
        setSceneRect(QRectF());
    }
}

void MapScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

void MapScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (!mMap || !mGridVisible)
        return;

    const int tileWidth = mMap->tileWidth();
    const int tileHeight = mMap->tileHeight();

    const int startX = (int) (rect.x() / tileWidth) * tileWidth;
    const int startY = (int) (rect.y() / tileHeight) * tileHeight;
    const int endX = qMin((int) rect.right(), mMap->width() * tileWidth + 1);
    const int endY = qMin((int) rect.bottom(),
                          mMap->height() * tileHeight + 1);

    painter->setPen(Qt::black);
    painter->setOpacity(0.5f);

    for (int x = startX; x < endX; x += tileWidth) {
        painter->drawLine(x, (int) rect.top(), x, endY - 1);
    }

    for (int y = startY; y < endY; y += tileHeight) {
        painter->drawLine((int) rect.left(), y, endX - 1, y);
    }
}
