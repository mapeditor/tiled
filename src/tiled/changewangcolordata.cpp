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
#include "tilesetdocument.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeWangColorName::ChangeWangColorName(TilesetDocument *tilesetDocument,
                                         WangColor *wangColor,
                                         const QString &newName)
    : mTilesetDocument(tilesetDocument)
    , mWangColor(wangColor)
    , mOldName(wangColor->name())
    , mNewName(newName)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Terrain Name"));
}

void ChangeWangColorName::undo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setName(mWangColor, mOldName);
}

void ChangeWangColorName::redo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setName(mWangColor, mNewName);
}

bool ChangeWangColorName::mergeWith(const QUndoCommand *other)
{
    auto o = static_cast<const ChangeWangColorName*>(other);
    if (mWangColor != o->mWangColor)
        return false;

    mNewName = o->mNewName;
    return true;
}


ChangeWangColorImage::ChangeWangColorImage(TilesetDocument *tilesetDocument,
                                           WangColor *wangColor,
                                           int newImageId,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangColor(wangColor)
    , mOldImageId(wangColor->imageId())
    , mNewImageId(newImageId)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Terrain Image"));
}

void ChangeWangColorImage::undo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setImage(mWangColor, mOldImageId);
}

void ChangeWangColorImage::redo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setImage(mWangColor, mNewImageId);
}


ChangeWangColorColor::ChangeWangColorColor(TilesetDocument *tilesetDocument,
                                           WangColor *wangColor,
                                           const QColor &newColor)
    : mTilesetDocument(tilesetDocument)
    , mWangColor(wangColor)
    , mOldColor(wangColor->color())
    , mNewColor(newColor)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Terrain Color"));
}

void ChangeWangColorColor::undo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setColor(mWangColor, mOldColor);
}

void ChangeWangColorColor::redo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setColor(mWangColor, mNewColor);
}


ChangeWangColorProbability::ChangeWangColorProbability(TilesetDocument *tilesetDocument,
                                                       WangColor *wangColor,
                                                       qreal newProbability)
    : mTilesetDocument(tilesetDocument)
    , mWangColor(wangColor)
    , mOldProbability(wangColor->probability())
    , mNewProbability(newProbability)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Terrain Probability"));
}

void ChangeWangColorProbability::undo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setProbability(mWangColor, mOldProbability);
}

void ChangeWangColorProbability::redo()
{
    auto wangColorModel = mTilesetDocument->wangColorModel(mWangColor->wangSet());
    wangColorModel->setProbability(mWangColor, mNewProbability);
}
