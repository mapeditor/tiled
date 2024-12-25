Global Tile IDs
===============

Several of the map formats supported by Tiled, including its native
:doc:`TMX <tmx-map-format>` and :doc:`JSON <json-map-format>` map formats, use
the same data representation for individual tiles in layers: global tile IDs
with flip flags. These GIDs are "global" because they may refer to a tile from
any of the tilesets used by the map, rather than being local to a specific
tileset. To get at a specific tile from a GID, you will first need to extract
and clear the flip flags, then you will need to determine which tileset the
tile belongs to, and which tile within the tileset it is.

.. note::

    Despite the "global" name, GIDs are only global within a single map.
    A given tile may have a different GID in a different map, if that map has
    different tilesets, or has its tilesets in a different order.

.. _gid-tile-flipping:

Tile Flipping
-------------

The highest four bits of the 32-bit GID are flip flags, and you will need to
read and clear them before you can access the GID itself to identify the tile.

Bit 32 is used for storing whether the tile is horizontally flipped, bit 31 is
used for the vertically flipped tiles. In orthogonal and isometric maps,
bit 30 indicates whether the tile is flipped (anti) diagonally, which enables
tile rotation, and bit 29 can be ignored. In hexagonal maps, bit 30 indicates
whether the tile is rotated 60 degrees clockwise, and bit 29 indicates
120 degrees clockwise rotation.

.. note::

    Even if you're parsing a non-hexagonal map, remember to clear bit 29 after
    you've read the flags. Tiled keeps and outputs that flag even if the map
    orientation is changed. If not cleared, you may get an invalid tile ID.

When rendering an orthographic or isometric tile, the order of operations
matters. The diagonal flip is done first, followed by the horizontal and
vertical flips. The diagonal flip should flip the bottom left and top right
corners of the tile, and can be thought of as an x/y axis swap. For hexagonal
tiles, the order does not matter.

.. _gid-mapping:

Mapping a GID to a Local Tile ID
--------------------------------

Every tileset has its own, independent local tile IDs, typically (but not
always) starting at 0. To avoid conflicts within maps using multiple tilesets,
GIDs are assigned in sequence based on the size of each tileset. Each tileset
is assigned a ``firstgid`` within the map, this is the GID that the tile with
local ID 0 in the tileset would have.

To figure out which tileset a tile belongs to, find the tileset that has the
largest ``firstgid`` that is smaller than or equal to the tile's GID. Once you
have identified the tileset, subtract its ``firstgid`` from the tile's GID to
get the local ID of the tile within the tileset.

.. note::

    The ``firstgid`` of the first tileset is always 1. A GID of 0 in a layer
    means that cell is empty.

As an example, here's an excerpt from a TMX file with three tilesets:

.. code:: xml

   <tileset firstgid="1" source="TilesetA.tsx"/>
   <tileset firstgid="65" source="TilesetB.tsx"/>
   <tileset firstgid="115" source="TilesetC.tsx"/>

In this map, tiles with GIDs 1-64 would be part of TilesetA, tiles with GIDs
65-114 would be part of TilesetB, and tiles with GIDs 115 and above would be
part of tileset C. A tile with GID 72 would be part of TilesetB since TilesetB
has the largest ``firstgid`` that's less than or equal to 72, and its local ID
would be 7 (72-65).

.. _gid-code-example:

Code example
------------

The following C++ pseudo-code, using TMX as an example, should make it all
clear, it deals with flags and deduces the appropriate tileset:

.. code:: cpp

   // Bits on the far end of the 32-bit global tile ID are used for tile flags
   const unsigned FLIPPED_HORIZONTALLY_FLAG  = 0x80000000;
   const unsigned FLIPPED_VERTICALLY_FLAG    = 0x40000000;
   const unsigned FLIPPED_DIAGONALLY_FLAG    = 0x20000000;
   const unsigned ROTATED_HEXAGONAL_120_FLAG = 0x10000000;

   ...

   // Extract the contents of the <data> element
   string tile_data = ...
   
   // If the data is encoded and compressed, decode and decompress:
   unsigned char *data = decompress(base64_decode(tile_data));
   
   unsigned tile_index = 0;

   // Here you should check that the data has the right size
   // (map_width * map_height * 4)

   for (int y = 0; y < map_height; ++y) {
     for (int x = 0; x < map_width; ++x) {
       //Read the GID in little-endian byte order:
       unsigned global_tile_id = data[tile_index] |
                                 data[tile_index + 1] << 8 |
                                 data[tile_index + 2] << 16 |
                                 data[tile_index + 3] << 24;
       tile_index += 4;

       // Read out the flags
       bool flipped_horizontally = (global_tile_id & FLIPPED_HORIZONTALLY_FLAG);
       bool flipped_vertically = (global_tile_id & FLIPPED_VERTICALLY_FLAG);
       bool flipped_diagonally = (global_tile_id & FLIPPED_DIAGONALLY_FLAG);
       bool rotated_hex120 = (global_tile_id & ROTATED_HEXAGONAL_120_FLAG);

       // Clear all four flags
       global_tile_id &= ~(FLIPPED_HORIZONTALLY_FLAG |
                           FLIPPED_VERTICALLY_FLAG |
                           FLIPPED_DIAGONALLY_FLAG |
                           ROTATED_HEXAGONAL_120_FLAG);

       // Resolve the tile
       for (int i = tileset_count - 1; i >= 0; --i) {
         Tileset *tileset = tilesets[i];

         if (tileset->first_gid() <= global_tile_id) {
           tiles[y][x] = tileset->get_tile(global_tile_id - tileset->first_gid());
           break;
         }
       }
     }
   }

(Since the above code was put together on this wiki page and can't be
directly tested, please make sure to report any errors you encounter
when basing your parsing code on it, thanks!)
