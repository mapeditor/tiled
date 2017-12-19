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
#include "plugin.h"
#include "mapformat.h"
#include "tilesetformat.h"

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
class LuaUtilWriter
{
public:
    void writeMapLua(LuaTableWriter &, const Tiled::Map *);
    void writeProperties(LuaTableWriter &, const Tiled::Properties &);
    void writeTilesetLua(LuaTableWriter &, const Tiled::Tileset &, unsigned firstGid, bool standalone=true);
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

protected:
    QDir mMapDir;
    Tiled::GidMapper mGidMapper;

};

class LUASHARED_EXPORT LuaPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;
};


class LUASHARED_EXPORT LuaMapFormat : public Tiled::WritableMapFormat, public LuaUtilWriter
{
    Q_OBJECT
//    Q_INTERFACES(Tiled::MapFormat)

public:
    LuaMapFormat(QObject *parent = nullptr) : Tiled::WritableMapFormat(parent) {}

    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Map *map, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;

};


class LUASHARED_EXPORT LuaTilesetFormat : public Tiled::WritableTilesetFormat, public LuaUtilWriter
{
    Q_OBJECT
//    Q_INTERFACES(Tiled::WritableTilesetFormat)

public:
    LuaTilesetFormat(QObject *parent = nullptr) : Tiled::WritableTilesetFormat(parent) {}

    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Tileset &tileset, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;

};



} // namespace Lua
