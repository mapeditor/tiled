TMX Map Format
==============

**Version 1.8**

TMX and TSX are `Tiled <http://www.mapeditor.org>`__'s own formats for storing
tile maps and tilesets, based on XML. TMX provides a flexible way to describe a
tile based map. It can describe maps with any tile size, any amount of
layers, any number of tile sets and it allows custom properties to be
set on most elements. Beside tile layers, it can also contain groups of
objects that can be placed freely.

Note that there are many :doc:`libraries and frameworks <support-for-tmx-maps>`
available that can work with TMX maps and TSX tilesets.

In this document we'll go through each element found in these file formats.
The elements are mentioned in the headers and the list of attributes of
the elements are listed right below, followed by a short explanation.
Attributes or elements that are deprecated or unsupported by the current
version of Tiled are formatted in italics. All optional attributes are
either marked as optional, or have a default value to imply that they are
optional.

Have a look at the :doc:`changelog <tmx-changelog>` when you're interested
in what changed between Tiled versions.

.. note::

    A DTD-file (Document Type Definition) is served at
    http://mapeditor.org/dtd/1.0/map.dtd. This file is not up-to-date but might
    be useful for XML-namespacing anyway.

.. note::

    For compatibility reasons, it is recommended to ignore unknown elements and
    attributes (or raise a warning). This makes it easier to add features
    without breaking backwards compatibility, and allows custom variants and
    additions to work with existing tools.

.. _tmx-map:

<map>
-----

-  **version:** The TMX format version. Was "1.0" so far, and will be
   incremented to match minor Tiled releases.
-  **tiledversion:** The Tiled version used to save the file (since Tiled
   1.0.1). May be a date (for snapshot builds). (optional)
-  **class:** The class of this map (since 1.9, defaults to "").
-  **orientation:** Map orientation. Tiled supports "orthogonal",
   "isometric", "staggered" and "hexagonal" (since 0.11).
-  **renderorder:** The order in which tiles on tile layers are rendered.
   Valid values are ``right-down`` (the default), ``right-up``,
   ``left-down`` and ``left-up``. In all cases, the map is drawn
   row-by-row. (only supported for orthogonal maps at the moment)
-  **compressionlevel:** The compression level to use for tile layer data
   (defaults to -1, which means to use the algorithm default).
-  **width:** The map width in tiles.
-  **height:** The map height in tiles.
-  **tilewidth:** The width of a tile.
-  **tileheight:** The height of a tile.
-  **hexsidelength:** Only for hexagonal maps. Determines the width or
   height (depending on the staggered axis) of the tile's edge, in
   pixels.
-  **staggeraxis:** For staggered and hexagonal maps, determines which axis
   ("x" or "y") is staggered. (since 0.11)
-  **staggerindex:** For staggered and hexagonal maps, determines whether
   the "even" or "odd" indexes along the staggered axis are shifted.
   (since 0.11)
-  **parallaxoriginx:** X coordinate of the parallax origin in pixels
   (defaults to 0). (since 1.8)
-  **parallaxoriginy:** Y coordinate of the parallax origin in pixels
   (defaults to 0). (since 1.8)
-  **backgroundcolor:** The background color of the map. (optional, may
   include alpha value since 0.15 in the form ``#AARRGGBB``. Defaults to
   fully transparent.)
-  **nextlayerid:** Stores the next available ID for new layers. This
   number is stored to prevent reuse of the same ID after layers have
   been removed. (since 1.2) (defaults to the highest layer id in the file
   + 1)
-  **nextobjectid:** Stores the next available ID for new objects. This
   number is stored to prevent reuse of the same ID after objects have
   been removed. (since 0.11) (defaults to the highest object id in the file
   + 1)
-  **infinite:** Whether this map is infinite. An infinite map has no fixed
   size and can grow in all directions. Its layer data is stored in chunks.
   (``0`` for false, ``1`` for true, defaults to 0)

The ``tilewidth`` and ``tileheight`` properties determine the general
grid size of the map. The individual tiles may have different sizes.
Larger tiles will extend at the top and right (anchored to the bottom
left).

A map contains three different kinds of layers. Tile layers were once
the only type, and are simply called ``layer``, object layers have the
``objectgroup`` tag and image layers use the ``imagelayer`` tag. The
order in which these layers appear is the order in which the layers are
rendered by Tiled.

The ``staggered`` orientation refers to an isometric map using staggered
axes.

The tilesets used by the map should always be listed before the layers.

Can contain at most one: :ref:`tmx-properties`,
:ref:`tmx-editorsettings` (since 1.3)

Can contain any number: :ref:`tmx-tileset`, :ref:`tmx-layer`,
:ref:`tmx-objectgroup`, :ref:`tmx-imagelayer`, :ref:`tmx-group` (since 1.0)

.. _tmx-editorsettings:

<editorsettings>
----------------

This element contains various editor-specific settings, which are generally
not relevant when reading a map.

Can contain at most one: :ref:`tmx-chunksize`, :ref:`tmx-export`

.. _tmx-chunksize:

<chunksize>
~~~~~~~~~~~

-  **width:** The width of chunks used for infinite maps (default to 16).
-  **height:** The width of chunks used for infinite maps (default to 16).

.. _tmx-export:

<export>
~~~~~~~~

-  **target:** The last file this map was exported to.
-  **format:** The short name of the last format this map was exported as.

.. _tmx-tileset:

<tileset>
---------

-  **firstgid:** The first global tile ID of this tileset (this global ID
   maps to the first tile in this tileset).
-  **source:** If this tileset is stored in an external TSX (Tile Set XML)
   file, this attribute refers to that file. That TSX file has the same
   structure as the :ref:`tmx-tileset` element described here. (There is the
   firstgid attribute missing and this source attribute is also not
   there. These two attributes are kept in the TMX map, since they are
   map specific.)
-  **name:** The name of this tileset.
-  **class:** The class of this tileset (since 1.9, defaults to "").
-  **tilewidth:** The (maximum) width of the tiles in this tileset. Irrelevant
   for image collection tilesets, but stores the maximum tile width.
-  **tileheight:** The (maximum) height of the tiles in this tileset.
   Irrelevant for image collection tilesets, but stores the maximum tile
   height.
-  **spacing:** The spacing in pixels between the tiles in this tileset
   (applies to the tileset image, defaults to 0). Irrelevant for image
   collection tilesets.
-  **margin:** The margin around the tiles in this tileset (applies to the
   tileset image, defaults to 0). Irrelevant for image collection tilesets.
-  **tilecount:** The number of tiles in this tileset (since 0.13). Note that
   there can be tiles with a higher ID than the tile count, in case the tileset
   is an image collection from which tiles have been removed.
-  **columns:** The number of tile columns in the tileset. For image
   collection tilesets it is editable and is used when displaying the
   tileset. (since 0.15)
-  **objectalignment:** Controls the alignment for tile objects.
   Valid values are ``unspecified``, ``topleft``, ``top``, ``topright``,
   ``left``, ``center``, ``right``, ``bottomleft``, ``bottom`` and
   ``bottomright``. The default value is ``unspecified``, for compatibility
   reasons. When unspecified, tile objects use ``bottomleft`` in orthogonal mode
   and ``bottom`` in isometric mode. (since 1.4)
-  **tilerendersize:** The size to use when rendering tiles from this tileset
   on a tile layer. Valid values are ``tile`` (the default) and ``grid``. When
   set to ``grid``, the tile is drawn at the tile grid size of the map. (since 1.9)
-  **fillmode:** The fill mode to use when rendering tiles from this tileset.
   Valid values are ``stretch`` (the default) and ``preserve-aspect-fit``. Only
   relevant when the tiles are not rendered at their native size, so this
   applies to resized tile objects or in combination with ``tilerendersize`` set
   to ``grid``. (since 1.9)

A tileset can be either *based on a single image*, which is cut into tiles
based on the given parameters, or a *collection of images*, in which case each
tile defines its own image. In the first case there is a single child
:ref:`tmx-image` element. In the latter case, each child
:ref:`tmx-tileset-tile` element contains an :ref:`tmx-image` element.

If there are multiple :ref:`tmx-tileset` elements, they are in ascending
order of their ``firstgid`` attribute. The first tileset always has a
``firstgid`` value of 1. Since Tiled 0.15, image collection tilesets do
not necessarily number their tiles consecutively since gaps can occur
when removing tiles.

Can contain at most one: :ref:`tmx-image`, :ref:`tmx-tileoffset`,
:ref:`tmx-grid` (since 1.0), :ref:`tmx-properties`, :ref:`tmx-terraintypes`,
:ref:`tmx-wangsets` (since 1.1), :ref:`tmx-tileset-transformations` (since 1.5)

Can contain any number: :ref:`tmx-tileset-tile`

.. _tmx-tileoffset:

<tileoffset>
~~~~~~~~~~~~

-  **x:** Horizontal offset in pixels. (defaults to 0)
-  **y:** Vertical offset in pixels (positive is down, defaults to 0)

This element is used to specify an offset in pixels, to be applied when
drawing a tile from the related tileset. When not present, no offset is
applied.

.. _tmx-grid:

<grid>
~~~~~~

-  **orientation:** Orientation of the grid for the tiles in this
   tileset (``orthogonal`` or ``isometric``, defaults to ``orthogonal``)
-  **width:** Width of a grid cell
-  **height:** Height of a grid cell

This element is only used in case of isometric orientation, and
determines how tile overlays for terrain and collision information are
rendered.

.. _tmx-image:

<image>
~~~~~~~

-  **format:** Used for embedded images, in combination with a :ref:`tmx-data`
   child element. Valid values are file extensions like ``png``,
   ``gif``, ``jpg``, ``bmp``, etc.
-  *id:* Used by some versions of Tiled Java. Deprecated and unsupported.
-  **source:** The reference to the tileset image file (Tiled supports most
   common image formats). Only used if the image is not embedded.
-  **trans:** Defines a specific color that is treated as transparent
   (example value: "#FF00FF" for magenta). Including the "#" is optional
   and Tiled leaves it out for compatibility reasons. (optional)
-  **width:** The image width in pixels (optional, used for tile index
   correction when the image changes)
-  **height:** The image height in pixels (optional)

Tiled maps or tilesets with embedded image data can currently only be created
using the :doc:`JavaScript API </manual/scripting>`, or in custom tools
based on ``libtiled`` (Qt/C++) or `tmxlib
<https://pypi.python.org/pypi/tmxlib>`__ (Python).

Can contain at most one: :ref:`tmx-data`

.. _tmx-terraintypes:

<terraintypes>
~~~~~~~~~~~~~~

**Deprecated:** This element has been deprecated since Tiled 1.5, in favour of
the :ref:`tmx-wangsets` element, which is more flexible. Tilesets containing
terrain types are automatically saved with a Wang set instead.

This element defines an array of terrain types, which can be referenced
from the ``terrain`` attribute of the :ref:`tmx-tileset-tile` element.

Can contain any number: :ref:`tmx-terrain`

.. _tmx-terrain:

<terrain>
^^^^^^^^^

**Deprecated:** This element has been deprecated since Tiled 1.5, in favour of
the :ref:`tmx-wangcolor` element.

-  **name:** The name of the terrain type.
-  **tile:** The local tile-id of the tile that represents the terrain
   visually.

Can contain at most one: :ref:`tmx-properties`

.. _tmx-tileset-transformations:

<transformations>
~~~~~~~~~~~~~~~~~

This element is used to describe which transformations can be applied to the
tiles (e.g. to extend a Wang set by transforming existing tiles).

- **hflip:** Whether the tiles in this set can be flipped horizontally (default 0)
- **vflip:** Whether the tiles in this set can be flipped vertically (default 0)
- **rotate:** Whether the tiles in this set can be rotated in 90 degree increments (default 0)
- **preferuntransformed:** Whether untransformed tiles remain preferred, otherwise
  transformed tiles are used to produce more variations (default 0)

.. _tmx-tileset-tile:

<tile>
~~~~~~

-  **id:** The local tile ID within its tileset.
-  **type:** The class of the tile. Is inherited by tile objects. (since 1.0,
   defaults to "", was saved as ``class`` in 1.9)
-  *terrain:* Defines the terrain type of each corner of the tile,
   given as comma-separated indexes in the terrain types array in the
   order top-left, top-right, bottom-left, bottom-right. Leaving out a
   value means that corner has no terrain. (deprecated since 1.5 in favour of
   :ref:`tmx-wangtile`)
-  **probability:** A percentage indicating the probability that this
   tile is chosen when it competes with others while editing with the
   terrain tool. (defaults to 0)
-  **x:** The X position of the sub-rectangle representing this tile (default: 0)
-  **y:** The Y position of the sub-rectangle representing this tile (default: 0)
-  **width:** The width of the sub-rectangle representing this tile (defaults to the image width)
-  **height:** The height of the sub-rectangle representing this tile (defaults to the image height)

Can contain at most one: :ref:`tmx-properties`, :ref:`tmx-image` (since
0.9), :ref:`tmx-objectgroup`, :ref:`tmx-animation`

.. _tmx-animation:

<animation>
^^^^^^^^^^^

Contains a list of animation frames.

Each tile can have exactly one animation associated with it. In the
future, there could be support for multiple named animations on a tile.

Can contain any number: :ref:`tmx-frame`

.. _tmx-frame:

<frame>
'''''''

-  **tileid:** The local ID of a tile within the parent
   :ref:`tmx-tileset`.
-  **duration:** How long (in milliseconds) this frame should be displayed
   before advancing to the next frame.

.. _tmx-wangsets:

<wangsets>
~~~~~~~~~~

Contains the list of Wang sets defined for this tileset.

Can contain any number: :ref:`tmx-wangset`

.. _tmx-wangset:

<wangset>
^^^^^^^^^

Defines a list of colors and any number of Wang tiles using these colors.

-  **name:** The name of the Wang set.
-  **class:** The class of the Wang set (since 1.9, defaults to "").
-  **tile:** The tile ID of the tile representing this Wang set.

Can contain at most one: :ref:`tmx-properties`

Can contain up to 254: :ref:`tmx-wangcolor` (255 since Tiled 1.5, 254 since Tiled 1.10.2)

Can contain any number: :ref:`tmx-wangtile`

.. _tmx-wangcolor:

<wangcolor>
'''''''''''

A color that can be used to define the corner and/or edge of a Wang tile.

-  **name:** The name of this color.
-  **class:** The class of this color (since 1.9, defaults to "").
-  **color:** The color in ``#RRGGBB`` format (example: ``#c17d11``).
-  **tile:** The tile ID of the tile representing this color.
-  **probability:** The relative probability that this color is chosen
   over others in case of multiple options. (defaults to 0)

Can contain at most one: :ref:`tmx-properties`

.. _tmx-wangtile:

<wangtile>
''''''''''

Defines a Wang tile, by referring to a tile in the tileset and
associating it with a certain Wang ID.

-  **tileid:** The tile ID.
-  **wangid:** The Wang ID, since Tiled 1.5 given by a comma-separated list of
   indexes (0-254) referring to the Wang colors in the Wang set in the order:
   top, top-right, right, bottom-right, bottom, bottom-left, left, top-left.
   Index 0 means *unset* and index 1 refers to the first Wang color. Before
   Tiled 1.5, the Wang ID was saved as a 32-bit unsigned integer stored in the
   format ``0xCECECECE`` (where each C is a corner color and each E is an edge
   color, in reverse order).
-  *hflip:* Whether the tile is flipped horizontally (removed in Tiled 1.5).
-  *vflip:* Whether the tile is flipped vertically (removed in Tiled 1.5).
-  *dflip:* Whether the tile is flipped on its diagonal (removed in Tiled 1.5).

.. _tmx-layer:

<layer>
-------

All :ref:`tmx-tileset` tags shall occur before the first :ref:`tmx-layer` tag
so that parsers may rely on having the tilesets before needing to resolve
tiles.

-  **id:** Unique ID of the layer (defaults to 0, with valid IDs being at least
   1). Each layer that added to a map gets a unique id. Even if a layer is
   deleted, no layer ever gets the same ID. Can not be changed in Tiled.
   (since Tiled 1.2)
-  **name:** The name of the layer. (defaults to "")
-  **class:** The class of the layer (since 1.9, defaults to "").
-  *x:* The x coordinate of the layer in tiles. Defaults to 0 and can not be changed in Tiled.
-  *y:* The y coordinate of the layer in tiles. Defaults to 0 and can not be changed in Tiled.
-  **width:** The width of the layer in tiles. Always the same as the map width for fixed-size maps.
-  **height:** The height of the layer in tiles. Always the same as the map height for fixed-size maps.
-  **opacity:** The opacity of the layer as a value from 0 to 1. Defaults to 1.
-  **visible:** Whether the layer is shown (1) or hidden (0). Defaults to 1.
-  **tintcolor:** A :ref:`tint color <tint-color>` that is multiplied with any tiles drawn by this layer in ``#AARRGGBB`` or ``#RRGGBB`` format (optional).
-  **offsetx:** Horizontal offset for this layer in pixels. Defaults to 0.
   (since 0.14)
-  **offsety:** Vertical offset for this layer in pixels. Defaults to 0.
   (since 0.14)
-  **parallaxx:** Horizontal :ref:`parallax factor <parallax-factor>` for this layer. Defaults to 1. (since 1.5)
-  **parallaxy:** Vertical :ref:`parallax factor <parallax-factor>` for this layer. Defaults to 1. (since 1.5)

Can contain at most one: :ref:`tmx-properties`, :ref:`tmx-data`

.. _tmx-data:

<data>
~~~~~~

-  **encoding:** The encoding used to encode the tile layer data. When used,
   it can be "base64" and, when used for tile layer data, "csv". (optional)
-  **compression:** The compression used to compress the tile layer data.
   Tiled supports "gzip", "zlib" and (as a compile-time option since Tiled 1.3)
   "zstd".

This element is usually used as a child of a :ref:`tmx-layer` element, and
contains the actual tile layer data. It can also occur as a child of
an :ref:`tmx-image` element, where it can store embedded image data.

When no encoding or compression is given, the tiles are stored as individual
XML :ref:`tmx-tilelayer-tile` elements, but this option is deprecated. Next to
that, the easiest format to parse is the "csv" (comma separated values) format.

The base64-encoded and optionally compressed layer data is somewhat more
complicated to parse. First you need to base64-decode it, then you may
need to decompress it. Now you have an array of bytes, which should be
interpreted as an array of unsigned 32-bit integers using little-endian
byte ordering.

Whatever format you choose for your layer data, you will always end up with so
called ":doc:`global-tile-ids`" (gids). They are called "global", since they
may refer to a tile from any of the tilesets used by the map. The IDs also
contain :ref:`flipping flags <gid-tile-flipping>`. The tilesets are always
stored with increasing ``firstgid``\ s.

Can contain any number: :ref:`tmx-tilelayer-tile`, :ref:`tmx-chunk`

.. _tmx-chunk:

<chunk>
~~~~~~~

-  **x:** The x coordinate of the chunk in tiles.
-  **y:** The y coordinate of the chunk in tiles.
-  **width:** The width of the chunk in tiles.
-  **height:** The height of the chunk in tiles.

This is currently added only for infinite maps. The contents of a chunk
element is same as that of the :ref:`tmx-data` element, except it stores the
data of the area specified in the attributes.

Can contain any number: :ref:`tmx-tilelayer-tile`

.. _tmx-tilelayer-tile:

<tile>
~~~~~~

-  **gid:** The global tile ID (default: 0).

Not to be confused with the :ref:`tmx-tileset-tile` element inside a
:ref:`tmx-tileset`, this element defines the value of a single tile on a tile
layer. This is however the most inefficient way of storing the tile layer data,
and should generally be avoided.

.. _tmx-objectgroup:

<objectgroup>
-------------

-  **id:** Unique ID of the layer (defaults to 0, with valid IDs being at least
   1). Each layer that added to a map gets a unique id. Even if a layer is
   deleted, no layer ever gets the same ID. Can not be changed in Tiled.
   (since Tiled 1.2)
-  **name:** The name of the object group. (defaults to "")
-  **class:** The class of the object group (since 1.9, defaults to "").
-  **color:** The color used to display the objects in this group. (optional)
-  *x:* The x coordinate of the object group in tiles. Defaults to 0 and
   can no longer be changed in Tiled.
-  *y:* The y coordinate of the object group in tiles. Defaults to 0 and
   can no longer be changed in Tiled.
-  *width:* The width of the object group in tiles. Meaningless.
-  *height:* The height of the object group in tiles. Meaningless.
-  **opacity:** The opacity of the layer as a value from 0 to 1. (defaults to
   1)
-  **visible:** Whether the layer is shown (1) or hidden (0). (defaults to 1)
-  **tintcolor:** A color that is multiplied with any tile objects drawn by this layer, in ``#AARRGGBB`` or ``#RRGGBB`` format (optional).
-  **offsetx:** Horizontal offset for this object group in pixels. (defaults
   to 0) (since 0.14)
-  **offsety:** Vertical offset for this object group in pixels. (defaults
   to 0) (since 0.14)
-  **parallaxx:** Horizontal :ref:`parallax factor <parallax-factor>` for this object group. Defaults to 1. (since 1.5)
-  **parallaxy:** Vertical :ref:`parallax factor <parallax-factor>` for this object group. Defaults to 1. (since 1.5)
-  **draworder:** Whether the objects are drawn according to the order of
   appearance ("index") or sorted by their y-coordinate ("topdown").
   (defaults to "topdown")

The object group is in fact a map layer, and is hence called "object
layer" in Tiled.

Can contain at most one: :ref:`tmx-properties`

Can contain any number: :ref:`tmx-object`

.. _tmx-object:

<object>
~~~~~~~~

-  **id:** Unique ID of the object (defaults to 0, with valid IDs being at
   least 1). Each object that is placed on a map gets a unique id. Even if an
   object was deleted, no object gets the same ID. Can not be changed in Tiled.
   (since Tiled 0.11)
-  **name:** The name of the object. An arbitrary string. (defaults to "")
-  **type:** The class of the object. An arbitrary string. (defaults to "",
   was saved as ``class`` in 1.9)
-  **x:** The x coordinate of the object in pixels. (defaults to 0)
-  **y:** The y coordinate of the object in pixels. (defaults to 0)
-  **width:** The width of the object in pixels. (defaults to 0)
-  **height:** The height of the object in pixels. (defaults to 0)
-  **rotation:** The rotation of the object in degrees clockwise around (x, y). 
   (defaults to 0)
-  **gid:** A reference to a tile. (optional)
-  **visible:** Whether the object is shown (1) or hidden (0). (defaults to
   1)
-  **template:** A reference to a :ref:`template file <tmx-template-files>`. (optional)

While tile layers are very suitable for anything repetitive aligned to
the tile grid, sometimes you want to annotate your map with other
information, not necessarily aligned to the grid. Hence the objects have
their coordinates and size in pixels, but you can still easily align
that to the grid when you want to.

You generally use objects to add custom information to your tile map,
such as spawn points, warps, exits, etc.

When the object has a ``gid`` set, then it is represented by the image
of the tile with that global ID. The image alignment currently depends
on the map orientation. In orthogonal orientation it's aligned to the
bottom-left while in isometric it's aligned to the bottom-center. The
image will rotate around the bottom-left or bottom-center, respectively.

When the object has a ``template`` set, it will borrow all the
properties from the specified template, properties saved with the object
will have higher priority, i.e. they will override the template
properties.

Can contain at most one: :ref:`tmx-properties`, :ref:`tmx-ellipse` (since
0.9), :ref:`tmx-point` (since 1.1), :ref:`tmx-polygon`, :ref:`tmx-polyline`,
:ref:`tmx-text` (since 1.0)

.. _tmx-ellipse:

<ellipse>
~~~~~~~~~

Used to mark an object as an ellipse. The existing ``x``, ``y``,
``width`` and ``height`` attributes are used to determine the size of
the ellipse.

.. _tmx-point:

<point>
~~~~~~~~~

Used to mark an object as a point. The existing ``x`` and ``y`` attributes
are used to determine the position of the point.

.. _tmx-polygon:

<polygon>
~~~~~~~~~

-  **points:** A list of x,y coordinates in pixels.

Each ``polygon`` object is made up of a space-delimited list of x,y
coordinates. The origin for these coordinates is the location of the
parent ``object``. By default, the first point is created as 0,0
denoting that the point will originate exactly where the ``object`` is
placed.

.. _tmx-polyline:

<polyline>
~~~~~~~~~~

-  **points:** A list of x,y coordinates in pixels.

A ``polyline`` follows the same placement definition as a ``polygon``
object.

.. _tmx-text:

<text>
~~~~~~

-  **fontfamily:** The font family used (defaults to "sans-serif")
-  **pixelsize:** The size of the font in pixels (not using points,
   because other sizes in the TMX format are also using pixels)
   (defaults to 16)
-  **wrap:** Whether word wrapping is enabled (1) or disabled (0).
   (defaults to 0)
-  **color:** Color of the text in ``#AARRGGBB`` or ``#RRGGBB`` format
   (defaults to #000000)
-  **bold:** Whether the font is bold (1) or not (0). (defaults to 0)
-  **italic:** Whether the font is italic (1) or not (0). (defaults to 0)
-  **underline:** Whether a line should be drawn below the text (1) or
   not (0). (defaults to 0)
-  **strikeout:** Whether a line should be drawn through the text (1) or
   not (0). (defaults to 0)
-  **kerning:** Whether kerning should be used while rendering the text
   (1) or not (0). (defaults to 1)
-  **halign:** Horizontal alignment of the text within the object
   (``left``, ``center``, ``right`` or ``justify``, defaults to ``left``)
   (since Tiled 1.2.1)
-  **valign:** Vertical alignment of the text within the object (``top``
   , ``center`` or ``bottom``, defaults to ``top``)

Used to mark an object as a text object. Contains the actual text as
character data.

For alignment purposes, the bottom of the text is the descender height of
the font, and the top of the text is the ascender height of the font. For
example, ``bottom`` alignment of the word "cat" will leave some space below
the text, even though it is unused for this word with most fonts. Similarly,
``top`` alignment of the word "cat" will leave some space above the "t" with
most fonts, because this space is used for diacritics.

If the text is larger than the object's bounds, it is clipped to the bounds
of the object.

.. _tmx-imagelayer:

<imagelayer>
------------

-  **id:** Unique ID of the layer (defaults to 0, with valid IDs being at least
   1). Each layer that added to a map gets a unique id. Even if a layer is
   deleted, no layer ever gets the same ID. Can not be changed in Tiled.
   (since Tiled 1.2)
-  **name:** The name of the image layer. (defaults to "")
-  **class:** The class of the image layer (since 1.9, defaults to "").
-  **offsetx:** Horizontal offset of the image layer in pixels. (defaults to
   0) (since 0.15)
-  **offsety:** Vertical offset of the image layer in pixels. (defaults to
   0) (since 0.15)
-  **parallaxx:** Horizontal :ref:`parallax factor <parallax-factor>` for this layer. Defaults to 1. (since 1.5)
-  **parallaxy:** Vertical :ref:`parallax factor <parallax-factor>` for this layer. Defaults to 1. (since 1.5)
-  *x:* The x position of the image layer in pixels. (defaults to 0, deprecated
   since 0.15)
-  *y:* The y position of the image layer in pixels. (defaults to 0, deprecated
   since 0.15)
-  **opacity:** The opacity of the layer as a value from 0 to 1. (defaults to
   1)
-  **visible:** Whether the layer is shown (1) or hidden (0). (defaults to 1)
-  **tintcolor:** A color that is multiplied with the image drawn by this layer in ``#AARRGGBB`` or ``#RRGGBB`` format (optional).
-  **repeatx:** Whether the image drawn by this layer is repeated along the X axis. (since Tiled 1.8)
-  **repeaty:** Whether the image drawn by this layer is repeated along the Y axis. (since Tiled 1.8)

A layer consisting of a single image.

Can contain at most one: :ref:`tmx-properties`, :ref:`tmx-image`

.. _tmx-group:

<group>
-------

-  **id:** Unique ID of the layer (defaults to 0, with valid IDs being at least
   1). Each layer that added to a map gets a unique id. Even if a layer is
   deleted, no layer ever gets the same ID. Can not be changed in Tiled.
   (since Tiled 1.2)
-  **name:** The name of the group layer. (defaults to "")
-  **class:** The class of the group layer (since 1.9, defaults to "").
-  **offsetx:** Horizontal offset of the group layer in pixels. (defaults to 0)
-  **offsety:** Vertical offset of the group layer in pixels. (defaults to 0)
-  **parallaxx:** Horizontal :ref:`parallax factor <parallax-factor>` for this group. Defaults to 1. (since 1.5)
-  **parallaxy:** Vertical :ref:`parallax factor <parallax-factor>` for this group. Defaults to 1. (since 1.5)
-  **opacity:** The opacity of the layer as a value from 0 to 1. (defaults to 1)
-  **visible:** Whether the layer is shown (1) or hidden (0). (defaults to 1)
-  **tintcolor:** A color that is multiplied with any graphics drawn by any child layers, in ``#AARRGGBB`` or ``#RRGGBB`` format (optional).

A group layer, used to organize the layers of the map in a hierarchy.
Its attributes ``offsetx``, ``offsety``, ``opacity``, ``visible`` and
``tintcolor`` recursively affect child layers.

Can contain at most one: :ref:`tmx-properties`

Can contain any number: :ref:`tmx-layer`,
:ref:`tmx-objectgroup`, :ref:`tmx-imagelayer`, :ref:`tmx-group`

.. _tmx-properties:

<properties>
------------

Wraps any number of custom properties. Can be used as a child of the
``map``, ``tileset``, ``tile`` (when part of a ``tileset``),
``terrain``, ``wangset``, ``wangcolor``, ``layer``, ``objectgroup``,
``object``, ``imagelayer``, ``group`` and ``property`` elements.

Can contain any number: :ref:`tmx-property`

.. _tmx-property:

<property>
~~~~~~~~~~

-  **name:** The name of the property.
-  **type:** The type of the property. Can be ``string`` (default), ``int``,
   ``float``, ``bool``, ``color``, ``file``, ``object`` or ``class`` (since
   0.16, with ``color`` and ``file`` added in 0.17, ``object`` added in 1.4 and
   ``class`` added in 1.8).
-  **propertytype:** The name of the
   :ref:`custom property type <custom-property-types>`, when applicable
   (since 1.8).
-  **value:** The value of the property. (default string is "", default
   number is 0, default boolean is "false", default color is #00000000, default
   file is "." (the current file's parent directory))

Boolean properties have a value of either "true" or "false".

Color properties are stored in the format ``#AARRGGBB``.

File properties are stored as paths relative from the location of the
map file.

Object properties can reference any object on the same map and are stored as an
integer (the ID of the referenced object, or 0 when no object is referenced).
When used on objects in the Tile Collision Editor, they can only refer to
other objects on the same tile.

Class properties will have their member values stored in a nested
:ref:`tmx-properties` element. Only the actually set members are saved. When no
members have been set the ``properties`` element is left out entirely.

When a string property contains newlines, the current version of Tiled
will write out the value as characters contained inside the ``property``
element rather than as the ``value`` attribute. It is possible that a
future version of the TMX format will switch to always saving property
values inside the element rather than as an attribute.

Can contain at most one: :ref:`tmx-properties` (since 1.8)

.. _tmx-template-files:

Template Files
--------------

Templates are saved in their own file, and are referenced by
:ref:`objects <tmx-object>` that are template instances.

.. _tmx-template:

<template>
~~~~~~~~~~

The template root element contains the saved :ref:`map object <tmx-object>`
and a :ref:`tileset <tmx-tileset>` element that points to an external
tileset, if the object is a tile object.

Example of a template file:

   .. code:: xml

    <?xml version="1.0" encoding="UTF-8"?>
    <template>
     <tileset firstgid="1" source="desert.tsx"/>
     <object name="cactus" gid="31" width="81" height="101"/>
    </template>

Any tileset reference should always come before the object. Embedded tilesets
are not supported.

Can contain at most one: :ref:`tmx-tileset`

Should contain exactly one: :ref:`tmx-object`

--------------

.. figure:: CC-BY-SA.png
   :alt: Creative Commons License

   Creative Commons License

The **TMX Map Format** by https://www.mapeditor.org is licensed under a
`Creative Commons Attribution-ShareAlike 3.0 Unported
License <http://creativecommons.org/licenses/by-sa/3.0/>`__.
