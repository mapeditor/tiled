/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#include "tileset.h"

#include <QDebug>

using namespace Tiled;

QPixmap Tileset::tileImageAt(int id) const
{
    return mTiles.at(id);
}

bool Tileset::loadFromImage(const QString &fileName)
{
    Q_ASSERT(mTileWidth > 0 && mTileHeight > 0);
    mTiles.clear();

    QPixmap img = QPixmap(fileName);
    if (img.isNull())
        return false;

    const int imgWidth = img.width();
    const int imgHeight = img.height();

    qDebug() << "Loaded image" << fileName << imgWidth << imgHeight;

    for (int y = 0; y + mTileHeight <= imgHeight; y += mTileHeight + mSpacing)
        for (int x = 0; x + mTileWidth <= imgWidth; x += mTileWidth + mSpacing)
            mTiles.append(img.copy(x, y, mTileWidth, mTileHeight));

    mSource = fileName;
    return true;
}
