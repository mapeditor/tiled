/*
 * changeselectedarea.cpp
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

#include "changeselectedarea.h"

#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeSelectedArea::ChangeSelectedArea(MapDocument *mapDocument,
                                       const QRegion &newSelection,
                                       QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Selection"),
                   parent)
    , mMapDocument(mapDocument)
    , mSelection(newSelection)
{
}

void ChangeSelectedArea::undo()
{
    swapSelection();
}

void ChangeSelectedArea::redo()
{
    swapSelection();
}

void ChangeSelectedArea::swapSelection()
{
    const QRegion oldSelection = mMapDocument->selectedArea();
    mMapDocument->setSelectedArea(mSelection);
    mSelection = oldSelection;
}
