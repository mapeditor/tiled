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
#include "editablemanager.h"
#include "editablemap.h"
#include "scriptmanager.h"

namespace Tiled {

EditableLayer::EditableLayer(std::unique_ptr<Layer> &&layer, QObject *parent)
    : EditableObject(nullptr, layer.get(), parent)
{
    mDetachedLayer = std::move(layer);
    EditableManager::instance().mEditableLayers.insert(this->layer(), this);
}

EditableLayer::EditableLayer(EditableAsset *asset, Layer *layer, QObject *parent)
    : EditableObject(asset, layer, parent)
{
}

EditableLayer::~EditableLayer()
{
    EditableManager::instance().mEditableLayers.remove(layer());
}

EditableMap *EditableLayer::map() const
{
    return asset()->isMap() ? static_cast<EditableMap*>(asset()) : nullptr;
}

bool EditableLayer::isSelected() const
{
    if (auto document = mapDocument())
        return document->selectedLayers().contains(layer());
    return false;
}

/**
 * Turns this layer reference into a stand-alone copy of the layer it was
 * referencing.
 */
void EditableLayer::detach()
{
    Q_ASSERT(asset());

    EditableManager::instance().mEditableLayers.remove(layer());
    setAsset(nullptr);

    mDetachedLayer.reset(layer()->clone());
    mDetachedLayer->resetIds();
    setObject(mDetachedLayer.get());
    EditableManager::instance().mEditableLayers.insert(layer(), this);
}

/**
 * Turns this stand-alone layer into a reference, with the layer now owned by
 * the given asset.
 */
void EditableLayer::attach(EditableAsset *asset)
{
    Q_ASSERT(!this->asset() && asset);

    setAsset(asset);
    mDetachedLayer.release();
}

/**
 * Take ownership of the referenced layer.
 */
void EditableLayer::hold()
{
    Q_ASSERT(!asset());         // if asset exists, it holds the layer (possibly indirectly)
    Q_ASSERT(!mDetachedLayer);  // can't already be holding the layer

    mDetachedLayer.reset(layer());
}

/**
 * Release ownership of the referenced layer.
 */
Layer *EditableLayer::release()
{
    Q_ASSERT(isOwning());

    return mDetachedLayer.release();
}

void EditableLayer::setName(const QString &name)
{
    if (auto doc = document())
        asset()->push(new SetLayerName(doc, layer(), name));
    else if (!checkReadOnly())
        layer()->setName(name);
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (auto doc = document())
        asset()->push(new SetLayerOpacity(doc, layer(), opacity));
    else if (!checkReadOnly())
        layer()->setOpacity(opacity);
}

void EditableLayer::setVisible(bool visible)
{
    if (auto doc = document())
        asset()->push(new SetLayerVisible(doc, layer(), visible));
    else if (!checkReadOnly())
        layer()->setVisible(visible);
}

void EditableLayer::setLocked(bool locked)
{
    if (auto doc = document())
        asset()->push(new SetLayerLocked(doc, layer(), locked));
    else if (!checkReadOnly())
        layer()->setLocked(locked);
}

void EditableLayer::setOffset(QPointF offset)
{
    if (auto doc = document())
        asset()->push(new SetLayerOffset(doc, layer(), offset));
    else if (!checkReadOnly())
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
            document->switchSelectedLayers(layers);
        }
    } else {
        int index = document->selectedLayers().indexOf(layer());
        if (index != -1) {
            auto layers = document->selectedLayers();
            layers.removeAt(index);
            document->switchSelectedLayers(layers);
        }
    }
}

MapDocument *EditableLayer::mapDocument() const
{
    return map() ? map()->mapDocument() : nullptr;
}

} // namespace Tiled
