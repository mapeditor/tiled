/*
 * changemapproperties.cpp
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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

#include "changemapproperties.h"

#include "map.h"
#include "mapdocument.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapProperties::ChangeMapProperties(MapDocument *mapDocument,
                                         const QColor &bgColor)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Map Properties"))
    , mMapDocument(mapDocument)
    , mUndoColor(mapDocument->map()->backgroundColor())
    , mRedoColor(bgColor)
{
}

void ChangeMapProperties::redo()
{
    mMapDocument->map()->setBackgroundColor(mRedoColor);
    mMapDocument->emitMapChanged();
}

void ChangeMapProperties::undo()
{
    mMapDocument->map()->setBackgroundColor(mUndoColor);
    mMapDocument->emitMapChanged();
}
