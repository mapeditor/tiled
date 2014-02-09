/*
 * changeimagelayerposition.cpp
 * Copyright 2014, Michael Aquilina <michaelaquilina@gmail.com>
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

#include "changeimagelayerposition.h"

#include "mapdocument.h"
#include "imagelayer.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeImageLayerPosition::ChangeImageLayerPosition(
        MapDocument *mapDocument,
        ImageLayer *imageLayer,
        const QPoint &newPos)
    : QUndoCommand(
          QCoreApplication::translate(
              "Undo Commands", "Change Image Layer Position"))
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mUndoPos(imageLayer->position())
    , mRedoPos(newPos)
{
}

void ChangeImageLayerPosition::redo()
{
    mImageLayer->setPosition(mRedoPos);
    mMapDocument->emitImageLayerChanged(mImageLayer);
}

void ChangeImageLayerPosition::undo()
{
    mImageLayer->setPosition(mUndoPos);
    mMapDocument->emitImageLayerChanged(mImageLayer);
}
