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

#include <QtMath>

using namespace Tiled;

WangTemplateModel::WangTemplateModel(WangSet *wangSet, QObject *parent)
    : QAbstractListModel(parent)
    , mWangSet(wangSet)
{
}

int WangTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (!mWangSet)
        return 0;

    const unsigned rows = mWangSet->completeSetSize();

    // arbitrary large cap on how many rows can be displayed.
    return static_cast<int>(std::min<unsigned>(rows, 0xffff));
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

    if (WangSet *set = wangSet()) {
        const int idIndex = index.row();
        if (idIndex < rowCount())
            return set->templateWangIdAt(idIndex);
    }

    return 0;
}

QModelIndex WangTemplateModel::wangIdIndex(WangId wangId) const
{
    if (!mWangSet)
        return QModelIndex();

    Q_ASSERT(mWangSet->wangIdIsValid(wangId));

    const int colors = mWangSet->colorCount();

    //as this is a model of template tiles, a valid wangId can't have wildcards
    if (colors > 1) {
        if (wangId.hasWildCards())
            return QModelIndex();

        wangId = wangId - 0x11111111;
    }

    // TODO: When we support WangSet type (Edges, Corners, etc.) this will need adjustment.
    int row = 0;
    int cornerEdgePermutations = colors * colors;

    for (int i = 0; i < WangId::NumIndexes; ++i) {
        int belowPermutations = qPow(cornerEdgePermutations, i/2) * ((i&1)? colors : 1);
        row += wangId.indexColor(i) * belowPermutations;
    }

    return index(row, 0);
}

void WangTemplateModel::setWangSet(WangSet *wangSet)
{
    beginResetModel();
    mWangSet = wangSet;
    endResetModel();
}

void WangTemplateModel::wangSetChanged()
{
    beginResetModel();
    endResetModel();
}
