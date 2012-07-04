/*
 * pixmapcache.h
 * Copyright 2012, devnewton <devnewton@bci.im>
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

#ifndef PIXMAPCACHE_H
#define PIXMAPCACHE_H

#include "tiled_global.h"

#include <QCache>
#include <QPixmap>

namespace Tiled {

/**
 * Provides a simple pixmap cache for Tiled
 */
class TILEDSHARED_EXPORT PixmapCache
{

public:
    static PixmapCache *instance();
    static void deleteInstance();

    QPixmap pixmap(const QString& source);

    int cacheLimit() const;
    void setCacheLimit(int limit);

private:
    PixmapCache();
    ~PixmapCache();

    QCache< QString, QPixmap > mCache;

    static PixmapCache *mInstance;
};

} // namespace Tiled

#endif // PIXMAPCACHE_H
