/*
 * editabletilelayer.cpp
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

#include "editabletilelayer.h"

namespace Tiled {

EditableTileLayer::EditableTileLayer(EditableMap *map,
                                     TileLayer *layer,
                                     QObject *parent)
    : EditableLayer(map, layer, parent)
{
}

RegionValueType EditableTileLayer::region() const
{
    return RegionValueType(tileLayer()->region());
}

Cell EditableTileLayer::cellAt(int x, int y) const
{
    return tileLayer()->cellAt(x, y);
}

} // namespace Tiled
