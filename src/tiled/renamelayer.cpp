/*
 * renamelayer.cpp
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

#include "renamelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

RenameLayer::RenameLayer(MapDocument *mapDocument,
                         int layerIndex,
                         const QString &name):
    mMapDocument(mapDocument),
    mLayerIndex(layerIndex),
    mName(name)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Layer"));
}

void RenameLayer::undo()
{
    swapName();
}

void RenameLayer::redo()
{
    swapName();
}

void RenameLayer::swapName()
{
    const Layer *layer = mMapDocument->map()->layerAt(mLayerIndex);
    const QString previousName = layer->name();
    mMapDocument->layerModel()->renameLayer(mLayerIndex, mName);
    mName = previousName;
}
