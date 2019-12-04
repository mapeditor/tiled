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
#include "fileformat.h"
#include "pluginmanager.h"
#include "utils.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeData>
#include <QSet>
#include <QUrl>

namespace Tiled {

class FolderScanner
{
public:
    FolderScanner(const QString &folder, const QStringList &nameFilters)
        : mFolder(folder)
        , mNameFilters(nameFilters)
    {}

    std::unique_ptr<FolderEntry> scan();

private:
    void scan(FolderEntry &folder);

    const QString mFolder;
    const QStringList mNameFilters;

    QSet<QString> mVisitedFolders;
};


ProjectModel::ProjectModel(Project project, QObject *parent)
    : QAbstractItemModel(parent)
    , mProject(std::move(project))
{
    mFileIconProvider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);

    mUpdateNameFiltersTimer.setInterval(100);
    mUpdateNameFiltersTimer.setSingleShot(true);
    connect(&mUpdateNameFiltersTimer, &QTimer::timeout,
            this, &ProjectModel::updateNameFilters);

    connect(PluginManager::instance(), &PluginManager::objectAdded,
            this, &ProjectModel::pluginObjectAddedOrRemoved);
    connect(PluginManager::instance(), &PluginManager::objectRemoved,
            this, &ProjectModel::pluginObjectAddedOrRemoved);
}

void ProjectModel::setProject(Project project)
{
    beginResetModel();
    mProject = std::move(project);
    scanFolders();
    endResetModel();
}

void ProjectModel::addFolder(const QString &folder)
{
    const int row = int(mProject.folders().size());

    beginInsertRows(QModelIndex(), row, row);

    mProject.addFolder(folder);
    mFolders.push_back(FolderScanner(folder, mNameFilters).scan());

    endInsertRows();
}

void ProjectModel::removeFolder(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    mProject.removeFolder(row);
    mFolders.erase(mFolders.begin() + row);
    endRemoveRows();
}

void ProjectModel::refreshFolders()
{
    beginResetModel();
    scanFolders();
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
        FolderEntry *entry = entryForIndex(parent);
        if (row < int(entry->entries.size()))
            return createIndex(row, column, entry->entries.at(row).get());
    } else {
        if (row < int(mProject.folders().size()))
            return createIndex(row, column, mFolders.at(row).get());
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

    QMimeData *data = new QMimeData;
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

    const auto &container = entry->parent ? entry->parent->entries : mFolders;
    const auto it = std::find_if(container.begin(), container.end(),
                                 [entry] (auto &value) { return value.get() == entry; });

    Q_ASSERT(it != container.end());
    return createIndex(std::distance(container.begin(), it), 0, entry);
}

void ProjectModel::pluginObjectAddedOrRemoved(QObject *object)
{
    if (auto format = qobject_cast<FileFormat*>(object))
        if (format->capabilities() & FileFormat::Read)
            mUpdateNameFiltersTimer.start();
}

void ProjectModel::updateNameFilters()
{
    QStringList nameFilters;

    const auto fileFormats = PluginManager::objects<FileFormat>();
    for (FileFormat *format : fileFormats) {
        if (!(format->capabilities() & FileFormat::Read))
            continue;

        const QString filter = format->nameFilter();
        nameFilters.append(Utils::cleanFilterList(filter));
    }

    if (mNameFilters != nameFilters) {
        mNameFilters = nameFilters;
        refreshFolders();
    }
}

void ProjectModel::scanFolders()
{
    // TODO: This process should run in a thread (potentially one job for each folder)
    mFolders.clear();

    for (const QString &folder : mProject.folders())
        mFolders.push_back(FolderScanner(folder, mNameFilters).scan());
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<FolderEntry> FolderScanner::scan()
{
    auto entry = std::make_unique<FolderEntry>(mFolder);
    scan(*entry);
    return entry;
}

void FolderScanner::scan(FolderEntry &folder)
{
    const auto list = QDir(folder.filePath).entryInfoList(mNameFilters,
                                                          QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                                          QDir::Name | QDir::LocaleAware | QDir::DirsFirst);

    for (const auto &fileInfo : list) {
        auto entry = std::make_unique<FolderEntry>(fileInfo.filePath(), &folder);

        if (fileInfo.isDir()) {
            const QString canonicalPath = fileInfo.canonicalFilePath();

            // prevent potential endless symlink loop
            if (!mVisitedFolders.contains(canonicalPath)) {
                mVisitedFolders.insert(canonicalPath);
                scan(*entry);
            }

            // Leave out empty directories
            if (entry->entries.empty())
                continue;
        }

        folder.entries.push_back(std::move(entry));
    }
}

} // namespace Tiled
