#ifndef TBIN_TILE_HPP
#define TBIN_TILE_HPP

#include <SFML/Config.hpp>
#include <vector>

#include "tbin/PropertyValue.hpp"

namespace tbin
{
    struct Tile
    {
        public:
            enum Type
            {
                Null,
                Static,
                Animated,
            };
            
            std::string tilesheet;
            
            struct
            {
                sf::Int32 tileIndex;
                sf::Uint8 blendMode;
            } staticData;
            
            struct
            {
                sf::Int32 frameInterval;
                std::vector< Tile > frames;
            } animatedData;
            
            Properties props;
            
            inline bool isNullTile() const { return staticData.tileIndex == -1 && animatedData.frames.size() == 0; }
    };
}

#endif // TBIN_TILE_HPP
                    
