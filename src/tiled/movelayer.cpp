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

void MoveLayer::moveLayer()
{
    LayerIterator iterator(mLayer);
    if (mDirection == Down)
        iterator.previous();
    else
        iterator.next();

    Q_ASSERT(iterator.currentLayer());

    const auto parent = mLayer->parentLayer();
    const int index = mLayer->siblingIndex();

    auto insertionParent = iterator.currentLayer()->parentLayer();
    int insertionIndex = iterator.currentSiblingIndex();

    if (mDirection == Down) {
        if (insertionParent != parent) {
            // Index adjustment to make sure we insert above (but exiting the group)
            ++insertionIndex;
        } else if (iterator.currentLayer()->isGroupLayer()) {
            // Enter the group from the top
            insertionParent = static_cast<GroupLayer*>(iterator.currentLayer());
            insertionIndex = insertionParent->layerCount();
        }
    } else {
        if (index + 1 == mLayer->siblings().size()) {
            // When existing a group make sure we insert above
            ++insertionIndex;
        } else if (iterator.currentLayer()->isGroupLayer()) {
            // Enter the group from the bottom
            insertionParent = static_cast<GroupLayer*>(iterator.currentLayer());
            insertionIndex = 0;
        }
    }

    const auto currentLayer = mMapDocument->currentLayer();

    LayerModel *layerModel = mMapDocument->layerModel();
    layerModel->takeLayerAt(parent, index);
    layerModel->insertLayer(insertionParent, insertionIndex, mLayer);

    // Change the direction
    mDirection = (mDirection == Down) ? Up : Down;

    mMapDocument->setCurrentLayer(currentLayer);
}

} // namespace Tiled
} // namespace Internal
