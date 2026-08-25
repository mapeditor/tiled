/*
 * resizemap.cpp
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

#include "resizemap.h"

#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"

#include <QCoreApplication>

namespace Tiled {

ResizeMap::ResizeMap(MapDocument *mapDocument,
                     QSize size,
                     QPoint offset,
                     QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Resize Map"),
                   parent)
    , mMapDocument(mapDocument)
    , mSize(size)
    , mOffset(offset)
{
}

void ResizeMap::undo()
{
    swapSize();
}

void ResizeMap::redo()
{
    swapSize();
}

void ResizeMap::swapSize()
{
    Map *map = mMapDocument->map();
    const MapRenderer *renderer = mMapDocument->renderer();

    const QSize oldSize = map->size();

    // Measure the bounding rect rather than the contents, since on hexagonal
    // and staggered maps the contents jump half a tile whenever the stagger
    // parity changes. Taken before the resize, because the isometric origin
    // follows the map height.
    const QRect oldBounds = renderer->boundingRect(QRect(QPoint(), oldSize));
    const QRect newBounds = renderer->boundingRect(QRect(-mOffset, mSize));
    const QPoint boundsOffset = newBounds.topLeft() - oldBounds.topLeft();

    map->setWidth(mSize.width());
    map->setHeight(mSize.height());
    mSize = oldSize;

    // The view goes the other way, so the map stays where it was on screen.
    emit mMapDocument->mapResized(-boundsOffset);

    // Shift the other way when this command is applied again, so that undo
    // and redo each restore the previous state.
    mOffset = -mOffset;
}

} // namespace Tiled
