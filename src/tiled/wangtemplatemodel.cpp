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

    // arbitrary large cap on how many rows can be displayed.
    return static_cast<int>(std::min<quint64>(0xffff, mWangSet->completeSetSize()));
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
        return {};

    if (WangSet *set = wangSet()) {
        const int idIndex = index.row();
        if (idIndex < rowCount())
            return set->templateWangIdAt(idIndex);
    }

    return {};
}

QModelIndex WangTemplateModel::wangIdIndex(WangId wangId) const
{
    if (!mWangSet)
        return QModelIndex();

    Q_ASSERT(mWangSet->wangIdIsValid(wangId));

    const int colors = mWangSet->colorCount();
    int row = 0;
    int multiplier = 1;

    switch (mWangSet->type()) {
    case WangSet::Corner:
        // As this is a model of template tiles, a valid WangId can't have wildcards
        if (wangId.hasCornerWildCards())
            return QModelIndex();

        for (int i = 0; i < WangId::NumCorners; ++i) {
            row += (wangId.cornerColor(i) - 1) * multiplier;
            multiplier *= colors;
        }

        break;
    case WangSet::Edge:
        if (wangId.hasEdgeWildCards())
            return QModelIndex();

        for (int i = 0; i < WangId::NumEdges; ++i) {
            row += (wangId.edgeColor(i) - 1) * multiplier;
            multiplier *= colors;
        }

        break;
    case WangSet::Mixed:
        if (wangId.hasWildCards())
            return QModelIndex();

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            row += (wangId.indexColor(i) - 1) * multiplier;
            multiplier *= colors;
        }

        break;
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

#include "moc_wangtemplatemodel.cpp"
