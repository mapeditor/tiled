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

AddRemoveMapCommand::AddRemoveMapCommand(WorldDocument *worldDocument,
                                         const QString &mapName,
                                         const QRect &rect,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mWorldDocument(worldDocument)
    , mMapName(mapName)
    , mRect(rect)
{
}

void AddRemoveMapCommand::addMap()
{
    auto world = mWorldDocument->world();
    world->addMap(mMapName, mRect);
    emit mWorldDocument->worldChanged();
}

void AddRemoveMapCommand::removeMap()
{
    auto world = mWorldDocument->world();

    const int index = world->mapIndex(mMapName);
    if (index < 0)
        return;

    world->removeMap(index);
    emit mWorldDocument->worldChanged();
}


AddMapCommand::AddMapCommand(WorldDocument *worldDocument,
                             const QString &mapName,
                             const QRect &rect)
    : AddRemoveMapCommand(worldDocument, mapName, rect)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Map to World"));
}


RemoveMapCommand::RemoveMapCommand(WorldDocument *worldDocument, const QString &mapName)
    : AddRemoveMapCommand(worldDocument, mapName, worldDocument->world()->mapRect(mapName))
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Map from World"));
}

void RemoveMapCommand::redo()
{
    // ensure we're switching to a different map in case the current map is removed
    DocumentManager *manager = DocumentManager::instance();
    if (manager->currentDocument() && manager->currentDocument()->fileName() == mMapName) {
        for (const WorldMapEntry &entry : mWorldDocument->world()->allMaps()) {
            if (entry.fileName != mMapName) {
                manager->switchToDocument(entry.fileName);
                break;
            }
        }
    }

    removeMap();
}


SetMapRectCommand::SetMapRectCommand(WorldDocument *worldDocument,
                                     const QString &mapName,
                                     const QRect &rect)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Move Map"))
    , mWorldDocument(worldDocument)
    , mMapName(mapName)
    , mRect(rect)
    , mPreviousRect(mWorldDocument->world()->mapRect(mMapName))
{
}

void SetMapRectCommand::setMapRect(const QRect &rect)
{
    auto world = mWorldDocument->world();

    int index = world->mapIndex(mMapName);
    if (index < 0)
        return;

    world->setMapRect(index, rect);
    emit mWorldDocument->worldChanged();
}

bool SetMapRectCommand::mergeWith(const QUndoCommand *other)
{
    auto o = static_cast<const SetMapRectCommand *>(other);
    if (mWorldDocument != o->mWorldDocument || mMapName != o->mMapName)
        return false;

    mRect = o->mRect;
    setObsolete(childCount() == 0 && mRect == mPreviousRect);
    return true;
}


SetMapLabelCommand::SetMapLabelCommand(WorldDocument *worldDocument,
                                       const QString &mapName,
                                       const QString &label)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Set Map Label"))
    , mWorldDocument(worldDocument)
    , mMapName(mapName)
    , mLabel(label)
{
    const int index = worldDocument->world()->mapIndex(mapName);
    mPreviousLabel = (index >= 0) ? worldDocument->world()->maps.at(index).label : QString();
}

void SetMapLabelCommand::setLabel(const QString &label)
{
    auto world = mWorldDocument->world();
    const int index = world->mapIndex(mMapName);
    if (index < 0)
        return;
    world->setMapLabel(index, label);
    emit mWorldDocument->worldChanged();
}


SetMapPosInLoadedWorld::SetMapPosInLoadedWorld(const QString &worldFileName,
                                               const QString &mapName,
                                               const QPoint &from,
                                               const QPoint &to,
                                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mWorldFileName(worldFileName)
    , mMapName(mapName)
    , mFrom(from)
    , mTo(to)
{}

void SetMapPosInLoadedWorld::setRect(QPoint pos)
{
    auto worldDoc = WorldManager::instance().findWorld(mWorldFileName);
    if (!worldDoc)
        return;

    auto world = worldDoc->world();
    const int idx = world->mapIndex(mMapName);
    if (idx < 0)
        return;

    // Only apply when the current position matches the expected state, to
    // avoid clobbering manual world moves
    QRect rect = world->mapRect(mMapName);
    const QPoint expectedPos = (pos == mTo) ? mFrom : mTo;
    if (rect.topLeft() != expectedPos)
        return;

    rect.moveTo(pos);

    worldDoc->undoStack()->push(new SetMapRectCommand(worldDoc.data(), mMapName, rect));
}

} // namespace Tiled
