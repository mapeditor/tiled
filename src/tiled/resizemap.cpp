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

    // Determine by how much the contents shift on screen, by sampling around
    // the resize. Sampling both sides is necessary because for isometric maps
    // the screen origin depends on the map height.
    const QPointF beforeResize = renderer->tileToScreenCoords(QPointF());

    QSize oldSize(map->width(), map->height());
    map->setWidth(mSize.width());
    map->setHeight(mSize.height());
    mSize = oldSize;

    // For staggered and hexagonal maps this is only exact when the offset
    // keeps the stagger parity, since there the shift depends on where the
    // contents are.
    const QPointF afterResize = renderer->tileToScreenCoords(mOffset);

    emit mMapDocument->mapResized(afterResize - beforeResize);

    // Shift the other way when this command is applied again, so that undo
    // and redo each restore the previous state.
    mOffset = -mOffset;
}

} // namespace Tiled
