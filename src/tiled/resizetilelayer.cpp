/*
 * resizetilelayer.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "resizetilelayer.h"

#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ResizeTileLayer::ResizeTileLayer(MapDocument *mapDocument,
                                 TileLayer *layer,
                                 const QSize &size,
                                 const QPoint &offset)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Resize Layer"))
    , mMapDocument(mapDocument)
    , mIndex(mapDocument->map()->layers().indexOf(layer))
    , mOriginalLayer(0)
{
    Q_ASSERT(mIndex != -1);

    // Create the resized layer (once)
    mResizedLayer = static_cast<TileLayer*>(layer->clone());
    mResizedLayer->resize(size, offset);
}

ResizeTileLayer::~ResizeTileLayer()
{
    delete mOriginalLayer;
    delete mResizedLayer;
}

void ResizeTileLayer::undo()
{
    Q_ASSERT(!mResizedLayer);
    mResizedLayer = static_cast<TileLayer*>(swapLayer(mOriginalLayer));
    mOriginalLayer = 0;
}

void ResizeTileLayer::redo()
{
    Q_ASSERT(!mOriginalLayer);
    mOriginalLayer = static_cast<TileLayer*>(swapLayer(mResizedLayer));
    mResizedLayer = 0;
}

Layer *ResizeTileLayer::swapLayer(Layer *layer)
{
    const int currentIndex = mMapDocument->currentLayerIndex();

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *replaced = layerModel->takeLayerAt(mIndex);
    layerModel->insertLayer(mIndex, layer);

    if (mIndex == currentIndex)
        mMapDocument->setCurrentLayerIndex(mIndex);

    return replaced;
}
