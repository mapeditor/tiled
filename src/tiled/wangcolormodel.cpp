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
    : QAbstractItemModel(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
{
}

QModelIndex WangColorModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column);

    if (parent.row() == 0)
        return createIndex(row, column, EdgeIndexId);

    if (parent.row() == 1)
        return createIndex(row, column, CornerIndexId);

    return QModelIndex();
}

QModelIndex WangColorModel::parent(const QModelIndex &child) const
{
    if (child.internalId() == EdgeIndexId)
        return index(0, 0);
    if (child.internalId() == CornerIndexId)
        return index(1, 0);

    return QModelIndex();
}

QModelIndex WangColorModel::edgeIndex(int color) const
{
    if (mWangSet)
        Q_ASSERT(color <= mWangSet->edgeColorCount());
    else
        return QModelIndex();

    return createIndex(color - 1, 0, EdgeIndexId);
}

QModelIndex WangColorModel::cornerIndex(int color) const
{
    if (mWangSet)
        Q_ASSERT(color <= mWangSet->cornerColorCount());
    else
        return QModelIndex();

    return createIndex(color - 1, 0, CornerIndexId);
}

int WangColorModel::rowCount(const QModelIndex &parent) const
{
    if (!mWangSet)
        return 0;

    if (!parent.isValid())
        return 2;

    if (!parent.parent().isValid()) {
        if (parent.row() == 0) {
            int n = mWangSet->edgeColorCount();
            return (n == 1)? 0 : n;
        }
        if (parent.row() == 1) {
            int n = mWangSet->cornerColorCount();
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

    if (index.parent().isValid()) {
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
        case EdgeOrCornerRole:
            return !isEdgeColorAt(index);
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
            if (index.row() == 0)
                return tr("Edge Colors");
            if (index.row() == 1)
                return tr("Corner Colors");
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
            return QBrush(bg);
        }
        }
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
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (!index.parent().isValid())
        defaultFlags &= ~Qt::ItemIsSelectable;
    else
        defaultFlags |= Qt::ItemIsEditable;

    return defaultFlags;
}

void WangColorModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

bool WangColorModel::isEdgeColorAt(const QModelIndex &index) const
{
    // Shouldn't use on invalid index
    Q_ASSERT(index.isValid());

    return index.parent().row() == 0;
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

    if (isEdgeColorAt(index))
        return mWangSet->edgeColorAt(colorAt(index));
    else
        return mWangSet->cornerColorAt(colorAt(index));
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
    QModelIndex i = wangColor->isEdge() ? edgeIndex(wangColor->colorIndex()) :
                                          cornerIndex(wangColor->colorIndex());
    emit dataChanged(i, i);
}
