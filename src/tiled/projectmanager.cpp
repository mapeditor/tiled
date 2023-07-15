/*
 * projectmanager.cpp
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

#include "projectmanager.h"

#include "fileformat.h"
#include "objecttypes.h"
#include "preferences.h"
#include "projectmodel.h"

namespace Tiled {

ProjectManager *ProjectManager::ourInstance;

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
    , mProjectModel(new ProjectModel(this))
{
    Q_ASSERT(!ourInstance);
    ourInstance = this;
}

/**
 * Replaces the current project with the given \a project.
 */
void ProjectManager::setProject(std::unique_ptr<Project> _project)
{
    mProjectModel->setProject(std::move(_project));

    auto &project = mProjectModel->project();

    // Automatically import object types if they are referenced by the project
    if (!project.mObjectTypesFile.isEmpty()) {
        ObjectTypes objectTypes;
        const ExportContext context(*project.propertyTypes(),
                                    QFileInfo(project.mObjectTypesFile).path());

        if (ObjectTypesSerializer().readObjectTypes(project.mObjectTypesFile, objectTypes, context)) {
            project.propertyTypes()->mergeObjectTypes(objectTypes);
            project.mObjectTypesFile.clear();
        }
    }

    Preferences *prefs = Preferences::instance();
    prefs->setPropertyTypes(project.propertyTypes());

    FileFormat::setCompatibilityVersion(project.mCompatibilityVersion);

    emit projectChanged();
}

Project &ProjectManager::project()
{
    return mProjectModel->project();
}

EditableAsset *ProjectManager::editableProject()
{
    return mProjectModel->editableProject();
}

} // namespace Tiled

#include "moc_projectmanager.cpp"
