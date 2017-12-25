/*
 * TBIN Tiled Plugin
 * Copyright 2017, Chase Warrington <spacechase0.and.cat@gmail.com>
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

#include "tbinplugin.h"

#include "tbin/Map.hpp"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tiled.h"
#include "tilelayer.h"

#include <cmath>
#include <fstream>
#include <map>
#include <QDebug>
#include <QDir>
#include <sstream>

namespace
{
    void tbinToTiledProperties(const tbin::Properties& props, Tiled::Object* obj)
    {
        for (const std::pair< std::string, tbin::PropertyValue >& prop : props) {
            if (prop.first[0] == '@')
                continue;
            switch (prop.second.type) {
                case tbin::PropertyValue::String:
                    obj->setProperty(prop.first.c_str(), QVariant(prop.second.dataStr.c_str()));
                    break;

                case tbin::PropertyValue::Bool:
                    obj->setProperty(prop.first.c_str(), QVariant(prop.second.data.b));
                    break;

                case tbin::PropertyValue::Float:
                    obj->setProperty(prop.first.c_str(), QVariant(prop.second.data.f));
                    break;

                case tbin::PropertyValue::Integer:
                    obj->setProperty(prop.first.c_str(), QVariant(prop.second.data.i));
                    break;
            }
        }
    }

    void tiledToTbinProperties(const Tiled::Object* obj, tbin::Properties& props)
    {
        for (auto it = obj->properties().cbegin(); it != obj->properties().cend(); ++it) {
            tbin::PropertyValue prop;
            if (it.value().type() == QVariant::Bool) {
                prop.type = tbin::PropertyValue::Bool;
                prop.data.b = it.value().toBool();
            }
            else if ((QMetaType::Type)it.value().type() == QMetaType::Float) {
                prop.type = tbin::PropertyValue::Float;
                prop.data.f = it.value().toFloat();
            }
            else if (it.value().type() == QVariant::Int) {
                prop.type = tbin::PropertyValue::Integer;
                prop.data.i = it.value().toInt();
            }
            else if (it.value().type() == QVariant::String) {
                prop.type = tbin::PropertyValue::String;
                prop.dataStr = it.value().toString().toStdString();
            }
            else throw std::invalid_argument(QT_TRANSLATE_NOOP("TbinMapFormat", "Unsupported property type"));
            props.insert(std::make_pair(it.key().toStdString(), prop));
        }
    }
}

namespace Tbin {

void TbinPlugin::initialize()
{
    addObject(new TbinMapFormat(this));
}


TbinMapFormat::TbinMapFormat(QObject *)
{
}

Tiled::Map *TbinMapFormat::read(const QString &fileName)
{
    std::ifstream file( fileName.toStdString(), std::ios::in | std::ios::binary );
    if (!file) {
        mError = tr("Could not open file for reading.");
        return nullptr;
    }

    tbin::Map tmap;
    Tiled::Map* map = nullptr;
    try
    {
        tmap.loadFromStream(file);
        map = new Tiled::Map(Tiled::Map::Orthogonal, tmap.layers[0].layerSize.x, tmap.layers[0].layerSize.y, tmap.layers[0].tileSize.x, tmap.layers[0].tileSize.y);
        tbinToTiledProperties(tmap.props, map);

        const QDir fileDir(QFileInfo(fileName).dir());

        std::map< std::string, int > tmapTilesheetMapping;
        for (std::size_t i = 0; i < tmap.tilesheets.size(); ++i) {
            const tbin::TileSheet& ttilesheet = tmap.tilesheets[i];
            tmapTilesheetMapping[ttilesheet.id] = i;

            if (ttilesheet.spacing.x != ttilesheet.spacing.y)
                throw std::invalid_argument(QT_TR_NOOP("Tilesheet must have equal spacings."));
            if (ttilesheet.margin.x != ttilesheet.margin.y)
                throw std::invalid_argument(QT_TR_NOOP("Tilesheet must have equal margins."));

            auto tilesheet = Tiled::Tileset::create(ttilesheet.id.c_str(), ttilesheet.tileSize.x, ttilesheet.tileSize.y, ttilesheet.spacing.x, ttilesheet.margin.x);
            tilesheet->setImageSource(Tiled::toUrl(QString::fromStdString(ttilesheet.image), fileDir));
            if (!tilesheet->loadImage()) {
                QList<Tiled::Tile*> tiles;
                for (int i = 0; i < ttilesheet.sheetSize.x * ttilesheet.sheetSize.y; ++i) {
                    tiles.append(new Tiled::Tile(i, tilesheet.data()));
                }
                tilesheet->addTiles(tiles);
            }
            tbinToTiledProperties(ttilesheet.props, tilesheet.data());

            for (auto prop : ttilesheet.props) {
                if (prop.first[0] != '@')
                    continue;

                QStringList strs = QString(prop.first.c_str()).split('@');
                if (strs[1] == "TileIndex") {
                    int index = strs[2].toInt();
                    tbin::Properties dummyProps;
                    dummyProps.insert(std::make_pair(strs[3].toStdString(), prop.second));
                    Tiled::Tile* tile = tilesheet->tileAt(index);
                    tbinToTiledProperties(dummyProps, tile);
                }
                // TODO: 'AutoTile' ?
                // Purely for map making. Appears to be similar to terrains
                // (In tIDE, right click a tilesheet and choose "Auto Tiles..."
            }

            map->addTileset(tilesheet);
        }
        for (const tbin::Layer& tlayer : tmap.layers) {
            if (tlayer.tileSize.x != tmap.layers[0].tileSize.x || tlayer.tileSize.y != tmap.layers[0].tileSize.y)
                throw std::invalid_argument(QT_TR_NOOP("Different tile sizes per layer are not supported."));

            Tiled::TileLayer* layer = new Tiled::TileLayer(tlayer.id.c_str(), 0, 0, tlayer.layerSize.x, tlayer.layerSize.y);
            tbinToTiledProperties(tlayer.props, layer);
            Tiled::ObjectGroup* objects = new Tiled::ObjectGroup(tlayer.id.c_str(), 0, 0);
            for (std::size_t i = 0; i < tlayer.tiles.size(); ++i) {
                const tbin::Tile& ttile = tlayer.tiles[i];
                int ix = i % tlayer.layerSize.x;
                int iy = i / tlayer.layerSize.x;

                if (ttile.isNullTile())
                    continue;

                Tiled::Cell cell;
                if (ttile.animatedData.frames.size() > 0) {
                    tbin::Tile tfirstTile = ttile.animatedData.frames[0];
                    Tiled::Tile* firstTile = map->tilesetAt(tmapTilesheetMapping[tfirstTile.tilesheet])->tileAt(tfirstTile.staticData.tileIndex);
                    QVector<Tiled::Frame> frames;
                    for (const tbin::Tile& tframe : ttile.animatedData.frames) {
                        if (tframe.isNullTile() || tframe.animatedData.frames.size() > 0 ||
                             tframe.tilesheet != tfirstTile.tilesheet)
                            throw std::invalid_argument(QT_TR_NOOP("Invalid animation frame."));

                        Tiled::Frame frame;
                        frame.tileId = tframe.staticData.tileIndex;
                        frame.duration = ttile.animatedData.frameInterval;
                        frames.append(frame);
                    }
                    firstTile->setFrames(frames);
                    cell = Tiled::Cell(firstTile);
                }
                else {
                    cell = Tiled::Cell(map->tilesetAt(tmapTilesheetMapping[ttile.tilesheet])->tileAt(ttile.staticData.tileIndex));
                }
                layer->setCell(ix, iy, cell);

                if (ttile.props.size() > 0) {
                    Tiled::MapObject* obj = new Tiled::MapObject("TileData", QString(), QPointF(ix * tlayer.tileSize.x, iy * tlayer.tileSize.y), QSizeF(tlayer.tileSize.x, tlayer.tileSize.y));
                    tbinToTiledProperties(ttile.props, obj);
                    objects->addObject(obj);
                }
            }
            map->addLayer(layer);
            map->addLayer(objects);
        }
    }
    catch (std::exception& e) {
        mError = tr((std::string("Exception: ") + e.what()).c_str());
    }

    return map;
}

bool TbinMapFormat::write(const Tiled::Map *map, const QString &fileName)
{
    try {
        tbin::Map tmap;
        //tmap.id = map->name();
        tiledToTbinProperties(map, tmap.props);

        const QDir fileDir(QFileInfo(fileName).dir());

        for (const Tiled::SharedTileset& tilesheet : map->tilesets()) {
            tbin::TileSheet ttilesheet;
            ttilesheet.id = tilesheet->name().toStdString();
            ttilesheet.image = Tiled::toFileReference(tilesheet->imageSource(), fileDir).replace("/", "\\").toStdString();
            ttilesheet.margin.x = ttilesheet.margin.y = tilesheet->margin();
            ttilesheet.spacing.x = ttilesheet.spacing.y = tilesheet->tileSpacing();
            ttilesheet.sheetSize.x = tilesheet->columnCount();
            ttilesheet.sheetSize.y = tilesheet->rowCount();
            ttilesheet.tileSize.x = tilesheet->tileSize().width();
            ttilesheet.tileSize.y = tilesheet->tileSize().height();
            tiledToTbinProperties(map, tmap.props);

            for (auto tile : tilesheet->tiles()) {
                Tiled::Object obj(Tiled::Object::TileType);
                auto props = tile->properties();
                for (auto it = props.begin(); it != props.end(); ++it) {
                    obj.setProperty("@TileIndex@" + QString::number(tile->id()) + "@" + it.key(), it.value());
                }
                tiledToTbinProperties(&obj, ttilesheet.props);
            }

            tmap.tilesheets.push_back(std::move(ttilesheet));
        }

        std::vector< Tiled::ObjectGroup* > objGroups;
        std::map< std::string, tbin::Layer* > tileLayerIdMap;
        tmap.layers.reserve(map->layers().size());
        for (Tiled::Layer* rawLayer : map->layers()) {
            if (Tiled::ObjectGroup* layer = rawLayer->asObjectGroup()) {
                objGroups.push_back(layer);
            }
            else if (Tiled::TileLayer* layer = rawLayer->asTileLayer()) {
                tbin::Layer tlayer;
                tlayer.id = layer->name().toStdString();
                tlayer.layerSize.x = layer->size().width();
                tlayer.layerSize.y = layer->size().height();
                tlayer.tileSize.x = map->tileSize().width();
                tlayer.tileSize.y = map->tileSize().height();
                //tlayer.visible = ???;
                for (int iy = 0; iy < tlayer.layerSize.y; ++iy) {
                    for (int ix = 0; ix < tlayer.layerSize.x; ++ix) {
                        Tiled::Cell cell = layer->cellAt(ix, iy);
                        tbin::Tile ttile;
                        ttile.staticData.tileIndex = -1;

                        if (Tiled::Tile *tile = cell.tile()) {
                            ttile.tilesheet = tile->tileset()->name().toStdString();
                            if (tile->frames().size() == 0) {
                                ttile.staticData.tileIndex = tile->id();
                                ttile.staticData.blendMode = 0;
                            }
                            else {
                                // TODO: Check all frame durations are the same
                                for (Tiled::Frame frame : tile->frames()) {
                                    ttile.animatedData.frameInterval = frame.duration;
                                    tbin::Tile tframe;
                                    tframe.tilesheet = ttile.tilesheet;
                                    tframe.staticData.tileIndex = frame.tileId;
                                    tframe.staticData.blendMode = 0;
                                    ttile.animatedData.frames.push_back(tframe);
                                }
                            }
                        }
                        tlayer.tiles.push_back(ttile);
                    }
                }
                tiledToTbinProperties(layer, tlayer.props);
                tmap.layers.push_back(std::move(tlayer));
                tileLayerIdMap[tmap.layers.back().id] = &tmap.layers.back();
            }
            else {
                throw std::invalid_argument(QT_TR_NOOP("Only object and tile layers supported."));
            }
        }

        for (Tiled::ObjectGroup* objs : objGroups) {
            tbin::Layer* tiles = tileLayerIdMap[objs->name().toStdString()];
            if (!tiles)
                continue;

            for (Tiled::MapObject* obj : objs->objects()) {
                if (obj->name() != "TileData")
                   continue;

                if (obj->size().width() != tiles->tileSize.x || obj->size().height() != tiles->tileSize.y ||
                    obj->position().x() / tiles->tileSize.x != std::floor(obj->position().x() / tiles->tileSize.x) ||
                    obj->position().y() / tiles->tileSize.y != std::floor(obj->position().y() / tiles->tileSize.y))
                   continue;

                int tileX = obj->position().x() / tiles->tileSize.x;
                int tileY = obj->position().y() / tiles->tileSize.y;
                int idx = static_cast< int >(tileX + tileY * tiles->layerSize.x);
                tiledToTbinProperties(obj, tiles->tiles[idx].props);
            }
        }

        std::ofstream file(fileName.toStdString(), std::ios::trunc | std::ios::binary);
        if (!file) {
            mError = tr("Could not open file for writing");
            return false;
        }
        tmap.saveToStream(file);
        file.close();
    }
    catch (std::exception& e)
    {
        mError = tr("Exception: %1").arg(tr(e.what()));
        return false;
    }

    return true;
}

QString TbinMapFormat::nameFilter() const
{
    return tr("Tbin map files (*.tbin)");
}

QString TbinMapFormat::shortName() const
{
    return QLatin1String("tbin");
}

bool TbinMapFormat::supportsFile(const QString &fileName) const
{
    std::ifstream file(fileName.toStdString(), std::ios::in | std::ios::binary);
    if (!file)
        return false;

    std::string magic(6, '\0');
    file.read(&magic[ 0 ], magic.length());

    return magic == "tBIN10";
}

QString TbinMapFormat::errorString() const
{
    return mError;
}

} // namespace Tbin
