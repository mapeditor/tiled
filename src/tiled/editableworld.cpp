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
#include "maprenderer.h"
#include "scriptmanager.h"
#include "worldmanager.h"

#include <QUndoStack>

namespace Tiled {

EditableWorld::EditableWorld(WorldDocument *worldDocument, QObject *parent)
    : EditableAsset(nullptr, parent)
{
    setObject(WorldManager::instance().worlds().value(worldDocument->fileName()));
    setDocument(worldDocument);
}

bool EditableWorld::containsMap(const QString &fileName) const
{
    return world()->containsMap(fileName);
}

bool EditableWorld::containsMap(EditableMap *map) const
{
    if (!map) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }

    if (map->fileName().isEmpty()) {
        return false;
    }

    return containsMap(map->fileName());
}

QVector<WorldMapEntry> EditableWorld::maps() const
{
    return world()->maps;
}

QVector<WorldMapEntry> EditableWorld::mapsInRect(const QRect &rect) const
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

void EditableWorld::setMapRect(const QString &mapFileName, const QRect &rect)
{
    const int mapIndex = world()->mapIndex(mapFileName);
    if (mapIndex < 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Map not found in this world"));
        return;
    }

    document()->undoStack()->push(new SetMapRectCommand(mapFileName, rect));
}

void EditableWorld::setMapPos(EditableMap *map, int x, int y)
{
    if (!map) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    const int mapIndex = world()->mapIndex(map->fileName());
    if (mapIndex < 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Map not found in this world"));
        return;
    }

    QRect rect = world()->maps.at(mapIndex).rect;
    rect.moveTo(x, y);
    document()->undoStack()->push(new SetMapRectCommand(map->fileName(), rect));
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

void EditableWorld::addMap(EditableMap *map, int x, int y)
{
    if (!map) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    if (map->fileName().isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't add unsaved map to a world"));
        return;
    }

    const QSize size = MapRenderer::create(map->map())->mapBoundingRect().size();
    addMap(map->fileName(), QRect(QPoint(x, y), size));
}

void EditableWorld::removeMap(const QString &mapFileName)
{
    const int mapIndex = world()->mapIndex(mapFileName);
    if (mapIndex < 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Map not found in this world"));
        return;
    }

    document()->undoStack()->push(new RemoveMapCommand(mapFileName));
}

void EditableWorld::removeMap(EditableMap *map)
{
    if (!map) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    removeMap(map->fileName());
}

QSharedPointer<Document> EditableWorld::createDocument()
{
    // We don't currently support opening a world in its own tab, which this
    // function is meant for.
    return nullptr;
}

} // namespace Tiled

#include "moc_editableworld.cpp"
