#ifndef FAKESFML_HPP
#define FAKESFML_HPP

#include <cstdlib>

namespace tbin
{
    struct Vector2i
    {
        int32_t x;
        int32_t y;

        Vector2i() : x(0),y(0) {}
        Vector2i( int32_t x, int32_t y ) : x(x), y(y) {}
    };
}

#endif // FAKESFML_HPP
