/*
 * worldmanager.cpp
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

#include "worldmanager.h"

#include "world.h"

namespace Tiled {

WorldManager *WorldManager::mInstance;

WorldManager::WorldManager()
{
    mIgnoreFileChangeEventForFile.clear();
    connect(&mFileSystemWatcher, &FileSystemWatcher::pathsChanged,
            this, &WorldManager::reloadWorldFiles);
}

WorldManager::~WorldManager()
{
    qDeleteAll(mWorlds);
}

WorldManager &WorldManager::instance()
{
    if (!mInstance)
        mInstance = new WorldManager;

    return *mInstance;
}

void WorldManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

void WorldManager::reloadWorldFiles(const QStringList &fileNames)
{
    bool changed = false;

    for (const QString &fileName : fileNames) {

        if (mWorlds.contains(fileName)) {

            if (mIgnoreFileChangeEventForFile == fileName) {
                mIgnoreFileChangeEventForFile.clear();
                continue;
            }

            if (auto world = World::load(fileName)) {
                std::unique_ptr<World> oldWorld { mWorlds.take(fileName) };
                oldWorld->clearErrorsAndWarnings();

                mWorlds.insert(fileName, world.release());

                changed = true;
                emit worldReloaded(fileName);
            }
        }
    }

    if (changed)
        emit worldsChanged();
}

World *WorldManager::addEmptyWorld(const QString &fileName, QString *errorString)
{
    if (mWorlds.contains(fileName)) {
        if (errorString)
            *errorString = QLatin1String("World already loaded");
        return nullptr;
    }

    auto world = std::make_unique<World>();
    world->fileName = fileName;

    if (saveWorld(*world, errorString)) {
        mWorlds.insert(fileName, world.release());
        mFileSystemWatcher.addPath(fileName);
        emit worldLoaded(fileName);
        emit worldsChanged();
        return mWorlds.value(fileName);
    }

    return nullptr;
}

/**
 * Loads the world with the given \a fileName.
 *
 * \returns the world if it was loaded successfully, optionally setting
 *          \a errorString when not.
 */
World *WorldManager::loadWorld(const QString &fileName, QString *errorString)
{
    auto world = mWorlds.value(fileName);
    if (!world) {
        world = loadAndStoreWorld(fileName, errorString);
        if (world)
            emit worldsChanged();
    }
    return world;
}

World *WorldManager::loadAndStoreWorld(const QString &fileName, QString *errorString)
{
    auto world = World::load(fileName, errorString);
    if (!world)
        return nullptr;

    if (mWorlds.contains(fileName))
        delete mWorlds.take(fileName);
    else
        mFileSystemWatcher.addPath(fileName);

    mWorlds.insert(fileName, world.release());
    emit worldLoaded(fileName);

    return mWorlds.value(fileName);
}

/**
 * Loads all given worlds. Faster than calling loadWorld individually,
 * because it emits worldsChanged only once.
 */
void WorldManager::loadWorlds(const QStringList &fileNames)
{
    bool anyWorldLoaded = false;

    for (const QString &fileName : fileNames)
        if (loadAndStoreWorld(fileName))
            anyWorldLoaded = true;

    if (anyWorldLoaded)
        emit worldsChanged();
}

bool WorldManager::saveWorld(const QString &fileName, QString *errorString)
{
    World *savingWorld = nullptr;

    for (auto world : std::as_const(mWorlds)) {
        if (world->fileName == fileName) {
            savingWorld = world;
            break;
        }
    }

    if (!savingWorld) {
        if (errorString)
            *errorString = tr("World not found");
        return false;
    }

    return saveWorld(*savingWorld, errorString);
}

bool WorldManager::saveWorld(World &world, QString *errorString)
{
    mIgnoreFileChangeEventForFile = world.fileName;

    if (World::save(world, errorString)) {
        emit worldSaved(world.fileName);
        return true;
    }

    return false;
}

/**
 * Unloads the world with the given \a fileName.
 */
void WorldManager::unloadWorld(const QString &fileName)
{
    std::unique_ptr<World> world { mWorlds.take(fileName) };
    if (world) {
        mFileSystemWatcher.removePath(fileName);
        emit worldsChanged();
        emit worldUnloaded(fileName);
    }
}

/**
 * Unloads all worlds. Faster than calling unloadWorld for each loaded world,
 * because it emits worldsChanged only once.
 */
void WorldManager::unloadAllWorlds()
{
    if (mWorlds.isEmpty())
        return;

    QMap<QString, World*> worlds;
    worlds.swap(mWorlds);

    for (World *world : std::as_const(worlds)) {
        emit worldUnloaded(world->fileName);
        delete world;
    }

    mFileSystemWatcher.clear();
    emit worldsChanged();
}

const World *WorldManager::worldForMap(const QString &fileName) const
{
    if (!fileName.isEmpty())
        for (auto world : mWorlds)
            if (world->containsMap(fileName))
                return world;

    return nullptr;
}

bool WorldManager::mapCanBeModified(const QString &fileName) const
{
    for (auto world : mWorlds) {
        if (!world->canBeModified())
            continue;

        int index = world->mapIndex(fileName);
        if (index >= 0)
            return true;
    }
    return false;
}

void WorldManager::setMapRect(const QString &fileName, const QRect &rect)
{
    for (auto world : std::as_const(mWorlds)) {
        int index = world->mapIndex(fileName);
        if (index < 0)
            continue;

        world->setMapRect(index, rect);
        emit worldsChanged();
        break;
    }
}

bool WorldManager::removeMap(const QString &fileName)
{
    for (auto world : std::as_const(mWorlds)) {
        int index = world->mapIndex(fileName);
        if (index < 0)
            continue;

        world->removeMap(index);
        emit worldsChanged();
        return true;
    }

    return false;
}

bool WorldManager::addMap(const QString &worldFileName, const QString &mapFileName, const QRect &rect)
{
    Q_ASSERT(!mapFileName.isEmpty());

    if (worldForMap(mapFileName))
        return false;

    for (auto world : std::as_const(mWorlds)) {
        if (world->fileName == worldFileName) {
            world->addMap(mapFileName, rect);
            emit worldsChanged();
            return true;
        }
    }
    return false;
}

} // namespace Tiled

#include "moc_worldmanager.cpp"
