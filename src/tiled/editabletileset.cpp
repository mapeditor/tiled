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

} // namespace Tiled
