/*
 * projectmodel.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "projectmodel.h"

#include "containerhelpers.h"

#include <QFileInfo>

namespace Tiled {

ProjectModel::ProjectModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mFileIconProvider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
}

void ProjectModel::setFolders(const std::vector<std::unique_ptr<FolderEntry> > *folders)
{
    beginResetModel();
    mFolders = folders;
    endResetModel();
}

QString ProjectModel::filePath(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    FolderEntry *entry = static_cast<FolderEntry*>(index.internalPointer());
    return entry->filePath;
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        FolderEntry *entry = static_cast<FolderEntry*>(parent.internalPointer());
        if (row < int(entry->entries.size()))
            return createIndex(row, column, entry->entries.at(row).get());
    } else {
        if (mFolders && row < int(mFolders->size()))
            return createIndex(row, column, mFolders->at(row).get());
    }

    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const
{
    FolderEntry *entry = static_cast<FolderEntry*>(index.internalPointer());
    return indexForEntry(entry->parent);
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mFolders ? mFolders->size() : 0;

    FolderEntry *entry = static_cast<FolderEntry*>(parent.internalPointer());
    return entry->entries.size();
}

int ProjectModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    FolderEntry *entry = static_cast<FolderEntry*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        return QFileInfo(entry->filePath).fileName();
    case Qt::DecorationRole:
        return mFileIconProvider.icon(QFileInfo(entry->filePath));
    case Qt::ToolTipRole:
        return entry->filePath;
    }

    return QVariant();
}

QModelIndex ProjectModel::indexForEntry(FolderEntry *entry) const
{
    if (!entry)
        return QModelIndex();

    const std::vector<std::unique_ptr<FolderEntry>> *container = entry->parent ? &entry->parent->entries : mFolders;
    auto it = std::find_if(container->begin(), container->end(), [entry] (const std::unique_ptr<FolderEntry> &value) { return value.get() == entry; });

    Q_ASSERT(it != container->end());
    return createIndex(std::distance(container->begin(), it), 0, entry);
}

} // namespace Tiled
