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
#include <QMimeData>
#include <QUrl>

namespace Tiled {

ProjectModel::ProjectModel(Project project, QObject *parent)
    : QAbstractItemModel(parent)
    , mProject(std::move(project))
{
    mFileIconProvider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
}

void ProjectModel::setProject(Project project)
{
    beginResetModel();
    mProject = std::move(project);
    endResetModel();
}

void ProjectModel::addFolder(const QString &folder)
{
    const int row = int(mProject.folders().size());

    beginInsertRows(QModelIndex(), row, row);
    mProject.addFolder(folder);
    endInsertRows();
}

void ProjectModel::removeFolder(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mProject.removeFolder(row);
    endRemoveRows();
}

void ProjectModel::refreshFolders()
{
    beginResetModel();
    mProject.refreshFolders();
    endResetModel();
}

QString ProjectModel::filePath(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    FolderEntry *entry = entryForIndex(index);
    return entry->filePath;
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        FolderEntry *entry = static_cast<FolderEntry*>(parent.internalPointer());
        if (row < int(entry->entries.size()))
            return createIndex(row, column, entry->entries.at(row).get());
    } else {
        if (row < int(mProject.folders().size()))
            return createIndex(row, column, mProject.folders().at(row).get());
    }

    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const
{
    FolderEntry *entry = entryForIndex(index);
    return indexForEntry(entry->parent);
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mProject.folders().size();

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

    FolderEntry *entry = entryForIndex(index);
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

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (FolderEntry *entry = entryForIndex(index))
        if (!QFileInfo(entry->filePath).isDir())
            flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QStringList ProjectModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

QMimeData *ProjectModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    for (const QModelIndex &index : indexes) {
        if (index.column() == 0) {
            const QFileInfo fileInfo(entryForIndex(index)->filePath);
            if (!fileInfo.isDir())
                urls << QUrl::fromLocalFile(fileInfo.filePath());
        }
    }

    if (urls.isEmpty())
        return nullptr;

    QMimeData *data = new QMimeData();
    data->setUrls(urls);
    return data;
}

FolderEntry *ProjectModel::entryForIndex(const QModelIndex &index) const
{
    return static_cast<FolderEntry*>(index.internalPointer());
}

QModelIndex ProjectModel::indexForEntry(FolderEntry *entry) const
{
    if (!entry)
        return QModelIndex();

    const auto &container = entry->parent ? entry->parent->entries : mProject.folders();
    const auto it = std::find_if(container.begin(), container.end(),
                                 [entry] (auto &value) { return value.get() == entry; });

    Q_ASSERT(it != container.end());
    return createIndex(std::distance(container.begin(), it), 0, entry);
}

} // namespace Tiled
