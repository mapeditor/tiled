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

class FolderScanner : public QObject
{
    Q_OBJECT

public:
    void setNameFilters(const QStringList &nameFilters);
    void scanFolder(const QString &folder);

signals:
    void scanFinished(FolderEntry *entry);

private:
    void scan(FolderEntry &folder, QSet<QString> &visitedFolders) const;

    QStringList mNameFilters;
};

///////////////////////////////////////////////////////////////////////////////

static FolderEntry *findEntry(const std::vector<std::unique_ptr<FolderEntry>> &entries, const QString &filePath)
{
    for (const auto &entry : entries) {
        if (!filePath.startsWith(entry->filePath))
            continue;

        if (filePath.length() == entry->filePath.length())
            return entry.get();

        if (FolderEntry *childEntry = findEntry(entry->entries, filePath))
            return childEntry;
    }

    return nullptr;
}

static void collectDirectories(const FolderEntry &entry, QStringList &filePaths)
{
    for (const auto &childEntry : entry.entries) {
        if (!childEntry->entries.empty()) {
            filePaths.append(childEntry->filePath);
            collectDirectories(*childEntry, filePaths);
        }
    }
}

static void findFiles(const FolderEntry &entry, int offset, const QStringList &words, QVector<ProjectModel::Match> &result)
{
    for (const auto &childEntry : entry.entries) {
        if (childEntry->entries.empty()) {
            const QStringRef relativePath = childEntry->filePath.midRef(offset);
            const int totalScore = Utils::matchingScore(words, relativePath);

            if (totalScore > 0) {
                result.append(ProjectModel::Match {
                                   totalScore,
                                   offset,
                                   childEntry->filePath
                               });
            }
        } else {
            findFiles(*childEntry, offset, words, result);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

ProjectModel::ProjectModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    FolderScanner *scanner = new FolderScanner;
    scanner->moveToThread(&mScanningThread);
    connect(&mScanningThread, &QThread::finished, scanner, &QObject::deleteLater);
    connect(this, &ProjectModel::nameFiltersChanged, scanner, &FolderScanner::setNameFilters);
    connect(this, &ProjectModel::scanFolder, scanner, &FolderScanner::scanFolder);
    connect(scanner, &FolderScanner::scanFinished, this, &ProjectModel::folderScanned);
    mScanningThread.start();

    mFileIconProvider.setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);

    updateNameFilters();
    mUpdateNameFiltersTimer.setInterval(100);
    mUpdateNameFiltersTimer.setSingleShot(true);
    connect(&mUpdateNameFiltersTimer, &QTimer::timeout, this, &ProjectModel::updateNameFilters);

    connect(PluginManager::instance(), &PluginManager::objectAdded,
            this, &ProjectModel::pluginObjectAddedOrRemoved);
    connect(PluginManager::instance(), &PluginManager::objectRemoved,
            this, &ProjectModel::pluginObjectAddedOrRemoved);

    connect(&mWatcher, &FileSystemWatcher::pathsChanged,
            this, &ProjectModel::pathsChanged);
}

ProjectModel::~ProjectModel()
{
    mFoldersPendingScan.clear();
    mScanningThread.requestInterruption();
    mScanningThread.quit();
    mScanningThread.wait();
}

void ProjectModel::setProject(Project project)
{
    if (mUpdateNameFiltersTimer.isActive())
        updateNameFilters();

    beginResetModel();

    mProject = std::move(project);
    mFolders.clear();
    mFoldersPendingScan.clear();

    for (const QString &folder : mProject.folders()) {
        mFolders.push_back(std::make_unique<FolderEntry>(folder));
        scheduleFolderScan(folder);
    }

    mWatcher.clear();
    mWatcher.addPaths(mProject.folders());

    endResetModel();
}

void ProjectModel::addFolder(const QString &folder)
{
    const int row = int(mProject.folders().size());

    beginInsertRows(QModelIndex(), row, row);

    mProject.addFolder(folder);
    mFolders.push_back(std::make_unique<FolderEntry>(folder));
    scheduleFolderScan(folder);

    endInsertRows();

    emit folderAdded(folder);
}

void ProjectModel::removeFolder(int row)
{
    const QString folder = mFolders.at(row)->filePath;

    beginRemoveRows(QModelIndex(), row, row);
    mProject.removeFolder(row);
    mFolders.erase(mFolders.begin() + row);
    endRemoveRows();

    emit folderRemoved(folder);
}

void ProjectModel::refreshFolders()
{
    if (mFolders.empty())
        return;

    for (const auto &folder : mFolders)
        scheduleFolderScan(folder->filePath);

    // Display the "Refreshing" label
    emit dataChanged(index(0, 0),
                     index(int(mFolders.size() - 1), 0), { Qt::DisplayRole });
}

QVector<ProjectModel::Match> ProjectModel::findFiles(const QStringList &words) const
{
    QVector<Match> result;
    for (const auto &entry : mFolders)
        Tiled::findFiles(*entry, entry->filePath.lastIndexOf(QLatin1Char('/')) + 1, words, result);
    return result;
}

QString ProjectModel::filePath(const QModelIndex &index) const
{
    if (!index.isValid())
        return QString();

    FolderEntry *entry = entryForIndex(index);
    return entry->filePath;
}

QModelIndex ProjectModel::index(const QString &filePath) const
{
    if (filePath.isEmpty())
        return QModelIndex();
    if (FolderEntry *entry = findEntry(mFolders, filePath))
        return indexForEntry(entry);
    return QModelIndex();
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        FolderEntry *entry = entryForIndex(parent);
        if (row < int(entry->entries.size()))
            return createIndex(row, column, entry->entries.at(unsigned(row)).get());
    } else {
        if (row < int(mFolders.size()))
            return createIndex(row, column, mFolders.at(unsigned(row)).get());
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
        return int(mFolders.size());

    FolderEntry *entry = entryForIndex(parent);
    return int(entry->entries.size());
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
    case Qt::DisplayRole: {
        QString name = QFileInfo(entry->filePath).fileName();
        if (!entry->parent && (mScanningFolder == entry->filePath || mFoldersPendingScan.contains(entry->filePath))) {
            name.append(QLatin1Char(' '));
            name.append(tr("(Refreshing)"));
        }
        return name;
    }
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
    return { QLatin1String("text/uri-list") };
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
                                 [entry] (const std::unique_ptr<FolderEntry> &value) { return value.get() == entry; });

    Q_ASSERT(it != container.end());
    return createIndex(int(std::distance(container.begin(), it)), 0, entry);
}

void ProjectModel::pluginObjectAddedOrRemoved(QObject *object)
{
    if (auto format = qobject_cast<FileFormat*>(object))
        if (format->capabilities() & FileFormat::Read)
            mUpdateNameFiltersTimer.start();
}

void ProjectModel::pathsChanged(const QStringList &paths)
{
    for (const std::unique_ptr<FolderEntry> &entry : mFolders) {
        if (std::any_of(paths.begin(), paths.end(),
                        [&entry] (const QString &path) { return path.startsWith(entry->filePath); })) {
            scheduleFolderScan(entry->filePath);
        }
    }
}

void ProjectModel::updateNameFilters()
{
    mUpdateNameFiltersTimer.stop();

    QStringList nameFilters;

    const auto fileFormats = PluginManager::objects<FileFormat>();
    for (FileFormat *format : fileFormats) {
        if (!(format->capabilities() & FileFormat::Read))
            continue;

        const QString filter = format->nameFilter();
        nameFilters.append(Utils::cleanFilterList(filter));
    }

    nameFilters.removeDuplicates();

    if (mNameFilters != nameFilters) {
        mNameFilters = nameFilters;
        emit nameFiltersChanged(nameFilters);
        refreshFolders();
    }
}

void ProjectModel::scheduleFolderScan(const QString &folder)
{
    if (mScanningFolder.isEmpty()) {
        mScanningFolder = folder;
        emit scanFolder(mScanningFolder);
    } else if (!mFoldersPendingScan.contains(folder)) {
        mFoldersPendingScan.append(folder);
    }
}

void ProjectModel::folderScanned(FolderEntry *resultPointer)
{
    const std::unique_ptr<FolderEntry> result { resultPointer };
    Q_ASSERT(!result->parent);

    const auto it = std::find_if(mFolders.begin(), mFolders.end(),
                                 [&] (const std::unique_ptr<FolderEntry> &value) { return value->filePath == result->filePath; });

    // The folder may have been removed in the meantime
    if (it == mFolders.end())
        return;

    // There appears to be no way to reset a subset of the model, so signal the
    // removal of all previous rows and re-add the new rows instead.

    const std::unique_ptr<FolderEntry> &entry = *it;
    const QModelIndex index = indexForEntry(entry.get());

    QStringList previousDirectories;
    QStringList newDirectories;
    collectDirectories(*entry, previousDirectories);
    collectDirectories(*result, newDirectories);

    // First add the new paths to avoid needlessly unwatching/watching paths
    mWatcher.addPaths(newDirectories);
    mWatcher.removePaths(previousDirectories);

    emit aboutToRefresh();

    if (!entry->entries.empty()) {
        beginRemoveRows(index, 0, int(entry->entries.size() - 1));
        entry->entries.clear();
        endRemoveRows();
    }

    if (!result->entries.empty()) {
        beginInsertRows(index, 0, int(result->entries.size() - 1));
        entry->entries.swap(result->entries);

        // Fix up parent pointers
        for (auto &childEntry: entry->entries)
            childEntry->parent = entry.get();

        endInsertRows();
    }

    emit refreshed();

    if (!mFoldersPendingScan.isEmpty()) {
        mScanningFolder = mFoldersPendingScan.takeFirst();
        emit scanFolder(mScanningFolder);
    } else {
        mScanningFolder.clear();
    }

    emit dataChanged(index, index, { Qt::DisplayRole });
}

///////////////////////////////////////////////////////////////////////////////

void FolderScanner::setNameFilters(const QStringList &nameFilters)
{
    mNameFilters = nameFilters;
}

void FolderScanner::scanFolder(const QString &folder)
{
    QSet<QString> visitedFolders;
    auto entry = std::make_unique<FolderEntry>(folder);
    scan(*entry, visitedFolders);

    emit scanFinished(entry.release());
}

void FolderScanner::scan(FolderEntry &folder, QSet<QString> &visitedFolders) const
{
    if (QThread::currentThread()->isInterruptionRequested())
        return;

    constexpr QDir::SortFlags sortFlags { QDir::Name | QDir::LocaleAware | QDir::DirsFirst };
    constexpr QDir::Filters filters { QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot };
    const auto list = QDir(folder.filePath).entryInfoList(mNameFilters, filters, sortFlags);

    for (const auto &fileInfo : list) {
        auto entry = std::make_unique<FolderEntry>(fileInfo.filePath(), &folder);

        if (fileInfo.isDir()) {
            const QString canonicalPath = fileInfo.canonicalFilePath();

            // prevent potential endless symlink loop
            if (!visitedFolders.contains(canonicalPath)) {
                visitedFolders.insert(canonicalPath);
                scan(*entry, visitedFolders);
            }

            // Leave out empty directories
            if (entry->entries.empty())
                continue;
        }

        folder.entries.push_back(std::move(entry));
    }
}

} // namespace Tiled

#include "projectmodel.moc"
