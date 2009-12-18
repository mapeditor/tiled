/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "offsetlayer.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

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
    mOffsetLayer->offset(offset, bounds, wrapX, wrapY);
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
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(mIndex);
    layerModel->insertLayer(mIndex, layer);

    if (mIndex == currentIndex)
        mMapDocument->setCurrentLayer(mIndex);

    return replaced;
}
