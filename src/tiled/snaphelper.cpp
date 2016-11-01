/*
 * snaphelper.cpp
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "snaphelper.h"

#include "preferences.h"
#include "mapobject.h"
#include "mapobjectitem.h"

#include "rtbcore.h"

#include <cmath>

namespace Tiled {
namespace Internal {

SnapHelper::SnapHelper(const MapRenderer *renderer,
                       Qt::KeyboardModifiers modifiers)
    : mRenderer(renderer)
{
    Preferences *preferences = Preferences::instance();
    mSnapToGrid = preferences->snapToGrid();
    mSnapToFineGrid = preferences->snapToFineGrid();

    if (modifiers & Qt::ControlModifier)
        toggleSnap();
}

void SnapHelper::toggleSnap()
{
    mSnapToGrid = !mSnapToGrid;
    mSnapToFineGrid = false;
}

void SnapHelper::snap(QPointF &pixelPos) const
{
    if (mSnapToFineGrid || mSnapToGrid) {
        QPointF tileCoords = mRenderer->pixelToTileCoords(pixelPos);
        if (mSnapToFineGrid) {
            int gridFine = Preferences::instance()->gridFine();
            tileCoords = (tileCoords * gridFine).toPoint();
            tileCoords /= gridFine;
        } else {
            tileCoords = tileCoords.toPoint();
        }
        pixelPos = mRenderer->tileToPixelCoords(tileCoords);
    }
}

void SnapHelper::snap(QPointF &pixelPos, QSet<MapObjectItem*> mapObjectItems, bool useHalfeTile) const
{
    // if half tiles not explicit forbidden check the selected objects if they are allowed
    if(useHalfeTile)
        useHalfeTile = RTBCore::instance()->isHalfTileAllowed(mapObjectItems);

    if (mSnapToFineGrid || mSnapToGrid) {
        QPointF tileCoords = mRenderer->pixelToTileCoords(pixelPos);
        if (mSnapToFineGrid) {
            int gridFine = Preferences::instance()->gridFine();
            tileCoords = (tileCoords * gridFine).toPoint();
            tileCoords /= gridFine;
        }
        else if(useHalfeTile)
        {
            tileCoords = roundedTileCoords(tileCoords);
        }
        else
        {
            tileCoords = tileCoords.toPoint();
        }
        pixelPos = mRenderer->tileToPixelCoords(tileCoords);
    }
}

QPointF SnapHelper::roundedTileCoords(QPointF &pixelPos) const
{
    qreal x = round(pixelPos.x() * 2) / 2;
    qreal y = round(pixelPos.y() * 2) / 2;
    return QPointF(x, y);
}
} // namespace Internal
} // namespace Tiled
