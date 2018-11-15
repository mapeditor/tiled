/*
 *
 * Copyright 2015, Your Name <your.name@domain>
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

#include "imagereference.h"

#include "imagecache.h"
#include "tileset.h"

namespace Tiled {

bool ImageReference::hasImage() const
{
    return !source.isEmpty() || !data.isEmpty();
}

QPixmap ImageReference::create() const
{
    if (source.isLocalFile())
        return ImageCache::loadPixmap(source.toLocalFile());
    else if (source.scheme() == QLatin1String("qrc"))
        return ImageCache::loadPixmap(QLatin1Char(':') + source.path());
    else if (!data.isEmpty())
        return QPixmap::fromImage(QImage::fromData(data, format));

    return QPixmap();
}

} // namespace Tiled
