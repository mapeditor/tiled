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

namespace Tiled {
namespace Internal {

SnapHelper::SnapHelper(const MapRenderer *renderer,
                       Qt::KeyboardModifiers modifiers)
    : mRenderer(renderer)
{
    Preferences *preferences = Preferences::instance();
    mSnapToGrid = preferences->snapToGrid();
    mSnapToFineGrid = preferences->snapToFineGrid();

    if (modifiers & Qt::ControlModifier) {
        mSnapToGrid = !mSnapToGrid;
        mSnapToFineGrid = false;
    }
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

} // namespace Internal
} // namespace Tiled
