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

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ResizeMap::ResizeMap(MapDocument *mapDocument, const QSize &size)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Resize Map"))
    , mMapDocument(mapDocument)
    , mSize(size)
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
    QSize oldSize(map->width(), map->height());
    map->setWidth(mSize.width());
    map->setHeight(mSize.height());
    mSize = oldSize;

    mMapDocument->emitMapChanged();
}

} // namespace Internal
} // namespace Tiled
