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
    const int currentIndex = mMapDocument->currentLayerIndex();
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
    mMapDocument->setCurrentLayerIndex(
                selectedBefore ? mIndex :
                                 (selectedAfter ? prevIndex : currentIndex));
}
