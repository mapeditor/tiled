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

#include "plugin.h"
#include "mapformat.h"
#include "tilesetformat.h"

namespace Lua {

/**
 * This plugin allows exporting maps as Lua files.
 */
class LUASHARED_EXPORT LuaPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;
};


class LUASHARED_EXPORT LuaMapFormat : public Tiled::WritableMapFormat
{
    Q_OBJECT

public:
    explicit LuaMapFormat(QObject *parent = nullptr)
        : WritableMapFormat(parent)
    {}

    bool write(const Tiled::Map *map, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;
};


class LUASHARED_EXPORT LuaTilesetFormat : public Tiled::WritableTilesetFormat
{
    Q_OBJECT

public:
    explicit LuaTilesetFormat(QObject *parent = nullptr)
        : WritableTilesetFormat(parent)
    {}

    bool write(const Tiled::Tileset &tileset, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;
};

} // namespace Lua
