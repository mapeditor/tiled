/*
 * worlddocument.h
 * Copyright 2019, Nils Kübler <nils-kuebler@web.de>
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

#include "document.h"
#include "editableasset.h"

class WorldManager;

namespace Tiled {

/**
 * Represents an editable world document.
 */
class WorldDocument : public Document
{
    Q_OBJECT

public:
    WorldDocument(const QString &fileName, QObject *parent = nullptr);

    // Document interface
    QString displayName() const override;
    bool save(const QString &fileName, QString *error) override;

    FileFormat *writerFormat() const override { return nullptr; }

    std::unique_ptr<EditableAsset> createEditable() override;

    // Exporting not supported for worlds
    QString lastExportFileName() const override { return QString(); }
    void setLastExportFileName(const QString &) override {}
    FileFormat *exportFormat() const override { return nullptr; }
    void setExportFormat(FileFormat *) override {}

private:
    void onWorldsChanged();
    void onWorldReloaded(const QString &filename);
    void onWorldSaved(const QString &fileName);

    // Document interface
    bool isModifiedImpl() const override;
};

} // namespace Tiled
