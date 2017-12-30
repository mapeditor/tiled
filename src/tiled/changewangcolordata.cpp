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

#include "changewangcolordata.h"

#include "wangcolormodel.h"

using namespace Tiled;
using namespace Internal;

ChangeWangColorName::ChangeWangColorName(const QString &newName,
                                         int colorIndex,
                                         bool isEdge,
                                         WangColorModel *wangColorModel)
    : mWangColorModel(wangColorModel)
    , mIndex(colorIndex)
    , mIsEdge(isEdge)
    , mNewName(newName)
{
    if (isEdge)
        mOldName = mWangColorModel->wangSet()->edgeColorAt(mIndex)->name();
    else
        mOldName = mWangColorModel->wangSet()->cornerColorAt(mIndex)->name();
}

void ChangeWangColorName::undo()
{
    mWangColorModel->setName(mOldName, mIsEdge, mIndex);
}

void ChangeWangColorName::redo()
{
    mWangColorModel->setName(mNewName, mIsEdge, mIndex);
}

ChangeWangColorImage::ChangeWangColorImage(int newImageId,
                                           int colorIndex,
                                           bool isEdge,
                                           WangColorModel *wangColorModel)
    : mWangColorModel(wangColorModel)
    , mIndex(colorIndex)
    , mIsEdge(isEdge)
    , mNewImageId(newImageId)
{
    if (isEdge)
        mOldImageId = mWangColorModel->wangSet()->edgeColorAt(mIndex)->imageId();
    else
        mOldImageId = mWangColorModel->wangSet()->cornerColorAt(mIndex)->imageId();
}

void ChangeWangColorImage::undo()
{
    mWangColorModel->setImage(mOldImageId, mIsEdge, mIndex);
}

void ChangeWangColorImage::redo()
{
    mWangColorModel->setImage(mNewImageId, mIsEdge, mIndex);
}

ChangeWangColorColor::ChangeWangColorColor(const QColor &newColor,
                                           int colorIndex,
                                           bool isEdge,
                                           WangColorModel *wangColorModel)
    : mWangColorModel(wangColorModel)
    , mIndex(colorIndex)
    , mIsEdge(isEdge)
    , mNewColor(newColor)
{
    if (isEdge)
        mOldColor = mWangColorModel->wangSet()->edgeColorAt(mIndex)->color();
    else
        mOldColor = mWangColorModel->wangSet()->cornerColorAt(mIndex)->color();
}

void ChangeWangColorColor::undo()
{
    mWangColorModel->setColor(mOldColor, mIsEdge, mIndex);
}

void ChangeWangColorColor::redo()
{
    mWangColorModel->setColor(mNewColor, mIsEdge, mIndex);
}

ChangeWangColorProbability::ChangeWangColorProbability(float newProbability,
                                                       int colorIndex,
                                                       bool isEdge,
                                                       WangColorModel *wangColorModel)
    : mWangColorModel(wangColorModel)
    , mIndex(colorIndex)
    , mIsEdge(isEdge)
    , mNewProbability(newProbability)
{
    if (mIsEdge)
        mOldProbability = mWangColorModel->wangSet()->edgeColorAt(mIndex)->probability();
    else
        mOldProbability = mWangColorModel->wangSet()->cornerColorAt(mIndex)->probability();
}

void ChangeWangColorProbability::undo()
{
    mWangColorModel->setProbability(mOldProbability, mIsEdge, mIndex);
}

void ChangeWangColorProbability::redo()
{
    mWangColorModel->setProbability(mNewProbability, mIsEdge, mIndex);
}
