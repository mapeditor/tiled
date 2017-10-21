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

#include "wangset.h"

#include <QUndoCommand>

namespace Tiled {

class Tileset;
class Tile;

namespace Internal {

class TilesetDocument;
class TilesetWangSetModel;

class ChangeWangSetEdges : public QUndoCommand
{
public:
    ChangeWangSetEdges(TilesetDocument *TilesetDocument,
                       int index,
                       int newValue);

    void undo() override;
    void redo() override;

private:
    struct WangColorChange {
        QSharedPointer<WangColor> wangColor;
        int index;
    };

    TilesetDocument *mTilesetDocument;
    TilesetWangSetModel *mWangSetModel;
    int mIndex;
    int mOldValue;
    int mNewValue;
    QList<WangColorChange> mRemovedWangColors;
};

class ChangeWangSetCorners : public QUndoCommand
{
public:
    ChangeWangSetCorners(TilesetDocument *TilesetDocument,
                       int index,
                       int newValue);

    void undo() override;
    void redo() override;

private:
    struct WangColorChange {
        QSharedPointer<WangColor> wangColor;
        int index;
    };

    TilesetDocument *mTilesetDocument;
    TilesetWangSetModel *mWangSetModel;
    int mIndex;
    int mOldValue;
    int mNewValue;
    QList<WangColorChange> mRemovedWangColors;
};

class RemoveWangSetColor : public QUndoCommand
{
public:
    RemoveWangSetColor(TilesetDocument *tilesetDocumnet,
                       int index,
                       int color,
                       bool isEdge);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    TilesetWangSetModel *mWangSetModel;
    int mIndex;
    int mColor;
    bool mIsEdge;
    QSharedPointer<WangColor> mRemovedWangColor;
    //When removing a color when there are two, both are actually removed,
    //this stores the extra if needed, and is null otherwise.
    QSharedPointer<WangColor> mExtraWangColor;
};

class SetWangSetImage : public QUndoCommand
{
public:
    SetWangSetImage(TilesetDocument *tilesetDocument,
                    int index,
                    int tileId);

    void undo() override;
    void redo() override;
private:
    TilesetWangSetModel *mWangSetModel;
    int mIndex;
    int mOldImageTileId;
    int mNewImageTileId;
};

} // namespace Internal
} // namespace Tiled
