/*
 * Tiled Map Editor (Qt)
 * Copyright 2010 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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

#include "addtileset.h"

#include "mapdocument.h"
#include "tilesetmanager.h"

#include <QCoreApplication>

using namespace Tiled::Internal;

AddTileset::AddTileset(MapDocument *mapDocument, Tileset *tileset) :
    QUndoCommand(QCoreApplication::translate("Undo Commands", "Add Tileset")),
    mMapDocument(mapDocument),
    mTileset(tileset)
{
    // Make sure the tileset manager keeps this tileset around
    TilesetManager::instance()->addReference(mTileset);
}

AddTileset::~AddTileset()
{
    TilesetManager::instance()->removeReference(mTileset);
}

void AddTileset::undo()
{
    mMapDocument->removeTileset(mTileset);
}

void AddTileset::redo()
{
    mMapDocument->addTileset(mTileset);
}
