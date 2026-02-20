/*
 * scriptpreferences.cpp
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptpreferences.h"

#include "sessionoptions.h"

#include <QSize>

namespace Tiled {

ScriptPreferences::ScriptPreferences(QObject *parent)
    : QObject(parent)
{
}

int ScriptPreferences::mapTileWidth() const
{
    return session::mapTileWidth.get();
}

void ScriptPreferences::setMapTileWidth(int width)
{
    session::mapTileWidth.set(width);
}

int ScriptPreferences::mapTileHeight() const
{
    return session::mapTileHeight.get();
}

void ScriptPreferences::setMapTileHeight(int height)
{
    session::mapTileHeight.set(height);
}

int ScriptPreferences::tilesetTileWidth() const
{
    return session::tileSize.get().width();
}

void ScriptPreferences::setTilesetTileWidth(int width)
{
    QSize size = session::tileSize.get();
    size.setWidth(width);
    session::tileSize.set(size);
}

int ScriptPreferences::tilesetTileHeight() const
{
    return session::tileSize.get().height();
}

void ScriptPreferences::setTilesetTileHeight(int height)
{
    QSize size = session::tileSize.get();
    size.setHeight(height);
    session::tileSize.set(size);
}

} // namespace Tiled

#include "moc_scriptpreferences.cpp"
