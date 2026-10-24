/*
 * flipmapobjects.cpp
 * Copyright 2013-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Klimov Viktor <vitek.fomino@bk.ru>
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

#include "flipmapobjects.h"

#include "addremovetileset.h"
#include "changeevents.h"
#include "document.h"

#include <QCoreApplication>

using namespace Tiled;

static constexpr MapObject::ChangedProperties propertiesChangedByFlip {
    MapObject::CellProperty,
    MapObject::PositionProperty,
    MapObject::RotationProperty,
    MapObject::ShapeProperty,
};

FlipMapObjects::FlipMapObjects(Document *document,
                               const QList<MapObject *> &mapObjects,
                               FlipDirection flipDirection,
                               QPointF flipOrigin)
    : mDocument(document)
    , mMapObjects(mapObjects)
    , mFlipDirection(flipDirection)
    , mFlipOrigin(flipOrigin)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Flip %n Object(s)",
                                        nullptr, mapObjects.size()));

    mOldChangedProperties.reserve(mMapObjects.size());
    mNewChangedProperties.reserve(mMapObjects.size());

    QSet<SharedTileset> tilesets;

    for (const MapObject *object : mMapObjects) {
        mOldChangedProperties.append(object->changedProperties());
        mNewChangedProperties.append(object->changedProperties() | propertiesChangedByFlip);

        if (Tileset *tileset = object->cell().tileset())
            tilesets.insert(tileset->sharedFromThis());
    }

    // Flipping a tile object makes its tile an explicit reference (for
    // example when it is a template instance), so make sure the map can
    // refer to the tileset.
    addMissingTilesets(document, tilesets, this);
}

void FlipMapObjects::undo()
{
    flip();
    QUndoCommand::undo(); // undo child commands
}

void FlipMapObjects::redo()
{
    QUndoCommand::redo(); // redo child commands
    flip();
}

void FlipMapObjects::flip()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        mMapObjects[i]->flip(mFlipDirection, mFlipOrigin);
        mMapObjects[i]->setChangedProperties(mNewChangedProperties[i]);
    }

    mOldChangedProperties.swap(mNewChangedProperties);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, propertiesChangedByFlip));
}
