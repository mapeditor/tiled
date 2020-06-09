/*
 * TBIN
 * Copyright 2017, Chase Warrington <spacechase0.and.cat@gmail.com>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Map.hpp"

#include <fstream>
#include <stdexcept>
#include <QDebug>

#include "FakeSfml.hpp"

namespace tbin
{
    template< typename T >
    T read( std::istream& in )
    {
        T t;
        in.read( reinterpret_cast< char* >( &t ), sizeof( T ) );
        return t;
    }

    template<>
    sf::Vector2i read< sf::Vector2i >( std::istream& in )
    {
        sf::Int32 x = read< sf::Int32 >( in );
        sf::Int32 y = read< sf::Int32 >( in );
        return sf::Vector2i( x, y );
    }

    template<>
    std::string read< std::string >( std::istream& in )
    {
        auto len = read< sf::Int32 >( in );
        std::string str( len, 0 );
        in.read( &str[ 0 ], len );
        return str;
    }

    template< typename T >
    void write( std::ostream& out, const T& t )
    {
        out.write( reinterpret_cast< const char* >( &t ), sizeof( T ) );
    }

    template<>
    void write< sf::Vector2i >( std::ostream& out, const sf::Vector2i& vec )
    {
        write< sf::Int32 >( out, vec.x );
        write< sf::Int32 >( out, vec.y );
    }

    template<>
    void write< std::string >( std::ostream& out, const std::string& str )
    {
        write< sf::Int32 >( out, str.length() );
        out.write( &str[ 0 ], str.length() );
    }

    Properties readProperties( std::istream& in )
    {
        Properties ret;

        int count = read< sf::Int32 >( in );
        for ( int i = 0; i < count; ++i )
        {
            std::string key;
            PropertyValue value;

            key = read< std::string >( in );
            value.type = static_cast< PropertyValue::Type >( read< sf::Uint8 >( in ) );
            switch ( value.type )
            {
                case PropertyValue::Bool:    value.data.b  = read< sf::Uint8   >( in ) > 0; break;
                case PropertyValue::Integer: value.data.i  = read< sf::Int32   >( in );     break;
                case PropertyValue::Float:   value.data.f  = read< float       >( in );     break;
                case PropertyValue::String:  value.dataStr = read< std::string >( in );     break;
                default: throw std::invalid_argument( QT_TRANSLATE_NOOP("TbinMapFormat", "Bad property type") );
            }

            ret[ key ] = value;
        }


        return ret;
    }

    void writeProperties( std::ostream& out, const Properties& props )
    {
        write< sf::Int32 >( out, props.size() );
        for ( const auto& prop : props )
        {
            write( out, prop.first );
            write< sf::Uint8 >( out, prop.second.type );
            switch ( prop.second.type )
            {
                case PropertyValue::Bool: write< sf::Uint8 >( out, prop.second.data.b ? 1 : 0 ); break;
                case PropertyValue::Integer: write( out, prop.second.data.i ); break;
                case PropertyValue::Float: write( out, prop.second.data.f ); break;
                case PropertyValue::String: write( out, prop.second.dataStr ); break;
                default: throw std::invalid_argument( QT_TRANSLATE_NOOP("TbinMapFormat", "Bad property type") );
            }
        }
    }

    TileSheet readTilesheet( std::istream& in )
    {
        TileSheet ret;
        ret.id = read< std::string >( in );
        ret.desc = read< std::string >( in );
        ret.image = read< std::string >( in );
        ret.sheetSize = read< sf::Vector2i >( in );
        ret.tileSize = read< sf::Vector2i >( in );
        ret.margin = read< sf::Vector2i >( in );
        ret.spacing = read< sf::Vector2i >( in );
        ret.props = readProperties( in );
        return ret;
    }

    void writeTilesheet( std::ostream& out, const TileSheet& ts )
    {
        write( out, ts.id );
        write( out, ts.desc );
        write( out, ts.image );
        write( out, ts.sheetSize );
        write( out, ts.tileSize );
        write( out, ts.margin );
        write( out, ts.spacing );
        writeProperties( out, ts.props );
    }

    Tile readStaticTile( std::istream& in, const std::string& currTilesheet )
    {
        Tile ret;
        ret.tilesheet = currTilesheet;
        ret.staticData.tileIndex = read< sf::Int32 >( in );
        ret.staticData.blendMode = read< sf::Uint8 >( in );
        ret.props = readProperties( in );
        return ret;
    }

    void writeStaticTile( std::ostream& out, const Tile& tile )
    {
        write( out, tile.staticData.tileIndex );
        write( out, tile.staticData.blendMode );
        writeProperties( out, tile.props );
    }

    Tile readAnimatedTile( std::istream& in )
    {
        Tile ret;
        ret.animatedData.frameInterval = read< sf::Int32 >( in );

        int frameCount = read< sf::Int32 >( in );
        ret.animatedData.frames.reserve( frameCount );
        std::string currTilesheet;
        for ( int i = 0; i < frameCount; )
        {
            char c = in.get();
            switch ( c )
            {
                case 'T':
                    currTilesheet = read< std::string >( in );
                    break;
                case 'S':
                    ret.animatedData.frames.push_back( readStaticTile( in, currTilesheet ) );
                    ++i;
                    break;
                default:
                    throw std::invalid_argument( QT_TRANSLATE_NOOP("TbinMapFormat", "Bad layer tile data") );
            }
        }

        ret.props = readProperties( in );

        return ret;
    }

    void writeAnimatedTile( std::ostream& out, const Tile& tile )
    {
        write( out, tile.animatedData.frameInterval );
        write< sf::Int32 >( out, tile.animatedData.frames.size() );

        std::string currTilesheet;
        for ( const Tile& frame : tile.animatedData.frames )
        {
            if ( frame.tilesheet != currTilesheet )
            {
                write< sf::Uint8 >( out, 'T' );
                write( out, frame.tilesheet );
                currTilesheet = frame.tilesheet;
            }

            write< sf::Uint8 >( out, 'S' );
            writeStaticTile( out, frame );
        }

        writeProperties( out, tile.props );
    }

    Layer readLayer( std::istream& in )
    {
        Layer ret;
        ret.id = read< std::string >( in );
        ret.visible = read< sf::Uint8 >( in ) > 0;
        ret.desc = read< std::string >( in );
        ret.layerSize = read< sf::Vector2i >( in );
        ret.tileSize = read< sf::Vector2i >( in );
        ret.props = readProperties( in );

        Tile nullTile; nullTile.staticData.tileIndex = -1;
        ret.tiles.resize( ret.layerSize.x * ret.layerSize.y, nullTile );

        std::string currTilesheet = "";
        for ( int iy = 0; iy < ret.layerSize.y; ++iy )
        {
            int ix = 0;
            while ( ix < ret.layerSize.x )
            {
                sf::Uint8 c = read< sf::Uint8 >( in );
                switch ( c )
                {
                    case 'N':
                        ix += read< sf::Int32 >( in );
                        break;
                    case 'S':
                        ret.tiles[ ix + iy * ret.layerSize.x ] = readStaticTile( in, currTilesheet );
                        ++ix;
                        break;
                    case 'A':
                        ret.tiles[ ix + iy * ret.layerSize.x ] = readAnimatedTile( in );
                        ++ix;
                        break;
                    case 'T':
                        currTilesheet = read< std::string >( in );
                        break;
                    default:
                        throw std::invalid_argument( QT_TRANSLATE_NOOP("TbinMapFormat", "Bad layer tile data") );
                }
            }
        }

        return ret;
    }

    void writeLayer( std::ostream& out, const Layer& layer )
    {
        write( out, layer.id );
        write< sf::Uint8 >( out, layer.visible ? 1 : 0 );
        write( out, layer.desc );
        write( out, layer.layerSize );
        write( out, layer.tileSize );
        writeProperties( out, layer.props );

        std::string currTilesheet = "";
        for ( int iy = 0; iy < layer.layerSize.y; ++iy )
        {
            sf::Int32 nulls = 0;
            for ( int ix = 0; ix < layer.layerSize.x; ++ix )
            {
                const Tile& tile = layer.tiles[ ix + iy * layer.layerSize.x ];

                if ( tile.isNullTile() )
                {
                    ++nulls;
                    continue;
                }

                if ( nulls > 0 )
                {
                    write< sf::Uint8 >( out, 'N' );
                    write( out, nulls );
                    nulls = 0;
                }

                if ( tile.tilesheet != currTilesheet )
                {
                    write< sf::Uint8 >( out, 'T' );
                    write( out, tile.tilesheet );
                    currTilesheet = tile.tilesheet;
                }

                if ( tile.animatedData.frames.size() == 0 )
                {
                    write< sf::Uint8 >( out, 'S' );
                    writeStaticTile( out, tile );
                }
                else
                {
                    write< sf::Uint8 >( out, 'A' );
                    writeAnimatedTile( out, tile );
                }
            }

            if ( nulls > 0 )
            {
                write< sf::Uint8 >( out, 'N' );
                write( out, nulls );
            }
        }
    }

    Q_DECL_CONSTEXPR const char* MAGIC_1_0 = "tBIN10";

    bool Map::loadFromFile( const std::string& path )
    {
        std::ifstream file( path, std::ifstream::binary );
        if ( !file )
        {
            throw std::runtime_error( QT_TRANSLATE_NOOP("TbinMapFormat", "Failed to open file.") );
        }

        return loadFromStream( file );
    }

    bool Map::loadFromStream( std::istream& in )
    {
        in.exceptions( std::ifstream::failbit );

        std::string magic( 6, '\0' );
        in.read( &magic[ 0 ], 6 );
        if ( magic != MAGIC_1_0 )
        {
            throw std::runtime_error( QT_TRANSLATE_NOOP("TbinMapFormat", "File is not a tbin file.") );
        }

        std::string id = read< std::string >( in );
        std::string desc = read< std::string >( in );
        Properties props = readProperties( in );

        std::vector< TileSheet > tilesheets;
        int tilesheetCount = read< sf::Int32 >( in );
        for ( int i = 0; i < tilesheetCount; ++i )
        {
            tilesheets.push_back( readTilesheet( in ) );
        }

        std::vector< Layer > layers;
        int layerCount = read< sf::Int32 >( in );
        for ( int i = 0; i < layerCount; ++i )
        {
            layers.push_back( readLayer( in ) );
        }

        std::swap( this->id, id );
        std::swap( this->desc, desc );
        std::swap( this->props, props );
        std::swap( this->tilesheets, tilesheets );
        std::swap( this->layers, layers );

        return true;
    }

    bool Map::saveToFile( const std::string& path ) const
    {
        std::ofstream file( path, std::ofstream::binary | std::ofstream::trunc );
        if ( !file )
        {
            throw std::runtime_error( QT_TRANSLATE_NOOP("TbinMapFormat", "Failed to open file") );
        }

        return saveToStream( file );
    }

    bool Map::saveToStream( std::ostream& out ) const
    {
        out.exceptions( std::ifstream::failbit );

        out.write( MAGIC_1_0, 6 );

        write( out, id );
        write( out, desc );
        writeProperties( out, props );

        write< sf::Int32 >( out, tilesheets.size() );
        for ( const TileSheet& ts : tilesheets )
            writeTilesheet( out, ts );

        write< sf::Int32 >( out, layers.size() );
        for ( const Layer& layer : layers )
            writeLayer( out, layer );

        return true;
    }
}
