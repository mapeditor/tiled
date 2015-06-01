/*
 * tilestamp.cpp
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

#include "tilestamp.h"

#include "tilesetmanager.h"

namespace Tiled {
namespace Internal {

TileStamp::TileStamp()
    : mQuickStampIndex(-1)
{
}

TileStamp::~TileStamp()
{
    TilesetManager *tilesetManager = TilesetManager::instance();

    // Decrease reference to tilesets and delete maps
    foreach (const TileStampVariation &variation, mVariations) {
        tilesetManager->removeReferences(variation.map->tilesets());
        delete variation.map;
    }
}

void TileStamp::addVariation(Map *map, qreal probability)
{
    mVariations.append(TileStampVariation(map, probability));
}

Map *TileStamp::takeVariation(int index)
{
    return mVariations.takeAt(index).map;
}

Map *TileStamp::randomVariation() const
{
    if (mVariations.isEmpty())
        return 0;

    return mVariations.at(rand() % mVariations.size()).map; // todo: take probability into account
}

} // namespace Internal
} // namespace Tiled
