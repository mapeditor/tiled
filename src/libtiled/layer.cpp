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

#include "imagelayer.h"
#include "objectgroup.h"
#include "tilelayer.h"


using namespace Tiled;

Layer::Layer(Type type, const QString &name, int x, int y,
             int width, int height) :
    mName(name),
    mType(type),
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height),
    mOpacity(1.0f),
    mVisible(true),
    mMap(0)
{
}

void Layer::resize(const QSize &size, const QPoint & /* offset */)
{
    mWidth = size.width();
    mHeight = size.height();
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
    clone->mOpacity = mOpacity;
    clone->mVisible = mVisible;
    clone->setProperties(properties());
    return clone;
}

TileLayer *Layer::asTileLayer()
{
    return isTileLayer() ? static_cast<TileLayer*>(this) : 0;
}

ObjectGroup *Layer::asObjectGroup()
{
    return isObjectGroup() ? static_cast<ObjectGroup*>(this) : 0;
}

ImageLayer *Layer::asImageLayer()
{
    return isImageLayer() ? static_cast<ImageLayer*>(this) : 0;
}
