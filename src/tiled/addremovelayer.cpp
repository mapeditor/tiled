/*
 * addremovelayer.cpp
 * Copyright 2009-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "mapdocument.h"

namespace Tiled {
namespace Internal {

AddRemoveLayer::AddRemoveLayer(MapDocument *mapDocument,
                               int index,
                               Layer *layer,
                               GroupLayer *parentLayer)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mParentLayer(parentLayer)
    , mIndex(index)
{
}

AddRemoveLayer::~AddRemoveLayer()
{
    delete mLayer;
}

void AddRemoveLayer::addLayer()
{
    mMapDocument->layerModel()->insertLayer(mParentLayer, mIndex, mLayer);
    mLayer = nullptr;
}

void AddRemoveLayer::removeLayer()
{
    mLayer = mMapDocument->layerModel()->takeLayerAt(mParentLayer, mIndex);
}

} // namespace Internal
} // namespace Tiled
