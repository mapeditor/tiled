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

class World;

class WorldDocument;

using WorldDocumentPtr = QSharedPointer<WorldDocument>;

/**
 * Represents an editable world document.
 */
class WorldDocument final : public Document
{
    Q_OBJECT

public:
    explicit WorldDocument(std::unique_ptr<World> world, QObject *parent = nullptr);
    ~WorldDocument();

    // Document interface
    QString displayName() const override;
    bool save(const QString &fileName, QString *error) override;

    bool canReload() const override;
    bool reload(QString *error);

    /**
     * Loads a world and returns a WorldDocument instance on success. Returns
     * null on error and sets the \a error message.
     */
    static WorldDocumentPtr load(const QString &fileName,
                                 QString *error = nullptr);

    FileFormat *writerFormat() const override { return nullptr; }

    // Exporting not supported for worlds
    QString lastExportFileName() const override { return QString(); }
    void setLastExportFileName(const QString &) override {}
    FileFormat *exportFormat() const override { return nullptr; }
    void setExportFormat(FileFormat *) override {}

    World *world() const { return mWorld.get(); }

    void swapWorld(std::unique_ptr<World> &other);

signals:
    void worldChanged();

private:
    // Document interface
    std::unique_ptr<EditableAsset> createEditable() override;

    std::unique_ptr<World> mWorld;
};

} // namespace Tiled
