/*
 * TBIN Tiled Plugin
 * Copyright 2026, ConcernedApe LLC <contact@stardewvalley.net>
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

#include "tidemapformat.h"

#include "tbinplugin.h"
#include "tbin/Map.hpp"

#include "logginginterface.h"
#include "map.h"

#include <QCoreApplication>
#include <QDir>
#include <QStringView>
#include <QXmlStreamReader>

#include <memory>

namespace Tbin {

TideMapFormat::TideMapFormat(QObject *parent)
    : Tiled::MapFormat(parent)
{
}

std::unique_ptr<Tiled::Map> TideMapFormat::read(const QString &fileName)
{
    QFile file( fileName );
    if ( !file.exists() || !file.open( QFile::ReadOnly | QFile::Text ) )
    {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    try
    {
        QXmlStreamReader xml( &file );
        if ( !xml.readNextStartElement() || xml.name() != QLatin1String( "Map" ) )
        {
            mError = QCoreApplication::translate("File Errors", "Not a tide file.");
            return nullptr;
        }

        tbin::Map tmap;

        const QXmlStreamAttributes mapAttrs = xml.attributes();
        tmap.id = mapAttrs.value( "Id" ).toString().toStdString();

        auto readProps = [&xml]() -> tbin::Properties
        {
            tbin::Properties props;

            while ( xml.readNextStartElement() )
            {
                if ( xml.name() == "Property" )
                {
                    const QXmlStreamAttributes attrs = xml.attributes();

                    const std::string key = attrs.value( "Key" ).toString().toStdString();
                    const QString type = attrs.value( "Type" ).toString();
                    const QString value = xml.readElementText();

                    tbin::PropertyValue prop;
                    if ( type == "Boolean" )
                    {
                        prop.type = tbin::PropertyValue::Bool;
                        prop.data.b = QString::compare( value, "true", Qt::CaseInsensitive ) == 0;
                    }
                    if ( type == "Int32" )
                    {
                        prop.type = tbin::PropertyValue::Integer;
                        prop.data.i = value.toInt();
                    }
                    if ( type == "Single" )
                    {
                        prop.type = tbin::PropertyValue::Float;
                        prop.data.f = value.toFloat();
                    }
                    if ( type == "String" )
                    {
                        prop.type = tbin::PropertyValue::String;
                        prop.dataStr = value.toStdString();
                    }

                    props.insert( std::make_pair( key, prop ) );
                }
                else xml.skipCurrentElement();
            }

            return props;
        };

        while ( xml.readNextStartElement() )
        {
            if ( xml.name() == QLatin1String( "Description" ) )
                tmap.desc = xml.readElementText().toStdString();
            else if ( xml.name() == "TileSheets" )
            {
                while ( xml.readNextStartElement() )
                {
                    if ( xml.name() != "TileSheet" )
                    {
                        xml.skipCurrentElement();
                        continue;
                    }

                    const QXmlStreamAttributes tsAttrs = xml.attributes();

                    tbin::TileSheet ts;
                    ts.id = tsAttrs.value( "Id" ).toString().toStdString();

                    while ( xml.readNextStartElement() )
                    {
                        if ( xml.name() == QLatin1String( "Description" ) )
                            ts.desc = xml.readElementText().toStdString();
                        else if ( xml.name() == "ImageSource" )
                            ts.image = xml.readElementText().toStdString();
                        else if ( xml.name() == "Alignment" )
                        {
                            const QXmlStreamAttributes alignAttrs = xml.attributes();

                            const QString sheetSizeStr = alignAttrs.value( "SheetSize" ).toString();
                            const QString tileSizeStr = alignAttrs.value( "TileSize" ).toString();
                            const QString marginStr = alignAttrs.value( "Margin" ).toString();
                            const QString spacingStr = alignAttrs.value( "Spacing" ).toString();

                            qsizetype sheetSizeSep = sheetSizeStr.indexOf( " x " );
                            qsizetype tileSizeSep = tileSizeStr.indexOf( " x " );
                            qsizetype marginSep = marginStr.indexOf( " x " );
                            qsizetype spacingSep = spacingStr.indexOf( " x " );

                            ts.sheetSize = tbin::Vector2i( sheetSizeStr.mid( 0, sheetSizeSep ).toInt(), sheetSizeStr.mid( sheetSizeSep + 3 ).toInt() );
                            ts.tileSize = tbin::Vector2i( tileSizeStr.mid( 0, tileSizeSep ).toInt(), tileSizeStr.mid( tileSizeSep + 3 ).toInt() );
                            ts.margin = tbin::Vector2i( marginStr.mid( 0, marginSep ).toInt(), marginStr.mid( marginSep + 3 ).toInt() );
                            ts.spacing = tbin::Vector2i( spacingStr.mid( 0, spacingSep ).toInt(), spacingStr.mid( spacingSep + 3 ).toInt() );

                            xml.skipCurrentElement();
                        }
                        else if (xml.name() == "Properties")
                            ts.props = readProps();
                        else xml.skipCurrentElement();
                    }

                    tmap.tilesheets.push_back( ts );
                }
            }
            else if ( xml.name() == "Layers" )
            {
                while ( xml.readNextStartElement() )
                {
                    if ( xml.name() != "Layer" )
                    {
                        xml.skipCurrentElement();
                        continue;
                    }
                    const QXmlStreamAttributes layerAttrs = xml.attributes();

                    tbin::Layer layer;
                    layer.id = layerAttrs.value( "Id" ).toString().toStdString();
                    auto vis = layerAttrs.value( "Visible" ).toString().toStdString();
                    layer.visible = QString::compare( layerAttrs.value( "Visible" ).toString(), "true", Qt::CaseInsensitive ) == 0;

                    while ( xml.readNextStartElement() )
                    {
                        if ( xml.name() == QLatin1String( "Description" ) )
                            layer.desc = xml.readElementText().toStdString();
                        else if ( xml.name() == "Dimensions" )
                        {
                            const QXmlStreamAttributes alignAttrs = xml.attributes();

                            const QString layerSizeStr = alignAttrs.value( "LayerSize" ).toString();
                            const QString tileSizeStr = alignAttrs.value( "TileSize" ).toString();

                            qsizetype layerSizeSep = layerSizeStr.indexOf( " x " );
                            qsizetype tileSizeSep = tileSizeStr.indexOf( " x " );

                            layer.layerSize = tbin::Vector2i( layerSizeStr.mid( 0, layerSizeSep ).toInt(), layerSizeStr.mid( layerSizeSep + 3 ).toInt() );
                            layer.tileSize = tbin::Vector2i( tileSizeStr.mid( 0, tileSizeSep ).toInt(), tileSizeStr.mid( tileSizeSep + 3 ).toInt() );

                            xml.skipCurrentElement();
                        }
                        else if ( xml.name() == "TileArray" )
                        {
                            tbin::Tile nullTile;
                            nullTile.staticData.tileIndex = -1;
                            layer.tiles.resize( layer.layerSize.x * layer.layerSize.y, nullTile );

                            auto readStaticTile = [&xml, &readProps](const std::string& tilesheetToUse)-> tbin::Tile
                            {
                                const QXmlStreamAttributes tileAttrs = xml.attributes();

                                tbin::Tile tile;
                                tile.tilesheet = tilesheetToUse;
                                tile.staticData.tileIndex = tileAttrs.value( "Index" ).toString().toInt();
                                tile.staticData.blendMode = tileAttrs.value( "BlendMode" ).toString() == "Alpha" ? 0 : 1;

                                // tIDE (the xTile tile editor) uses a self-closing element when there are no properties.
                                // However, Qt seems to represent these simply as empty elements when reading.
                                // We can just iterate as normal for reading.
                                // When writing, we'll still need to make sure it's an actual self-closing element,
                                // for compatibility with xTile code.
                                while ( xml.readNextStartElement() )
                                {
                                    if (xml.name() == "Properties")
                                        tile.props = readProps();
                                    else xml.skipCurrentElement();
                                }

                                return tile;
                            };

                            std::string lastTilesheet;
                            tbin::Vector2i tilePos;
                            while ( xml.readNextStartElement() )
                            {
                                if (xml.name() != "Row" )
                                {
                                    xml.skipCurrentElement();
                                    continue;
                                }

                                while ( xml.readNextStartElement() )
                                {
                                    if ( xml.name() == "TileSheet" )
                                    {
                                        const QXmlStreamAttributes tsAttrs = xml.attributes();
                                        lastTilesheet = tsAttrs.value( "Ref" ).toString().toStdString();
                                        xml.skipCurrentElement();
                                    }
                                    else if ( xml.name() == "Static" )
                                    {
                                        layer.tiles[ tilePos.x + tilePos.y * layer.layerSize.x ] = readStaticTile( lastTilesheet );
                                        tilePos.x += 1;
                                    }
                                    else if ( xml.name() == "Animated" )
                                    {
                                        const QXmlStreamAttributes tileAttrs = xml.attributes();

                                        tbin::Tile tile;
                                        tile.animatedData.frameInterval = tileAttrs.value( "Interval" ).toString().toInt();

                                        while ( xml.readNextStartElement() )
                                        {
                                            if (xml.name() == "Frames" )
                                            {
                                                std::string lastTilesheet; // Shadowing the previous declaration intentionally, because we explicitly do not want to use/affect it
                                                while ( xml.readNextStartElement() )
                                                {
                                                    if ( xml.name() == "TileSheet" )
                                                    {
                                                        const QXmlStreamAttributes tsAttrs = xml.attributes();
                                                        lastTilesheet = tsAttrs.value( "Ref" ).toString().toStdString();
                                                        xml.skipCurrentElement();
                                                    }
                                                    else if (xml.name() == "Static")
                                                        tile.animatedData.frames.push_back( readStaticTile( lastTilesheet ) );
                                                    else xml.skipCurrentElement();
                                                }
                                            }
                                            else if (xml.name() == "Properties")
                                                tile.props = readProps();
                                            else xml.skipCurrentElement();
                                        }

                                        layer.tiles[ tilePos.x + tilePos.y * layer.layerSize.x ] = tile;
                                        tilePos.x += 1;
                                    }
                                    else if ( xml.name() == "Null" )
                                    {
                                        const QXmlStreamAttributes tileAttrs = xml.attributes();
                                        tilePos.x += tileAttrs.value( "Count" ).toString().toInt();
                                        xml.skipCurrentElement();
                                    }
                                    else xml.skipCurrentElement();
                                }

                                tilePos.x = 0;
                                tilePos.y += 1;
                            }
                        }
                        else if (xml.name() == "Properties")
                            layer.props = readProps();
                        else xml.skipCurrentElement();
                    }

                    tmap.layers.push_back( layer );
                }
            }
            else if (xml.name() == "Properties")
                tmap.props = readProps();
            else xml.skipCurrentElement();
        }

        const QDir fileDir(QFileInfo(fileName).dir());
        return TbinPlugin::fromTbin( tmap, fileDir );
    }
    catch (std::exception& e) {
        mError = tr((std::string("Exception: ") + e.what()).c_str());
        return nullptr;
    }
}

bool TideMapFormat::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    QFile file( fileName );
    if ( !file.open( QFile::WriteOnly | QFile::Text | QFile::Truncate ) )
    {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    try {
        const QDir fileDir(QFileInfo(fileName).dir());
        tbin::Map tmap = TbinPlugin::toTbin( map, fileDir );

        QXmlStreamWriter xml;
        xml.setDevice( &file );
        xml.setAutoFormatting(true); // Not strictly necessary, but tIDE outputs formatted ones as well. Plus, it makes diffs easier
        xml.setAutoFormattingIndent( 2 ); // Not strictly necessary, but good for matching existing tIDE files for diff purposes (or at least, for the test files I used).

        auto writeProps = [&xml](const tbin::Properties& props)
        {
            xml.writeStartElement( "Properties" );
            for ( const auto& prop : props )
            {
                xml.writeStartElement( "Property" );
                xml.writeAttribute( "Key", QString(prop.first.c_str()) );
                switch ( prop.second.type )
                {
                    case tbin::PropertyValue::Bool:
                        xml.writeAttribute( "Type", "Boolean" );
                        xml.writeCDATA( prop.second.data.b ? "True" : "False" );
                        break;
                    case tbin::PropertyValue::Integer:
                        xml.writeAttribute( "Type", "Int32" );
                        xml.writeCDATA( QString::number( prop.second.data.i ) );
                        break;
                    case tbin::PropertyValue::Float:
                        xml.writeAttribute( "Type", "Single" );
                        xml.writeCDATA( QString::number( prop.second.data.f ) );
                        break;
                    case tbin::PropertyValue::String:
                        xml.writeAttribute( "Type", "String" );
                        xml.writeCDATA( QString(prop.second.dataStr.c_str()) );
                        break;
                    default:
                        throw std::invalid_argument( "Unknown property type" );
                }
                xml.writeEndElement();
            }
            xml.writeEndElement();
        };

        xml.writeStartDocument();
        xml.writeStartElement( "Map" );
        {
            xml.writeAttribute( "Id", QString(tmap.id.c_str()) );

            xml.writeStartElement( "Description" );
            xml.writeCDATA( QString(tmap.desc.c_str()) );
            xml.writeEndElement();

            xml.writeStartElement("TileSheets");
            {
                for ( const auto& ts : tmap.tilesheets )
                {
                    xml.writeStartElement( "TileSheet" );
                    xml.writeAttribute( "Id", QString(ts.id.c_str()) );

                    xml.writeStartElement( "Description" );
                    xml.writeCDATA( QString(ts.desc.c_str()) );
                    xml.writeEndElement();

                    xml.writeStartElement( "ImageSource" );
                    xml.writeCDATA( QString(ts.image.c_str()) );
                    xml.writeEndElement();

                    xml.writeEmptyElement( "Alignment" );
                    xml.writeAttribute( "SheetSize", QStringLiteral( "%1 x %2" ).arg( ts.sheetSize.x ).arg( ts.sheetSize.y ) );
                    xml.writeAttribute( "TileSize",  QStringLiteral( "%1 x %2" ).arg( ts.tileSize.x  ).arg( ts.tileSize.y  ) );
                    xml.writeAttribute( "Margin",    QStringLiteral( "%1 x %2" ).arg( ts.margin.x    ).arg( ts.margin.y    ) );
                    xml.writeAttribute( "Spacing",   QStringLiteral( "%1 x %2" ).arg( ts.spacing.x   ).arg( ts.spacing.y   ) );

                    writeProps( ts.props );

                    xml.writeEndElement();
                }
            }
            xml.writeEndElement();

            xml.writeStartElement("Layers");
            {
                for ( const auto& layer : tmap.layers )
                {
                    xml.writeStartElement( "Layer" );
                    xml.writeAttribute( "Id", QString(layer.id.c_str()) );
                    xml.writeAttribute( "Visible", layer.visible ? "True" : "False" );

                    xml.writeStartElement( "Description" );
                    xml.writeCDATA( QString(layer.desc.c_str()) );
                    xml.writeEndElement();

                    xml.writeEmptyElement( "Dimensions" );
                    xml.writeAttribute( "LayerSize", QStringLiteral( "%1 x %2" ).arg( layer.layerSize.x ).arg( layer.layerSize.y ) );
                    xml.writeAttribute( "TileSize",  QStringLiteral( "%1 x %2" ).arg( layer.tileSize.x  ).arg( layer.tileSize.y  ) );

                    auto writeStaticTile = [&xml, &writeProps](const tbin::Tile& tile)
                    {
                        if ( tile.props.size() == 0 )
                            xml.writeEmptyElement( "Static" );
                        else
                            xml.writeStartElement( "Static" );
                        xml.writeAttribute( "Index", QString::number( tile.staticData.tileIndex ) );
                        switch ( tile.staticData.blendMode )
                        {
                            case 0: xml.writeAttribute( "BlendMode", "Alpha" ); break;
                            case 1: xml.writeAttribute( "BlendMode", "Additive" ); break;
                            default: throw std::invalid_argument( "Unsupported tile blend mode" );
                        }
                        if ( tile.props.size() > 0 )
                        {
                            writeProps( tile.props );
                            xml.writeEndElement();
                        }
                    };
                    xml.writeStartElement( "TileArray" );
                    std::string lastTilesheet;
                    for ( int iy = 0; iy < layer.layerSize.y; ++iy )
                    {
                        xml.writeStartElement( "Row" );
                        for ( int ix = 0; ix < layer.layerSize.x; ++ix )
                        {
                            tbin::Tile tile = layer.tiles[ ix + iy * layer.layerSize.x ];
                            if ( tile.tilesheet != "" && tile.tilesheet != lastTilesheet )
                            {
                                xml.writeEmptyElement( "TileSheet" );
                                xml.writeAttribute( "Ref", QString(tile.tilesheet.c_str()) );
                                lastTilesheet = tile.tilesheet;
                            }

                            if ( tile.isNullTile() )
                            {
                                int nullCount = 1;
                                for ( ix += 1; ix < layer.layerSize.x; ++ix )
                                {
                                    if ( !layer.tiles[ ix + iy * layer.layerSize.x ].isNullTile() )
                                    {
                                        --ix;
                                        break;
                                    }
                                    nullCount += 1;
                                }

                                xml.writeEmptyElement( "Null" );
                                xml.writeAttribute( "Count", QString::number( nullCount ) );
                            }
                            else if ( tile.staticData.tileIndex != -1 )
                            {
                                writeStaticTile( tile );
                            }
                            else if ( tile.animatedData.frames.size() > 0 )
                            {
                                xml.writeStartElement( "Animated" );
                                xml.writeAttribute( "Interval", QString::number( tile.animatedData.frameInterval ) );

                                xml.writeStartElement( "Frames" );
                                std::string lastTilesheet; // Shadowing the previous declaration intentionally, because we explicitly do not want to use/affect it
                                for ( const auto& tile : tile.animatedData.frames )
                                {
                                    if ( tile.tilesheet != "" && tile.tilesheet != lastTilesheet )
                                    {
                                        xml.writeEmptyElement( "TileSheet" );
                                        xml.writeAttribute( "Ref", QString(tile.tilesheet.c_str()) );
                                        lastTilesheet = tile.tilesheet;
                                    }
                                    writeStaticTile( tile );
                                }
                                xml.writeEndElement();

                                if ( tile.props.size() > 0 )
                                    writeProps(tile.props);

                                xml.writeEndElement();
                            }
                        }
                        xml.writeEndElement();
                    }
                    xml.writeEndElement();

                    writeProps( layer.props );

                    xml.writeEndElement();
                }
            }
            xml.writeEndElement();

            writeProps(tmap.props);
        }
        xml.writeEndElement();
        xml.writeEndDocument();
        file.close();

        return true;
    }
    catch (std::exception& e)
    {
        mError = tr("Exception: %1").arg(tr(e.what()));
        return false;
    }
}

QString TideMapFormat::nameFilter() const
{
    return tr("tIDE map files (*.tide)");
}

QString TideMapFormat::shortName() const
{
    return QStringLiteral("tide");
}

bool TideMapFormat::supportsFile(const QString &fileName) const
{
    QFile file( fileName );
    if ( !file.open( QFile::ReadOnly | QFile::Text ) )
        return false;

    QXmlStreamReader xml(&file);
    return xml.readNextStartElement() && xml.name() == QLatin1String("Map");
}

QString TideMapFormat::errorString() const
{
    return mError;
}

} // namespace Tbin
