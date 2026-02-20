#ifndef TBIN_VECTOR2I_HPP
#define TBIN_VECTOR2I_HPP

#include <cstdint>

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

#endif // TBIN_VECTOR2I_HPP
