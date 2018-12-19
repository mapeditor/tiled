/*
 * reversingproxymodel.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

/*
 * This code is largely based on qidentityproxymodel.cpp (qtbase).
 */

#include "reversingproxymodel.h"

namespace Tiled {

// Stupid hack needed because createIndex is protected
class FriendModel : public QAbstractItemModel
{
    friend class ReversingProxyModel;
};

ReversingProxyModel::ReversingProxyModel(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

QModelIndex ReversingProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || !sourceModel())
        return QModelIndex();

    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    const int rows = rowCount(parent);
    if (row >= rows)
        return QModelIndex();

    const QModelIndex sourceParent = mapToSource(parent);
    const QModelIndex sourceIndex = sourceModel()->index(rows - row - 1, column, sourceParent);
    return createIndex(row, column, sourceIndex.internalId());
}

QModelIndex ReversingProxyModel::parent(const QModelIndex &child) const
{
    Q_ASSERT(child.isValid() ? child.model() == this : true);
    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

int ReversingProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    if (!sourceModel())
        return 0;

    return sourceModel()->rowCount(mapToSource(parent));
}

int ReversingProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);
    if (!sourceModel())
        return 0;

    return sourceModel()->columnCount(mapToSource(parent));
}

bool ReversingProxyModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                          int row, int column, const QModelIndex &parent) const
{
    int sourceDestinationRow;
    QModelIndex sourceParent;
    mapDropCoordinatesToSource(row, parent, &sourceDestinationRow, &sourceParent);
    return sourceModel()->canDropMimeData(data, action, sourceDestinationRow, column, sourceParent);
}

bool ReversingProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                       int row, int column, const QModelIndex &parent)
{
    int sourceDestinationRow;
    QModelIndex sourceParent;
    mapDropCoordinatesToSource(row, parent, &sourceDestinationRow, &sourceParent);
    return sourceModel()->dropMimeData(data, action, sourceDestinationRow, column, sourceParent);
}

void ReversingProxyModel::mapDropCoordinatesToSource(int row, const QModelIndex &parent,
                                                     int *sourceRow, QModelIndex *sourceParent) const
{
    *sourceParent = mapToSource(parent);

    if (row == -1)
        *sourceRow = -1;
    else
        *sourceRow = sourceModel()->rowCount(*sourceParent) - row;
}

QModelIndex ReversingProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();

    // This relies on the fact that the parent and its rowCount are the same for each sibling.
    const QModelIndex sourceSiblingIndex = static_cast<FriendModel*>(sourceModel())->createIndex(proxyIndex.row(),
                                                                                                 proxyIndex.column(),
                                                                                                 proxyIndex.internalId());
    const QModelIndex sourceParent = sourceSiblingIndex.parent();
    const int rowCount = sourceModel()->rowCount(sourceParent);

    Q_ASSERT(proxyIndex.row() >= 0 && proxyIndex.row() < rowCount);
    const int row = rowCount - proxyIndex.row() - 1;
    return sourceModel()->index(row, proxyIndex.column(), sourceParent);
}

QModelIndex ReversingProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel() || !sourceIndex.isValid())
        return QModelIndex();

    const int rowCount = sourceModel()->rowCount(sourceIndex.parent());

    Q_ASSERT(sourceIndex.row() >= 0 && sourceIndex.row() < rowCount);
    const int row = rowCount - sourceIndex.row() - 1;
    return createIndex(row, sourceIndex.column(), sourceIndex.internalId());
}

void ReversingProxyModel::setSourceModel(QAbstractItemModel* newSourceModel)
{
    beginResetModel();

    if (QAbstractItemModel *source = sourceModel()) {
        disconnect(source, &QAbstractItemModel::rowsAboutToBeInserted,
                   this, &ReversingProxyModel::sourceRowsAboutToBeInserted);
        disconnect(source, &QAbstractItemModel::rowsInserted,
                   this, &ReversingProxyModel::sourceRowsInserted);
        disconnect(source, &QAbstractItemModel::rowsAboutToBeRemoved,
                   this, &ReversingProxyModel::sourceRowsAboutToBeRemoved);
        disconnect(source, &QAbstractItemModel::rowsRemoved,
                   this, &ReversingProxyModel::sourceRowsRemoved);
        disconnect(source, &QAbstractItemModel::rowsAboutToBeMoved,
                   this, &ReversingProxyModel::sourceRowsAboutToBeMoved);
        disconnect(source, &QAbstractItemModel::rowsMoved,
                   this, &ReversingProxyModel::sourceRowsMoved);
        disconnect(source, &QAbstractItemModel::columnsAboutToBeInserted,
                   this, &ReversingProxyModel::sourceColumnsAboutToBeInserted);
        disconnect(source, &QAbstractItemModel::columnsInserted,
                   this, &ReversingProxyModel::sourceColumnsInserted);
        disconnect(source, &QAbstractItemModel::columnsAboutToBeRemoved,
                   this, &ReversingProxyModel::sourceColumnsAboutToBeRemoved);
        disconnect(source, &QAbstractItemModel::columnsRemoved,
                   this, &ReversingProxyModel::sourceColumnsRemoved);
        disconnect(source, &QAbstractItemModel::columnsAboutToBeMoved,
                   this, &ReversingProxyModel::sourceColumnsAboutToBeMoved);
        disconnect(source, &QAbstractItemModel::columnsMoved,
                   this, &ReversingProxyModel::sourceColumnsMoved);
        disconnect(source, &QAbstractItemModel::modelAboutToBeReset,
                   this, &ReversingProxyModel::sourceModelAboutToBeReset);
        disconnect(source, &QAbstractItemModel::modelReset,
                   this, &ReversingProxyModel::sourceModelReset);
        disconnect(source, &QAbstractItemModel::dataChanged,
                   this, &ReversingProxyModel::sourceDataChanged);
        disconnect(source, &QAbstractItemModel::headerDataChanged,
                   this, &ReversingProxyModel::sourceHeaderDataChanged);
        disconnect(source, &QAbstractItemModel::layoutAboutToBeChanged,
                   this, &ReversingProxyModel::sourceLayoutAboutToBeChanged);
        disconnect(source, &QAbstractItemModel::layoutChanged,
                   this, &ReversingProxyModel::sourceLayoutChanged);
    }

    // Skip QIdentityPoxyModel
    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (QAbstractItemModel *source = sourceModel()) {
        connect(source, &QAbstractItemModel::rowsAboutToBeInserted,
                this, &ReversingProxyModel::sourceRowsAboutToBeInserted);
        connect(source, &QAbstractItemModel::rowsInserted,
                this, &ReversingProxyModel::sourceRowsInserted);
        connect(source, &QAbstractItemModel::rowsAboutToBeRemoved,
                this, &ReversingProxyModel::sourceRowsAboutToBeRemoved);
        connect(source, &QAbstractItemModel::rowsRemoved,
                this, &ReversingProxyModel::sourceRowsRemoved);
        connect(source, &QAbstractItemModel::rowsAboutToBeMoved,
                this, &ReversingProxyModel::sourceRowsAboutToBeMoved);
        connect(source, &QAbstractItemModel::rowsMoved,
                this, &ReversingProxyModel::sourceRowsMoved);
        connect(source, &QAbstractItemModel::columnsAboutToBeInserted,
                this, &ReversingProxyModel::sourceColumnsAboutToBeInserted);
        connect(source, &QAbstractItemModel::columnsInserted,
                this, &ReversingProxyModel::sourceColumnsInserted);
        connect(source, &QAbstractItemModel::columnsAboutToBeRemoved,
                this, &ReversingProxyModel::sourceColumnsAboutToBeRemoved);
        connect(source, &QAbstractItemModel::columnsRemoved,
                this, &ReversingProxyModel::sourceColumnsRemoved);
        connect(source, &QAbstractItemModel::columnsAboutToBeMoved,
                this, &ReversingProxyModel::sourceColumnsAboutToBeMoved);
        connect(source, &QAbstractItemModel::columnsMoved,
                this, &ReversingProxyModel::sourceColumnsMoved);
        connect(source, &QAbstractItemModel::modelAboutToBeReset,
                this, &ReversingProxyModel::sourceModelAboutToBeReset);
        connect(source, &QAbstractItemModel::modelReset,
                this, &ReversingProxyModel::sourceModelReset);
        connect(source, &QAbstractItemModel::dataChanged,
                this, &ReversingProxyModel::sourceDataChanged);
        connect(source, &QAbstractItemModel::headerDataChanged,
                this, &ReversingProxyModel::sourceHeaderDataChanged);
        connect(source, &QAbstractItemModel::layoutAboutToBeChanged,
                this, &ReversingProxyModel::sourceLayoutAboutToBeChanged);
        connect(source, &QAbstractItemModel::layoutChanged,
                this, &ReversingProxyModel::sourceLayoutChanged);
    }

    endResetModel();
}

void ReversingProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    beginInsertColumns(mapFromSource(parent), start, end);
}

void ReversingProxyModel::sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void ReversingProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    beginRemoveColumns(mapFromSource(parent), start, end);
}

void ReversingProxyModel::sourceColumnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endInsertColumns();
}

void ReversingProxyModel::sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)

    endMoveColumns();
}

void ReversingProxyModel::sourceColumnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endRemoveColumns();
}

void ReversingProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_ASSERT(topLeft.isValid() ? topLeft.model() == sourceModel() : true);
    Q_ASSERT(bottomRight.isValid() ? bottomRight.model() == sourceModel() : true);

    const QModelIndex sourceParent = topLeft.parent();
    const QModelIndex bottomLeft = sourceModel()->index(bottomRight.row(), topLeft.column(), sourceParent);
    const QModelIndex topRight = sourceModel()->index(topLeft.row(), bottomRight.column(), sourceParent);

    const int rows = sourceModel()->rowCount(sourceParent);
    const int proxyTop = rows - bottomRight.row() - 1;
    const int proxyBottom = rows - topLeft.row() - 1;

    const QModelIndex proxyTopLeft = createIndex(proxyTop, topLeft.column(), bottomLeft.internalId());
    const QModelIndex proxyBottomRight = createIndex(proxyBottom, bottomRight.column(), topRight.internalId());

    dataChanged(proxyTopLeft, proxyBottomRight, roles);
}

void ReversingProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last)
{
    if (orientation == Qt::Vertical) {
        const int rows = sourceModel()->rowCount();
        const int proxyFirst = rows - last - 1;
        const int proxyLast = rows - first - 1;
        first = proxyFirst;
        last = proxyLast;
    }
    headerDataChanged(orientation, first, last);
}

void ReversingProxyModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    const auto proxyPersistentIndexes = persistentIndexList();
    for (const QPersistentModelIndex &proxyPersistentIndex : proxyPersistentIndexes) {
        mProxyIndexes << proxyPersistentIndex;
        Q_ASSERT(proxyPersistentIndex.isValid());
        const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
        Q_ASSERT(srcPersistentIndex.isValid());
        mLayoutChangePersistentIndexes << srcPersistentIndex;
    }

    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = mapFromSource(parent);
        Q_ASSERT(mappedParent.isValid());
        parents << mappedParent;
    }

    layoutAboutToBeChanged(parents, hint);
}

void ReversingProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    for (int i = 0; i < mProxyIndexes.size(); ++i) {
        changePersistentIndex(mProxyIndexes.at(i),
                              mapFromSource(mLayoutChangePersistentIndexes.at(i)));
    }

    mLayoutChangePersistentIndexes.clear();
    mProxyIndexes.clear();

    QList<QPersistentModelIndex> parents;
    parents.reserve(sourceParents.size());
    for (const QPersistentModelIndex &parent : sourceParents) {
        if (!parent.isValid()) {
            parents << QPersistentModelIndex();
            continue;
        }
        const QModelIndex mappedParent = mapFromSource(parent);
        Q_ASSERT(mappedParent.isValid());
        parents << mappedParent;
    }

    layoutChanged(parents, hint);
}

void ReversingProxyModel::sourceModelAboutToBeReset()
{
    beginResetModel();
}

void ReversingProxyModel::sourceModelReset()
{
    endResetModel();
}

void ReversingProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    const int rows = sourceModel()->rowCount(parent);
    const int proxyStart = rows - end;
    const int proxyEnd = rows - start;

    beginInsertRows(mapFromSource(parent), proxyStart, proxyEnd);
}

void ReversingProxyModel::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    const int sourceRows = sourceModel()->rowCount(sourceParent);
    const int destRows = sourceModel()->rowCount(destParent);

    const int proxySourceStart = sourceRows - sourceEnd - 1;
    const int proxySourceEnd = sourceRows - sourceStart - 1;
    const int proxyDest = destRows - dest;

    beginMoveRows(mapFromSource(sourceParent), proxySourceStart, proxySourceEnd, mapFromSource(destParent), proxyDest);
}

void ReversingProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    const int rows = sourceModel()->rowCount(parent);
    const int proxyStart = rows - end - 1;
    const int proxyEnd = rows - start - 1;
    const QModelIndex proxyParent = mapFromSource(parent);

    beginRemoveRows(proxyParent, proxyStart, proxyEnd);
}

void ReversingProxyModel::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endInsertRows();
}

void ReversingProxyModel::sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destParent)
    Q_UNUSED(dest)

    endMoveRows();
}

void ReversingProxyModel::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endRemoveRows();
}

} // namespace Tiled
