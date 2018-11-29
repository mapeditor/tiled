/*
 * editablelayer.cpp
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

#include "editablelayer.h"

#include "changelayer.h"
#include "editablemap.h"
#include "renamelayer.h"

namespace Tiled {
namespace Internal {

EditableLayer::EditableLayer(EditableMap *map, Layer *layer, QObject *parent)
    : QObject(parent)
    , mMap(map)
    , mLayer(layer)
{
}

void EditableLayer::setName(const QString &name)
{
    mMap->push(new RenameLayer(mMap->mapDocument(), mLayer, name));
}

void EditableLayer::setOpacity(qreal opacity)
{
    mMap->push(new SetLayerOpacity(mMap->mapDocument(), mLayer, opacity));
}

void EditableLayer::setVisible(bool visible)
{
    mMap->push(new SetLayerVisible(mMap->mapDocument(), mLayer, visible));
}

void EditableLayer::setLocked(bool locked)
{
    mMap->push(new SetLayerLocked(mMap->mapDocument(), mLayer, locked));
}

void EditableLayer::setOffset(QPointF offset)
{
    mMap->push(new SetLayerOffset(mMap->mapDocument(), mLayer, offset));
}

} // namespace Internal
} // namespace Tiled
