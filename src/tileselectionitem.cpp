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

#include "tileselectionitem.h"

#include "map.h"
#include "mapdocument.h"
#include "tileselectionmodel.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

TileSelectionItem::TileSelectionItem(MapDocument *mapDocument)
    : mMapDocument(mapDocument)
{
}

QRectF TileSelectionItem::boundingRect() const
{
    const QRect b = mMapDocument->selectionModel()->selection().boundingRect();
    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();

    return QRectF(b.x() * tileWidth,
                  b.y() * tileHeight,
                  b.width() * tileWidth,
                  b.height() * tileHeight);
}

void TileSelectionItem::paint(QPainter *painter,
                              const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    // TODO: Paint only the areas of the selection model intersecting with the
    //       to be painted area.
    Q_UNUSED(option);
    Q_UNUSED(widget);

    const QRegion selection = mMapDocument->selectionModel()->selection();
    const Map *map = mMapDocument->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();
    const QColor color(255, 0, 0, 128);

    foreach (const QRect &r, selection.rects()) {
        painter->fillRect(QRect(r.x() * tileWidth,
                                r.y() * tileHeight,
                                r.width() * tileWidth,
                                r.height() * tileHeight),
                          color);
    }
}
