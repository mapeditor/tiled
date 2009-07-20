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

#include "resizelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

using namespace Tiled;
using namespace Tiled::Internal;

ResizeLayer::ResizeLayer(MapDocument *mapDocument,
                         int index,
                         const QSize &size,
                         const QPoint &offset)
    : QUndoCommand(QObject::tr("Resize Layer"))
    , mMapDocument(mapDocument)
    , mIndex(index)
    , mOriginalLayer(0)
{
    // Create the resized layer (once)
    Layer *layer = mMapDocument->map()->layerAt(mIndex);
    mResizedLayer = layer->clone();
    mResizedLayer->resize(size, offset);
}

ResizeLayer::~ResizeLayer()
{
    delete mOriginalLayer;
    delete mResizedLayer;
}

void ResizeLayer::undo()
{
    Q_ASSERT(!mResizedLayer);
    mResizedLayer = swapLayer(mOriginalLayer);
    mOriginalLayer = 0;
}

void ResizeLayer::redo()
{
    Q_ASSERT(!mOriginalLayer);
    mOriginalLayer = swapLayer(mResizedLayer);
    mResizedLayer = 0;
}

Layer *ResizeLayer::swapLayer(Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(mIndex);
    layerModel->insertLayer(mIndex, layer);

    if (mIndex == currentIndex)
        mMapDocument->setCurrentLayer(mIndex);

    return replaced;
}
