/*
 * TBIN Tiled Plugin
 * Copyright 2017, Casey Warrington <spacechase0.and.cat@gmail.com>
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

#include "tbinmapformat.h"
#include "tidemapformat.h"
#include "tbin/Map.hpp"

#include "logginginterface.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tile.h"
#include "tiled.h"
#include "tilelayer.h"

#include <QCoreApplication>
#include <QDir>
#include <QStringView>

namespace
{
    void tbinToTiledProperties(const tbin::Properties& props, Tiled::Object& obj)
    {
        for (const auto& prop : props) {
            if (prop.first[0] == '@')
                continue;
            switch (prop.second.type) {
            case tbin::PropertyValue::String:
                obj.setProperty(QString::fromStdString(prop.first), QString::fromStdString(prop.second.dataStr));
                break;

            case tbin::PropertyValue::Bool:
                obj.setProperty(QString::fromStdString(prop.first), prop.second.data.b);
                break;

            case tbin::PropertyValue::Float:
                obj.setProperty(QString::fromStdString(prop.first), prop.second.data.f);
                break;

            case tbin::PropertyValue::Integer:
                obj.setProperty(QString::fromStdString(prop.first), prop.second.data.i);
                break;
            }
        }
    }

    void tiledToTbinProperties(const Tiled::Properties& properties, tbin::Properties& tprops, const QDir& dir, bool allowOverwrite = false)
    {
        for (auto it = properties.cbegin(), it_end = properties.cend(); it != it_end; ++it) {
            const auto& name = it.key();
            auto value = it.value();

            if (value.userType() == Tiled::filePathTypeId()) {
                Tiled::WARNING(QStringLiteral("tBIN: Saving 'file' property \"%1\" as 'string'.").arg(name));
                const auto filePath = value.value<Tiled::FilePath>();
                value = Tiled::toFileReference(filePath.url, dir);
            }
            else if (value.userType() == Tiled::objectRefTypeId()) {
                Tiled::WARNING(QStringLiteral("tBIN: Saving 'object' property \"%1\" as 'int'.").arg(name));
                const auto objectRef = value.value<Tiled::ObjectRef>();
                value = Tiled::ObjectRef::toInt(objectRef);
            }
            else if (value.userType() == QMetaType::QColor) {
                Tiled::WARNING(QStringLiteral("tBIN: Saving 'color' property \"%1\" as 'string'.").arg(name));
                const auto color = value.value<QColor>();
                value = color.isValid() ? color.name(QColor::HexArgb) : QString();
            }

            tbin::PropertyValue prop;

            switch (value.userType()) {
            case QMetaType::Bool:
                prop.type = tbin::PropertyValue::Bool;
                prop.data.b = value.toBool();
                break;
            case QMetaType::Double:
            case QMetaType::Float:
                prop.type = tbin::PropertyValue::Float;
                prop.data.f = value.toFloat();
                break;
            case QMetaType::Int:
                prop.type = tbin::PropertyValue::Integer;
                prop.data.i = value.toInt();
                break;
            case QMetaType::QString:
                prop.type = tbin::PropertyValue::String;
                prop.dataStr = value.toString().toStdString();
                break;
            default:
                throw std::invalid_argument(QT_TRANSLATE_NOOP("TbinMapFormat", "Unsupported property type"));
            }

            if (tprops.find( name.toStdString() ) != tprops.end())
            {
                if ( allowOverwrite )
                    tprops[name.toStdString()] = prop;
            }
            else
                tprops.insert(std::make_pair(name.toStdString(), prop));
        }
    }

    bool hasUnsupportedFlags(const Tiled::Cell& cell)
    {
        return cell.flippedHorizontally() ||
            cell.flippedVertically() ||
            cell.flippedAntiDiagonally() ||
            cell.rotatedHexagonal120();
    }
}

namespace Tbin {

void TbinPlugin::initialize()
{
    addObject(new TbinMapFormat(this));
    addObject(new TideMapFormat(this));
}

std::unique_ptr< Tiled::Map > TbinPlugin::fromTbin( const tbin::Map& tmap, const QDir fileDir )
{
    std::unique_ptr<Tiled::Map> map;

    if (tmap.layers.empty())
        throw std::invalid_argument(QT_TR_NOOP("Map contains no layers."));

    auto &firstLayer = tmap.layers[0];

    Tiled::Map::Parameters mapParameters;
    mapParameters.width = firstLayer.layerSize.x;
    mapParameters.height = firstLayer.layerSize.y;
    mapParameters.tileWidth = firstLayer.tileSize.x;
    mapParameters.tileHeight = firstLayer.tileSize.y;

    map = std::make_unique<Tiled::Map>(mapParameters);

    tbinToTiledProperties(tmap.props, *map);

    std::map< std::string, int > tmapTilesheetMapping;
    for (std::size_t i = 0; i < tmap.tilesheets.size(); ++i) {
        const tbin::TileSheet& ttilesheet = tmap.tilesheets[i];
        tmapTilesheetMapping[ttilesheet.id] = static_cast<int>(i);

        if (ttilesheet.spacing.x != ttilesheet.spacing.y)
            throw std::invalid_argument(QT_TR_NOOP("Tilesheet must have equal spacings."));
        if (ttilesheet.margin.x != ttilesheet.margin.y)
            throw std::invalid_argument(QT_TR_NOOP("Tilesheet must have equal margins."));

        auto tileset = Tiled::Tileset::create(ttilesheet.id.c_str(), ttilesheet.tileSize.x, ttilesheet.tileSize.y, ttilesheet.spacing.x, ttilesheet.margin.x);
        tileset->setImageSource(Tiled::toUrl(QString::fromStdString(ttilesheet.image).replace("\\", "/"), fileDir));
        tileset->loadImage();

        tbinToTiledProperties(ttilesheet.props, *tileset);

        for (const auto &prop : ttilesheet.props) {
            if (prop.first[0] != '@')
                continue;

            const QString name = QString::fromStdString(prop.first);
            const auto strs = QStringView(name).split(QLatin1Char('@'));
            if (strs[1] == QLatin1String("TileIndex")) {
                int index = strs[2].toInt();
                tbin::Properties dummyProps;
                dummyProps.insert(std::make_pair(strs[3].toUtf8().constData(), prop.second));
                Tiled::Tile *tile = tileset->findOrCreateTile(index);
                tbinToTiledProperties(dummyProps, *tile);
            }
            // TODO: 'AutoTile' ?
            // Purely for map making. Appears to be similar to terrains
            // (In tIDE, right click a tilesheet and choose "Auto Tiles..."
        }

        map->addTileset(tileset);
    }
    for (const tbin::Layer& tlayer : tmap.layers) {
        if (tlayer.tileSize.x != firstLayer.tileSize.x || tlayer.tileSize.y != firstLayer.tileSize.y)
            throw std::invalid_argument(QT_TR_NOOP("Different tile sizes per layer are not supported."));

        auto layer = std::make_unique<Tiled::TileLayer>(QString::fromStdString(tlayer.id), 0, 0, tlayer.layerSize.x, tlayer.layerSize.y);
        layer->setVisible( tlayer.visible );
        tbinToTiledProperties(tlayer.props, *layer);
        auto objects = std::make_unique<Tiled::ObjectGroup>(QString::fromStdString(tlayer.id), 0, 0);
        objects->setVisible( tlayer.visible );
        for (std::size_t i = 0; i < tlayer.tiles.size(); ++i) {
            const tbin::Tile& ttile = tlayer.tiles[i];
            int ix = static_cast<int>(i % static_cast<std::size_t>(tlayer.layerSize.x));
            int iy = static_cast<int>(i / static_cast<std::size_t>(tlayer.layerSize.x));

            if (ttile.isNullTile())
                continue;

            Tiled::Cell cell;
            if (ttile.animatedData.frames.size() > 0) {
                tbin::Tile tfirstTile = ttile.animatedData.frames[0];
                Tiled::Tile* firstTile = map->tilesetAt(tmapTilesheetMapping[tfirstTile.tilesheet])->findOrCreateTile(tfirstTile.staticData.tileIndex);
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
                cell = Tiled::Cell(map->tilesetAt(tmapTilesheetMapping[ttile.tilesheet]).data(), ttile.staticData.tileIndex);
            }
            layer->setCell(ix, iy, cell);

            if (ttile.props.size() > 0)
            {
                auto obj = std::make_unique<Tiled::MapObject>("TileData");
                obj->setPosition(QPointF(ix * tlayer.tileSize.x, iy * tlayer.tileSize.y));
                obj->setSize(QSizeF(tlayer.tileSize.x, tlayer.tileSize.y));
                tbinToTiledProperties(ttile.props, *obj);
                objects->addObject(std::move(obj));
            }
        }
        
        bool doMergePass = true;
        while ( doMergePass )
        {
            doMergePass = false;

            // Consolidate duplicate TileData into a larger single TileData, if possible
            // Not a perfectly efficient implementation as far as the results, but better than every tile being separate.
            for ( const auto& obj : ( * objects ) )
            {
                for ( const auto& otherObj : ( * objects ) )
                {
                    if ( &obj == &otherObj )
                        continue;

                    int myX = (int) obj->x() / tlayer.tileSize.x;
                    int myY = (int) obj->y() / tlayer.tileSize.y;
                    int myWidth = (int) obj->width() / tlayer.tileSize.x;
                    int myHeight = (int) obj->height() / tlayer.tileSize.y;
                    
                    int otherX = (int) otherObj->x() / tlayer.tileSize.x;
                    int otherY = (int) otherObj->y() / tlayer.tileSize.y;
                    int otherWidth = (int) otherObj->width() / tlayer.tileSize.x;
                    int otherHeight = (int) otherObj->height() / tlayer.tileSize.y;

                    bool tryMerge = false;
                    int mergeW = 0;
                    int mergeH = 0;

                    // Merge left if possible
                    if ( otherX + otherWidth == myX && otherY == myY && otherHeight == myHeight )
                    {
                        tryMerge = true;
                        mergeW = 1;
                    }
                    // Merge up if possible
                    else if ( otherX == myX && otherY + otherHeight == myY && otherWidth == myWidth )
                    {
                        tryMerge = true;
                        mergeH = 1;
                    }

                    if ( !tryMerge )
                        continue;
                    
                    Tiled::Properties& myProps = obj->properties();
                    Tiled::Properties& otherProps = otherObj->properties();

                    // Make sure that we only merge if the target is strictly a subset of this tile
                    bool anyMissingOrNonmatching = false;
                    for ( const auto& propKey : otherProps.keys() )
                    {
                        if ( !myProps.contains( propKey ) || myProps[ propKey ] != otherProps[ propKey ] )
                        {
                            anyMissingOrNonmatching = true;
                            break;
                        }
                    }

                    if ( anyMissingOrNonmatching )
                        continue;
                    
                    // Merge what we can
                    for ( const auto& propKey : otherProps.keys() )
                    {
                        myProps.remove( propKey );
                    }
                    otherObj->setSize( (otherWidth + mergeW) * tlayer.tileSize.x, (otherHeight + mergeH) * tlayer.tileSize.y );

                    doMergePass = true;
                    break;
                }

                if ( doMergePass )
                {
                    // If there's nothing left in this object after merging, remove it
                    if ( obj->properties().size() == 0 )
                        objects->removeObject( obj );
                    
                    break;
                }
            }
        }
        map->addLayer(std::move(layer));
        map->addLayer(std::move(objects));
    }

    return map;
}

tbin::Map TbinPlugin::toTbin( const Tiled::Map* map, const QDir fileDir )
{
    tbin::Map tmap;
    
    //tmap.id = map->name();
    tiledToTbinProperties(map->properties(), tmap.props, fileDir);

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
        tiledToTbinProperties(tilesheet->properties(), ttilesheet.props, fileDir);

        Tiled::Properties tilesetTileProperties;
        for (auto tile : tilesheet->tiles()) {
            const auto &props = tile->properties();
            for (auto it = props.begin(), it_end = props.end(); it != it_end; ++it) {
                tilesetTileProperties.insert("@TileIndex@" + QString::number(tile->id()) + "@" + it.key(), it.value());
            }
        }
        tiledToTbinProperties(tilesetTileProperties, ttilesheet.props, fileDir);

        tmap.tilesheets.push_back(std::move(ttilesheet));
    }

    std::vector< Tiled::ObjectGroup* > objGroups;
    std::map< std::string, tbin::Layer* > tileLayerIdMap;
    tmap.layers.reserve(static_cast<std::size_t>(map->layers().size()));
    for (Tiled::Layer* rawLayer : map->layers()) {
        if (Tiled::ObjectGroup* layer = rawLayer->asObjectGroup()) {
            objGroups.push_back(layer);
        }
        else if (Tiled::TileLayer* layer = rawLayer->asTileLayer()) {
            tbin::Layer tlayer;
            tlayer.id = layer->name().toStdString();
            tlayer.layerSize.x = layer->width();
            tlayer.layerSize.y = layer->height();
            tlayer.tileSize.x = map->tileWidth();
            tlayer.tileSize.y = map->tileHeight();
            tlayer.visible = layer->isVisible();
            for (int iy = 0; iy < tlayer.layerSize.y; ++iy) {
                for (int ix = 0; ix < tlayer.layerSize.x; ++ix) {
                    Tiled::Cell cell = layer->cellAt(ix, iy);
                    tbin::Tile ttile;
                    ttile.staticData.tileIndex = -1;

                    if (hasUnsupportedFlags(cell)) {
                        Tiled::ERROR("tBIN: Flipped and/or rotated tiles are not supported.",
                                        Tiled::JumpToTile { map, QPoint(ix + layer->x(), iy + layer->y()), layer });
                    }

                    if (Tiled::Tile *tile = cell.tile()) {
                        ttile.tilesheet = tile->tileset()->name().toStdString();
                        if (tile->frames().size() == 0) {
                            ttile.staticData.tileIndex = tile->id();
                            ttile.staticData.blendMode = 0;
                        }
                        else {
                            ttile.animatedData.frameInterval = tile->frames().at(0).duration;

                            for (Tiled::Frame frame : tile->frames()) {
                                if (frame.duration != ttile.animatedData.frameInterval) {
                                    Tiled::ERROR("tBIN: Frames with different duration are not supported.",
                                                    Tiled::SelectTile { tile });
                                }

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
            tiledToTbinProperties(layer->properties(), tlayer.props, fileDir);
            tmap.layers.push_back(std::move(tlayer));
            tileLayerIdMap[tmap.layers.back().id] = &tmap.layers.back();
        }
        else {
            throw std::invalid_argument(QT_TR_NOOP("Only object and tile layers supported."));
        }
    }

    for (Tiled::ObjectGroup* objs : objGroups) {
        const auto groupName = objs->name();

        tbin::Layer* tiles = tileLayerIdMap[groupName.toStdString()];
        if (!tiles) {
            Tiled::WARNING(QStringLiteral("tBIN: Ignoring object layer \"%1\" without matching tile layer.").arg(groupName),
                            Tiled::SelectLayer { objs });
            continue;
        }

        for (Tiled::MapObject* obj : objs->objects()) {
            if (obj->name() != QLatin1String("TileData")) {
                Tiled::WARNING(QStringLiteral("tBIN: Ignoring object %1 with name different from 'TileData'.").arg(obj->id()),
                                Tiled::JumpToObject { obj });
                continue;
            }

            if (obj->properties().isEmpty()) {
                Tiled::WARNING(QStringLiteral("tBIN: Ignoring object %1 without custom properties.").arg(obj->id()),
                                Tiled::JumpToObject { obj });
                continue;
            }

            if (obj->shape() != Tiled::MapObject::Shape::Rectangle) {
                Tiled::WARNING(QStringLiteral("tBIN: Ignoring object %1 that isn't a rectangle.").arg(obj->id()),
                                Tiled::JumpToObject { obj });
                continue;
            }

            if (static_cast<int>(obj->width()) % tiles->tileSize.x != 0 ||
                    static_cast<int>(obj->height()) % tiles->tileSize.y != 0 ||
                    obj->x() / tiles->tileSize.x != std::floor(obj->x() / tiles->tileSize.x) ||
                    obj->y() / tiles->tileSize.y != std::floor(obj->y() / tiles->tileSize.y)) {
                Tiled::WARNING(QStringLiteral("tBIN: Object %1 is not aligned to the tile grid.").arg(obj->id()),
                                Tiled::JumpToObject { obj });
            }

            for ( int iw = 0; iw < (int) obj->width() / tiles->tileSize.x; ++iw )
            {
                for ( int ih = 0; ih < (int) obj->height() / tiles->tileSize.y; ++ih )
                {
                    int tileX = (int) obj->x() / tiles->tileSize.x + iw;
                    int tileY = (int) obj->y() / tiles->tileSize.y + ih;

                    // Make sure the object is within the map boundaries (also makes sure values are positive)
                    if ( tileX < 0 || tileY < 0 || tileX >= tiles->layerSize.x || tileY >= tiles->layerSize.y )
                        continue;

                    bool isSingleTile = (int) obj->width() == tiles->tileSize.x && (int) obj->width() == tiles->tileSize.x;

                    std::size_t idx = static_cast<std::size_t>(tileX + tileY * tiles->layerSize.x);
                    tiledToTbinProperties(obj->properties(), tiles->tiles[idx].props, fileDir, isSingleTile);
                }
            }

        }
    }

    return tmap;
}

} // namespace Tbin
