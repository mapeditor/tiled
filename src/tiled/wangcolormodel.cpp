/*
 * wangcolormodel.cpp
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

#include "wangcolormodel.h"

#include "changewangcolordata.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "wangset.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;

WangColorModel::WangColorModel(TilesetDocument *tilesetDocument,
                               WangSet *wangSet,
                               QObject *parent)
    : QAbstractListModel(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
{
}

QModelIndex WangColorModel::colorIndex(int color) const
{
    if (mWangSet)
        Q_ASSERT(color <= mWangSet->colorCount());
    else
        return QModelIndex();

    return createIndex(color - 1, 0);
}

int WangColorModel::rowCount(const QModelIndex &parent) const
{
    if (!mWangSet || parent.isValid())
        return 0;

    int n = mWangSet->colorCount();
    return (n == 1) ? 0 : n;
}

int WangColorModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant WangColorModel::data(const QModelIndex &index, int role) const
{
    if (!mWangSet)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return wangColorAt(index)->name();
    case Qt::DecorationRole:
        if (Tile *tile =  mWangSet->tileset()->findTile(wangColorAt(index)->imageId()))
            return tile->image();
        break;
    case Qt::BackgroundRole:
        return QBrush(wangColorAt(index)->color());
    case ColorRole:
        return colorAt(index);
    }

    return QVariant();
}

bool WangColorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        const QString newName = value.toString();
        WangColor *wangColor = wangColorAt(index).data();
        if (wangColor->name() != newName) {
            auto command = new ChangeWangColorName(mTilesetDocument, wangColor, newName);
            mTilesetDocument->undoStack()->push(command);
        }

        return true;
    }

    return false;
}

Qt::ItemFlags WangColorModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void WangColorModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

int WangColorModel::colorAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return index.row() + 1;
}

QSharedPointer<WangColor> WangColorModel::wangColorAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return QSharedPointer<WangColor>();

    return mWangSet->colorAt(colorAt(index));
}

void WangColorModel::setName(WangColor *wangColor, const QString &name)
{
    wangColor->setName(name);
    emitDataChanged(wangColor);
}

void WangColorModel::setImage(WangColor *wangColor, int imageId)
{
    wangColor->setImageId(imageId);
    emitDataChanged(wangColor);
}

void WangColorModel::setColor(WangColor *wangColor, const QColor &color)
{
    wangColor->setColor(color);
    emitDataChanged(wangColor);
}

void WangColorModel::setProbability(WangColor *wangColor, qreal probability)
{
    wangColor->setProbability(probability);
    // no data changed signal because probability not exposed by model
}

void WangColorModel::emitDataChanged(WangColor *wangColor)
{
    const QModelIndex i = colorIndex(wangColor->colorIndex());
    emit dataChanged(i, i);
}
