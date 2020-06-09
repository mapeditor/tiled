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

#include <algorithm>

namespace Tiled {

MoveLayer::MoveLayer(MapDocument *mapDocument, Layer *layer, Direction direction):
    mMapDocument(mapDocument),
    mLayer(layer),
    mDirection(direction)
{
    setText((direction == Down) ?
            QCoreApplication::translate("Undo Commands", "Lower Layer") :
            QCoreApplication::translate("Undo Commands", "Raise Layer"));
}

bool MoveLayer::canMoveUp(const Layer &layer)
{
    return layer.parentLayer() || layer.siblingIndex() < layer.siblings().size() - 1;
}

bool MoveLayer::canMoveDown(const Layer &layer)
{
    return layer.parentLayer() || layer.siblingIndex() > 0;
}

bool MoveLayer::canMoveUp(const QList<Layer *> &layers)
{
    return std::all_of(layers.begin(), layers.end(),
                       [] (Layer *layer) { return canMoveUp(*layer); });
}

bool MoveLayer::canMoveDown(const QList<Layer *> &layers)
{
    return std::all_of(layers.begin(), layers.end(),
                       [] (Layer *layer) { return canMoveDown(*layer); });
}

void MoveLayer::moveLayer()
{
    const auto parent = mLayer->parentLayer();
    const auto siblings = mLayer->siblings();
    const int index = mLayer->siblingIndex();

    // Common case is just to move up/down among siblings
    GroupLayer *insertionParent = parent;
    int insertionIndex = mDirection == Down ? index - 1 : index + 1;

    if (mDirection == Down) {
        if (insertionIndex < 0) {
            // Moving down when already first child means moving into parent below the group
            Q_ASSERT(insertionParent);
            insertionIndex = insertionParent->siblingIndex();
            insertionParent = insertionParent->parentLayer();
        } else if (siblings.at(insertionIndex)->isGroupLayer()) {
            // Enter the group from the top
            insertionParent = static_cast<GroupLayer*>(siblings.at(insertionIndex));
            insertionIndex = insertionParent->layerCount();
        }
    } else {
        if (insertionIndex >= siblings.size()) {
            // Moving up when already at last child means moving into parent above the group
            Q_ASSERT(insertionParent);
            insertionIndex = insertionParent->siblingIndex() + 1;
            insertionParent = insertionParent->parentLayer();
        } else if (siblings.at(insertionIndex)->isGroupLayer()) {
            // Enter the group from the bottom
            insertionParent = static_cast<GroupLayer*>(siblings.at(insertionIndex));
            insertionIndex = 0;
        }
    }

    const auto currentLayer = mMapDocument->currentLayer();
    const auto selectedLayers = mMapDocument->selectedLayers();

    LayerModel *layerModel = mMapDocument->layerModel();
    layerModel->moveLayer(parent, index, insertionParent, insertionIndex);

    // Change the direction
    mDirection = (mDirection == Down) ? Up : Down;

    // Restore current layer and selected layers, which get broken due to the
    // temporary removal of the layer from the map.
    mMapDocument->setCurrentLayer(currentLayer);
    mMapDocument->setSelectedLayers(selectedLayers);
}

} // namespace Tiled
