/*
 * worldmanager.h
 * Copyright 2017-2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilededitor_global.h"

#include "worlddocument.h"

#include <QMap>
#include <QObject>

namespace Tiled {

class World;

class TILED_EDITOR_EXPORT WorldManager : public QObject
{
    Q_OBJECT

    WorldManager();
    ~WorldManager() override;

public:
    static WorldManager &instance();
    static void deleteInstance();

    WorldDocumentPtr findWorld(const QString &fileName) const;

    WorldDocumentPtr addEmptyWorld(const QString &fileName, QString *errorString);
    WorldDocumentPtr loadWorld(const QString &fileName, QString *errorString = nullptr);
    void loadWorlds(const QStringList &fileNames);
    void unloadWorld(const WorldDocumentPtr &worldDocument);
    void unloadAllWorlds();

    const QVector<WorldDocumentPtr> &worlds() const { return mWorldDocuments; }
    const QStringList worldFileNames() const;

    WorldDocumentPtr worldForMap(const QString &fileName) const;

signals:
    void worldsChanged();
    void worldLoaded(WorldDocument *worldDocument);
    void worldUnloaded(WorldDocument *worldDocument);

private:
    WorldDocumentPtr loadAndStoreWorld(const QString &fileName, QString *errorString = nullptr);

    QVector<WorldDocumentPtr> mWorldDocuments;

    static WorldManager *mInstance;
};

} // namespace Tiled
