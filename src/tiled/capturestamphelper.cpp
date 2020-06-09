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
#include "tilelayer.h"

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
    std::unique_ptr<Map> stamp { new Map(mapDocument.map()->orientation(),
                                         captured.width(),
                                         captured.height(),
                                         mapDocument.map()->tileWidth(),
                                         mapDocument.map()->tileHeight()) };

    // Iterate all layers to make sure we're adding layers in the right order
    LayerIterator it(mapDocument.map(), Layer::TileLayerType);
    while (auto tileLayer = static_cast<TileLayer*>(it.next())) {
        if (!mapDocument.selectedLayers().contains(tileLayer))
            continue;

        // Intersect with the layer and translate to layer coordinates
        QRect capturedFromLayer = captured.intersected(tileLayer->bounds());
        if (!captured.isValid())
            continue;
        capturedFromLayer.translate(-tileLayer->position());

        auto capture = tileLayer->copy(capturedFromLayer);
        capture->setName(tileLayer->name());
        capture->setPosition(capturedFromLayer.topLeft() - captured.topLeft());

        stamp->addLayer(std::move(capture));
    }

    if (stamp->layerCount() > 0) {
        auto staggerAxis = mapDocument.map()->staggerAxis();
        auto staggerIndex = mapDocument.map()->staggerIndex();

        // Gets if the relative stagger should be the same as the base layer
        int staggerIndexOffSet;
        if (staggerAxis == Map::StaggerX)
            staggerIndexOffSet = captured.x() % 2;
        else
            staggerIndexOffSet = captured.y() % 2;

        stamp->setStaggerAxis(staggerAxis);
        stamp->setStaggerIndex(static_cast<Map::StaggerIndex>((staggerIndex + staggerIndexOffSet) % 2));

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
    QRect captured = QRect(mCaptureStart, tilePosition).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

} // namespace Tiled
