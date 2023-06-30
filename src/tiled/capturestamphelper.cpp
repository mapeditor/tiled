/*
 * capturestamphelper.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include "mapdocument.h"

#include <memory>

namespace Tiled {

CaptureStampHelper::CaptureStampHelper()
    : mActive(false)
{
}

void CaptureStampHelper::beginCapture(QPoint tilePosition)
{
    mActive = true;
    mCaptureStart = tilePosition;
}

TileStamp CaptureStampHelper::endCapture(const MapDocument &mapDocument, QPoint tilePosition)
{
    mActive = false;

    QRect captured = capturedArea(tilePosition);

    Map::Parameters mapParameters = mapDocument.map()->parameters();
    mapParameters.width = captured.width();
    mapParameters.height = captured.height();
    mapParameters.infinite = false;

    auto stamp = std::make_unique<Map>(mapParameters);

    mapDocument.map()->copyLayers(mapDocument.selectedLayers(),
                                  captured,
                                  *stamp);

    if (stamp->layerCount() > 0) {
        stamp->normalizeTileLayerPositionsAndMapSize();

        // Add tileset references to map
        stamp->addTilesets(stamp->usedTilesets());

        return TileStamp(std::move(stamp));
    }

    return TileStamp();
}

void CaptureStampHelper::reset()
{
    mActive = false;
}

QRect CaptureStampHelper::capturedArea(QPoint tilePosition) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRect captured = QRect(mCaptureStart, tilePosition).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
#else
    return QRect::span(mCaptureStart, tilePosition);
#endif
}

} // namespace Tiled
