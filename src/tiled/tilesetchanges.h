/*
 * tilesetchanges.h
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

#ifndef TILESETCHANGES_H
#define TILESETCHANGES_H

#include "undocommands.h"

#include <QPoint>
#include <QUndoCommand>

namespace Tiled {

class Tileset;

namespace Internal {

class MapDocument;

class RenameTileset : public QUndoCommand
{
public:
    RenameTileset(MapDocument *mapDocument,
                  Tileset *tileset,
                  const QString &newName);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    Tileset *mTileset;
    QString mOldName;
    QString mNewName;
};

class ChangeTilesetTileOffset : public QUndoCommand
{
public:
    ChangeTilesetTileOffset(MapDocument *mapDocument,
                            Tileset *tileset,
                            QPoint tileOffset);

    void undo();
    void redo();

    int id() const { return Cmd_ChangeTilesetTileOffset; }
    bool mergeWith(const QUndoCommand *other);

private:
    MapDocument *mMapDocument;
    Tileset *mTileset;
    QPoint mOldTileOffset;
    QPoint mNewTileOffset;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETCHANGES_H
