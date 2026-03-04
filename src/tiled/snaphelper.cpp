/*
 * snaphelper.cpp
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2016, Mamed Ibrahimov <ibramlab@gmail.com>
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

namespace Tiled {

SnapHelper::SnapHelper(const MapRenderer *renderer,
                       Qt::KeyboardModifiers modifiers)
    : mRenderer(renderer)
    , mSnapMode(Preferences::instance()->snapMode())
{
    if (modifiers & Qt::ControlModifier) {
        if (modifiers & Qt::ShiftModifier) {
            toggleFineSnap();
        } else {
            toggleSnap();
        }
    }
}

void SnapHelper::toggleSnap()
{
    switch (mSnapMode) {
    case SnapMode::None:
    case SnapMode::Pixels:
        mSnapMode = SnapMode::Grid;
        break;
    case SnapMode::Grid:
    case SnapMode::FineGrid:
        mSnapMode = SnapMode::None;
        break;
    }
}

void SnapHelper::toggleFineSnap()
{
    switch (mSnapMode) {
    case SnapMode::None:
    case SnapMode::Grid:
    case SnapMode::Pixels:
        mSnapMode = SnapMode::FineGrid;
        break;
    case SnapMode::FineGrid:
        mSnapMode = SnapMode::Grid;
        break;
    }
}

void SnapHelper::snap(QPointF &pixelPos) const
{
    switch (mSnapMode) {
    case SnapMode::None:
        break;
    case SnapMode::FineGrid: {
        const int gridFine = Preferences::instance()->gridFine();
        pixelPos = mRenderer->snapToGrid(pixelPos, gridFine);
        break;
    }
    case SnapMode::Grid:
        pixelPos = mRenderer->snapToGrid(pixelPos);
        break;
    case SnapMode::Pixels: {
        QPointF screenPos = mRenderer->pixelToScreenCoords(pixelPos);
        pixelPos = mRenderer->screenToPixelCoords(screenPos.toPoint());
        break;
    }
    }
}

} // namespace Tiled
