/*
 * wangtemplatemodel.cpp
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

#include "wangtemplatemodel.h"

using namespace Tiled;
using namespace Internal;

//For now this is arbitrary, but could be set dynamicly based on zoom
static const int COLUMN_COUNT = 4;

WangTemplateModel::WangTemplateModel(WangSet *wangSet, QObject *parent)
    : QAbstractListModel(parent)
    , mWangSet(wangSet)
{
    refreshWangIds();
}

int WangTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const int wangIdCount = mWangIds.size();
    const int columns = columnCount();

    int rows = 1;
    if (columns > 0) {
        rows = wangIdCount / columns;
        if (wangIdCount % columns)
            ++rows;
    }

    return rows;
}

int WangTemplateModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return COLUMN_COUNT;
}

QVariant WangTemplateModel::data(const QModelIndex &index, int role) const
{
    if (role == WangIdRole)
        return QVariant::fromValue(wangIdAt(index));

    return QVariant();
}

WangId WangTemplateModel::wangIdAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    const int idIndex = index.column() + index.row() * columnCount();

    if (idIndex < mWangIds.size())
        return mWangIds.at(idIndex);

    return 0;
}

QModelIndex WangTemplateModel::wangIdIndex(WangId wangId) const
{
    if (!mWangSet)
        return QModelIndex();

    Q_ASSERT(mWangSet->wangIdIsValid(wangId));

    //Only wangIds with all edges/corners assigned are valid here
    if (mWangSet->edgeColors() > 1)
        Q_ASSERT(!wangId.hasEdgeWildCards());
    if (mWangSet->cornerColors() > 1)
        Q_ASSERT(!wangId.hasCornerWildCards());

    int idIndex = mWangIds.indexOf(wangId);

    int row = idIndex / columnCount();
    int column = idIndex % columnCount();

    return index(row, column);
}

void WangTemplateModel::setWangSet(WangSet *wangSet)
{
    mWangSet = wangSet;
    wangSetChanged();
}

void WangTemplateModel::wangSetChanged()
{
    beginResetModel();
    refreshWangIds();
    endResetModel();
}

void WangTemplateModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

void WangTemplateModel::refreshWangIds()
{
    if (mWangSet)
        mWangIds = mWangSet->templateWangIds();
    else
        mWangIds.clear();
}
