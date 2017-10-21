/*
 * capturestamphelper.cpp
 * Copyright 2017, Your Name <your.name@domain>
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

#include "capturestamphelper.h"

#include "map.h"
#include "tilelayer.h"

namespace Tiled {
namespace Internal {

CaptureStampHelper::CaptureStampHelper()
    : mActive(false)
{
}

void CaptureStampHelper::beginCapture(QPoint tilePosition)
{
    mActive = true;
    mCaptureStart = tilePosition;
}

TileStamp CaptureStampHelper::endCapture(const TileLayer *tileLayer, QPoint tilePosition)
{
    mActive = false;

    if (!tileLayer)
        return TileStamp();

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea(tilePosition).intersected(tileLayer->bounds());
    if (!captured.isValid())
        return TileStamp();

    captured.translate(-tileLayer->x(), -tileLayer->y());
    Map *map = tileLayer->map();
    TileLayer *capture = tileLayer->copy(captured);
    Map *stamp = new Map(map->orientation(),
                         capture->width(),
                         capture->height(),
                         map->tileWidth(),
                         map->tileHeight());

    // Gets if the relative stagger should be the same as the base layer
    int staggerIndexOffSet;
    if (tileLayer->map()->staggerAxis() == Map::StaggerX)
        staggerIndexOffSet = captured.x() % 2;
    else
        staggerIndexOffSet = captured.y() % 2;

    stamp->setStaggerAxis(map->staggerAxis());
    stamp->setStaggerIndex((Map::StaggerIndex)((map->staggerIndex() + staggerIndexOffSet) % 2));

    // Add tileset references to map
    stamp->addTilesets(capture->usedTilesets());

    stamp->addLayer(capture);

    return TileStamp(stamp);
}

void CaptureStampHelper::reset()
{
    mActive = false;
}

QRect CaptureStampHelper::capturedArea(QPoint tilePosition) const
{
    QRect captured = QRect(mCaptureStart, tilePosition).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

} // namespace Internal
} // namespace Tiled
