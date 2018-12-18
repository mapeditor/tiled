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
#include "scriptmanager.h"

namespace Tiled {
namespace Internal {

EditableLayer::EditableLayer(EditableMap *map, Layer *layer, QObject *parent)
    : QObject(parent)
    , mMap(map)
    , mLayer(layer)
{
}

EditableLayer::~EditableLayer()
{
    if (mMap)
        mMap->editableLayerDeleted(this);
}

void EditableLayer::detach()
{
    Q_ASSERT(mMap);

    mMap = nullptr;
    mDetachedLayer.reset(mLayer->clone());
    mLayer = mDetachedLayer.get();
}

void EditableLayer::attach(EditableMap *map)
{
    Q_ASSERT(!mMap && map);

    mMap = map;
    mDetachedLayer.release();
}

void EditableLayer::setName(const QString &name)
{
    if (mMap)
        mMap->push(new RenameLayer(mMap->mapDocument(), mLayer, name));
    else
        mLayer->setName(name);
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (mMap)
        mMap->push(new SetLayerOpacity(mMap->mapDocument(), mLayer, opacity));
    else
        mLayer->setOpacity(opacity);
}

void EditableLayer::setVisible(bool visible)
{
    if (mMap)
        mMap->push(new SetLayerVisible(mMap->mapDocument(), mLayer, visible));
    else
        mLayer->setVisible(visible);
}

void EditableLayer::setLocked(bool locked)
{
    if (mMap)
        mMap->push(new SetLayerLocked(mMap->mapDocument(), mLayer, locked));
    else
        mLayer->setLocked(locked);
}

void EditableLayer::setOffset(QPointF offset)
{
    if (mMap)
        mMap->push(new SetLayerOffset(mMap->mapDocument(), mLayer, offset));
    else
        mLayer->setOffset(offset);
}

} // namespace Internal
} // namespace Tiled
