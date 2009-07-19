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

#include "addremovelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "mapdocument.h"

namespace Tiled {
namespace Internal {

AddRemoveLayer::AddRemoveLayer(MapDocument *mapDocument,
                               int index,
                               Layer *layer)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mIndex(index)
{
}

AddRemoveLayer::~AddRemoveLayer()
{
    delete mLayer;
}

void AddRemoveLayer::addLayer()
{
    const int currentLayer = mMapDocument->currentLayer();

    mMapDocument->layerModel()->insertLayer(mIndex, mLayer);
    mLayer = 0;

    // Insertion below or at the current layer increases current layer index
    if (mIndex <= currentLayer)
        mMapDocument->setCurrentLayer(currentLayer + 1);
}

void AddRemoveLayer::removeLayer()
{
    const int currentLayer = mMapDocument->currentLayer();

    mLayer = mMapDocument->layerModel()->takeLayerAt(mIndex);

    // Removal below the current layer decreases the current layer index
    if (mIndex < currentLayer)
        mMapDocument->setCurrentLayer(currentLayer - 1);
}

} // namespace Internal
} // namespace Tiled
