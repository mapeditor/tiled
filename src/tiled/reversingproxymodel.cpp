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
namespace Internal {

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

QModelIndex ReversingProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel() || !proxyIndex.isValid())
        return QModelIndex();

    // This relies on the fact that the parent and its rowCount are the same for each subling.
    const QModelIndex sourceSiblingIndex =
        static_cast<FriendModel *>(sourceModel())
            ->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalId());
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

void ReversingProxyModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
    beginResetModel();

    if (QAbstractItemModel *source = sourceModel()) {
        disconnect(source,
                   SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
                   this,
                   SLOT(sourceRowsAboutToBeInserted(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(rowsInserted(QModelIndex, int, int)),
                   this,
                   SLOT(sourceRowsInserted(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
                   this,
                   SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(rowsRemoved(QModelIndex, int, int)),
                   this,
                   SLOT(sourceRowsRemoved(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
                   this,
                   SLOT(sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
        disconnect(source,
                   SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
                   this,
                   SLOT(sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));
        disconnect(source,
                   SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
                   this,
                   SLOT(sourceColumnsAboutToBeInserted(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(columnsInserted(QModelIndex, int, int)),
                   this,
                   SLOT(sourceColumnsInserted(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
                   this,
                   SLOT(sourceColumnsAboutToBeRemoved(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(columnsRemoved(QModelIndex, int, int)),
                   this,
                   SLOT(sourceColumnsRemoved(QModelIndex, int, int)));
        disconnect(source,
                   SIGNAL(columnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
                   this,
                   SLOT(sourceColumnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
        disconnect(source,
                   SIGNAL(columnsMoved(QModelIndex, int, int, QModelIndex, int)),
                   this,
                   SLOT(sourceColumnsMoved(QModelIndex, int, int, QModelIndex, int)));
        disconnect(source, SIGNAL(modelAboutToBeReset()), this, SLOT(sourceModelAboutToBeReset()));
        disconnect(source, SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        disconnect(source,
                   SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                   this,
                   SLOT(sourceDataChanged(QModelIndex, QModelIndex, QVector<int>)));
        disconnect(source,
                   SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                   this,
                   SLOT(sourceHeaderDataChanged(Qt::Orientation, int, int)));
        disconnect(source,
                   SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,
                                                 QAbstractItemModel::LayoutChangeHint)),
                   this,
                   SLOT(sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>,
                                                     QAbstractItemModel::LayoutChangeHint)));
        disconnect(source,
                   SIGNAL(layoutChanged(QList<QPersistentModelIndex>,
                                        QAbstractItemModel::LayoutChangeHint)),
                   this,
                   SLOT(sourceLayoutChanged(QList<QPersistentModelIndex>,
                                            QAbstractItemModel::LayoutChangeHint)));
    }

    // Skip QIdentityPoxyModel
    QAbstractProxyModel::setSourceModel(newSourceModel);

    if (QAbstractItemModel *source = sourceModel()) {
        connect(source,
                SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
                SLOT(sourceRowsAboutToBeInserted(QModelIndex, int, int)));
        connect(source,
                SIGNAL(rowsInserted(QModelIndex, int, int)),
                SLOT(sourceRowsInserted(QModelIndex, int, int)));
        connect(source,
                SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
                SLOT(sourceRowsAboutToBeRemoved(QModelIndex, int, int)));
        connect(source,
                SIGNAL(rowsRemoved(QModelIndex, int, int)),
                SLOT(sourceRowsRemoved(QModelIndex, int, int)));
        connect(source,
                SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
                SLOT(sourceRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
        connect(source,
                SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
                SLOT(sourceRowsMoved(QModelIndex, int, int, QModelIndex, int)));
        connect(source,
                SIGNAL(columnsAboutToBeInserted(QModelIndex, int, int)),
                SLOT(sourceColumnsAboutToBeInserted(QModelIndex, int, int)));
        connect(source,
                SIGNAL(columnsInserted(QModelIndex, int, int)),
                SLOT(sourceColumnsInserted(QModelIndex, int, int)));
        connect(source,
                SIGNAL(columnsAboutToBeRemoved(QModelIndex, int, int)),
                SLOT(sourceColumnsAboutToBeRemoved(QModelIndex, int, int)));
        connect(source,
                SIGNAL(columnsRemoved(QModelIndex, int, int)),
                SLOT(sourceColumnsRemoved(QModelIndex, int, int)));
        connect(source,
                SIGNAL(columnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
                SLOT(sourceColumnsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
        connect(source,
                SIGNAL(columnsMoved(QModelIndex, int, int, QModelIndex, int)),
                SLOT(sourceColumnsMoved(QModelIndex, int, int, QModelIndex, int)));
        connect(source, SIGNAL(modelAboutToBeReset()), SLOT(sourceModelAboutToBeReset()));
        connect(source, SIGNAL(modelReset()), SLOT(sourceModelReset()));
        connect(source,
                SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)),
                SLOT(sourceDataChanged(QModelIndex, QModelIndex, QVector<int>)));
        connect(source,
                SIGNAL(headerDataChanged(Qt::Orientation, int, int)),
                SLOT(sourceHeaderDataChanged(Qt::Orientation, int, int)));
        connect(source,
                SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,
                                              QAbstractItemModel::LayoutChangeHint)),
                SLOT(sourceLayoutAboutToBeChanged(QList<QPersistentModelIndex>,
                                                  QAbstractItemModel::LayoutChangeHint)));
        connect(source,
                SIGNAL(layoutChanged(QList<QPersistentModelIndex>,
                                     QAbstractItemModel::LayoutChangeHint)),
                SLOT(sourceLayoutChanged(QList<QPersistentModelIndex>,
                                         QAbstractItemModel::LayoutChangeHint)));
    }

    endResetModel();
}

void ReversingProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex &parent,
                                                         int start,
                                                         int end)
{
    Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);

    beginInsertColumns(mapFromSource(parent), start, end);
}

void ReversingProxyModel::sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent,
                                                      int sourceStart,
                                                      int sourceEnd,
                                                      const QModelIndex &destParent,
                                                      int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    beginMoveColumns(
        mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destParent), dest);
}

void ReversingProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex &parent,
                                                        int start,
                                                        int end)
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

void ReversingProxyModel::sourceColumnsMoved(const QModelIndex &sourceParent,
                                             int sourceStart,
                                             int sourceEnd,
                                             const QModelIndex &destParent,
                                             int dest)
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

void ReversingProxyModel::sourceDataChanged(const QModelIndex &topLeft,
                                            const QModelIndex &bottomRight,
                                            const QVector<int> &roles)
{
    Q_ASSERT(topLeft.isValid() ? topLeft.model() == sourceModel() : true);
    Q_ASSERT(bottomRight.isValid() ? bottomRight.model() == sourceModel() : true);

    const QModelIndex sourceParent = topLeft.parent();
    const QModelIndex bottomLeft =
        sourceModel()->index(bottomRight.row(), topLeft.column(), sourceParent);
    const QModelIndex topRight =
        sourceModel()->index(topLeft.row(), bottomRight.column(), sourceParent);

    const int rows = sourceModel()->rowCount(sourceParent);
    const int proxyTop = rows - bottomRight.row() - 1;
    const int proxyBottom = rows - topLeft.row() - 1;

    const QModelIndex proxyTopLeft =
        createIndex(proxyTop, topLeft.column(), bottomLeft.internalId());
    const QModelIndex proxyBottomRight =
        createIndex(proxyBottom, bottomRight.column(), topRight.internalId());

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

void ReversingProxyModel::sourceLayoutAboutToBeChanged(
    const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
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

void ReversingProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents,
                                              QAbstractItemModel::LayoutChangeHint hint)
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

void ReversingProxyModel::sourceRowsAboutToBeMoved(const QModelIndex &sourceParent,
                                                   int sourceStart,
                                                   int sourceEnd,
                                                   const QModelIndex &destParent,
                                                   int dest)
{
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == sourceModel() : true);

    const int sourceRows = sourceModel()->rowCount(sourceParent);
    const int destRows = sourceModel()->rowCount(destParent);

    const int proxySourceStart = sourceRows - sourceEnd - 1;
    const int proxySourceEnd = sourceRows - sourceStart - 1;
    const int proxyDest = destRows - dest;

    beginMoveRows(mapFromSource(sourceParent),
                  proxySourceStart,
                  proxySourceEnd,
                  mapFromSource(destParent),
                  proxyDest);
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

void ReversingProxyModel::sourceRowsMoved(const QModelIndex &sourceParent,
                                          int sourceStart,
                                          int sourceEnd,
                                          const QModelIndex &destParent,
                                          int dest)
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
} // namespace Internal
