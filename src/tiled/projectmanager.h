/*
 * projectmanager.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QObject>

namespace Tiled {

class EditableAsset;
class ProjectModel;

/**
 * Singleton for managing the current project.
 *
 * No dependencies.
 */
class TILED_EDITOR_EXPORT ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);

    static ProjectManager *instance();

    void setProject(std::unique_ptr<Project> project);
    Project &project();
    EditableAsset *editableProject();

    ProjectModel *projectModel();

signals:
    void projectChanged();

private:
    ProjectModel *mProjectModel;

    static ProjectManager *ourInstance;
};


inline ProjectManager *ProjectManager::instance()
{
    return ourInstance;
}

inline ProjectModel *ProjectManager::projectModel()
{
    return mProjectModel;
}

} // namespace Tiled
