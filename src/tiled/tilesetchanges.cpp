/*
 * tilesetchanges.cpp
 * Copyright 2011, Maus <Zeitmaus@github>
 * Copyright 2011-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilesetchanges.h"

#include "mapdocument.h"
#include "tileset.h"
#include "tilesetmanager.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

RenameTileset::RenameTileset(MapDocument *mapDocument,
                             Tileset *tileset,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Tileset Name"))
    , mMapDocument(mapDocument)
    , mTileset(tileset)
    , mOldName(tileset->name())
    , mNewName(newName)
{}

void RenameTileset::undo()
{
    mMapDocument->setTilesetName(mTileset, mOldName);
}

void RenameTileset::redo()
{
    mMapDocument->setTilesetName(mTileset, mNewName);
}


ChangeTilesetTileOffset::ChangeTilesetTileOffset(MapDocument *mapDocument,
                                                 Tileset *tileset,
                                                 QPoint tileOffset)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Drawing Offset"))
    , mMapDocument(mapDocument)
    , mTileset(tileset)
    , mOldTileOffset(tileset->tileOffset())
    , mNewTileOffset(tileOffset)
{}

void ChangeTilesetTileOffset::undo()
{
    mMapDocument->setTilesetTileOffset(mTileset, mOldTileOffset);
}

void ChangeTilesetTileOffset::redo()
{
    mMapDocument->setTilesetTileOffset(mTileset, mNewTileOffset);
}

bool ChangeTilesetTileOffset::mergeWith(const QUndoCommand *other)
{
    const ChangeTilesetTileOffset *o = static_cast<const ChangeTilesetTileOffset*>(other);
    if (!(mMapDocument == o->mMapDocument && mTileset == o->mTileset))
        return false;

    mNewTileOffset = o->mNewTileOffset;
    return true;
}


TilesetParameters::TilesetParameters(const Tileset &tileset)
    : imageSource(tileset.imageSource())
    , transparentColor(tileset.transparentColor())
    , tileSize(tileset.tileSize())
    , tileSpacing(tileset.tileSpacing())
    , margin(tileset.margin())
{
}

bool TilesetParameters::operator != (const TilesetParameters &other) const
{
    return imageSource          != other.imageSource        ||
           transparentColor     != other.transparentColor   ||
           tileSize             != other.tileSize           ||
           tileSpacing          != other.tileSpacing        ||
           margin               != other.margin;
}

ChangeTilesetParameters::ChangeTilesetParameters(MapDocument *mapDocument,
                                                 Tileset &tileset,
                                                 const TilesetParameters &parameters)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Edit Tileset"))
    , mMapDocument(mapDocument)
    , mTileset(tileset)
    , mOldParameters(tileset)
    , mNewParameters(parameters)
{
}

void ChangeTilesetParameters::undo()
{
    apply(mOldParameters);
}

void ChangeTilesetParameters::redo()
{
    apply(mNewParameters);
}

void ChangeTilesetParameters::apply(const TilesetParameters &parameters)
{
    QString oldImageSource = mTileset.imageSource();

    mTileset.setImageSource(parameters.imageSource);
    mTileset.setTransparentColor(parameters.transparentColor);
    mTileset.setTileSize(parameters.tileSize);
    mTileset.setTileSpacing(parameters.tileSpacing);
    mTileset.setMargin(parameters.margin);

    auto tilesetManager = TilesetManager::instance();

    if (oldImageSource != mTileset.imageSource())
        tilesetManager->tilesetImageSourceChanged(mTileset, oldImageSource);
    if (mTileset.loadImage())
        emit tilesetManager->tilesetChanged(&mTileset);

    emit mMapDocument->tilesetChanged(&mTileset);
}

ChangeTilesetColumnCount::ChangeTilesetColumnCount(MapDocument *mapDocument,
                                                   Tileset &tileset,
                                                   int columnCount)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Change Columns"))
    , mMapDocument(mapDocument)
    , mTileset(tileset)
    , mColumnCount(columnCount)
{
}

void ChangeTilesetColumnCount::swap()
{
    int oldColumnCount = mTileset.columnCount();
    mTileset.setColumnCount(mColumnCount);
    mColumnCount = oldColumnCount;

    emit mMapDocument->tilesetChanged(&mTileset);
}

} // namespace Internal
} // namespace Tiled
