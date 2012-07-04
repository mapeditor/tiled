/*
 * preferences.cpp
 * Copyright 2012, devnewton <devnewton@bci.Im>
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

#include "pixmapcache.h"
#include <memory>

using namespace Tiled;

PixmapCache *PixmapCache::mInstance = 0;

PixmapCache *PixmapCache::instance()
{
    if (!mInstance)
        mInstance = new PixmapCache;
    return mInstance;
}

void PixmapCache::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

PixmapCache::PixmapCache()
{
    mCache.setMaxCost(64 * 1024 * 1024);
}

PixmapCache::~PixmapCache()
{
}

QPixmap PixmapCache::pixmap(const QString& source)
{
    QPixmap result;
    QPixmap* pixmap = mCache.object(source);
    if(pixmap)
        result = *pixmap;
    else
    {
        result.load(source);
        mCache.insert(source, new QPixmap(result), result.width() * result.height() * result.depth());
    }
    return result;
}

int PixmapCache::cacheLimit() const
{
    return mCache.maxCost();
}
void PixmapCache::setCacheLimit(int limit)
{
    mCache.setMaxCost(limit);
}
