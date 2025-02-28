/*
 * Remixed Dungeon Tiled Plugin
 * Copyright 2025, Mikhael Danilov <mikhael.danilov@gmail.com>
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

#include "rpd_global.h"

#include "layer.h"
#include "mapformat.h"
#include "plugin.h"

#include <QObject>

namespace Tiled {
class Map;
}

namespace Rpd {

class RPDSHARED_EXPORT RpdPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;
};


class RPDSHARED_EXPORT RpdMapFormat : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    enum TileId {
        ENTRANCE = 7,
        EXIT = 8,
        LOCKED_EXIT = 25,
        UNLOCKED_EXIT = 26
    };

    explicit RpdMapFormat(QObject *parent = nullptr);

    bool write(const Tiled::Map *map, const QString &fileName, Options options = Options()) override;
    QString shortName() const override;
    QString nameFilter() const override;
    QString errorString() const override;

protected:
    QString mLatestError;

private:
    bool insertTilesetFile(const Tiled::Layer &layer, const QString &tiles_name, QJsonObject &mapJson);
    bool validateMap(const Tiled::Map *map);
    void validateAndWriteProperties(const Tiled::Map *map, QJsonObject &mapJson);
    bool writeLogicLayer(QJsonObject &mapJson, const Tiled::TileLayer &layer);
    bool writeDecoLayer(QJsonObject &mapJson, const Tiled::TileLayer &layer);
    void writeObjectLayer(QJsonObject &mapJson, const Tiled::ObjectGroup &objectLayer);

    void logError(const QString &msg);
};

} // namespace Rpd
