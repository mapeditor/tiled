/*
 * addremovetileset.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovetileset.h"

#include "map.h"
#include "mapdocument.h"

using namespace Tiled;

AddRemoveTileset::AddRemoveTileset(MapDocument *mapDocument,
                                   int index,
                                   const SharedTileset &tileset,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mTileset(tileset)
    , mIndex(index)
{
}

AddRemoveTileset::~AddRemoveTileset()
{
}

void AddRemoveTileset::removeTileset()
{
    mMapDocument->removeTilesetAt(mIndex);
}

void AddRemoveTileset::addTileset()
{
    mMapDocument->insertTileset(mIndex, mTileset);
}


AddTileset::AddTileset(MapDocument *mapDocument, const SharedTileset &tileset,
                       QUndoCommand *parent)
    : AddRemoveTileset(mapDocument,
                       mapDocument->map()->tilesets().size(),
                       tileset,
                       parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Tileset"));
}

AddTileset *AddTileset::clone(QUndoCommand *parent) const
{
    auto c = new AddTileset(mMapDocument, mTileset, parent);
    c->mIndex = mIndex;
    return c;
}


RemoveTileset::RemoveTileset(MapDocument *mapDocument, int index)
    : AddRemoveTileset(mapDocument,
                       index,
                       mapDocument->map()->tilesetAt(index))
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Remove Tileset"));
}


void Tiled::addMissingTilesets(Document *document,
                               const QSet<SharedTileset> &tilesets,
                               QUndoCommand *parent)
{
    if (document->type() != Document::MapDocumentType)
        return;

    auto mapDocument = static_cast<MapDocument*>(document);
    const auto &mapTilesets = mapDocument->map()->tilesets();

    for (const SharedTileset &tileset : tilesets)
        if (!mapTilesets.contains(tileset))
            new AddTileset(mapDocument, tileset, parent);
}
