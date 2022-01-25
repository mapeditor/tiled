/*
 * changewangsetdata.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#pragma once

#include "undocommands.h"
#include "wangset.h"

#include <QUndoCommand>

namespace Tiled {

class Tileset;
class Tile;

class TilesetDocument;

class RenameWangSet : public QUndoCommand
{
public:
    RenameWangSet(TilesetDocument *tilesetDocument,
                  WangSet *wangSet,
                  const QString &newName);

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_ChangeWangSetName; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    QString mOldName;
    QString mNewName;
};

class ChangeWangSetType : public QUndoCommand
{
public:
    ChangeWangSetType(TilesetDocument *tilesetDocument,
                      WangSet *wangSet,
                      WangSet::Type newType,
                      QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    const WangSet::Type mOldType;
    const WangSet::Type mNewType;
};

class ChangeWangSetColorCount : public QUndoCommand
{
public:
    ChangeWangSetColorCount(TilesetDocument *TilesetDocument,
                            WangSet *wangSet,
                            int newValue);

    void undo() override;
    void redo() override;

private:
    struct WangColorChange {
        QSharedPointer<WangColor> wangColor;
        int index;
    };

    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    const int mOldValue;
    const int mNewValue;
    QVector<WangColorChange> mRemovedWangColors;
};

class RemoveWangSetColor : public QUndoCommand
{
public:
    RemoveWangSetColor(TilesetDocument *tilesetDocumnet,
                       WangSet *wangSet,
                       int color);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    const int mColor;
    QSharedPointer<WangColor> mRemovedWangColor;
};

class SetWangSetImage : public QUndoCommand
{
public:
    SetWangSetImage(TilesetDocument *tilesetDocument,
                    WangSet *wangSet,
                    int tileId,
                    QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    const int mOldImageTileId;
    const int mNewImageTileId;
};

} // namespace Tiled
