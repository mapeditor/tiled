/*
 * reversingproxymodel.h
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

#ifndef REVERSINGPROXYMODEL_H
#define REVERSINGPROXYMODEL_H

#include <QAbstractProxyModel>

namespace Tiled {
namespace Internal {

/**
 * Displays the source model "upside down".
 */
class ReversingProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    ReversingProxyModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    // QAbstractProxyModel interface
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

private slots:
    void sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    void sourceRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

    void sourceColumnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void sourceColumnsInserted(const QModelIndex &parent, int start, int end);
    void sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void sourceColumnsRemoved(const QModelIndex &parent, int start, int end);
    void sourceColumnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    void sourceColumnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

    void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceLayoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void sourceModelAboutToBeReset();
    void sourceModelReset();

private:
    QList<QPersistentModelIndex> mLayoutChangePersistentIndexes;
    QModelIndexList mProxyIndexes;
};

} // namespace Tiled
} // namespace Internal

#endif // REVERSINGPROXYMODEL_H
