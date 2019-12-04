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

#include "project.h"

#include <QAbstractListModel>
#include <QFileIconProvider>

namespace Tiled {

class ProjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ProjectModel(Project project, QObject *parent = nullptr);

    void setProject(Project project);
    Project &project();

    void addFolder(const QString &folder);
    void removeFolder(int row);
    void refreshFolders();

    QString filePath(const QModelIndex &index) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QModelIndex indexForEntry(FolderEntry *entry) const;

    Project mProject;
    QFileIconProvider mFileIconProvider;
};


inline Project &ProjectModel::project()
{
    return mProject;
}

} // namespace Tiled
