/*
 * changewangcolordata.h
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

#include <QUndoCommand>

#include <QColor>

namespace Tiled {

class WangColor;

class TilesetDocument;

class ChangeWangColorName : public QUndoCommand
{
public:
    ChangeWangColorName(TilesetDocument *tilesetDocument,
                        WangColor *wangColor,
                        const QString &newName);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangColor *mWangColor;
    QString mOldName;
    QString mNewName;
};

class ChangeWangColorImage : public QUndoCommand
{
public:
    ChangeWangColorImage(TilesetDocument *tilesetDocument,
                         WangColor *wangColor,
                         int newImageId,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangColor *mWangColor;
    int mOldImageId;
    int mNewImageId;
};

class ChangeWangColorColor : public QUndoCommand
{
public:
    ChangeWangColorColor(TilesetDocument *tilesetDocument,
                         WangColor *wangColor,
                         const QColor &newColor);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangColor *mWangColor;
    QColor mOldColor;
    QColor mNewColor;
};

class ChangeWangColorProbability : public QUndoCommand
{
public:
    ChangeWangColorProbability(TilesetDocument *tilesetDocument,
                               WangColor *wangColor,
                               qreal newProbability);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    WangColor *mWangColor;
    qreal mOldProbability;
    qreal mNewProbability;
};

} // namespace Tiled
