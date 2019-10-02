JSON Map Format
===============

Tiled can export maps as JSON files. To do so, simply select "File >
Export As" and select the JSON file type. You can export json from the
command line with the ``--export-map`` option.

The fields found in the JSON format differ slightly from those in the
:doc:`tmx-map-format`, but the meanings should remain the same.

The following fields can be found in a Tiled JSON file:

.. _json-map:

Map
---

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    backgroundcolor,  string,           "Hex-formatted color (#RRGGBB or #AARRGGBB) (optional)"
    height,           int,              "Number of tile rows"
    hexsidelength,    int,              "Length of the side of a hex tile in pixels (hexagonal maps only)"
    infinite,         bool,             "Whether the map has infinite dimensions"
    layers,           array,            "Array of :ref:`Layers <json-layer>`"
    nextlayerid,      int,              "Auto-increments for each layer"
    nextobjectid,     int,              "Auto-increments for each placed object"
    orientation,      string,           "``orthogonal``, ``isometric``, ``staggered`` or ``hexagonal``"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    renderorder,      string,           "``right-down`` (the default), ``right-up``, ``left-down`` or ``left-up`` (orthogonal maps only)"
    staggeraxis,      string,           "``x`` or ``y`` (staggered / hexagonal maps only)"
    staggerindex,     string,           "``odd`` or ``even`` (staggered / hexagonal maps only)"
    tiledversion,     string,           "The Tiled version used to save the file"
    tileheight,       int,              "Map grid height"
    tilesets,         array,            "Array of :ref:`Tilesets <json-tileset>`"
    tilewidth,        int,              "Map grid width"
    type,             string,           "``map`` (since 1.0)"
    version,          number,           "The JSON format version"
    width,            int,              "Number of tile columns"

Map Example
~~~~~~~~~~~

.. code:: json

    {
      "backgroundcolor":"#656667",
      "height":4,
      "layers":[ ],
      "nextobjectid":1,
      "orientation":"orthogonal",
      "properties":[
        {
          "name":"mapProperty1",
          "type":"one",
          "value":"string"
        },
        {
          "name":"mapProperty2",
          "type":"two",
          "value":"string"
        }],
      "renderorder":"right-down",
      "tileheight":32,
      "tilesets":[ ],
      "tilewidth":32,
      "version":1,
      "tiledversion":"1.0.3",
      "width":4
    }

.. _json-layer:

Layer
-----

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    chunks,           array,            "Array of :ref:`chunks <json-chunk>` (optional). ``tilelayer`` only."
    compression,      string,           "``zlib``, ``gzip`` or empty (default). ``tilelayer`` only."
    data,             array or string,  "Array of ``unsigned int`` (GIDs) or base64-encoded data. ``tilelayer`` only."
    draworder,        string,           "``topdown`` (default) or ``index``. ``objectgroup`` only."
    encoding,         string,           "``csv`` (default) or ``base64``. ``tilelayer`` only."
    height,           int,              "Row count. Same as map height for fixed-size maps."
    id,               int,              "Incremental id - unique across all layers"
    image,            string,           "Image used by this layer. ``imagelayer`` only."
    layers,           array,            "Array of :ref:`layers <json-layer>`. ``group`` only."
    name,             string,           "Name assigned to this layer"
    objects,          array,            "Array of :ref:`objects <json-object>`. ``objectgroup`` only."
    offsetx,          double,           "Horizontal layer offset in pixels (default: 0)"
    offsety,          double,           "Vertical layer offset in pixels (default: 0)"
    opacity,          double,           "Value between 0 and 1"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    startx,           int,              "X coordinate where layer content starts (for infinite maps)"
    starty,           int,              "Y coordinate where layer content starts (for infinite maps)"
    transparentcolor, string,           "Hex-formatted color (#RRGGBB) (optional). ``imagelayer`` only."
    type,             string,           "``tilelayer``, ``objectgroup``, ``imagelayer`` or ``group``"
    visible,          bool,             "Whether layer is shown or hidden in editor"
    width,            int,              "Column count. Same as map width for fixed-size maps."
    x,                int,              "Horizontal layer offset in tiles. Always 0."
    y,                int,              "Vertical layer offset in tiles. Always 0."

Tile Layer Example
~~~~~~~~~~~~~~~~~~

.. code:: json

    {
      "data":[1, 2, 1, 2, 3, 1, 3, 1, 2, 2, 3, 3, 4, 4, 4, 1],
      "height":4,
      "name":"ground",
      "opacity":1,
      "properties":[
        {
          "name":"tileLayerProp",
          "type":"int",
          "value":1
        }],
      "type":"tilelayer",
      "visible":true,
      "width":4,
      "x":0,
      "y":0
    }

Object Layer Example
~~~~~~~~~~~~~~~~~~~~

.. code:: json

    {
      "draworder":"topdown",
      "height":0,
      "name":"people",
      "objects":[ ],
      "opacity":1,
      "properties":[
        {
          "name":"layerProp1",
          "type":"string",
          "value":"someStringValue"
        }],
      "type":"objectgroup",
      "visible":true,
      "width":0,
      "x":0,
      "y":0
    }

.. _json-chunk:

Chunk
-----

Chunks are used to store the tile layer data for
:doc:`infinite maps </manual/using-infinite-maps>`.

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    data,             array or string,  "Array of ``unsigned int`` (GIDs) or base64-encoded data"
    height,           int,              "Height in tiles"
    width,            int,              "Width in tiles"
    x,                int,              "X coordinate in tiles"
    y,                int,              "Y coordinate in tiles"

Chunk Example
~~~~~~~~~~~~~

.. code:: json

    {
      "data":[1, 2, 1, 2, 3, 1, 3, 1, 2, 2, 3, 3, 4, 4, 4, 1, ...],
      "height":16,
      "width":16,
      "x":0,
      "y":-16,
    }

.. _json-object:

Object
------

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    ellipse,          bool,             "Used to mark an object as an ellipse"
    gid,              int,              "Global tile ID, only if object represents a tile"
    height,           double,           "Height in pixels."
    id,               int,              "Incremental id, unique across all objects"
    name,             string,           "String assigned to name field in editor"
    point,            bool,             "Used to mark an object as a point"
    polygon,          array,            "Array of :ref:`Points <json-point>`, in case the object is a polygon"
    polyline,         array,            "Array of :ref:`Points <json-point>`, in case the object is a polyline"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    rotation,         double,           "Angle in degrees clockwise"
    template,         string,           "Reference to a template file, in case object is a :doc:`template instance </manual/using-templates>`"
    text,             :ref:`json-object-text`, "Only used for text objects"
    type,             string,           "String assigned to type field in editor"
    visible,          bool,             "Whether object is shown in editor."
    width,            double,           "Width in pixels."
    x,                double,           "X coordinate in pixels"
    y,                double,           "Y coordinate in pixels"

Object Example
~~~~~~~~~~~~~~

.. code:: json

    {
      "gid":5,
      "height":0,
      "id":1,
      "name":"villager",
      "properties":[
        {
          "name":"hp",
          "type":"int",
          "value":12
        }],
      "rotation":0,
      "type":"npc",
      "visible":true,
      "width":0,
      "x":32,
      "y":32
    }

Ellipse Example
~~~~~~~~~~~~~~~

.. code:: json

    {
      "ellipse":true,
      "height":152,
      "id":13,
      "name":"",
      "rotation":0,
      "type":"",
      "visible":true,
      "width":248,
      "x":560,
      "y":808
    }

Rectangle Example
~~~~~~~~~~~~~~~~~

.. code:: json

    {
      "height":184,
      "id":14,
      "name":"",
      "rotation":0,
      "type":"",
      "visible":true,
      "width":368,
      "x":576,
      "y":584
    }

Point Example
~~~~~~~~~~~~~

.. code:: json

    {
      "point":true,
      "height":0,
      "id":20,
      "name":"",
      "rotation":0,
      "type":"",
      "visible":true,
      "width":0,
      "x":220,
      "y":350
    }

Polygon Example
~~~~~~~~~~~~~~~

.. code:: json

    {
      "height":0,
      "id":15,
      "name":"",
      "polygon":[
      {
        "x":0,
        "y":0
      },
      {
        "x":152,
        "y":88
      },
      {
        "x":136,
        "y":-128
      },
      {
        "x":80,
        "y":-280
      },
      {
        "x":16,
        "y":-288
      }],
      "rotation":0,
      "type":"",
      "visible":true,
      "width":0,
      "x":-176,
      "y":432
    }

Polyline Example
~~~~~~~~~~~~~~~~

.. code:: json

    {
      "height":0,
      "id":16,
      "name":"",
      "polyline":[
      {
        "x":0,
        "y":0
      },
      {
        "x":248,
        "y":-32
      },
      {
        "x":376,
        "y":72
      },
      {
        "x":544,
        "y":288
      },
      {
        "x":656,
        "y":120
      },
      {
        "x":512,
        "y":0
      }],
      "rotation":0,
      "type":"",
      "visible":true,
      "width":0,
      "x":240,
      "y":88
    }

Text Example
~~~~~~~~~~~~

.. code:: json

    {
      "height":19,
      "id":15,
      "name":"",
      "text":
      {
        "text":"Hello World",
        "wrap":true
      },
      "rotation":0,
      "type":"",
      "visible":true,
      "width":248,
      "x":48,
      "y":136
    }

.. _json-object-text:

Text
----

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    bold,             bool,             "Whether to use a bold font (default: ``false``)"
    color,            string,           "Hex-formatted color (#RRGGBB or #AARRGGBB) (default: ``#000000``)"
    fontfamily,       string,           "Font family (default: ``sans-serif``)"
    halign,           string,           "Horizontal alignment (``center``, ``right``, ``justify`` or ``left`` (default))"
    italic,           bool,             "Whether to use an italic font (default: ``false``)"
    kerning,          bool,             "Whether to use kerning when placing characters (default: ``true``)"
    pixelsize,        int,              "Pixel size of font (default: 16)"
    strikeout,        bool,             "Whether to strike out the text (default: ``false``)"
    text,             string,           "Text"
    underline,        bool,             "Whether to underline the text (default: ``false``)"
    valign,           string,           "Vertical alignment (``center``, ``bottom`` or ``top`` (default))"
    wrap,             bool,             "Whether the text is wrapped within the object bounds (default: ``false``)"


.. _json-tileset:

Tileset
-------

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    backgroundcolor,  string,           "Hex-formatted color (#RRGGBB or #AARRGGBB) (optional)"
    columns,          int,              "The number of tile columns in the tileset"
    firstgid,         int,              "GID corresponding to the first tile in the set"
    grid,             :ref:`json-tileset-grid`, "(optional)"
    image,            string,           "Image used for tiles in this set"
    imageheight,      int,              "Height of source image in pixels"
    imagewidth,       int,              "Width of source image in pixels"
    margin,           int,              "Buffer between image edge and first tile (pixels)"
    name,             string,           "Name given to this tileset"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    source,           string,           "The external file that contains this tilesets data"
    spacing,          int,              "Spacing between adjacent tiles in image (pixels)"
    terrains,         array,            "Array of :ref:`Terrains <json-terrain>` (optional)"
    tilecount,        int,              "The number of tiles in this tileset"
    tiledversion,     string,           "The Tiled version used to save the file"
    tileheight,       int,              "Maximum height of tiles in this set"
    tileoffset,       :ref:`json-tileset-tileoffset`, "(optional)"
    tiles,            array,            "Array of :ref:`Tiles <json-tile>` (optional)"
    tilewidth,        int,              "Maximum width of tiles in this set"
    transparentcolor, string,           "Hex-formatted color (#RRGGBB) (optional)"
    type,             string,           "``tileset`` (for tileset files, since 1.0)"
    version,          number,           "The JSON format version"
    wangsets,         array,            "Array of :ref:`Wang sets <json-wangset>` (since 1.1.5)"

Each tileset has a ``firstgid`` (first global ID) property which
tells you the global ID of its first tile (the one with local 
tile ID 0). This allows you to map the global IDs back to the 
right tileset, and then calculate the local tile ID by 
subtracting the ``firstgid`` from the global tile ID. The first 
tileset always has a ``firstgid`` value of 1.

.. _json-tileset-grid:

Grid
~~~~

Specifies common grid settings used for tiles in a tileset. See
:ref:`tmx-grid` in the TMX Map Format.

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    height,           int,              "Cell height of tile grid"
    orientation,      string,           "``orthogonal`` (default) or ``isometric``"
    width,            int,              "Cell width of tile grid"

.. _json-tileset-tileoffset:

Tile Offset
~~~~~~~~~~~

See :ref:`tmx-tileoffset` in the TMX Map Format.

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    x,                int,              "Horizontal offset in pixels"
    y,                int,              "Vertical offset in pixels (positive is down)"

Tileset Example
~~~~~~~~~~~~~~~

.. code:: json

            {
             "columns":19,
             "firstgid":1,
             "image":"..\/image\/fishbaddie_parts.png",
             "imageheight":480,
             "imagewidth":640,
             "margin":3,
             "name":"",
             "properties":[
               {
                 "name":"myProperty1",
                 "type":"string",
                 "value":"myProperty1_value"
               }],
             "spacing":1,
             "tilecount":266,
             "tileheight":32,
             "tilewidth":32
            }

.. _json-tile:

Tile (Definition)
~~~~~~~~~~~~~~~~~

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    animation,        array,              "Array of :ref:`Frames <json-frame>`"
    id,               int,                "Local ID of the tile"
    image,            string,             "Image representing this tile (optional)"
    imageheight,      int,                "Height of the tile image in pixels"
    imagewidth,       int,                "Width of the tile image in pixels"
    objectgroup,      :ref:`json-layer`,  "Layer with type ``objectgroup``, when collision shapes are specified (optional)"
    probability,      double,             "Percentage chance this tile is chosen when competing with others in the editor (optional)"
    properties,       array,              "Array of :ref:`Properties <json-property>`"
    terrain,          array,              "Index of terrain for each corner of tile (optional)"
    type,             string,             "The type of the tile (optional)"

A tileset that associates information with each tile, like its image
path or terrain type, may include a ``tiles`` array property. Each tile
has an ``id`` property, which specifies the local ID within the tileset.

For the terrain information, each value is a length-4 array where each
element is the index of a :ref:`terrain <json-terrain>` on one corner
of the tile. The order of indices is: top-left, top-right, bottom-left,
bottom-right.

Example:

.. code:: json

    "tiles":[
      {
        "id":0,
        "properties":[
          {
            "name":"myProperty1",
            "type":"string",
            "value":"myProperty1_value"
          }],
        "terrain":[0, 0, 0, 0]
      },
      {
        "id":11,
        "properties":[
          {
            "name":"myProperty2",
            "type":"string",
            "value":"myProperty2_value"
          }],
        "terrain":[0, 1, 0, 1]
      },
      {
        "id":12,
        "properties":[
          {
            "name":"myProperty3",
            "type":"string",
            "value":"myProperty3_value"
          }],
        "terrain":[1, 1, 1, 1]
      }
    ]

.. _json-frame:

Frame
~~~~~

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    duration,         int,              "Frame duration in milliseconds"
    tileid,           int,              "Local tile ID representing this frame"

.. _json-terrain:

Terrain
~~~~~~~

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    name,             string,           "Name of terrain"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    tile,             int,              "Local ID of tile representing terrain"

Example:

.. code:: json

    "terrains":[
    {
      "name":"ground",
      "tile":0
    },
    {
      "name":"chasm",
      "tile":12
    },
    {
      "name":"cliff",
      "tile":36
    }],

.. _json-wangset:

Wang Set
~~~~~~~~

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    cornercolors,     array,            "Array of :ref:`Wang colors <json-wangcolor>`"
    edgecolors,       array,            "Array of :ref:`Wang colors <json-wangcolor>`"
    name,             string,           "Name of the Wang set"
    properties,       array,            "Array of :ref:`Properties <json-property>`"
    tile,             int,              "Local ID of tile representing the Wang set"
    wangtiles,        array,            "Array of :ref:`Wang tiles <json-wangtile>`"

.. _json-wangcolor:

Wang Color
^^^^^^^^^^

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    color,            string,           "Hex-formatted color (#RRGGBB or #AARRGGBB)"
    name,             string,           "Name of the Wang color"
    probability,      double,           "Probability used when randomizing"
    tile,             int,              "Local ID of tile representing the Wang color"

Example:

.. code:: json

    {
      "color": "#d31313",
      "name": "Rails",
      "probability": 1,
      "tile": 18
    }

.. _json-wangtile:

Wang Tile
^^^^^^^^^

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    dflip,            bool,             "Tile is flipped diagonally (default: ``false``)"
    hflip,            bool,             "Tile is flipped horizontally (default: ``false``)"
    tileid,           int,              "Local ID of tile"
    vflip,            bool,             "Tile is flipped vertically (default: ``false``)"
    wangid,           array,            "Array of Wang color indexes (``uchar[8]``)"

Example:

.. code:: json

    {
      "dflip": false,
      "hflip": false,
      "tileid": 0,
      "vflip": false,
      "wangid": [2, 0, 1, 0, 1, 0, 2, 0]
    }

.. _json-objecttemplate:

Object Template
---------------

An object template is written to its own file and referenced by any
instances of that template.

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    type,             string,              "``template``"
    tileset,          :ref:`json-tileset`, "External tileset used by the template (optional)"
    object,           :ref:`json-object`,  "The object instantiated by this template"

.. _json-property:

Property
--------

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    name,             string,           "Name of the property"
    type,             string,           "Type of the property (``string`` (default), ``int``, ``float``, ``bool``, ``color`` or ``file`` (since 0.16, with ``color`` and ``file`` added in 0.17))"
    value,            value,            "Value of the property"

.. _json-point:

Point
-----

A point on a polygon or a polyline, relative to the position of the object.

.. csv-table::
    :header: Field, Type, Description
    :widths: 1, 1, 4

    x,                double,           "X coordinate in pixels"
    y,                double,           "Y coordinate in pixels"

Changelog
---------

Tiled 1.2
~~~~~~~~~

* Added ``nextlayerid`` to the :ref:`json-map` object.

* Added ``id`` to the :ref:`json-layer` object.

* The tiles in a :ref:`json-tileset` are now stored as an array instead
  of an object. Previously the tile IDs were stored as string keys of
  the "tiles" object, now they are stored as ``id`` property of each
  :ref:`Tile <json-tile>` object.

* Custom tile properties are now stored within each
  :ref:`Tile <json-tile>` instead of being included as
  ``tileproperties`` in the :ref:`json-tileset` object.

* Custom properties are now stored in an array instead of an object
  where the property names were the keys. Each property is now an object
  that stores the name, type and value of the property. The separate
  ``propertytypes`` and ``tilepropertytypes`` attributes have been
  removed.

Tiled 1.1
~~~~~~~~~

* Added a :ref:`chunked data format <json-chunk>`, currently used for
  :doc:`infinite maps </manual/using-infinite-maps>`.

* :doc:`Templates </manual/using-templates>` were added. Templates can
  be stored as JSON files with an :ref:`json-objecttemplate` object.

* :ref:`Tilesets <json-tileset>` can now contain
  :doc:`Wang tiles </manual/using-wang-tiles>`. They are saved in the
  new :ref:`json-wangset` object (since Tiled 1.1.5).
