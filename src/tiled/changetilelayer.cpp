/*
 * changetilelayer.cpp
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changetilelayer.h"

#include "mapdocument.h"
#include "tilelayer.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

SetLayerHorizontalOffset::SetLayerHorizontalOffset(MapDocument* mapDocument,
                                                   TileLayer* layer,
                                                   int offset)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mOldHorizontalOffset(layer->horizontalOffset())
    , mNewHorizontalOffset(offset)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Layer Horizontal Offset"));
}

void SetLayerHorizontalOffset::setOffset(int offset)
{
    mLayer->setHorizontalOffset(offset);
    mMapDocument->emitMapChanged();
}

SetLayerVerticalOffset::SetLayerVerticalOffset(MapDocument* mapDocument,
                                               TileLayer* layer,
                                               int offset)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mOldVerticalOffset(layer->verticalOffset())
    , mNewVerticalOffset(offset)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Layer Vertical Offset"));
}

void SetLayerVerticalOffset::setOffset(int offset)
{
    mLayer->setVerticalOffset(offset);
    mMapDocument->emitMapChanged();
}

