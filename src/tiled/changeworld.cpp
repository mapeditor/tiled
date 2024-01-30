/*
 * changeworld.cpp
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changeworld.h"

#include "documentmanager.h"
#include "world.h"
#include "worldmanager.h"

#include <QCoreApplication>

namespace Tiled {

AddMapCommand::AddMapCommand(const QString &worldName, const QString &mapName, const QRect &rect)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Add Map to World"))
    , mWorldName(worldName)
    , mMapName(mapName)
    , mRect(rect)
{
}

void AddMapCommand::undo()
{
    WorldManager::instance().removeMap(mMapName);
}

void AddMapCommand::redo()
{
    WorldManager::instance().addMap(mWorldName, mMapName, mRect);
}


RemoveMapCommand::RemoveMapCommand(const QString &mapName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Remove Map from World"))
    , mMapName(mapName)
{
    const WorldManager &manager = WorldManager::instance();
    const World *world = manager.worldForMap(mMapName);
    mPreviousRect = world->mapRect(mMapName);
    mWorldName = world->fileName;
}

void RemoveMapCommand::undo()
{
    WorldManager::instance().addMap(mWorldName, mMapName, mPreviousRect);
}

void RemoveMapCommand::redo()
{
    // ensure we're switching to a different map in case the current map is removed
    DocumentManager *manager = DocumentManager::instance();
    if (manager->currentDocument() && manager->currentDocument()->fileName() == mMapName) {
        const World *world = WorldManager::instance().worldForMap(mMapName);
        for (const WorldMapEntry &entry : world->allMaps()) {
            if (entry.fileName != mMapName) {
                manager->switchToDocument(entry.fileName);
                break;
            }
        }
    }
    WorldManager::instance().removeMap(mMapName);
}


SetMapRectCommand::SetMapRectCommand(const QString &mapName, QRect rect)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Move Map"))
    , mMapName(mapName)
    , mRect(rect)
{
    const WorldManager &manager = WorldManager::instance();
    mPreviousRect = manager.worldForMap(mMapName)->mapRect(mMapName);
}

void SetMapRectCommand::undo()
{
    WorldManager::instance().setMapRect(mMapName, mPreviousRect);
}

void SetMapRectCommand::redo()
{
    WorldManager::instance().setMapRect(mMapName, mRect);
}

} // namespace Tiled
