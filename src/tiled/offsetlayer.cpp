/*
 * offsetlayer.cpp
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 * Copyright 2009-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "offsetlayer.h"

#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "tilelayer.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

OffsetLayer::OffsetLayer(MapDocument *mapDocument,
                         int index,
                         const QPoint &offset,
                         const QRect &bounds,
                         bool wrapX,
                         bool wrapY)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Offset Layer"))
    , mMapDocument(mapDocument)
    , mIndex(index)
    , mOriginalLayer(0)
{
    // Create the offset layer (once)
    Layer *layer = mMapDocument->map()->layerAt(mIndex);
    mOffsetLayer = layer->clone();

    switch (mOffsetLayer->layerType()) {
    case Layer::TileLayerType:
        static_cast<TileLayer*>(mOffsetLayer)->offset(offset, bounds, wrapX, wrapY);
        break;
    case Layer::ObjectGroupType: {
        // Object groups need offset and bounds converted to pixel units
        MapRenderer *renderer = mapDocument->renderer();
        const QPointF origin = renderer->tileToPixelCoords(QPointF());
        const QPointF pixelOffset = renderer->tileToPixelCoords(offset) - origin;
        const QRectF pixelBounds = renderer->tileToPixelCoords(bounds);
        static_cast<ObjectGroup*>(mOffsetLayer)->offset(pixelOffset, pixelBounds, wrapX, wrapY);
        break;
    }
    case Layer::ImageLayerType:
        // Nothing done for the image layer at the moment
        break;
    }
}

OffsetLayer::~OffsetLayer()
{
    delete mOriginalLayer;
    delete mOffsetLayer;
}

void OffsetLayer::undo()
{
    Q_ASSERT(!mOffsetLayer);
    mOffsetLayer = swapLayer(mOriginalLayer);
    mOriginalLayer = 0;
}

void OffsetLayer::redo()
{
    Q_ASSERT(!mOriginalLayer);
    mOriginalLayer = swapLayer(mOffsetLayer);
    mOffsetLayer = 0;
}

Layer *OffsetLayer::swapLayer(Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayerIndex();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(mIndex);
    layerModel->insertLayer(mIndex, layer);

    if (mIndex == currentIndex)
        mMapDocument->setCurrentLayerIndex(mIndex);

    return replaced;
}
