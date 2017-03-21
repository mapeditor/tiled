/*
 * reparentlayers.cpp
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

#include "reparentlayers.h"

#include "grouplayer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

namespace Tiled {
namespace Internal {

ReparentLayers::ReparentLayers(MapDocument *mapDocument,
                               const QList<Layer *> &layers,
                               GroupLayer *layerParent,
                               int index,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mLayers(layers)
    , mLayerParent(layerParent)
    , mIndex(index)
{
}

void ReparentLayers::undo()
{
    auto layerModel = mMapDocument->layerModel();
    auto currentLayer = mMapDocument->currentLayer();

    for (int i = mUndoInfo.size() - 1; i >= 0; --i) {
        auto &undoInfo = mUndoInfo.at(i);
        auto layer = mLayers.at(i);

        layerModel->takeLayerAt(mLayerParent, undoInfo.newIndex);
        layerModel->insertLayer(undoInfo.parent, undoInfo.oldIndex, layer);
    }

    mUndoInfo.clear();

    mMapDocument->setCurrentLayer(currentLayer);
}

void ReparentLayers::redo()
{
    auto layerModel = mMapDocument->layerModel();
    auto currentLayer = mMapDocument->currentLayer();

    Q_ASSERT(mUndoInfo.isEmpty());
    mUndoInfo.reserve(mLayers.size());

    int index = mIndex;

    for (auto layer : mLayers) {
        UndoInfo undoInfo;
        undoInfo.parent = layer->parentLayer();
        undoInfo.oldIndex = layer->siblingIndex();
        undoInfo.newIndex = index++;

        layerModel->takeLayerAt(undoInfo.parent, undoInfo.oldIndex);
        layerModel->insertLayer(mLayerParent, undoInfo.newIndex, layer);

        mUndoInfo.append(undoInfo);
    }

    mMapDocument->setCurrentLayer(currentLayer);
}

} // namespace Internal
} // namespace Tiled
