Binary Map Format Specification
===============================

Simplest format possible. Consist of a 16 bytes long header and then data.
All numbers are little-endian, and except of the magic all are uint16_t's.
It does not store map properties, tileset specification, objects, etc.;
deliberately just the pure tile identifiers and nothing else.

Header
------

| Offset | Size | Description               |
|-------:|-----:|---------------------------|
|      0 |    3 | magic bytes `MAP`         |
|      3 |    1 | format version (always 0) |
|      4 |    2 | map width in tiles        |
|      6 |    2 | map height in tiles       |
|      8 |    2 | number of layers          |
|     10 |    2 | tile width in pixels      |
|     12 |    2 | tile height in pixels     |
|     14 |    2 | orientation               |

Orientation goes like: 0 = orthographic, 1 = isometric, 2 = hexagonal horizontal, 3 = hexagonal vertical.

Layer Data
----------

The header is followed by *number of layers* layer blocks, each *map height x map width x 2* bytes long.

Within each layer, first comes the top row, then the second row, etc. Within each row first the left most
column comes, then the second column, etc. Each cell value is a tile id stored as an unsigned short value
on 2 bytes (just the global tile id, without flags).
