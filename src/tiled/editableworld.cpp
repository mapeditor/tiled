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
#include "worlddocument.h"
#include "worldmanager.h"

namespace Tiled {

ScriptWorld::ScriptWorld(World *world)
    : Object(*this)
    , world(world)
{
}

EditableWorld::EditableWorld(WorldDocument *worldDocument, QObject *parent)
    : EditableAsset(worldDocument, &mWorldObject, parent),
    mWorldObject(ScriptWorld(WorldManager::instance().worlds().value(worldDocument->fileName())))
{
}


ScriptWorldMapEntry::ScriptWorldMapEntry(World::MapEntry *mapEntry)
    : mMapEntry(mapEntry)
{

}

QString ScriptWorldMapEntry::fileName() const
{
    return mMapEntry->fileName;
}

QRect ScriptWorldMapEntry::rect() const
{
    return mMapEntry->rect;
}

QString EditableWorld::displayName() const
{
    return world()->displayName();
}

bool EditableWorld::containsMap(const QString &fileName)
{
    return world()->containsMap(fileName);
}

QVector<ScriptWorldMapEntry*> EditableWorld::allMaps() const
{
    QVector<ScriptWorldMapEntry*> maps;
    for (auto &entry : world()->allMaps())
        maps.append(new ScriptWorldMapEntry(&entry));
    return maps;
}

QVector<ScriptWorldMapEntry*> EditableWorld::mapsInRect(const QRect &rect) const
{
    QVector<ScriptWorldMapEntry*> maps;
    for (auto &entry : world()->mapsInRect(rect))
        maps.append(new ScriptWorldMapEntry(&entry));
    return maps;
}

bool EditableWorld::isReadOnly() const
{
    return !world()->canBeModified();
}

QSharedPointer<Document> EditableWorld::createDocument()
{
    // We don't currently support opening a world in its own tab, which this
    // function is meant for.
    return nullptr;
}

} // namespace Tiled
