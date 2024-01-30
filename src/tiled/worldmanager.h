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

#include "filesystemwatcher.h"

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

    World *addEmptyWorld(const QString &fileName, QString *errorString);
    World *loadWorld(const QString &fileName, QString *errorString = nullptr);
    void loadWorlds(const QStringList &fileNames);
    void unloadWorld(const QString &fileName);
    void unloadAllWorlds();
    bool saveWorld(const QString &fileName, QString *errorString = nullptr);

    const QMap<QString, World*> &worlds() const { return mWorlds; }

    const World *worldForMap(const QString &fileName) const;

    void setMapRect(const QString &fileName, const QRect &rect);
    bool mapCanBeModified(const QString &fileName) const;
    bool removeMap(const QString &fileName);
    bool addMap(const QString &worldFileName, const QString &mapFileName, const QRect &rect);

signals:
    void worldsChanged();
    void worldLoaded(const QString &fileName);
    void worldReloaded(const QString &fileName);
    void worldUnloaded(const QString &fileName);
    void worldSaved(const QString &fileName);

private:
    bool saveWorld(World &world, QString *errorString = nullptr);
    World *loadAndStoreWorld(const QString &fileName, QString *errorString = nullptr);
    void reloadWorldFiles(const QStringList &fileNames);

    QMap<QString, World*> mWorlds;

    FileSystemWatcher mFileSystemWatcher;
    QString mIgnoreFileChangeEventForFile;

    static WorldManager *mInstance;
};

} // namespace Tiled
