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

EditableLayer::EditableLayer(std::unique_ptr<Layer> &&layer, QObject *parent)
    : EditableObject(nullptr, layer.get(), parent)
{
    mDetachedLayer = std::move(layer);
}

EditableLayer::EditableLayer(EditableMap *map, Layer *layer, QObject *parent)
    : EditableObject(map, layer, parent)
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

bool EditableLayer::isSelected() const
{
    if (auto document = mapDocument())
        return document->selectedLayers().contains(layer());
    return false;
}

void EditableLayer::detach()
{
    Q_ASSERT(map());
    Q_ASSERT(map()->mEditableLayers.contains(layer()));

    map()->mEditableLayers.remove(layer());
    setAsset(nullptr);

    mDetachedLayer.reset(layer()->clone());
    setObject(mDetachedLayer.get());
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
        asset()->push(new RenameLayer(mapDocument(), layer(), name));
    else
        layer()->setName(name);
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (asset())
        asset()->push(new SetLayerOpacity(mapDocument(), layer(), opacity));
    else
        layer()->setOpacity(opacity);
}

void EditableLayer::setVisible(bool visible)
{
    if (asset())
        asset()->push(new SetLayerVisible(mapDocument(), layer(), visible));
    else
        layer()->setVisible(visible);
}

void EditableLayer::setLocked(bool locked)
{
    if (asset())
        asset()->push(new SetLayerLocked(mapDocument(), layer(), locked));
    else
        layer()->setLocked(locked);
}

void EditableLayer::setOffset(QPointF offset)
{
    if (asset())
        asset()->push(new SetLayerOffset(mapDocument(), layer(), offset));
    else
        layer()->setOffset(offset);
}

void EditableLayer::setSelected(bool selected)
{
    auto document = mapDocument();
    if (!document)
        return;

    if (selected) {
        if (!document->selectedLayers().contains(layer())) {
            auto layers = document->selectedLayers();
            layers.append(layer());
            document->setSelectedLayers(layers);
        }
    } else {
        int index = document->selectedLayers().indexOf(layer());
        if (index != -1) {
            auto layers = document->selectedLayers();
            layers.removeAt(index);
            document->setSelectedLayers(layers);
        }
    }
}

MapDocument *EditableLayer::mapDocument() const
{
    return map() ? map()->mapDocument() : nullptr;
}

} // namespace Tiled
