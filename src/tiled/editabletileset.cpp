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

EditableTileset::EditableTileset(TilesetDocument *tilesetDocument,
                                 QObject *parent)
    : EditableAsset(parent)
    , mTilesetDocument(tilesetDocument)
{
    connect(tilesetDocument, &Document::fileNameChanged, this, &EditableAsset::fileNameChanged);
}

QString EditableTileset::fileName() const
{
    return mTilesetDocument->fileName();
}

void EditableTileset::setName(const QString &name)
{
    push(new RenameTileset(mTilesetDocument, name));
}

void EditableTileset::setTileOffset(QPoint tileOffset)
{
    push(new ChangeTilesetTileOffset(mTilesetDocument, tileOffset));
}

void EditableTileset::setBackgroundColor(const QColor &color)
{
    push(new ChangeTilesetBackgroundColor(mTilesetDocument, color));
}

Tileset *EditableTileset::tileset() const
{
    return mTilesetDocument->tileset().data();
}

} // namespace Tiled
