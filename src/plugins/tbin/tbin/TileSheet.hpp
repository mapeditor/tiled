#ifndef TBIN_TILESHEET_HPP
#define TBIN_TILESHEET_HPP

#include <SFML/System/Vector2.hpp>
#include <string>

#include "tbin/PropertyValue.hpp"

namespace tbin
{
    struct TileSheet
    {
        public:
            std::string id;
            std::string desc;
            std::string image;
            sf::Vector2i sheetSize;
            sf::Vector2i tileSize;
            sf::Vector2i margin;
            sf::Vector2i spacing;
            Properties props;
    };
}

#endif // TBIN_TILESHEET_HPP
