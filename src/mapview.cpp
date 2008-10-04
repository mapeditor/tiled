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

#include "mapview.h"

#include "mapscene.h"

using namespace Tiled::Internal;

MapView::MapView(QWidget *parent):
    QGraphicsView(parent)
{
}

/* TODO: Find a better way to show/hide the brush when necessary
 *
 * These events apply to the MapView, which includes the scrollbars. But when
 * the mouse is over the scrollbars, the brush should be hidden. It would be
 * nice when these events would also be sent to the QGraphicsScene, but they
 * don't seem to exist there.
 */

void MapView::enterEvent(QEvent *event)
{
    if (MapScene *mapScene = dynamic_cast<MapScene*>(scene()))
        mapScene->setBrushVisible(true);
}

void MapView::leaveEvent(QEvent *event)
{
    if (MapScene *mapScene = dynamic_cast<MapScene*>(scene()))
        mapScene->setBrushVisible(false);
}
