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
#include "editablegrouplayer.h"
#include "editableimagelayer.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "editabletilelayer.h"

namespace Tiled {

EditableLayer::EditableLayer(std::unique_ptr<Layer> layer, QObject *parent)
    : EditableObject(nullptr, layer.get(), parent)
{
    mDetachedLayer = std::move(layer);
}

EditableLayer::EditableLayer(EditableAsset *asset, Layer *layer, QObject *parent)
    : EditableObject(asset, layer, parent)
{
}

EditableLayer::~EditableLayer()
{
    // Prevent owned object from trying to delete us again
    if (mDetachedLayer)
        setObject(nullptr);
}

EditableMap *EditableLayer::map() const
{
    return (asset() && asset()->isMap()) ? static_cast<EditableMap*>(asset())
                                         : nullptr;
}

EditableGroupLayer *EditableLayer::parentLayer() const
{
    GroupLayer *parent = layer()->parentLayer();
    return static_cast<EditableGroupLayer*>(EditableLayer::get(map(), parent));
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
    setAsset(nullptr);

    if (!moveOwnershipToJavaScript())
        return;

    mDetachedLayer.reset(layer()->clone());
//    mDetachedLayer->resetIds();
    setObject(mDetachedLayer.get());
}

/**
 * Turns this stand-alone layer into a reference, with the layer now owned by
 * a map, group layer or tile (in case of object group).
 *
 * The given \a asset may be a nullptr in case the layer is added to a group
 * layer which isn't part of a map.
 *
 * Returns nullptr if the editable wasn't owning its layer.
 */
Layer *EditableLayer::attach(EditableAsset *asset)
{
    Q_ASSERT(!this->asset());

    setAsset(asset);
    moveOwnershipToCpp();
    return mDetachedLayer.release();
}

/**
 * Take ownership of the referenced layer or delete it.
 */
void EditableLayer::hold(std::unique_ptr<Layer> layer)
{
    Q_ASSERT(!mDetachedLayer);  // can't already be holding the layer
    Q_ASSERT(this->layer() == layer.get());

    if (!moveOwnershipToJavaScript())
        return;

    setAsset(nullptr);
    mDetachedLayer = std::move(layer);
}

EditableLayer *EditableLayer::get(EditableMap *map, Layer *layer)
{
    if (!layer)
        return nullptr;

    auto editable = find(layer);
    if (editable)
        return editable;

    Q_ASSERT(!map || layer->map() == map->map());

    switch (layer->layerType()) {
    case Layer::TileLayerType:
        editable = new EditableTileLayer(map, static_cast<TileLayer*>(layer));
        break;
    case Layer::ObjectGroupType:
        editable = new EditableObjectGroup(map, static_cast<ObjectGroup*>(layer));
        break;
    case Layer::ImageLayerType:
        editable = new EditableImageLayer(map, static_cast<ImageLayer*>(layer));
        break;
    case Layer::GroupLayerType:
        editable = new EditableGroupLayer(map, static_cast<GroupLayer*>(layer));
        break;
    }

    editable->moveOwnershipToCpp();

    return editable;
}

void EditableLayer::release(Layer *layer)
{
    std::unique_ptr<Layer> owned { layer };
    if (auto editable = EditableLayer::find(layer))
        editable->hold(std::move(owned));
}

void EditableLayer::setName(const QString &name)
{
    if (auto doc = document())
        asset()->push(new SetLayerName(doc, { layer() }, name));
    else if (!checkReadOnly())
        layer()->setName(name);
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (auto doc = document())
        asset()->push(new SetLayerOpacity(doc, { layer() }, opacity));
    else if (!checkReadOnly())
        layer()->setOpacity(opacity);
}

void EditableLayer::setTintColor(const QColor &color)
{
    if (auto doc = document())
        asset()->push(new SetLayerTintColor(doc, { layer() }, color));
    else if (!checkReadOnly())
        layer()->setTintColor(color);
}

void EditableLayer::setVisible(bool visible)
{
    if (auto doc = document())
        asset()->push(new SetLayerVisible(doc, { layer() }, visible));
    else if (!checkReadOnly())
        layer()->setVisible(visible);
}

void EditableLayer::setLocked(bool locked)
{
    if (auto doc = document())
        asset()->push(new SetLayerLocked(doc, { layer() }, locked));
    else if (!checkReadOnly())
        layer()->setLocked(locked);
}

void EditableLayer::setOffset(QPointF offset)
{
    if (auto doc = document())
        asset()->push(new SetLayerOffset(doc, { layer() }, offset));
    else if (!checkReadOnly())
        layer()->setOffset(offset);
}

void EditableLayer::setParallaxFactor(QPointF factor)
{
    if (auto doc = document())
        asset()->push(new SetLayerParallaxFactor(doc, { layer() }, factor));
    else if (!checkReadOnly())
        layer()->setParallaxFactor(factor);
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

#include "moc_editablelayer.cpp"
