/*
 * editableworld.cpp
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

#include "editableworld.h"

#include "changeworld.h"
#include "scriptmanager.h"
#include "worldmanager.h"

#include <QUndoStack>

namespace Tiled {

EditableWorld::EditableWorld(WorldDocument *worldDocument, QObject *parent)
    : EditableAsset(worldDocument, nullptr, parent)
{
    setObject(WorldManager::instance().worlds().value(worldDocument->fileName()));
}

bool EditableWorld::containsMap(const QString &fileName)
{
    return world()->containsMap(fileName);
}

QVector<WorldMapEntry> EditableWorld::maps() const
{
    return world()->maps;
}

QVector<WorldMapEntry> EditableWorld::mapsInRect(const QRect &rect)
{
    return world()->mapsInRect(rect);
}

QVector<WorldMapEntry> EditableWorld::allMaps() const
{
    return world()->allMaps();
}

QVector<WorldPattern> EditableWorld::patterns() const
{
    return world()->patterns;
}

bool EditableWorld::isReadOnly() const
{
    return !world()->canBeModified();
}

int EditableWorld::mapIndex(const QString &fileName) const
{
    return world()->mapIndex(fileName);
}

void EditableWorld::setMapRect(int mapIndex, const QRect &rect)
{
    if (mapIndex < 0 || mapIndex >= world()->maps.size()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    const QString &fileName = world()->maps.at(mapIndex).fileName;
    document()->undoStack()->push(new SetMapRectCommand(fileName, rect));
}

void EditableWorld::addMap(const QString &mapFileName, const QRect &rect)
{
    if (mapFileName.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return;
    }

    if (WorldManager::instance().worldForMap(mapFileName)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Map is already part of a loaded world"));
        return;
    }

    document()->undoStack()->push(new AddMapCommand(fileName(), mapFileName, rect));
}

void EditableWorld::removeMap(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= world()->maps.size()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    const QString &fileName = world()->maps.at(mapIndex).fileName;
    document()->undoStack()->push(new RemoveMapCommand(fileName));
}

bool EditableWorld::save()
{
    return WorldManager::instance().saveWorld(world()->fileName);
}

QSharedPointer<Document> EditableWorld::createDocument()
{
    // We don't currently support opening a world in its own tab, which this
    // function is meant for.
    return nullptr;
}

} // namespace Tiled
