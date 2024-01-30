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
#include "editablemap.h"
#include "world.h"
#include "worlddocument.h"

namespace Tiled {

/**
 * @brief The EditableWorld class provides access to Worlds via scripting.
 */
class EditableWorld final : public EditableAsset
{
    Q_OBJECT
    Q_PROPERTY(QVector<WorldMapEntry> maps READ maps)
    Q_PROPERTY(QVector<WorldPattern> patterns READ patterns)

public:
    EditableWorld(WorldDocument *worldDocument, QObject *parent = nullptr);

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::World; }

    World *world() const;

    QVector<WorldMapEntry> maps() const;
    QVector<WorldPattern> patterns() const;

    Q_INVOKABLE QVector<WorldMapEntry> mapsInRect(const QRect &rect) const;
    Q_INVOKABLE QVector<WorldMapEntry> allMaps() const;
    Q_INVOKABLE bool containsMap(const QString &fileName) const;
    Q_INVOKABLE bool containsMap(EditableMap *fileName) const;
    Q_INVOKABLE void setMapRect(const QString &mapFileName, const QRect &rect);
    Q_INVOKABLE void setMapPos(EditableMap *map, int x, int y);
    Q_INVOKABLE void addMap(const QString &mapFileName, const QRect &rect);
    Q_INVOKABLE void addMap(EditableMap *map, int x, int y);
    Q_INVOKABLE void removeMap(const QString &mapFileName);
    Q_INVOKABLE void removeMap(EditableMap *map);

    QSharedPointer<Document> createDocument() override;
};

inline World *EditableWorld::world() const
{
    return static_cast<World*>(object());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableWorld*)
