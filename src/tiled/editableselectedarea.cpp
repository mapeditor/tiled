/*
 * editableselectedarea.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableselectedarea.h"

#include "changeselectedarea.h"
#include "mapdocument.h"

#include <QUndoStack>

namespace Tiled {

EditableSelectedArea::EditableSelectedArea(MapDocument *mapDocument, QObject *parent)
    : QObject(parent)
    , mMapDocument(mapDocument)
{
}

RegionValueType EditableSelectedArea::get() const
{
    return RegionValueType(mMapDocument->selectedArea());
}

void EditableSelectedArea::set(const QRect &rect)
{
    set(QRegion(rect));
}

void EditableSelectedArea::set(const RegionValueType &region)
{
    set(region.region());
}

void EditableSelectedArea::add(const QRect &rect)
{
    set(mMapDocument->selectedArea().united(rect));
}

void EditableSelectedArea::add(const RegionValueType &region)
{
    set(mMapDocument->selectedArea().united(region.region()));
}

void EditableSelectedArea::subtract(const QRect &rect)
{
    set(mMapDocument->selectedArea().subtracted(rect));
}

void EditableSelectedArea::subtract(const RegionValueType &region)
{
    set(mMapDocument->selectedArea().subtracted(region.region()));
}

void EditableSelectedArea::intersect(const QRect &rect)
{
    set(mMapDocument->selectedArea().intersected(rect));
}

void EditableSelectedArea::intersect(const RegionValueType &region)
{
    set(mMapDocument->selectedArea().intersected(region.region()));
}

void EditableSelectedArea::set(const QRegion &region)
{
    if (mMapDocument->selectedArea() != region) {
        auto undoStack = mMapDocument->undoStack();
        undoStack->push(new ChangeSelectedArea(mMapDocument, region));
    }
}

} // namespace Tiled
