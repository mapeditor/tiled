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

#include <QColor>
#include <QPoint>
#include <QSize>
#include <QUndoCommand>

namespace Tiled {

class Tileset;

namespace Internal {

class TilesetDocument;

class RenameTileset : public QUndoCommand
{
public:
    RenameTileset(TilesetDocument *tilesetDocument,
                  const QString &newName);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    QString mOldName;
    QString mNewName;
};

class ChangeTilesetTileOffset : public QUndoCommand
{
public:
    ChangeTilesetTileOffset(TilesetDocument *tilesetDocument,
                            QPoint tileOffset);

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_ChangeTilesetTileOffset; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    TilesetDocument *mTilesetDocument;
    QPoint mOldTileOffset;
    QPoint mNewTileOffset;
};

struct TilesetParameters
{
    TilesetParameters()
        : tileSpacing(0)
        , margin(0)
    {}

    explicit TilesetParameters(const Tileset &tileset);

    bool operator != (const TilesetParameters &other) const;

    QString imageSource;
    QColor transparentColor;
    QSize tileSize;
    int tileSpacing;
    int margin;
};

class ChangeTilesetParameters : public QUndoCommand
{
public:
    ChangeTilesetParameters(TilesetDocument *tilesetDocument,
                            const TilesetParameters &parameters);

    void undo() override;
    void redo() override;

private:
    void apply(const TilesetParameters &parameters);

    TilesetDocument *mTilesetDocument;
    Tileset &mTileset;
    TilesetParameters mOldParameters;
    TilesetParameters mNewParameters;
};

class ChangeTilesetColumnCount : public QUndoCommand
{
public:
    ChangeTilesetColumnCount(TilesetDocument *tilesetDocument,
                             int columnCount);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    Tileset &mTileset;
    int mColumnCount;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETCHANGES_H
