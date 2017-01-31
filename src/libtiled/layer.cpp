/*
 * layer.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "layer.h"

#include "grouplayer.h"
#include "imagelayer.h"
#include "map.h"
#include "objectgroup.h"
#include "tilelayer.h"

using namespace Tiled;

Layer::Layer(TypeFlag type, const QString &name, int x, int y) :
    Object(LayerType),
    mName(name),
    mLayerType(type),
    mX(x),
    mY(y),
    mOpacity(1.0f),
    mVisible(true),
    mMap(nullptr),
    mParentLayer(nullptr)
{
}

/**
 * Returns the depth of this layer in the hierarchy.
 */
int Layer::depth() const
{
    int d = 0;
    GroupLayer *p = mParentLayer;
    while (p) {
        ++d;
        p = p->parentLayer();
    }
    return d;
}

/**
 * Returns the index of this layer among its siblings.
 */
int Layer::siblingIndex() const
{
    if (mParentLayer)
        return mParentLayer->layers().indexOf(const_cast<Layer*>(this));
    if (mMap)
        return mMap->layers().indexOf(const_cast<Layer*>(this));
    return 0;
}

/**
 * Returns the list of siblings of this layer, including this layer.
 */
QList<Layer *> Layer::siblings() const
{
    if (mParentLayer)
        return mParentLayer->layers();
    if (mMap)
        return mMap->layers();

    return QList<Layer *>();
}

/**
 * A helper function for initializing the members of the given instance to
 * those of this layer. Used by subclasses when cloning.
 *
 * Layer name, position and size are not cloned, since they are assumed to have
 * already been passed to the constructor. Also, map ownership is not cloned,
 * since the clone is not added to the map.
 *
 * \return the initialized clone (the same instance that was passed in)
 * \sa clone()
 */
Layer *Layer::initializeClone(Layer *clone) const
{
    clone->mOffset = mOffset;
    clone->mOpacity = mOpacity;
    clone->mVisible = mVisible;
    clone->setProperties(properties());
    return clone;
}

TileLayer *Layer::asTileLayer()
{
    return isTileLayer() ? static_cast<TileLayer*>(this) : nullptr;
}

ObjectGroup *Layer::asObjectGroup()
{
    return isObjectGroup() ? static_cast<ObjectGroup*>(this) : nullptr;
}

ImageLayer *Layer::asImageLayer()
{
    return isImageLayer() ? static_cast<ImageLayer*>(this) : nullptr;
}

GroupLayer *Layer::asGroupLayer()
{
    return isGroupLayer() ? static_cast<GroupLayer*>(this) : nullptr;
}


Layer *LayerIterator::next()
{
    // Traverse to parent layer if last child
    if (mCurrentLayer && mSiblingIndex == mCurrentLayer->siblings().size() - 1) {
        mCurrentLayer = mCurrentLayer->parentLayer();
        mSiblingIndex = mCurrentLayer ? mCurrentLayer->siblingIndex() : -1;
        return mCurrentLayer;
    }

    if (!mCurrentLayer) {
        // Traverse to the first layer of the map
        if (mMap && mSiblingIndex == -1 && mMap->layerCount() > 0) {
            mCurrentLayer = mMap->layerAt(0);
            mSiblingIndex = 0;
            return mCurrentLayer;
        }
        return nullptr;
    }

    // Traverse to next sibling
    const auto siblings = mCurrentLayer->siblings();
    int index = mSiblingIndex + 1;
    Layer *layer = siblings.at(index);

    // If next layer is a group, traverse to its first child
    while (layer->isGroupLayer()) {
        auto groupLayer = static_cast<GroupLayer*>(layer);
        if (groupLayer->layerCount() > 0) {
            index = 0;
            layer = groupLayer->layerAt(0);
        } else {
            break;
        }
    }

    mCurrentLayer = layer;
    mSiblingIndex = index;

    return layer;
}

Layer *LayerIterator::previous()
{
    Layer *layer = mCurrentLayer;
    int index = mSiblingIndex - 1;

    if (!layer) {
        // Traverse to the last layer of the map if at the end
        if (mMap && index < mMap->layerCount() && mMap->layerCount() > 0) {
            layer = mMap->layerAt(index);
        } else {
            return nullptr;
        }
    } else {
        // Traverse down to last child if applicable
        if (GroupLayer *groupLayer = layer->asGroupLayer()) {
            if (groupLayer->layerCount() > 0) {
                mSiblingIndex = groupLayer->layerCount() - 1;
                mCurrentLayer = groupLayer->layerAt(mSiblingIndex);
                return mCurrentLayer;
            }
        }

        // Traverse to previous sibling (possibly of a parent)
        do {
            if (index >= 0) {
                const auto siblings = layer->siblings();
                layer = siblings.at(index);
                break;
            }

            layer = layer->parentLayer();
            if (layer)
                index = layer->siblingIndex() - 1;
        } while (layer);
    }

    mCurrentLayer = layer;
    mSiblingIndex = index;

    return layer;
}
