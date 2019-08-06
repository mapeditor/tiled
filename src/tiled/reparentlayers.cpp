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
#include "mapdocument.h"
#include "map.h"

#include "qtcompat_p.h"

namespace Tiled {

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
    // Sort layers by global index (visual order)
    std::sort(mLayers.begin(), mLayers.end(), [] (Layer *a, Layer *b) {
        return globalIndex(a) < globalIndex(b);
    });
}

void ReparentLayers::undo()
{
    auto layerModel = mMapDocument->layerModel();

    const auto currentLayer = mMapDocument->currentLayer();
    const auto selectedLayers = mMapDocument->selectedLayers();

    for (int i = mUndoInfo.size() - 1; i >= 0; --i) {
        auto& undoInfo = mUndoInfo.at(i);
        auto layer = mLayers.at(i);

        layerModel->takeLayerAt(mLayerParent, layer->siblingIndex());
        layerModel->insertLayer(undoInfo.parent, undoInfo.oldIndex, layer);
    }

    mUndoInfo.clear();

    mMapDocument->setCurrentLayer(currentLayer);
    mMapDocument->setSelectedLayers(selectedLayers);
}

void ReparentLayers::redo()
{
    auto layerModel = mMapDocument->layerModel();

    const auto currentLayer = mMapDocument->currentLayer();
    const auto selectedLayers = mMapDocument->selectedLayers();

    Q_ASSERT(mUndoInfo.isEmpty());
    mUndoInfo.reserve(mLayers.size());

    int index = mIndex;

    for (auto layer : qAsConst(mLayers)) {
        UndoInfo undoInfo;
        undoInfo.parent = layer->parentLayer();
        undoInfo.oldIndex = layer->siblingIndex();

        bool sameParent = undoInfo.parent == mLayerParent;

        // Adjust the insertion index when it is affected by the layer removal
        if (sameParent && undoInfo.oldIndex < index)
            --index;

        layerModel->takeLayerAt(undoInfo.parent, undoInfo.oldIndex);
        layerModel->insertLayer(mLayerParent, index, layer);

        ++index;

        mUndoInfo.append(undoInfo);
    }

    mMapDocument->setCurrentLayer(currentLayer);
    mMapDocument->setSelectedLayers(selectedLayers);
}

} // namespace Tiled
