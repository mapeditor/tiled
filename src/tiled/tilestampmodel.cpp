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
    else if (isStamp(parent))
        return createIndex(row, column, parent.row() + 1);

    return QModelIndex();
}

QModelIndex TileStampModel::parent(const QModelIndex &index) const
{
    if (quintptr id = index.internalId())
        return createIndex(id - 1, 0);

    return QModelIndex();
}

int TileStampModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mStamps.size();
    } else if (isStamp(parent)) {
        const TileStamp &stamp = mStamps.at(parent.row());
        return stamp.variations().size();
    }

    return 0;
}

int TileStampModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2; // stamp | probability //| delete
}

QVariant TileStampModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Stamp");
        case 1: return tr("Probability");
//        case 2: return tr("Delete");
        }
    }
    return QVariant();
}

bool TileStampModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (isStamp(index)) {
        TileStamp &stamp = mStamps[index.row()];
        if (index.column() == 0) {
            switch (role) {
            case Qt::EditRole:
                stamp.setName(value.toString());
                return true;
                break;
            default:
                break;
            }
        }
    } else {
        QModelIndex parent = index.parent();
        if (isStamp(parent)) {
            TileStamp &stamp = mStamps[parent.row()];
            stamp.setProbability(index.row(), value.toReal());
            return true;
        }
    }

    return false;
}

QVariant TileStampModel::data(const QModelIndex &index, int role) const
{
    if (isStamp(index)) {
        const TileStamp &stamp = mStamps.at(index.row());
        if (index.column() == 0) {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return stamp.name();
            case Qt::DecorationRole: {
                Map *map = stamp.variations().first().map;
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
    } else if (const TileStampVariation *variation = variationAt(index)) {
        if (index.column() == 0) {
            switch (role) {
            case Qt::DecorationRole: {
                Map *map = variation->map;
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
        } else if (index.column() == 1) {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
                return variation->probability;
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags TileStampModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    const bool validParent = index.parent().isValid();
    if ((!validParent && index.column() == 0) ||   // can edit stamp names
            (validParent && index.column() == 1))  // and variation probability
        rc |= Qt::ItemIsEditable;
    return rc;
}

const TileStamp &TileStampModel::stampAt(const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(!index.parent().isValid()); // stamps don't have parents

    return mStamps.at(index.row());
}

bool TileStampModel::isStamp(const QModelIndex &index) const
{
    return index.isValid()
            && !index.parent().isValid()
            && index.row() < mStamps.size();
}

const TileStampVariation *TileStampModel::variationAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    QModelIndex parent = index.parent();
    if (isStamp(parent)) {
        const TileStamp &stamp = mStamps.at(parent.row());
        return &stamp.variations().at(index.row());
    }

    return 0;
}

void TileStampModel::addStamp(const TileStamp &stamp)
{
    if (mStamps.contains(stamp))
        return;

    beginInsertRows(QModelIndex(), mStamps.size(), mStamps.size());
    mStamps.append(stamp);
    endInsertRows();
}

void TileStampModel::removeStamp(const TileStamp &stamp)
{
    int index = mStamps.indexOf(stamp);
    Q_ASSERT(index != -1);

    beginRemoveRows(QModelIndex(), index, index);
    mStamps.removeAt(index);
    endRemoveRows();

    foreach (const TileStampVariation &variation, stamp.variations())
        mThumbnailCache.remove(variation.map);
}

} // namespace Internal
} // namespace Tiled
