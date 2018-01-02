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
namespace Internal {

class WangColorModel;

class ChangeWangColorName : public QUndoCommand
{
public:
    ChangeWangColorName(const QString &newName,
                        int colorIndex,
                        bool isEdge,
                        WangColorModel *wangColorModel);

    void undo() override;
    void redo() override;

private:
    WangColorModel *mWangColorModel;
    int mIndex;
    bool mIsEdge;
    QString mOldName;
    QString mNewName;
};

class ChangeWangColorImage : public QUndoCommand
{
public:
    ChangeWangColorImage(int newImageId,
                         int colorIndex,
                         bool isEdge,
                         WangColorModel *wangColorModel);

    void undo() override;
    void redo() override;

private:
    WangColorModel *mWangColorModel;
    int mIndex;
    bool mIsEdge;
    int mOldImageId;
    int mNewImageId;
};

class ChangeWangColorColor : public QUndoCommand
{
public:
    ChangeWangColorColor(const QColor &newColor,
                         int colorIndex,
                         bool isEdge,
                         WangColorModel *wangColorModel);

    void undo() override;
    void redo() override;

private:
    WangColorModel *mWangColorModel;
    int mIndex;
    bool mIsEdge;
    QColor mOldColor;
    QColor mNewColor;
};

class ChangeWangColorProbability : public QUndoCommand
{
public:
    ChangeWangColorProbability(qreal newProbability,
                               int colorIndex,
                               bool isEdge,
                               WangColorModel *wangColorModel);

    void undo() override;
    void redo() override;

private:
    WangColorModel *mWangColorModel;
    int mIndex;
    bool mIsEdge;
    qreal mOldProbability;
    qreal mNewProbability;
};

} // namespace Internal
} // namespace Tiled
