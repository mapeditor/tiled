/*
 * projectmodel.h
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

#pragma once

#include "filesystemwatcher.h"
#include "project.h"

#include <QAbstractListModel>
#include <QFileIconProvider>
#include <QThread>
#include <QTimer>

#include <memory>
#include <vector>

namespace Tiled {

struct FolderEntry
{
    explicit FolderEntry(const QString &filePath, FolderEntry *parent = nullptr)
        : filePath(filePath)
        , parent(parent)
    {}

    QString filePath;
    std::vector<std::unique_ptr<FolderEntry>> entries;
    FolderEntry *parent = nullptr;
};

class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ProjectModel(QObject *parent = nullptr);
    ~ProjectModel() override;

    void updateNameFilters();

    void setProject(Project project);
    Project &project();

    void addFolder(const QString &folder);
    void removeFolder(int row);
    void refreshFolders();

    struct Match {
        int score;
        int offset;
        QString path;
    };

    QVector<Match> findFiles(const QStringList &words) const;

    QString filePath(const QModelIndex &index) const;

    QModelIndex index(const QString &filePath) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;

signals:
    void folderAdded(const QString &folder);
    void folderRemoved(const QString &folder);

    void nameFiltersChanged(const QStringList &nameFilters);
    void scanFolder(const QString &folder);

    void aboutToRefresh();
    void refreshed();

private:
    FolderEntry *entryForIndex(const QModelIndex &index) const;
    QModelIndex indexForEntry(FolderEntry *entry) const;

    void pluginObjectAddedOrRemoved(QObject *object);

    void pathsChanged(const QStringList &paths);

    void scheduleFolderScan(const QString &folder);
    void folderScanned(FolderEntry *entry);

    Project mProject;
    QFileIconProvider mFileIconProvider;
    QStringList mNameFilters;
    QTimer mUpdateNameFiltersTimer;

    std::vector<std::unique_ptr<FolderEntry>> mFolders;

    QThread mScanningThread;
    QString mScanningFolder;
    QStringList mFoldersPendingScan;
    FileSystemWatcher mWatcher;
};


inline Project &ProjectModel::project()
{
    return mProject;
}

} // namespace Tiled
