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

    int rows = mWangSet->edgeColorCount() * mWangSet->cornerColorCount();
    rows *= rows;
    rows *= rows;
    rows &= ~(1 << 31);

    // arbitrary large cap on how many rows can be displayed.
    // could eventually be moved to pagination...
    return std::min(rows, 0xffff);
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

    const int idIndex = index.row();

    if (WangSet *set = wangSet())
        if (idIndex < rowCount())
            return set->templateWangIdAt(idIndex);

    return 0;
}

QModelIndex WangTemplateModel::wangIdIndex(WangId wangId) const
{
    if (!mWangSet)
        return QModelIndex();

    Q_ASSERT(mWangSet->wangIdIsValid(wangId));

    int edges = mWangSet->edgeColorCount();
    int corners = mWangSet->cornerColorCount();

    //as this is a model of template tiles, a valid wangId can't have wildcards
    if (edges > 1) {
        if (wangId.hasEdgeWildCards())
            return QModelIndex();

        wangId = wangId - 0x01010101;
    }
    if (corners > 1) {
        if (wangId.hasCornerWildCards())
            return QModelIndex();

        wangId = wangId - 0x10101010;
    }

    int row = 0;
    int cornerEdgePermutations = edges * corners;

    for (int i = 0; i < 8; ++i) {
        int belowPermutations = qPow(cornerEdgePermutations, i/2) * ((i&1)? edges : 1);
        if (i&1)
            row += wangId.cornerColor(i/2) * belowPermutations;
        else
            row += wangId.edgeColor(i/2) * belowPermutations;
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
