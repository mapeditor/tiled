/*
 * editableproject.cpp
 * Copyright 2023, Chris Boehm AKA dogboydog
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableproject.h"
#include "projectdocument.h"

namespace Tiled {

EditableProject::EditableProject(Project *project, QObject *parent)
    : EditableAsset(nullptr, nullptr, parent)
    , mProject(project)
{
}

QString EditableProject::extensionsPath() const
{
   return mProject->mExtensionsPath;
}

QString EditableProject::automappingRulesFile() const
{
   return mProject->mAutomappingRulesFile;
}

QString EditableProject::fileName() const
{
    return mProject->fileName();
}

QStringList EditableProject::folders() const
{
   return mProject->folders();
}

bool EditableProject::isReadOnly() const
{
   return false;
}
QSharedPointer<Document> EditableProject:createDocument()
{
    return ProjectDocumentPtr::create(mProject);
}
} // namespace Tiled

#include "moc_editableproject.cpp"
