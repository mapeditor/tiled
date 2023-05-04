/*
 * swaptiles.cpp
 * Copyright 2015, Alexander "theHacker" Münch <git@thehacker.biz>
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

#include "swaptiles.h"

#include "changeevents.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tilelayer.h"

#include <QCoreApplication>

namespace Tiled {

SwapTiles::SwapTiles(MapDocument *mapDocument,
                     Tile *tile1,
                     Tile *tile2)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Swap Tiles"))
    , mMapDocument(mapDocument)
    , mTile1(tile1)
    , mTile2(tile2)
{}

void SwapTiles::swap()
{
    Tile * const tile1 = mTile1;
    Tile * const tile2 = mTile2;

    const bool tileSizeChanged = tile1->size() != tile2->size();

    QList<MapObject*> changedObjects;

    auto isTile1 = [=](const Cell &cell) { return cell.refersTile(tile1); };
    auto isTile2 = [=](const Cell &cell) { return cell.refersTile(tile2); };

    auto swapObjectTile = [=,&changedObjects](MapObject *object, Tile *fromTile, Tile *toTile) {
        Cell cell = object->cell();
        cell.setTile(toTile);
        object->setCell(cell);
        if (tileSizeChanged && object->size() == fromTile->size())
            object->setSize(toTile->size());
        changedObjects.append(object);
    };

    LayerIterator it(mMapDocument->map());
    while (Layer *layer = it.next()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            auto tileLayer = static_cast<TileLayer*>(layer);
            auto region1 = tileLayer->region(isTile1);
            auto region2 = tileLayer->region(isTile2);

            tileLayer->setTiles(region1, tile2);
            tileLayer->setTiles(region2, tile1);

            emit mMapDocument->regionChanged(region1 | region2, tileLayer);

            break;
        }
        case Layer::ObjectGroupType: {
            for (MapObject *object : *static_cast<ObjectGroup*>(layer)) {
                if (object->cell().refersTile(tile1))
                    swapObjectTile(object, tile1, tile2);
                else if (object->cell().refersTile(tile2))
                    swapObjectTile(object, tile2, tile1);
            }
            break;
        }
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            break;
        }
    }

    if (!changedObjects.isEmpty()) {
        MapObject::ChangedProperties changedProperties = MapObject::CellProperty;
        if (tileSizeChanged)
            changedProperties |= MapObject::SizeProperty;

        emit mMapDocument->changed(MapObjectsChangeEvent(changedObjects, changedProperties));
    }
}

} // namespace Tiled
