/*
 * movelayer.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "movelayer.h"

#include "grouplayer.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

MoveLayer::MoveLayer(MapDocument *mapDocument, Layer *layer, Direction direction):
    mMapDocument(mapDocument),
    mLayer(layer),
    mDirection(direction)
{
    setText((direction == Down) ?
            QCoreApplication::translate("Undo Commands", "Lower Layer") :
            QCoreApplication::translate("Undo Commands", "Raise Layer"));
}

void MoveLayer::redo()
{
    moveLayer();
}

void MoveLayer::undo()
{
    moveLayer();
}

// todo: this code should be able to move a layer through the whole hierachy
void MoveLayer::moveLayer()
{
    const auto currentLayer = mMapDocument->currentLayer();
    const bool selectedBefore = (mLayer == currentLayer);

    LayerModel *layerModel = mMapDocument->layerModel();
    const auto parentLayer = mLayer->parentLayer();

    const int index = mLayer->siblingIndex();
    const int insertionIndex = (mDirection == Down) ? index - 1 : index + 1;

    layerModel->takeLayerAt(parentLayer, index);
    layerModel->insertLayer(parentLayer, insertionIndex, mLayer);

    // Change the direction
    mDirection = (mDirection == Down) ? Up : Down;

    if (selectedBefore)
        mMapDocument->setCurrentLayer(mLayer);
}

} // namespace Tiled
} // namespace Internal
