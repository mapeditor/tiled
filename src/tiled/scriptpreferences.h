/*
 * scriptpreferences.h
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QObject>

namespace Tiled {

/**
 * Exposes user preferences to the scripting API.
 */
class ScriptPreferences : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int mapTileWidth READ mapTileWidth WRITE setMapTileWidth)
    Q_PROPERTY(int mapTileHeight READ mapTileHeight WRITE setMapTileHeight)
    Q_PROPERTY(int tilesetTileWidth READ tilesetTileWidth WRITE setTilesetTileWidth)
    Q_PROPERTY(int tilesetTileHeight READ tilesetTileHeight WRITE setTilesetTileHeight)

public:
    explicit ScriptPreferences(QObject *parent = nullptr);

    int mapTileWidth() const;
    void setMapTileWidth(int width);

    int mapTileHeight() const;
    void setMapTileHeight(int height);

    int tilesetTileWidth() const;
    void setTilesetTileWidth(int width);

    int tilesetTileHeight() const;
    void setTilesetTileHeight(int height);
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptPreferences*)
