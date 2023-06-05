/*
 * projectdocument.cpp
 * Copyright 2022, Chris Boehm AKA dogboydog
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "projectdocument.h"

#include "editableproject.h"

namespace Tiled {

ProjectDocument::ProjectDocument(Project *project, QObject *parent)
    : Document(ProjectDocumentType, project->fileName(), parent)
{
    mCurrentObject = project;
    mProject = project;
}

QString ProjectDocument::displayName() const
{
    return mProject->fileName();
}

bool ProjectDocument::save(const QString &/* fileName */, QString */* error */)
{
    return mProject->save();
}

FileFormat *ProjectDocument::writerFormat() const
{
    return nullptr;
}

void ProjectDocument::setExportFormat(FileFormat *)
{
    // do nothing
}

FileFormat *ProjectDocument::exportFormat() const
{
    return nullptr;
}

QString ProjectDocument::lastExportFileName() const
{
    return mProject->fileName();
}

void ProjectDocument::setLastExportFileName(const QString &/* fileName */)
{
    // do nothing
}

std::unique_ptr<EditableAsset> ProjectDocument::createEditable()
{
    return std::make_unique<EditableProject>(mProject, this);
}

} // namespace Tiled

#include "moc_projectdocument.cpp"
