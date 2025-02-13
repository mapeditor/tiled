/*
 * editableproject.h
 * Copyright 2023, dogboydog
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

#pragma once

#include "editableasset.h"
#include "project.h"

#include <QObject>

namespace Tiled {

class ProjectDocument;

class EditableProject final : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(QString extensionsPath READ extensionsPath)
    Q_PROPERTY(QString automappingRulesFile READ automappingRulesFile)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QStringList folders READ folders)

public:
    EditableProject(ProjectDocument *projectDocument, QObject *parent = nullptr);

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::Project; }

    QString extensionsPath() const;
    QString automappingRulesFile() const;
    QString fileName() const;
    QStringList folders() const;

    Project *project() const;

    QSharedPointer<Document> createDocument() override;

    static EditableProject *find(Project *project);
    static EditableProject *get(ProjectDocument *projectDocument);
};

inline Project *EditableProject::project() const
{
    return static_cast<Project*>(object());
}

inline EditableProject *EditableProject::find(Project *project)
{
    return static_cast<EditableProject*>(EditableObject::find(project));
}

} // namespace Tiled
