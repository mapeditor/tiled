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

#include <QFileInfo>

namespace Tiled {

WorldManager *WorldManager::mInstance;

WorldManager::WorldManager() = default;
WorldManager::~WorldManager() = default;

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

/**
 * Returns a loaded world with the given \a fileName, or null if no such
 * world is loaded.
 */
WorldDocumentPtr WorldManager::findWorld(const QString &fileName) const
{
    const auto canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    for (auto &worldDocument : mWorldDocuments)
        if (worldDocument->canonicalFilePath() == canonicalFilePath)
            return worldDocument;
    return {};
}

WorldDocumentPtr WorldManager::addEmptyWorld(const QString &fileName, QString *errorString)
{
    if (findWorld(fileName)) {
        if (errorString)
            *errorString = QLatin1String("World already loaded");
        return nullptr;
    }

    auto world = std::make_unique<World>();
    world->fileName = fileName;
    auto worldDocument = WorldDocumentPtr::create(std::move(world));

    if (worldDocument->save(worldDocument->fileName(), errorString)) {
        mWorldDocuments.append(worldDocument);

        connect(worldDocument.data(), &WorldDocument::worldChanged,
                this, [this] { emit worldsChanged(); });

        emit worldLoaded(worldDocument.data());
        emit worldsChanged();
        return worldDocument;
    }

    return {};
}

/**
 * Loads the world with the given \a fileName.
 *
 * \returns the world if it was loaded successfully, optionally setting
 *          \a errorString when not.
 */
WorldDocumentPtr WorldManager::loadWorld(const QString &fileName, QString *errorString)
{
    auto worldDocument = findWorld(fileName);
    if (!worldDocument) {
        worldDocument = loadAndStoreWorld(fileName, errorString);
        if (worldDocument)
            emit worldsChanged();
    }
    return worldDocument;
}

WorldDocumentPtr WorldManager::loadAndStoreWorld(const QString &fileName, QString *errorString)
{
    auto worldDocument = findWorld(fileName);
    if (!worldDocument) {
        worldDocument = WorldDocument::load(fileName, errorString);
        if (worldDocument) {
            mWorldDocuments.append(worldDocument);

            connect(worldDocument.data(), &WorldDocument::worldChanged,
                    this, [this] { emit worldsChanged(); });

            emit worldLoaded(worldDocument.data());
        }
    }
    return worldDocument;
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

const QStringList WorldManager::worldFileNames() const
{
    QStringList fileNames;
    for (auto &world : mWorldDocuments)
        fileNames.append(world->fileName());
    return fileNames;
}

/**
 * Unloads the world with the given \a worldDocument.
 */
void WorldManager::unloadWorld(const WorldDocumentPtr &worldDocument)
{
    if (mWorldDocuments.removeOne(worldDocument)) {
        worldDocument.data()->disconnect(this);

        emit worldsChanged();
        emit worldUnloaded(worldDocument.data());
    }
}

/**
 * Unloads all worlds. Faster than calling unloadWorld for each loaded world,
 * because it emits worldsChanged only once.
 */
void WorldManager::unloadAllWorlds()
{
    if (mWorldDocuments.isEmpty())
        return;

    QVector<WorldDocumentPtr> worldDocuments;
    worldDocuments.swap(mWorldDocuments);

    for (auto &worldDocument : std::as_const(worldDocuments)) {
        worldDocument.data()->disconnect(this);
        emit worldUnloaded(worldDocument.data());
    }

    emit worldsChanged();
}

WorldDocumentPtr WorldManager::worldForMap(const QString &fileName) const
{
    if (!fileName.isEmpty())
        for (auto &worldDocument : mWorldDocuments)
            if (worldDocument->world()->containsMap(fileName))
                return worldDocument;

    return nullptr;
}

} // namespace Tiled

#include "moc_worldmanager.cpp"
