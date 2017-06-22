#ifndef TBIN_LAYER_HPP
#define TBIN_LAYER_HPP

#include <SFML/System/Vector2.hpp>
#include <string>
#include <vector>

#include "fakeSfml.hpp"
#include "tbin/PropertyValue.hpp"
#include "tbin/Tile.hpp"

namespace tbin
{
    struct Layer
    {
        public:
            std::string id;
            bool visible;
            std::string desc;
            sf::Vector2i layerSize;
            sf::Vector2i tileSize;
            Properties props;
            std::vector< Tile > tiles;
    };
}

#endif // TBIN_LAYER_HPP
