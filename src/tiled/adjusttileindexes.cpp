/*
 * adjusttileindexes.cpp
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

#include "adjusttileindexes.h"

#include "changemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

AdjustTileIndexes::AdjustTileIndexes(MapDocument *mapDocument,
                                     Tileset *tileset)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Adjust Tile Indexes"))
{
    int oldColumnCount = tileset->expectedColumnCount();
    int newColumnCount = tileset->columnCount();

    auto isFromTileset = [=](const Cell &cell) -> bool {
        return cell.tile && cell.tile->tileset() == tileset;
    };

    auto adjustTile = [=](Tile *tile) -> Tile* {
        int tileIndex = tile->id();
        int row = tileIndex / oldColumnCount;
        int column = tileIndex % oldColumnCount;
        int newTileIndex = row * newColumnCount + column;

        return tileset->findTile(newTileIndex);
    };

    QVector<MapObjectChange> objectChanges;

    for (Layer *layer : mapDocument->map()->layers()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            TileLayer *tileLayer = static_cast<TileLayer*>(layer);
            QRegion region = tileLayer->region(isFromTileset).translated(-layer->position());

            if (!region.isEmpty()) {
                const QRect boundingRect(region.boundingRect());
                auto changedLayer = new TileLayer(QString(), 0, 0,
                                                  boundingRect.width(),
                                                  boundingRect.height());

                for (const QRect &rect : region.rects()) {
                    for (int x = rect.left(); x <= rect.right(); ++x) {
                        for (int y = rect.top(); y <= rect.bottom(); ++y) {
                            Cell cell = tileLayer->cellAt(x, y);
                            cell.tile = adjustTile(cell.tile);

                            changedLayer->setCell(x - boundingRect.x(),
                                                  y - boundingRect.y(),
                                                  cell);
                        }
                    }
                }

                new PaintTileLayer(mapDocument, tileLayer,
                                   boundingRect.x() + tileLayer->x(),
                                   boundingRect.y() + tileLayer->y(),
                                   changedLayer,
                                   this);

                delete changedLayer;
            }

            break;
        }

        case Layer::ObjectGroupType:
            for (MapObject *mapObject : *static_cast<ObjectGroup*>(layer)) {
                if (isFromTileset(mapObject->cell())) {
                    MapObjectChange change;
                    change.object = mapObject;
                    change.tile = adjustTile(mapObject->cell().tile);;
                    objectChanges.append(change);
                }
            }
            break;

        case Layer::ImageLayerType:
            break;
        }
    }

    if (!objectChanges.isEmpty()) {
        new ChangeMapObjects(mapDocument,
                             objectChanges,
                             ChangeMapObjects::ChangeTile,
                             this);
    }
}

} // namespace Internal
} // namespace Tiled
