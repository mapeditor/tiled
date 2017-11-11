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

#ifndef TBIN_TILE_HPP
#define TBIN_TILE_HPP

//#include <SFML/Config.hpp>
#include <vector>

#include "FakeSfml.hpp"
#include "PropertyValue.hpp"

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
                    
