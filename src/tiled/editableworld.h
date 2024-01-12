/*
 * editableworld.h
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

#pragma once

#include "editableasset.h"
#include "worlddocument.h"
#include "worldmanager.h"

#include <QObject>

namespace Tiled {

/**
 * Wrapper which allows world structs to be used with the EditableAsset
 * class.
 */
class ScriptWorld : public Object
{
public:
    ScriptWorld(World *world);

    World *world;
};

/*
 * Exposes the World::MapEntry struct to scripting
 */
class ScriptWorldMapEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QRect rect READ rect CONSTANT)

public:
    ScriptWorldMapEntry(World::MapEntry *mapEntry);
    QString fileName() const;
    QRect rect() const;

private:
    World::MapEntry *mMapEntry;
};

/**
 * @brief The EditableWorld class provides access to Worlds via scripting.
 */
class EditableWorld final : public EditableAsset
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QVector<ScriptWorldMapEntry*> allMaps READ allMaps)

public:
    EditableWorld(WorldDocument *worldDocument, QObject *parent = nullptr);

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::World; }

    World *world() const;
    QString displayName() const;
    Q_INVOKABLE QVector<ScriptWorldMapEntry*> allMaps() const;
    Q_INVOKABLE QVector<ScriptWorldMapEntry*> mapsInRect(const QRect &rect) const;
    Q_INVOKABLE bool containsMap(const QString &fileName);

    QSharedPointer<Document> createDocument() override;

private:
    ScriptWorld mWorldObject;
};

inline World *EditableWorld::world() const
{
    return static_cast<ScriptWorld*>(object())->world;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableWorld*)
Q_DECLARE_METATYPE(Tiled::ScriptWorldMapEntry*)
