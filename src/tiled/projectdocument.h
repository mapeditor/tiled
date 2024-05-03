/*
 * projectdocument.h
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

#pragma once

#include "document.h"
#include "project.h"

namespace Tiled {

class ProjectDocument : public Document
{
    Q_OBJECT

public:
    ProjectDocument(std::unique_ptr<Project> project, QObject *parent = nullptr);
    ~ProjectDocument() override;

    QString displayName() const override;
    FileFormat *writerFormat() const override;
    bool save(const QString &fileName, QString *error) override;
    void setExportFormat(FileFormat *format) override;
    FileFormat *exportFormat() const override;
    QString lastExportFileName() const override;
    void setLastExportFileName(const QString &fileName) override;
    std::unique_ptr<EditableAsset> createEditable() override;

    Project &project() { return *mProject; }

private:
    std::unique_ptr<Project> mProject;
};

using ProjectDocumentPtr = QSharedPointer<ProjectDocument>;

} // namespace Tiled
