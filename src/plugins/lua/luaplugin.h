/*
 * Lua Tiled Plugin
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "lua_global.h"

#include "gidmapper.h"
#include "map.h"
#include "mapformat.h"

#include <QDir>
#include <QObject>

namespace Tiled {
class GroupLayer;
class MapObject;
class ObjectGroup;
class Properties;
class TileLayer;
class Tileset;
}

namespace Lua {

class LuaTableWriter;

/**
 * This plugin allows exporting maps as Lua files.
 */
class LUASHARED_EXPORT LuaPlugin : public Tiled::WritableMapFormat
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    LuaPlugin();

    bool write(const Tiled::Map *map, const QString &fileName) override;
    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

private:
    void writeMap(LuaTableWriter &, const Tiled::Map *);
    void writeProperties(LuaTableWriter &, const Tiled::Properties &);
    void writeTileset(LuaTableWriter &, const Tiled::Tileset *, unsigned firstGid);
    void writeLayers(LuaTableWriter &,
                     const QList<Tiled::Layer*> &layers,
                     Tiled::Map::LayerDataFormat format);
    void writeTileLayer(LuaTableWriter &, const Tiled::TileLayer *,
                        Tiled::Map::LayerDataFormat);
    void writeTileLayerData(LuaTableWriter &, const Tiled::TileLayer *,
                            Tiled::Map::LayerDataFormat format,
                            QRect bounds);
    void writeObjectGroup(LuaTableWriter &, const Tiled::ObjectGroup *,
                          const QByteArray &key = QByteArray());
    void writeImageLayer(LuaTableWriter &, const Tiled::ImageLayer *);
    void writeGroupLayer(LuaTableWriter &, const Tiled::GroupLayer *,
                         Tiled::Map::LayerDataFormat);
    void writeMapObject(LuaTableWriter &, const Tiled::MapObject *);
    void writePolygon(LuaTableWriter &, const Tiled::MapObject *);
    void writeTextProperties(LuaTableWriter &, const Tiled::MapObject *);

    QString mError;
    QDir mMapDir;     // The directory in which the map is being saved
    Tiled::GidMapper mGidMapper;
};

} // namespace Lua
