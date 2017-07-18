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

#include "tileset.h"
#include "wangset.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;
using namespace Internal;

WangColorModel::WangColorModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mWangSet(nullptr)
    , mEdgeText(new QString(QLatin1String("Edge Colors")))
    , mCornerText(new QString(QLatin1String("CornerColors")))
{
}

QModelIndex WangColorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column);

    if (parent.row() == 0) {
        return createIndex(row, column, mEdgeText);
    }
    if (parent.row() == 1)
        return createIndex(row, column, mCornerText);

    return QModelIndex();
}

QModelIndex WangColorModel::parent(const QModelIndex &child) const
{
    if (child.internalPointer() == mEdgeText)
        return index(0, 0, QModelIndex());
    if (child.internalPointer() == mCornerText)
        return index(1, 0, QModelIndex());

    return QModelIndex();
}

QModelIndex WangColorModel::edgeIndex(int color) const
{
    if (mWangSet)
        Q_ASSERT(color <= mWangSet->edgeColors());
    else
        return QModelIndex();

    return createIndex(color - 1, 0, mEdgeText);
}

QModelIndex WangColorModel::cornerIndex(int color) const
{
    if (mWangSet)
        Q_ASSERT(color <= mWangSet->cornerColors());
    else
        return QModelIndex();

    return createIndex(color - 1, 0, mCornerText);
}

int WangColorModel::rowCount(const QModelIndex &parent) const
{
    if (!mWangSet)
        return 0;

    if (!parent.isValid())
        return 2;

    if (!parent.parent().isValid()) {
        if (parent.row() == 0) {
            int n = mWangSet->edgeColors();
            return (n == 1)? 0 : n;
        }
        if (parent.row() == 1) {
            int n = mWangSet->cornerColors();
            return (n == 1)? 0 : n;
        }
    }

    return 0;
}

int WangColorModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant WangColorModel::data(const QModelIndex &index, int role) const
{
    if (!mWangSet)
        return QVariant();

    if (!index.isValid())
        return QVariant();

    //Unused roles will eventually be used. TODO
    if (index.parent().isValid()) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::DecorationRole:
            break;
        case Qt::SizeHintRole:
            return QSize(1, 20);
        case Qt::BackgroundColorRole: {
            QColor c;
            if (index.parent().row() == 0)
                c = QColor::fromHsvF((float) index.row() / mWangSet->edgeColors(), 1, 1);
            else if (index.parent().row() == 1)
                c = QColor::fromHsvF((float) index.row() / mWangSet->cornerColors(), 1, 1);
            else
                c = QColor(Qt::red);

            return c;
        }
        case ColorRole:
            return index.row();
        case EdgeOrCornerRole:
            return index.parent().row();
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
            if (index.row() == 0)
                return *mEdgeText;
            if (index.row() == 1)
                return *mCornerText;
            break;
        case Qt::SizeHintRole:
            return QSize(1, 32);
        case Qt::FontRole: {
            QFont font = QApplication::font();
            font.setBold(true);
            return font;
        }
        case Qt::BackgroundRole: {
            QColor bg = QApplication::palette().alternateBase().color();
            return bg;
        }
        }
    }

    return QVariant();
}

Qt::ItemFlags WangColorModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (!index.parent().isValid())
        defaultFlags &= ~Qt::ItemIsSelectable;

    return defaultFlags;
}

void WangColorModel::setWangSet(WangSet *wangSet)
{
    beginResetModel();
    mWangSet = wangSet;
    endResetModel();
}
