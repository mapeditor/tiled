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

EditableLayer::EditableLayer(EditableMap *map, Layer *layer, QObject *parent)
    : EditableObject(map, layer, parent)
    , mLayer(layer)
{
}

EditableLayer::~EditableLayer()
{
    if (map())
        map()->mEditableLayers.remove(layer());
}

EditableMap *EditableLayer::map() const
{
    return static_cast<EditableMap*>(asset());
}

void EditableLayer::detach()
{
    Q_ASSERT(map());
    Q_ASSERT(map()->mEditableLayers.contains(layer()));

    map()->mEditableLayers.remove(mLayer);
    setAsset(nullptr);

    mDetachedLayer.reset(mLayer->clone());
    mLayer = mDetachedLayer.get();
}

void EditableLayer::attach(EditableMap *map)
{
    Q_ASSERT(!asset() && map);
    Q_ASSERT(!map->mEditableLayers.contains(layer()));

    setAsset(map);
    map->mEditableLayers.insert(layer(), this);
    mDetachedLayer.release();
}

void EditableLayer::setName(const QString &name)
{
    if (asset())
        asset()->push(new RenameLayer(mapDocument(), mLayer, name));
    else
        mLayer->setName(name);
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (asset())
        asset()->push(new SetLayerOpacity(mapDocument(), mLayer, opacity));
    else
        mLayer->setOpacity(opacity);
}

void EditableLayer::setVisible(bool visible)
{
    if (asset())
        asset()->push(new SetLayerVisible(mapDocument(), mLayer, visible));
    else
        mLayer->setVisible(visible);
}

void EditableLayer::setLocked(bool locked)
{
    if (asset())
        asset()->push(new SetLayerLocked(mapDocument(), mLayer, locked));
    else
        mLayer->setLocked(locked);
}

void EditableLayer::setOffset(QPointF offset)
{
    if (asset())
        asset()->push(new SetLayerOffset(mapDocument(), mLayer, offset));
    else
        mLayer->setOffset(offset);
}

MapDocument *EditableLayer::mapDocument() const
{
    return map() ? map()->mapDocument() : nullptr;
}

} // namespace Tiled
