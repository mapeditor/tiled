/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "tileselectionmodel.h"

#include "map.h"
#include "mapdocument.h"

using namespace Tiled;
using namespace Tiled::Internal;

TileSelectionModel::TileSelectionModel(MapDocument *mapDocument,
                                       QObject *parent)
    : QObject(parent)
    , mMapDocument(mapDocument)
{
}

void TileSelectionModel::setSelection(const QRegion &selection)
{
    if (selection != mSelection) {
        const QRegion oldSelection = mSelection;
        mSelection = selection;
        emit selectionChanged(mSelection, oldSelection);
    }
}

void TileSelectionModel::selectAll()
{
    const Map *map = mMapDocument->map();
    setSelection(QRect(0, 0, map->width(), map->height()));
}

void TileSelectionModel::selectNone()
{
    setSelection(QRegion());
}

void TileSelectionModel::addRect(const QRect &rect)
{
    setSelection(mSelection + rect);
}

void TileSelectionModel::subtractRect(const QRect &rect)
{
    setSelection(mSelection - rect);
}

void TileSelectionModel::intersectRect(const QRect &rect)
{
    setSelection(mSelection & rect);
}

void TileSelectionModel::xorRect(const QRect &rect)
{
    setSelection(mSelection ^ rect);
}
