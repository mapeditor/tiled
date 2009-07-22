/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "layer.h"

using namespace Tiled;

Layer::Layer(const QString &name, int x, int y, int width, int height):
    mName(name),
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
    clone->mMap = mMap;
    clone->mProperties = mProperties;
    return clone;
}
