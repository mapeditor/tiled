/*
 * changetileselection.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changetileselection.h"

#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled::Internal;

ChangeTileSelection::ChangeTileSelection(MapDocument *mapDocument,
                                         const QRegion &newSelection)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Selection"))
    , mMapDocument(mapDocument)
    , mSelection(newSelection)
{
}

void ChangeTileSelection::undo()
{
    swapSelection();
}

void ChangeTileSelection::redo()
{
    swapSelection();
}

void ChangeTileSelection::swapSelection()
{
    const QRegion oldSelection = mMapDocument->tileSelection();
    mMapDocument->setTileSelection(mSelection);
    mSelection = oldSelection;
}
