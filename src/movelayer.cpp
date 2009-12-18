/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "movelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled::Internal;

MoveLayer::MoveLayer(MapDocument *mapDocument, int index, Direction direction):
    mMapDocument(mapDocument),
    mIndex(index),
    mDirection(direction)
{
    setText((direction == Down) ?
            QCoreApplication::translate("Undo Commands", "Move Layer Down") :
            QCoreApplication::translate("Undo Commands", "Move Layer Up"));
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
    const int currentIndex = mMapDocument->currentLayer();
    const bool selectedBefore = (mIndex == currentIndex);
    const int prevIndex = mIndex;

    LayerModel *layerModel = mMapDocument->layerModel();
    Layer *layer = layerModel->takeLayerAt(mIndex);

    // Change the direction and index to swap undo/redo
    mIndex = (mDirection == Down) ? mIndex - 1 : mIndex + 1;
    mDirection = (mDirection == Down) ? Up : Down;

    const bool selectedAfter = (mIndex == currentIndex);

    layerModel->insertLayer(mIndex, layer);

    // Set the layer that is now supposed to be selected
    mMapDocument->setCurrentLayer(selectedBefore ? mIndex :
                                  (selectedAfter ? prevIndex : currentIndex));
}
