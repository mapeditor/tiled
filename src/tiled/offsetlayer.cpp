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

#include "changeevents.h"
#include "imagelayer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "tilelayer.h"

#include <QCoreApplication>

#include "qtcompat_p.h"

using namespace Tiled;

/**
 * Creates an undo command that offsets the layer at \a index by \a offset,
 * within \a bounds, and can optionally wrap on the x or y axis.
 *
 * If \a bounds is empty, the \a offset is applied everywhere and the wrapping
 * is ignored.
 */
OffsetLayer::OffsetLayer(MapDocument *mapDocument,
                         Layer *layer,
                         QPoint offset,
                         const QRect &bounds,
                         bool wrapX,
                         bool wrapY)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Offset Layer"))
    , mMapDocument(mapDocument)
    , mDone(false)
    , mOriginalLayer(layer)
    , mOffsetLayer(nullptr)
{
    switch (mOriginalLayer->layerType()) {
    case Layer::TileLayerType:
        mOffsetLayer = layer->clone();
        if (bounds.isEmpty())
            static_cast<TileLayer*>(mOffsetLayer)->offsetTiles(offset);
        else
            static_cast<TileLayer*>(mOffsetLayer)->offsetTiles(offset, bounds, wrapX, wrapY);
        break;
    case Layer::ObjectGroupType:
        mOffsetLayer = layer->clone();
        Q_FALLTHROUGH();
    case Layer::ImageLayerType:
    case Layer::GroupLayerType: {
        // These layers need offset and bounds converted to pixel units
        MapRenderer *renderer = mapDocument->renderer();
        const QPointF origin = renderer->tileToPixelCoords(QPointF());
        const QPointF pixelOffset = renderer->tileToPixelCoords(offset) - origin;
        const QRectF pixelBounds = renderer->tileToPixelCoords(bounds);

        if (mOriginalLayer->layerType() == Layer::ObjectGroupType) {
            static_cast<ObjectGroup*>(mOffsetLayer)->offsetObjects(pixelOffset, pixelBounds, wrapX, wrapY);
        } else {
            // (wrapping not supported for image layers and group layers)
            mOldOffset = mOriginalLayer->offset();
            mNewOffset = mOldOffset + pixelOffset;
        }
        break;
    }
    }
}

OffsetLayer::~OffsetLayer()
{
    if (mOffsetLayer) {
        if (mDone)
            delete mOriginalLayer;
        else
            delete mOffsetLayer;
    }
}

void OffsetLayer::undo()
{
    Q_ASSERT(mDone);
    LayerModel *layerModel = mMapDocument->layerModel();
    if (mOffsetLayer) {
        layerModel->replaceLayer(mOffsetLayer, mOriginalLayer);
    } else {
        mOriginalLayer->setOffset(mOldOffset);
        emit mMapDocument->changed(LayerChangeEvent(mOriginalLayer, LayerChangeEvent::OffsetProperty));
    }
    mDone = false;
}

void OffsetLayer::redo()
{
    Q_ASSERT(!mDone);
    LayerModel *layerModel = mMapDocument->layerModel();
    if (mOffsetLayer) {
        layerModel->replaceLayer(mOriginalLayer, mOffsetLayer);
    } else {
        mOriginalLayer->setOffset(mNewOffset);
        emit mMapDocument->changed(LayerChangeEvent(mOriginalLayer, LayerChangeEvent::OffsetProperty));
    }
    mDone = true;
}
