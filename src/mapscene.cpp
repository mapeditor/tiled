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

#include "mapscene.h"

#include "layertablemodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "tilelayeritem.h"

#include <QPainter>

using namespace Tiled::Internal;

MapScene::MapScene(QObject *parent):
    QGraphicsScene(parent),
    mMapDocument(0),
    mGridVisible(true)
{
    setBackgroundBrush(Qt::darkGray);
}

void MapScene::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->layerModel()->disconnect(this);

    mMapDocument = mapDocument;
    refreshScene();

    // TODO: This should really be more optimal (adding/removing as necessary)
    if (mMapDocument) {
        LayerTableModel *layerModel = mMapDocument->layerModel();
        connect(layerModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                this, SLOT(refreshScene()));
        connect(layerModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(refreshScene()));
    }
}

void MapScene::refreshScene()
{
    // Clear any existing items
    clear();

    if (!mMapDocument) {
        setSceneRect(QRectF());
        return;
    }

    const Map *map = mMapDocument->map();

    // The +1 is to allow space for the right and bottom grid lines
    setSceneRect(0, 0,
            map->width() * map->tileWidth() + 1,
            map->height() * map->tileHeight() + 1);

    int z = 0;
    foreach (Layer *layer, map->layers()) {
        if (dynamic_cast<TileLayer*>(layer) != 0) {
            TileLayerItem *item =
                new TileLayerItem(static_cast<TileLayer*>(layer));
            item->setZValue(z++);
            addItem(item);
        } else if (dynamic_cast<ObjectGroup*>(layer) != 0) {
            ObjectGroup *objectGroup = static_cast<ObjectGroup*>(layer);
            foreach (MapObject *object, objectGroup->objects()) {
                MapObjectItem *item = new MapObjectItem(object);
                item->setZValue(z++);
                addItem(item);
            }
        }
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
    if (!mMapDocument || !mGridVisible)
        return;

    Map *map = mMapDocument->map();

    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    const int startX = (int) (rect.x() / tileWidth) * tileWidth;
    const int startY = (int) (rect.y() / tileHeight) * tileHeight;
    const int endX = qMin((int) rect.right(), map->width() * tileWidth + 1);
    const int endY = qMin((int) rect.bottom(),
                          map->height() * tileHeight + 1);

    painter->setPen(Qt::black);
    painter->setOpacity(0.5f);

    for (int x = startX; x < endX; x += tileWidth) {
        painter->drawLine(x, (int) rect.top(), x, endY - 1);
    }

    for (int y = startY; y < endY; y += tileHeight) {
        painter->drawLine((int) rect.left(), y, endX - 1, y);
    }
}
