/*
 * tilestampmodel.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilestampmodel.h"

#include "thumbnailrenderer.h"
#include "tilestamp.h"

namespace Tiled {
namespace Internal {

TileStampModel::TileStampModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QModelIndex TileStampModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column);
    else if (TileStamp *stamp = stampAt(parent))
        return createIndex(row, column, stamp);

    return QModelIndex();
}

QModelIndex TileStampModel::parent(const QModelIndex &index) const
{
    if (TileStamp *stamp = static_cast<TileStamp*>(index.internalPointer())) {
        int row = mStamps.indexOf(stamp);
        Q_ASSERT(row != -1);
        return createIndex(row, 0);
    }

    return QModelIndex();
}

int TileStampModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mStamps.size();
    else if (TileStamp *stamp = stampAt(parent))
        return stamp->variations().size();

    return 0;
}

int TileStampModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3; // stamp | probability | delete
}

QVariant TileStampModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Stamp");
        case 1: return tr("Probability");
        case 2: return tr("Delete");
        }
    }
    return QVariant();
}

bool TileStampModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // todo
    return false;
}

QVariant TileStampModel::data(const QModelIndex &index, int role) const
{
    if (TileStamp *stamp = stampAt(index)) {
        if (index.column() == 0) {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return stamp->name();
            case Qt::DecorationRole: {
                Map *map = stamp->variations().first().map;
                QPixmap thumbnail = mThumbnailCache.value(map);
                if (thumbnail.isNull()) {
                    ThumbnailRenderer renderer(map);
                    thumbnail = QPixmap::fromImage(renderer.render(QSize(32, 32)));
                    mThumbnailCache.insert(map, thumbnail);
                }
                return thumbnail;
                break;
            }
            }
        }
    }
    // todo: information about variations

    return QVariant();
}

Qt::ItemFlags TileStampModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if ((!index.parent().isValid() && index.column() == 0) ||   // can edit stamp names
            (index.parent().isValid() && index.column() == 1))  // and variation probability
        rc |= Qt::ItemIsEditable;
    return rc;
}

TileStamp *TileStampModel::stampAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.parent().isValid()) // stamps don't have parents
        return 0;
    if (index.row() >= mStamps.size())
        return 0;

    return mStamps.at(index.row());
}

const TileStampVariation *TileStampModel::variationAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (TileStamp *stamp = static_cast<TileStamp*>(index.internalPointer()))
        return &stamp->variations().at(index.row());

    return 0;
}

void TileStampModel::addStamp(TileStamp *stamp)
{
    Q_ASSERT(!mStamps.contains(stamp));

    beginInsertRows(QModelIndex(), mStamps.size(), mStamps.size());
    mStamps.append(stamp);
    endInsertRows();
}

void TileStampModel::removeStamp(TileStamp *stamp)
{
    int index = mStamps.indexOf(stamp);
    Q_ASSERT(index != -1);

    beginRemoveRows(QModelIndex(), index, index);
    mStamps.removeAt(index);
    endRemoveRows();

    foreach (const TileStampVariation &variation, stamp->variations())
        mThumbnailCache.remove(variation.map);
}

} // namespace Internal
} // namespace Tiled
