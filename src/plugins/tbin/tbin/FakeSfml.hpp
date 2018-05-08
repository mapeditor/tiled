#ifndef FAKESFML_HPP
#define FAKESFML_HPP

namespace sf
{
    typedef int Int32;
    typedef unsigned char Uint8;
    struct Vector2i
    {
        Int32 x;
        Int32 y;

        Vector2i() : x(0),y(0) {}
        Vector2i( Int32 x, Int32 y ) : x(x), y(y) {}
    };
}

#endif // FAKESFML_HPP
