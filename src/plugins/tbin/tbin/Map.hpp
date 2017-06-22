#ifndef TBIN_MAP_HPP
#define TBIN_MAP_HPP

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "Layer.hpp"
#include "PropertyValue.hpp"
#include "TileSheet.hpp"

namespace tbin
{
    class Map
    {
        public:
            bool loadFromFile( const std::string& path );
            bool loadFromStream( std::istream& in );
            
            bool saveToFile( const std::string& path ) const;
            bool saveToStream( std::ostream& out ) const;
            
            std::string id;
            std::string desc;
            Properties props;
            std::vector< TileSheet > tilesheets;
            std::vector< Layer > layers;
    };
}

#endif // TBIN_MAP_HPP
