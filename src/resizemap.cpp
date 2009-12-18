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
