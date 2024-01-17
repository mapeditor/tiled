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
#include "world.h"
#include "worlddocument.h"

#include <QObject>

namespace Tiled {

/*
 * Exposes the World::MapEntry struct to scripting
 */
class ScriptWorldMapEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QRect rect READ rect CONSTANT)

public:
    ScriptWorldMapEntry(World::MapEntry mapEntry)
        : mMapEntry(mapEntry)
    {}

    const QString &fileName() const { return mMapEntry.fileName; }
    QRect rect() const { return mMapEntry.rect; }

private:
    World::MapEntry mMapEntry;
};
/*
 * Exposes the World::Pattern struct to scripting
 */
class ScriptWorldPattern : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString regExp READ regExp CONSTANT);
    Q_PROPERTY(int multiplierX READ multiplierX CONSTANT);
    Q_PROPERTY(int multiplierY READ multiplierY CONSTANT);
    Q_PROPERTY(QPoint offset READ offset CONSTANT);
    Q_PROPERTY(QSize mapSize READ mapSize CONSTANT);

public:
    ScriptWorldPattern (World::Pattern pattern)
        : mPattern(pattern)
    {}

    QString regExp() { return mPattern.regexp.pattern(); }
    const int multiplierX() const { return mPattern.multiplierX; }
    const int multiplierY() const { return mPattern.multiplierY; }
    const QPoint offset() const { return mPattern.offset; }
    const QSize mapSize() const { return mPattern.mapSize; }

private:
    World::Pattern mPattern;
};
/**
 * @brief The EditableWorld class provides access to Worlds via scripting.
 */
class EditableWorld final : public EditableAsset
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName)
    Q_PROPERTY(QList<ScriptWorldMapEntry*> maps READ maps)

public:
    EditableWorld(WorldDocument *worldDocument, QObject *parent = nullptr);

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::World; }

    World *world() const;
    QString displayName() const;

    Q_INVOKABLE QList<ScriptWorldMapEntry*> maps() const;
    Q_INVOKABLE QList<ScriptWorldMapEntry*> mapsInRect(const QRect &rect);
    Q_INVOKABLE QList<ScriptWorldMapEntry*> allMaps() const;
    Q_INVOKABLE QList<ScriptWorldPattern*> patterns() const;
    Q_INVOKABLE bool containsMap(const QString &fileName);
    Q_INVOKABLE int mapIndex(const QString &fileName) const;
    Q_INVOKABLE void setMapRect(int mapIndex, const QRect &rect);
    Q_INVOKABLE void addMap(const QString &mapFileName, const QRect &rect);
    Q_INVOKABLE void removeMap(int mapIndex);
    Q_INVOKABLE bool save();

    QSharedPointer<Document> createDocument() override;
};

inline World *EditableWorld::world() const
{
    return static_cast<World*>(object());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableWorld*)
Q_DECLARE_METATYPE(Tiled::ScriptWorldMapEntry*)
Q_DECLARE_METATYPE(Tiled::ScriptWorldPattern*)
