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

#ifndef LUAPLUGIN_H
#define LUAPLUGIN_H

#include "lua_global.h"

#include "gidmapper.h"
#include "mapwriterinterface.h"

#include <QDir>
#include <QObject>

namespace Tiled {
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
class LUASHARED_EXPORT LuaPlugin : public QObject,
                                   public Tiled::MapWriterInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapWriterInterface)

public:
    LuaPlugin();

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);
    QString nameFilter() const;
    QString errorString() const;

private:
    void writeMap(LuaTableWriter &, const Tiled::Map *);
    void writeProperties(LuaTableWriter &, const Tiled::Properties &);
    void writeTileset(LuaTableWriter &, const Tiled::Tileset *, uint firstGid);
    void writeTileLayer(LuaTableWriter &, const Tiled::TileLayer *);
    void writeObjectGroup(LuaTableWriter &, const Tiled::ObjectGroup *);
    void writeMapObject(LuaTableWriter &, const Tiled::MapObject *);

    QString mError;
    QDir mMapDir;     // The directory in which the map is being saved
    Tiled::GidMapper mGidMapper;
};

} // namespace Lua

#endif // LUAPLUGIN_H
