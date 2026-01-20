/*
 * Enigma Tiled Plugin
 * Copyright 2022, Kartik Shrivastava <shrivastavakartik19@gmail.com>
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

#include "mapformat.h"
#include "gidmapper.h"
#include "egm.h"

#include "enigma_global.h"

namespace Enigma {

class ENIGMASHARED_EXPORT EnigmaPlugin : public Tiled::MapFormat
{
  Q_OBJECT
  Q_INTERFACES(Tiled::MapFormat)
  Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    EnigmaPlugin();

    std::unique_ptr<Tiled::Map> read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;
    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

private:
    QString mError;

    int readBackgrounds(buffers::TreeNode *root, std::unordered_map<std::string,
                        buffers::resources::Background *> &backgroundNamePtrMap);
    int readRoom(buffers::TreeNode *root, buffers::resources::EGMRoom *&egmRoom);
    void addTileset(const buffers::resources::EGMRoom_Tileset &tilesetProto, Tiled::Map *map, Tiled::GidMapper &mGidMapper,
                    std::unordered_map<std::string, buffers::resources::Background *> backgroundNamePtrMap);
    QString parseIntColor(int intColor);
    void addObjectGroup(const buffers::resources::EGMRoom_ObjectGroup &objGrp, Tiled::Map *map, Tiled::GidMapper &mGidMapper);
    void addObject(const buffers::resources::EGMRoom_ObjectGroup_Object &obj, Tiled::Layer *objectGroupLayer,
                   Tiled::GidMapper &mGidMapper);
    Tiled::Cell cellForGid(unsigned gid, Tiled::GidMapper &mGidMapper);
    void addTileLayer(const buffers::resources::EGMRoom_TileLayer &tileLayer, Tiled::Map *map, Tiled::GidMapper &mGidMapper);

};

}
