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

ResizeTileLayer::ResizeTileLayer(MapDocument *mapDocument,
                                 TileLayer *layer,
                                 QSize size,
                                 QPoint offset,
                                 QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Resize Layer"),
                   parent)
    , mMapDocument(mapDocument)
    , mDone(false)
    , mOriginalLayer(layer)
{
    // Create the resized layer (once)
    mResizedLayer = layer->clone();
    mResizedLayer->resize(size, offset);
}

ResizeTileLayer::~ResizeTileLayer()
{
    if (mDone)
        delete mOriginalLayer;
    else
        delete mResizedLayer;
}

void ResizeTileLayer::undo()
{
    Q_ASSERT(mDone);
    LayerModel *layerModel = mMapDocument->layerModel();
    layerModel->replaceLayer(mResizedLayer, mOriginalLayer);
    mDone = false;
}

void ResizeTileLayer::redo()
{
    Q_ASSERT(!mDone);
    LayerModel *layerModel = mMapDocument->layerModel();
    layerModel->replaceLayer(mOriginalLayer, mResizedLayer);
    mDone = true;
}
