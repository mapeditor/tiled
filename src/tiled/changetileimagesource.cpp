/*
 * changetileimagesource.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changetileimagesource.h"

#include "mapdocument.h"
#include "tile.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ChangeTileImageSource::ChangeTileImageSource(MapDocument *mapDocument,
                                             Tile *tile,
                                             const QString &imageSource)
    : mMapDocument(mapDocument)
    , mTile(tile)
    , mOldImageSource(tile->imageSource())
    , mNewImageSource(imageSource)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Image"));
}

void ChangeTileImageSource::apply(const QString &imageSource)
{
    mTile->tileset()->setTileImage(mTile,
                                   QPixmap(imageSource),
                                   imageSource);

    emit mMapDocument->tileImageSourceChanged(mTile);
}

} // namespace Internal
} // namespace Tiled
