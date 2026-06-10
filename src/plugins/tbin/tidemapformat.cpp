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
        tmap.id = mapAttrs.value( QLatin1String("Id") ).toString().toStdString();

        auto readProps = [&xml]() -> tbin::Properties
        {
            tbin::Properties props;

            while ( xml.readNextStartElement() )
            {
                if ( xml.name() == QLatin1String("Property") )
                {
                    const QXmlStreamAttributes attrs = xml.attributes();

                    const std::string key = attrs.value( QLatin1String("Key") ).toString().toStdString();
                    const QString type = attrs.value( QLatin1String("Type") ).toString();
                    const QString value = xml.readElementText();

                    tbin::PropertyValue prop;
                    if ( type == QLatin1String("Boolean") )
                    {
                        prop.type = tbin::PropertyValue::Bool;
                        prop.data.b = QString::compare( value, QLatin1String("true"), Qt::CaseInsensitive ) == 0;
                    }
                    if ( type == QLatin1String("Int32") )
                    {
                        prop.type = tbin::PropertyValue::Integer;
                        prop.data.i = value.toInt();
                    }
                    if ( type == QLatin1String("Single") )
                    {
                        prop.type = tbin::PropertyValue::Float;
                        prop.data.f = value.toFloat();
                    }
                    if ( type == QLatin1String("String") )
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
            else if ( xml.name() == QLatin1String("TileSheets") )
            {
                while ( xml.readNextStartElement() )
                {
                    if ( xml.name() != QLatin1String("TileSheet") )
                    {
                        xml.skipCurrentElement();
                        continue;
                    }

                    const QXmlStreamAttributes tsAttrs = xml.attributes();

                    tbin::TileSheet ts;
                    ts.id = tsAttrs.value( QLatin1String("Id") ).toString().toStdString();

                    while ( xml.readNextStartElement() )
                    {
                        if ( xml.name() == QLatin1String( "Description" ) )
                            ts.desc = xml.readElementText().toStdString();
                        else if ( xml.name() == QLatin1String("ImageSource") )
                            ts.image = xml.readElementText().toStdString();
                        else if ( xml.name() == QLatin1String("Alignment") )
                        {
                            const QXmlStreamAttributes alignAttrs = xml.attributes();

                            const QString sheetSizeStr = alignAttrs.value( QLatin1String("SheetSize") ).toString();
                            const QString tileSizeStr = alignAttrs.value( QLatin1String("TileSize") ).toString();
                            const QString marginStr = alignAttrs.value( QLatin1String("Margin") ).toString();
                            const QString spacingStr = alignAttrs.value( QLatin1String("Spacing") ).toString();

                            qsizetype sheetSizeSep = sheetSizeStr.indexOf( QLatin1String(" x ") );
                            qsizetype tileSizeSep = tileSizeStr.indexOf( QLatin1String(" x ") );
                            qsizetype marginSep = marginStr.indexOf( QLatin1String(" x ") );
                            qsizetype spacingSep = spacingStr.indexOf( QLatin1String(" x ") );

                            ts.sheetSize = tbin::Vector2i( sheetSizeStr.mid( 0, sheetSizeSep ).toInt(), sheetSizeStr.mid( sheetSizeSep + 3 ).toInt() );
                            ts.tileSize = tbin::Vector2i( tileSizeStr.mid( 0, tileSizeSep ).toInt(), tileSizeStr.mid( tileSizeSep + 3 ).toInt() );
                            ts.margin = tbin::Vector2i( marginStr.mid( 0, marginSep ).toInt(), marginStr.mid( marginSep + 3 ).toInt() );
                            ts.spacing = tbin::Vector2i( spacingStr.mid( 0, spacingSep ).toInt(), spacingStr.mid( spacingSep + 3 ).toInt() );

                            xml.skipCurrentElement();
                        }
                        else if (xml.name() == QLatin1String("Properties"))
                            ts.props = readProps();
                        else xml.skipCurrentElement();
                    }

                    tmap.tilesheets.push_back( ts );
                }
            }
            else if ( xml.name() == QLatin1String("Layers") )
            {
                while ( xml.readNextStartElement() )
                {
                    if ( xml.name() != QLatin1String("Layer") )
                    {
                        xml.skipCurrentElement();
                        continue;
                    }
                    const QXmlStreamAttributes layerAttrs = xml.attributes();

                    tbin::Layer layer;
                    layer.id = layerAttrs.value( QLatin1String("Id") ).toString().toStdString();
                    auto vis = layerAttrs.value( QLatin1String("Visible") ).toString().toStdString();
                    layer.visible = QString::compare( layerAttrs.value( QLatin1String("Visible") ).toString(), QLatin1String("true"), Qt::CaseInsensitive ) == 0;

                    while ( xml.readNextStartElement() )
                    {
                        if ( xml.name() == QLatin1String( "Description" ) )
                            layer.desc = xml.readElementText().toStdString();
                        else if ( xml.name() == QLatin1String("Dimensions") )
                        {
                            const QXmlStreamAttributes alignAttrs = xml.attributes();

                            const QString layerSizeStr = alignAttrs.value( QLatin1String("LayerSize") ).toString();
                            const QString tileSizeStr = alignAttrs.value( QLatin1String("TileSize") ).toString();

                            qsizetype layerSizeSep = layerSizeStr.indexOf( QLatin1String(" x ") );
                            qsizetype tileSizeSep = tileSizeStr.indexOf( QLatin1String(" x ") );

                            layer.layerSize = tbin::Vector2i( layerSizeStr.mid( 0, layerSizeSep ).toInt(), layerSizeStr.mid( layerSizeSep + 3 ).toInt() );
                            layer.tileSize = tbin::Vector2i( tileSizeStr.mid( 0, tileSizeSep ).toInt(), tileSizeStr.mid( tileSizeSep + 3 ).toInt() );

                            xml.skipCurrentElement();
                        }
                        else if ( xml.name() == QLatin1String("TileArray") )
                        {
                            tbin::Tile nullTile;
                            nullTile.staticData.tileIndex = -1;
                            layer.tiles.resize( layer.layerSize.x * layer.layerSize.y, nullTile );

                            auto readStaticTile = [&xml, &readProps](const std::string& tilesheetToUse)-> tbin::Tile
                            {
                                const QXmlStreamAttributes tileAttrs = xml.attributes();

                                tbin::Tile tile;
                                tile.tilesheet = tilesheetToUse;
                                tile.staticData.tileIndex = tileAttrs.value( QLatin1String("Index") ).toString().toInt();
                                tile.staticData.blendMode = tileAttrs.value( QLatin1String("BlendMode") ).toString() == QLatin1String("Alpha") ? 0 : 1;

                                // tIDE (the xTile tile editor) uses a self-closing element when there are no properties.
                                // However, Qt seems to represent these simply as empty elements when reading.
                                // We can just iterate as normal for reading.
                                // When writing, we'll still need to make sure it's an actual self-closing element,
                                // for compatibility with xTile code.
                                while ( xml.readNextStartElement() )
                                {
                                    if (xml.name() == QLatin1String("Properties"))
                                        tile.props = readProps();
                                    else xml.skipCurrentElement();
                                }

                                return tile;
                            };

                            std::string lastTilesheet;
                            tbin::Vector2i tilePos;
                            while ( xml.readNextStartElement() )
                            {
                                if (xml.name() != QLatin1String("Row") )
                                {
                                    xml.skipCurrentElement();
                                    continue;
                                }

                                while ( xml.readNextStartElement() )
                                {
                                    if ( xml.name() == QLatin1String("TileSheet") )
                                    {
                                        const QXmlStreamAttributes tsAttrs = xml.attributes();
                                        lastTilesheet = tsAttrs.value( QLatin1String("Ref") ).toString().toStdString();
                                        xml.skipCurrentElement();
                                    }
                                    else if ( xml.name() == QLatin1String("Static") )
                                    {
                                        layer.tiles[ tilePos.x + tilePos.y * layer.layerSize.x ] = readStaticTile( lastTilesheet );
                                        tilePos.x += 1;
                                    }
                                    else if ( xml.name() == QLatin1String("Animated") )
                                    {
                                        const QXmlStreamAttributes tileAttrs = xml.attributes();

                                        tbin::Tile tile;
                                        tile.animatedData.frameInterval = tileAttrs.value( QLatin1String("Interval") ).toString().toInt();

                                        while ( xml.readNextStartElement() )
                                        {
                                            if (xml.name() == QLatin1String("Frames") )
                                            {
                                                std::string lastTilesheet; // Shadowing the previous declaration intentionally, because we explicitly do not want to use/affect it
                                                while ( xml.readNextStartElement() )
                                                {
                                                    if ( xml.name() == QLatin1String("TileSheet") )
                                                    {
                                                        const QXmlStreamAttributes tsAttrs = xml.attributes();
                                                        lastTilesheet = tsAttrs.value( QLatin1String("Ref") ).toString().toStdString();
                                                        xml.skipCurrentElement();
                                                    }
                                                    else if (xml.name() == QLatin1String("Static"))
                                                        tile.animatedData.frames.push_back( readStaticTile( lastTilesheet ) );
                                                    else xml.skipCurrentElement();
                                                }
                                            }
                                            else if (xml.name() == QLatin1String("Properties"))
                                                tile.props = readProps();
                                            else xml.skipCurrentElement();
                                        }

                                        layer.tiles[ tilePos.x + tilePos.y * layer.layerSize.x ] = tile;
                                        tilePos.x += 1;
                                    }
                                    else if ( xml.name() == QLatin1String("Null") )
                                    {
                                        const QXmlStreamAttributes tileAttrs = xml.attributes();
                                        tilePos.x += tileAttrs.value( QLatin1String("Count") ).toString().toInt();
                                        xml.skipCurrentElement();
                                    }
                                    else xml.skipCurrentElement();
                                }

                                tilePos.x = 0;
                                tilePos.y += 1;
                            }
                        }
                        else if (xml.name() == QLatin1String("Properties"))
                            layer.props = readProps();
                        else xml.skipCurrentElement();
                    }

                    tmap.layers.push_back( layer );
                }
            }
            else if (xml.name() == QLatin1String("Properties"))
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
            xml.writeStartElement( QStringLiteral("Properties") );
            for ( const auto& prop : props )
            {
                xml.writeStartElement( QStringLiteral("Property") );
                xml.writeAttribute( QStringLiteral("Key"), QString::fromStdString(prop.first) );
                switch ( prop.second.type )
                {
                    case tbin::PropertyValue::Bool:
                        xml.writeAttribute( QStringLiteral("Type"), QStringLiteral("Boolean") );
                        xml.writeCDATA( prop.second.data.b ? QStringLiteral("True") : QStringLiteral("False") );
                        break;
                    case tbin::PropertyValue::Integer:
                        xml.writeAttribute( QStringLiteral("Type"), QStringLiteral("Int32") );
                        xml.writeCDATA( QString::number( prop.second.data.i ) );
                        break;
                    case tbin::PropertyValue::Float:
                        xml.writeAttribute( QStringLiteral("Type"), QStringLiteral("Single") );
                        xml.writeCDATA( QString::number( prop.second.data.f ) );
                        break;
                    case tbin::PropertyValue::String:
                        xml.writeAttribute( QStringLiteral("Type"), QStringLiteral("String") );
                        xml.writeCDATA( QString::fromStdString(prop.second.dataStr) );
                        break;
                    default:
                        throw std::invalid_argument( "Unknown property type" );
                }
                xml.writeEndElement();
            }
            xml.writeEndElement();
        };

        xml.writeStartDocument();
        xml.writeStartElement( QStringLiteral("Map") );
        {
            xml.writeAttribute( QStringLiteral("Id"), QString::fromStdString(tmap.id) );

            xml.writeStartElement( QStringLiteral("Description") );
            xml.writeCDATA( QString::fromStdString(tmap.desc) );
            xml.writeEndElement();

            xml.writeStartElement(QStringLiteral("TileSheets"));
            {
                for ( const auto& ts : tmap.tilesheets )
                {
                    xml.writeStartElement( QStringLiteral("TileSheet") );
                    xml.writeAttribute( QStringLiteral("Id"), QString::fromStdString(ts.id) );

                    xml.writeStartElement( QStringLiteral("Description") );
                    xml.writeCDATA( QString::fromStdString(ts.desc) );
                    xml.writeEndElement();

                    xml.writeStartElement( QStringLiteral("ImageSource") );
                    xml.writeCDATA( QString::fromStdString(ts.image) );
                    xml.writeEndElement();

                    xml.writeEmptyElement( QStringLiteral("Alignment") );
                    xml.writeAttribute( QStringLiteral("SheetSize"), QStringLiteral( "%1 x %2" ).arg( ts.sheetSize.x ).arg( ts.sheetSize.y ) );
                    xml.writeAttribute( QStringLiteral("TileSize"),  QStringLiteral( "%1 x %2" ).arg( ts.tileSize.x  ).arg( ts.tileSize.y  ) );
                    xml.writeAttribute( QStringLiteral("Margin"),    QStringLiteral( "%1 x %2" ).arg( ts.margin.x    ).arg( ts.margin.y    ) );
                    xml.writeAttribute( QStringLiteral("Spacing"),   QStringLiteral( "%1 x %2" ).arg( ts.spacing.x   ).arg( ts.spacing.y   ) );

                    writeProps( ts.props );

                    xml.writeEndElement();
                }
            }
            xml.writeEndElement();

            xml.writeStartElement(QStringLiteral("Layers"));
            {
                for ( const auto& layer : tmap.layers )
                {
                    xml.writeStartElement( QStringLiteral("Layer") );
                    xml.writeAttribute( QStringLiteral("Id"), QString::fromStdString(layer.id) );
                    xml.writeAttribute( QStringLiteral("Visible"), layer.visible ? QStringLiteral("True") : QStringLiteral("False") );

                    xml.writeStartElement( QStringLiteral("Description") );
                    xml.writeCDATA( QString::fromStdString(layer.desc) );
                    xml.writeEndElement();

                    xml.writeEmptyElement( QStringLiteral("Dimensions") );
                    xml.writeAttribute( QStringLiteral("LayerSize"), QStringLiteral( "%1 x %2" ).arg( layer.layerSize.x ).arg( layer.layerSize.y ) );
                    xml.writeAttribute( QStringLiteral("TileSize"),  QStringLiteral( "%1 x %2" ).arg( layer.tileSize.x  ).arg( layer.tileSize.y  ) );

                    auto writeStaticTile = [&xml, &writeProps](const tbin::Tile& tile)
                    {
                        if ( tile.props.size() == 0 )
                            xml.writeEmptyElement( QStringLiteral("Static") );
                        else
                            xml.writeStartElement( QStringLiteral("Static") );
                        xml.writeAttribute( QStringLiteral("Index"), QString::number( tile.staticData.tileIndex ) );
                        switch ( tile.staticData.blendMode )
                        {
                            case 0: xml.writeAttribute( QStringLiteral("BlendMode"), QStringLiteral("Alpha") ); break;
                            case 1: xml.writeAttribute( QStringLiteral("BlendMode"), QStringLiteral("Additive") ); break;
                            default: throw std::invalid_argument( "Unsupported tile blend mode" );
                        }
                        if ( tile.props.size() > 0 )
                        {
                            writeProps( tile.props );
                            xml.writeEndElement();
                        }
                    };
                    xml.writeStartElement( QStringLiteral("TileArray") );
                    std::string lastTilesheet;
                    for ( int iy = 0; iy < layer.layerSize.y; ++iy )
                    {
                        xml.writeStartElement( QStringLiteral("Row") );
                        for ( int ix = 0; ix < layer.layerSize.x; ++ix )
                        {
                            tbin::Tile tile = layer.tiles[ ix + iy * layer.layerSize.x ];
                            if ( tile.tilesheet != "" && tile.tilesheet != lastTilesheet )
                            {
                                xml.writeEmptyElement( QStringLiteral("TileSheet") );
                                xml.writeAttribute( QStringLiteral("Ref"), QString::fromStdString(tile.tilesheet) );
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

                                xml.writeEmptyElement( QStringLiteral("Null") );
                                xml.writeAttribute( QStringLiteral("Count"), QString::number( nullCount ) );
                            }
                            else if ( tile.staticData.tileIndex != -1 )
                            {
                                writeStaticTile( tile );
                            }
                            else if ( tile.animatedData.frames.size() > 0 )
                            {
                                xml.writeStartElement( QStringLiteral("Animated") );
                                xml.writeAttribute( QStringLiteral("Interval"), QString::number( tile.animatedData.frameInterval ) );

                                xml.writeStartElement( QStringLiteral("Frames") );
                                std::string lastTilesheet; // Shadowing the previous declaration intentionally, because we explicitly do not want to use/affect it
                                for ( const auto& tile : tile.animatedData.frames )
                                {
                                    if ( tile.tilesheet != "" && tile.tilesheet != lastTilesheet )
                                    {
                                        xml.writeEmptyElement( QStringLiteral("TileSheet") );
                                        xml.writeAttribute( QStringLiteral("Ref"), QString::fromStdString(tile.tilesheet) );
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
