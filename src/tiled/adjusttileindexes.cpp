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
#include "changeproperties.h"
#include "changetileanimation.h"
#include "changetileobjectgroup.h"
#include "changetileprobability.h"
#include "changetileterrain.h"
#include "changetilewangid.h"
#include "changewangcolordata.h"
#include "changewangsetdata.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "wangset.h"

#include <QCoreApplication>

namespace Tiled {

AdjustTileIndexes::AdjustTileIndexes(MapDocument *mapDocument,
                                     const Tileset &tileset)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Adjust Tile Indexes"))
{
    int oldColumnCount = tileset.expectedColumnCount();
    int newColumnCount = tileset.columnCount();

    auto isFromTileset = [&](const Cell &cell) -> bool {
        return cell.tileset() == &tileset;
    };

    auto adjustCell = [&](Cell cell) -> Cell {
        int tileIndex = cell.tileId();
        int row = tileIndex / oldColumnCount;
        int column = tileIndex % oldColumnCount;

        if (column < newColumnCount) {
            int newTileIndex = row * newColumnCount + column;
            cell.setTile(cell.tileset(), newTileIndex);
        } else {
            cell.setTile(nullptr);
        }

        return cell;
    };

    QVector<MapObjectCell> objectChanges;

    // Adjust tile references from map layers
    LayerIterator iterator(mapDocument->map());
    while (Layer *layer = iterator.next()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            TileLayer *tileLayer = static_cast<TileLayer*>(layer);
            const QRegion region = tileLayer->region(isFromTileset).translated(-layer->position());

            if (!region.isEmpty()) {
                const QRect boundingRect(region.boundingRect());
                auto changedLayer = new TileLayer(QString(), 0, 0,
                                                  boundingRect.width(),
                                                  boundingRect.height());

#if QT_VERSION < 0x050800
                const auto rects = region.rects();
                for (const QRect &rect : rects) {
#else
                for (const QRect &rect : region) {
#endif
                    for (int x = rect.left(); x <= rect.right(); ++x) {
                        for (int y = rect.top(); y <= rect.bottom(); ++y) {
                            Cell cell = adjustCell(tileLayer->cellAt(x, y));
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
                if ((!mapObject->isTemplateInstance() || mapObject->propertyChanged(MapObject::CellProperty))
                        && isFromTileset(mapObject->cell())) {
                    MapObjectCell change;
                    change.object = mapObject;
                    change.cell = adjustCell(mapObject->cell());
                    objectChanges.append(change);
                }
            }
            break;

        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            break;
        }
    }

    if (!objectChanges.isEmpty())
        new ChangeMapObjectCells(mapDocument, objectChanges, this);
}

AdjustTileMetaData::AdjustTileMetaData(TilesetDocument *tilesetDocument)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Adjust Tile Indexes"))
{
    const Tileset &tileset = *tilesetDocument->tileset();

    int oldColumnCount = tileset.expectedColumnCount();
    int newColumnCount = tileset.columnCount();

    auto adjustTile = [&](const Tile *tile) -> Tile* {
        int tileIndex = tile->id();
        int row = tileIndex / oldColumnCount;
        int column = tileIndex % oldColumnCount;

        if (column < newColumnCount) {
            int newTileIndex = row * newColumnCount + column;
            return tileset.findTile(newTileIndex);
        }

        return nullptr;
    };

    // Adjust tile meta data
    QList<Tile*> tilesChangingProbability;
    QList<qreal> tileProbabilities;
    ChangeTileTerrain::Changes terrainChanges;
    QSet<Tile*> tilesToReset;

    auto adjustAnimationFrames = [&](const QVector<Frame> &frames) -> QVector<Frame> {
        QVector<Frame> newFrames;
        for (const Frame &frame : frames) {
            if (Tile *tile = tileset.findTile(frame.tileId)) {
                if (Tile *newTile = adjustTile(tile))
                    newFrames.append(Frame { newTile->id(), frame.duration });
            }
        }
        return newFrames;
    };

    auto applyMetaData = [&](Tile *toTile,
                             const Properties &properties,
                             unsigned terrain,
                             qreal probability,
                             std::unique_ptr<ObjectGroup> objectGroup,
                             const QVector<Frame> &frames)
    {
        if (properties != toTile->properties()) {
            new ChangeProperties(tilesetDocument,
                                 QCoreApplication::translate("MapDocument", "Tile"),
                                 toTile,
                                 properties,
                                 this);
        }

        if (terrain != toTile->terrain()) {
            terrainChanges.insert(toTile, ChangeTileTerrain::Change(toTile->terrain(),
                                                                    terrain));
        }

        if (probability != toTile->probability()) {
            tilesChangingProbability.append(toTile);
            tileProbabilities.append(probability);
        }

        if (objectGroup.get() != toTile->objectGroup())
            new ChangeTileObjectGroup(tilesetDocument, toTile, std::move(objectGroup), this);

        if (frames != toTile->frames())
            new ChangeTileAnimation(tilesetDocument, toTile, frames, this);
    };

    auto moveMetaData = [&](Tile *fromTile) {
        Tile *toTile = adjustTile(fromTile);
        if (toTile == fromTile)
            return;

        tilesToReset.insert(fromTile);  // We may still need to reset "fromTile"

        if (!toTile)
            return;

        tilesToReset.remove(toTile);    // "toTile" no longer needs to get reset

        // Copy meta data from "fromTile" to "toTile"

        std::unique_ptr<ObjectGroup> objectGroup;
        if (fromTile->objectGroup())
            objectGroup.reset(fromTile->objectGroup()->clone());

        applyMetaData(toTile,
                      fromTile->properties(),
                      fromTile->terrain(),
                      fromTile->probability(),
                      std::move(objectGroup),
                      adjustAnimationFrames(fromTile->frames()));
    };

    QMapIterator<int, Tile *> iterator{tileset.tiles()};

    if (newColumnCount > oldColumnCount) {
        // Increasing column count means information is copied to higher tiles,
        // so we need to iterate backwards.
        iterator.toBack();
        while (iterator.hasPrevious())
            moveMetaData(iterator.previous().value());
    } else {
        while (iterator.hasNext())
            moveMetaData(iterator.next().value());
    }

    // Reset meta data on tiles that nothing was copied to
    QSetIterator<Tile*> resetIterator(tilesToReset);
    while (resetIterator.hasNext()) {
        applyMetaData(resetIterator.next(),
                      Properties(), -1, 1.0, nullptr, QVector<Frame>());
    }

    // Translate tile references in Wang sets and Wang colors
    for (WangSet *wangSet : tileset.wangSets()) {
        // WangSet tile image
        if (Tile *fromTile = tileset.findTile(wangSet->imageTileId())) {
            if (Tile *newTile = adjustTile(fromTile))
                if (fromTile != newTile)
                    new SetWangSetImage(tilesetDocument, wangSet, newTile->id(), this);
        }

        // WangColor tile images
        for (const QSharedPointer<WangColor> &wangColor : wangSet->edgeColors()) {
            if (Tile *fromTile = tileset.findTile(wangColor->imageId()))
                if (Tile *newTile = adjustTile(fromTile))
                    if (fromTile != newTile)
                        new ChangeWangColorImage(tilesetDocument, wangColor.data(), newTile->id(), this);
        }
        for (const QSharedPointer<WangColor> &wangColor : wangSet->cornerColors()) {
            if (Tile *fromTile = tileset.findTile(wangColor->imageId()))
                if (Tile *newTile = adjustTile(fromTile))
                    if (fromTile != newTile)
                        new ChangeWangColorImage(tilesetDocument, wangColor.data(), newTile->id(), this);
        }

        QVector<ChangeTileWangId::WangIdChange> changes;

        // Move all WangIds to their new tiles
        for (const WangTile &wangTile : wangSet->wangTilesByWangId()) {
            if (Tile *fromTile = wangTile.tile()) {
                if (Tile *newTile = adjustTile(fromTile)) {
                    WangId fromWangId = wangSet->wangIdOfTile(newTile);
                    WangId toWangId = wangTile.wangId();
                    changes.append(ChangeTileWangId::WangIdChange(fromWangId, toWangId, newTile));
                }
            }
        }

        // Clear WangIds from other tiles
        for (const WangTile &wangTile : wangSet->wangTilesByWangId()) {
            if (Tile *fromTile = wangTile.tile()) {
                auto matchesTile = [fromTile](const ChangeTileWangId::WangIdChange &change) {
                    return change.tile == fromTile;
                };
                if (!std::any_of(changes.begin(), changes.end(), matchesTile))
                    changes.append(ChangeTileWangId::WangIdChange(wangTile.wangId(), WangId(), fromTile));
            }
        }

        if (!changes.isEmpty())
            new ChangeTileWangId(tilesetDocument, wangSet, changes, this);
    }

    if (!tilesChangingProbability.isEmpty()) {
        new ChangeTileProbability(tilesetDocument,
                                  tilesChangingProbability,
                                  tileProbabilities,
                                  this);
    }

    if (!terrainChanges.isEmpty())
        new ChangeTileTerrain(tilesetDocument, terrainChanges, this);
}

} // namespace Tiled
