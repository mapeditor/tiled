/*
 * editabletileset.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editabletileset.h"

#include "editabletile.h"
#include "scriptmanager.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"

namespace Tiled {

EditableTileset::EditableTileset(const QString &name,
                                 QObject *parent)
    : EditableAsset(nullptr, nullptr, parent)
{
    mTileset = Tileset::create(name, 0, 0);
    setObject(mTileset.data());
}

EditableTileset::EditableTileset(TilesetDocument *tilesetDocument,
                                 QObject *parent)
    : EditableAsset(tilesetDocument, tilesetDocument->tileset().data(), parent)
{
    connect(tilesetDocument, &Document::fileNameChanged, this, &EditableAsset::fileNameChanged);
    connect(tilesetDocument, &TilesetDocument::tilesRemoved, this, &EditableTileset::detachTiles);
}

EditableTileset::~EditableTileset()
{
    // Operate on copy since original container will get modified
    const auto editableTiles = mEditableTiles;
    for (auto editable : editableTiles)
        editable->detach();
}

EditableTile *EditableTileset::tile(int id)
{
    Tile *tile = tileset()->findTile(id);

    if (!tile) {
        ScriptManager::instance().throwError(tr("Invalid tile ID"));
        return nullptr;
    }

    return editableTile(tile);
}

QList<QObject*> EditableTileset::tiles()
{
    QList<QObject*> tiles;
    for (Tile *tile : tileset()->tiles())
        tiles.append(editableTile(tile));
    return tiles;
}

TilesetDocument *EditableTileset::tilesetDocument() const
{
    return static_cast<TilesetDocument*>(document());
}

void EditableTileset::setName(const QString &name)
{
    if (tilesetDocument())
        push(new RenameTileset(tilesetDocument(), name));
    else
        tileset()->setName(name);
}

void EditableTileset::setTileOffset(QPoint tileOffset)
{
    if (tilesetDocument())
        push(new ChangeTilesetTileOffset(tilesetDocument(), tileOffset));
    else
        tileset()->setTileOffset(tileOffset);
}

void EditableTileset::setBackgroundColor(const QColor &color)
{
    if (tilesetDocument())
        push(new ChangeTilesetBackgroundColor(tilesetDocument(), color));
    else
        tileset()->setBackgroundColor(color);
}

void EditableTileset::detachTiles(const QList<Tile *> &tiles)
{
    for (Tile *tile : tiles) {
        auto iterator = mEditableTiles.constFind(tile);
        if (iterator != mEditableTiles.constEnd())
            (*iterator)->detach();
    }
}

EditableTile *EditableTileset::editableTile(Tile *tile)
{
    Q_ASSERT(tile->tileset() == tileset());

    EditableTile* &editableTile = mEditableTiles[tile];
    if (!editableTile)
        editableTile = new EditableTile(this, tile);

    return editableTile;
}

} // namespace Tiled
