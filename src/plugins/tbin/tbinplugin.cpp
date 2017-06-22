/*
 * JSON Tiled Plugin
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
#include "tile.h"
#include "tilelayer.h"

#include <map>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QDebug>
#include <sstream>

namespace
{
    void doProperties( Tiled::Object* obj, const tbin::Properties& props )
    {
        for ( const std::pair< std::string, tbin::PropertyValue >& prop : props )
        {
            switch ( prop.second.type )
            {
                case tbin::PropertyValue::String:
                    obj->setProperty( prop.first.c_str(), QVariant( prop.second.dataStr.c_str() ) );
                    break;

                case tbin::PropertyValue::Bool:
                    obj->setProperty( prop.first.c_str(), QVariant( prop.second.data.b ) );
                    break;

                case tbin::PropertyValue::Float:
                    obj->setProperty( prop.first.c_str(), QVariant( prop.second.data.f ) );
                    break;

                case tbin::PropertyValue::Integer:
                    obj->setProperty( prop.first.c_str(), QVariant( prop.second.data.i ) );
                    break;
            }
        }
    }
}

namespace Tbin {

void TbinPlugin::initialize()
{
    addObject(new TbinMapFormat(this));
}


TbinMapFormat::TbinMapFormat(QObject *parent)
{
}

Tiled::Map *TbinMapFormat::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        mError = tr("Could not open file for reading.");
        return nullptr;
    }

    QByteArray contents = file.readAll();
    std::istringstream ss( std::string( contents.constData(), contents.length() ) );

    tbin::Map tmap;
    Tiled::Map* map;
    try
    {
        tmap.loadFromStream( ss );
        map = new Tiled::Map( Tiled::Map::Orthogonal, tmap.layers[ 0 ].layerSize.x, tmap.layers[ 0 ].layerSize.y, tmap.layers[ 0 ].tileSize.x, tmap.layers[ 0 ].tileSize.y );

        std::map< std::string, int > tmapTilesheetMapping;
        for ( int i = 0; i < tmap.tilesheets.size(); ++i )
        {
            const tbin::TileSheet& ttilesheet = tmap.tilesheets[ i ];
            tmapTilesheetMapping[ ttilesheet.id ] = i;

            if ( ttilesheet.spacing.x != ttilesheet.spacing.y )
                throw std::invalid_argument( "Tilesheet must have equal spacings." );
            if ( ttilesheet.margin.x != ttilesheet.margin.y )
                throw std::invalid_argument( "Tilesheet must have equal margins." );

            auto tilesheet = Tiled::Tileset::create( ttilesheet.id.c_str(), ttilesheet.tileSize.x, ttilesheet.tileSize.y, ttilesheet.spacing.x, ttilesheet.margin.x );
            tilesheet->setImageSource( ttilesheet.image.c_str() );
            doProperties( tilesheet.data(), ttilesheet.props );

            QList<Tiled::Tile*> tiles;
            for ( int i = 0; i < ttilesheet.sheetSize.x * ttilesheet.sheetSize.y; ++i )
            {
                tiles.append( new Tiled::Tile( i, tilesheet.data() ) );
            }
            tilesheet->addTiles( tiles );

            map->addTileset( tilesheet );
        }
        for ( const tbin::Layer& tlayer : tmap.layers )
        {
            if ( tlayer.tileSize.x != tmap.layers[0].tileSize.x || tlayer.tileSize.y != tmap.layers[0].tileSize.y )
                throw std::invalid_argument( "Different tile sizes per layer are not supported." );

            Tiled::TileLayer* layer = new Tiled::TileLayer( tlayer.id.c_str(), 0, 0, tlayer.layerSize.x, tlayer.layerSize.y );
            Tiled::ObjectGroup* objects = new Tiled::ObjectGroup( tlayer.id.c_str(), 0, 0 );
            for ( int i = 0; i < tlayer.tiles.size(); ++i )
            {
                const tbin::Tile& ttile = tlayer.tiles[ i ];
                int ix = i % tlayer.layerSize.x;
                int iy = i / tlayer.layerSize.x;

                if ( ttile.isNullTile() )
                    continue;

                Tiled::Cell cell;
                if ( ttile.animatedData.frames.size() > 0 )
                {
                    tbin::Tile tfirstTile = ttile.animatedData.frames[ 0 ];
                    Tiled::Tile* firstTile = map->tilesetAt( tmapTilesheetMapping[ tfirstTile.tilesheet ] )->tileAt( tfirstTile.staticData.tileIndex );
                    QVector<Tiled::Frame> frames;
                    for ( const tbin::Tile& tframe : ttile.animatedData.frames )
                    {
                        if ( tframe.isNullTile() || tframe.animatedData.frames.size() > 0 ||
                             tframe.tilesheet != tfirstTile.tilesheet )
                            throw std::invalid_argument( "Invalid animation frame." );

                        Tiled::Frame frame;
                        frame.tileId = tframe.staticData.tileIndex;
                        frame.duration = ttile.animatedData.frameInterval;
                        frames.append( frame );
                    }
                    firstTile->setFrames( frames );
                    cell = Tiled::Cell( firstTile );
                }
                else
                {
                    cell = Tiled::Cell( map->tilesetAt( tmapTilesheetMapping[ ttile.tilesheet ] )->tileAt( ttile.staticData.tileIndex ) );
                }
                layer->setCell( ix, iy, cell );

                if ( ttile.props.size() > 0 )
                {
                    qDebug() <<"propc:"<<ttile.props.size();
                    Tiled::MapObject* obj = new Tiled::MapObject( "TileData", "", QPointF( ix * tlayer.tileSize.x, iy * tlayer.tileSize.y ), QSizeF( 1 * tlayer.tileSize.x, 1 * tlayer.tileSize.y ) );
                    doProperties( obj, ttile.props );
                    objects->addObject( obj );
                }
            }
            map->addLayer( layer );
            map->addLayer( objects );
        }
    }
    catch ( std::exception& e )
    {
        mError = QString("Exception: ") + e.what();
    }

    return map;
}

bool TbinMapFormat::write(const Tiled::Map *map, const QString &fileName)
{
    // ...

    return false;
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
    // ...

    return false;
}

QString TbinMapFormat::errorString() const
{
    return mError;
}

} // namespace Tbin
