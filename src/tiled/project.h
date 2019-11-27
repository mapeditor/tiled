/*
 * project.h
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

#include <QObject>
#include <QSet>

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

class Project
{
public:
    Project();

    QString fileName() const;
    bool save(const QString &fileName);
    bool load(const QString &fileName);
    void clear();

    void addFolder(const QString &folder);
    void refreshFolders();

    const std::vector<std::unique_ptr<FolderEntry>>* folders() const;

private:
    void updateNameFilters();

    void refreshFolder(FolderEntry &folder);

    QString mFileName;
    std::vector<std::unique_ptr<FolderEntry>> mFolders;

    QStringList mNameFilters;
    QSet<QString> mVisitedFolders;
};


inline QString Project::fileName() const
{
    return mFileName;
}

inline const std::vector<std::unique_ptr<FolderEntry> > *Project::folders() const
{
    return &mFolders;
}

} // namespace Tiled
