/*
 * projectdocument.h
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

#pragma once

#include "document.h"
#include "project.h"
#include "editableproject.h"

namespace Tiled {

class ProjectDocument : public Document
{
    Q_OBJECT

public:
    ProjectDocument(Project *project);
    QString displayName() const override;
    FileFormat *writerFormat() const override;
    bool save(const QString&, QString*) override;
    void setExportFormat(FileFormat *format) override;
    FileFormat *exportFormat() const override;
    QString lastExportFileName() const override;
    void setLastExportFileName(const QString &fileName) override;
    std::unique_ptr<EditableAsset> createEditable() override;

private:
    Project *mProject;
};

using ProjectDocumentPtr = QSharedPointer<ProjectDocument>;
} // namespace Tiled
