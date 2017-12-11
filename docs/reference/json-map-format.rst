JSON Map Format
===============

Tiled can export maps as JSON files. To do so, simply select "File >
Export As" and select the JSON file type. You can export json from the
command line with the ``--export-map`` option.

The fields found in the JSON format differ slightly from those in the
:doc:`tmx-map-format`, but the meanings should remain the same.

The following fields can be found in a Tiled JSON file:

Map
---

+-------------------+----------+----------------------------------------------------------+
| Field             | Type     | Description                                              |
+===================+==========+==========================================================+
| backgroundcolor   | string   | Hex-formatted color (#RRGGBB or #AARRGGBB) (optional)    |
+-------------------+----------+----------------------------------------------------------+
| height            | int      | Number of tile rows                                      |
+-------------------+----------+----------------------------------------------------------+
| infinite          | bool     | Whether the map has infinite dimensions                  |
+-------------------+----------+----------------------------------------------------------+
| layers            | array    | Array of :ref:`layers <json-layer>`                      |
+-------------------+----------+----------------------------------------------------------+
| nextobjectid      | int      | Auto-increments for each placed object                   |
+-------------------+----------+----------------------------------------------------------+
| orientation       | string   | ``orthogonal``, ``isometric``, ``staggered`` or          |
|                   |          | ``hexagonal``                                            |
+-------------------+----------+----------------------------------------------------------+
| properties        | object   | String key-value pairs                                   |
+-------------------+----------+----------------------------------------------------------+
| renderorder       | string   | Rendering direction (orthogonal maps only)               |
+-------------------+----------+----------------------------------------------------------+
| tiledversion      | string   | The Tiled version used to save the file                  |
+-------------------+----------+----------------------------------------------------------+
| tileheight        | int      | Map grid height                                          |
+-------------------+----------+----------------------------------------------------------+
| tilesets          | array    | Array of :ref:`tilesets <json-tileset>`                  |
+-------------------+----------+----------------------------------------------------------+
| tilewidth         | int      | Map grid width                                           |
+-------------------+----------+----------------------------------------------------------+
| type              | string   | ``map`` (since 1.0)                                      |
+-------------------+----------+----------------------------------------------------------+
| version           | number   | The JSON format version                                  |
+-------------------+----------+----------------------------------------------------------+
| width             | int      | Number of tile columns                                   |
+-------------------+----------+----------------------------------------------------------+

Map Example
~~~~~~~~~~~

.. code:: json

    {
      "backgroundcolor":"#656667",
      "height":4,
      "layers":[ ],
      "nextobjectid":1,
      "orientation":"orthogonal",
      "properties":
      {
        "mapProperty1":"one",
        "mapProperty2":"two"
      },
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

+--------------+----------+---------------------------------------------------------------+
| Field        | Type     | Description                                                   |
+==============+==========+===============================================================+
| data         | int      | Array of GIDs. ``tilelayer`` only.                            |
+--------------+----------+---------------------------------------------------------------+
| draworder    | string   | ``topdown`` (default) or ``index``. ``objectgroup`` only.     |
+--------------+----------+---------------------------------------------------------------+
| height       | int      | Row count. Same as map height for fixed-size maps.            |
+--------------+----------+---------------------------------------------------------------+
| layers       | array    | Array of :ref:`layers <json-layer>`. ``group`` on             |
+--------------+----------+---------------------------------------------------------------+
| name         | string   | Name assigned to this layer                                   |
+--------------+----------+---------------------------------------------------------------+
| objects      | object   | Array of :ref:`objects <json-object>`. ``objectgroup`` only.  |
+--------------+----------+---------------------------------------------------------------+
| opacity      | float    | Value between 0 and 1                                         |
+--------------+----------+---------------------------------------------------------------+
| properties   | object   | string key-value pairs.                                       |
+--------------+----------+---------------------------------------------------------------+
| type         | string   | ``tilelayer``, ``objectgroup``, ``imagelayer`` or ``group``   |
+--------------+----------+---------------------------------------------------------------+
| visible      | bool     | Whether layer is shown or hidden in editor                    |
+--------------+----------+---------------------------------------------------------------+
| width        | int      | Column count. Same as map width for fixed-size maps.          |
+--------------+----------+---------------------------------------------------------------+
| x            | int      | Horizontal layer offset in tiles. Always 0.                   |
+--------------+----------+---------------------------------------------------------------+
| y            | int      | Vertical layer offset in tiles. Always 0.                     |
+--------------+----------+---------------------------------------------------------------+

Tile Layer Example
~~~~~~~~~~~~~~~~~~

.. code:: json

    {
      "data":[1, 2, 1, 2, 3, 1, 3, 1, 2, 2, 3, 3, 4, 4, 4, 1],
      "height":4,
      "name":"ground",
      "opacity":1,
      "properties":
         {
          "tileLayerProp":"1"
         },
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
      "properties":
      {
        "layerProp1": "someStringValue"
      },
      "type":"objectgroup",
      "visible":true,
      "width":0,
      "x":0,
      "y":0
    }

.. _json-object:

Object
------

+--------------+----------+----------------------------------------------+
| Field        | Type     | Description                                  |
+==============+==========+==============================================+
| ellipse      | bool     | Used to mark an object as an ellipse         |
+--------------+----------+----------------------------------------------+
| gid          | int      | GID, only if object comes from a Tilemap     |
+--------------+----------+----------------------------------------------+
| height       | int      | Height in pixels. Ignored if using a gid.    |
+--------------+----------+----------------------------------------------+
| id           | int      | Incremental id - unique across all objects   |
+--------------+----------+----------------------------------------------+
| name         | string   | String assigned to name field in editor      |
+--------------+----------+----------------------------------------------+
| point        | bool     | Used to mark an object as a point            |
+--------------+----------+----------------------------------------------+
| polygon      | array    | A list of x,y coordinates in pixels          |
+--------------+----------+----------------------------------------------+
| polyline     | array    | A list of x,y coordinates in pixels          |
+--------------+----------+----------------------------------------------+
| properties   | object   | String key-value pairs                       |
+--------------+----------+----------------------------------------------+
| rotation     | float    | Angle in degrees clockwise                   |
+--------------+----------+----------------------------------------------+
| text         | object   | String key-value pairs                       |
+--------------+----------+----------------------------------------------+
| type         | string   | String assigned to type field in editor      |
+--------------+----------+----------------------------------------------+
| visible      | bool     | Whether object is shown in editor.           |
+--------------+----------+----------------------------------------------+
| width        | int      | Width in pixels. Ignored if using a gid.     |
+--------------+----------+----------------------------------------------+
| x            | int      | x coordinate in pixels                       |
+--------------+----------+----------------------------------------------+
| y            | int      | y coordinate in pixels                       |
+--------------+----------+----------------------------------------------+

Object Example
~~~~~~~~~~~~~~

.. code:: json

    {
      "gid":5,
      "height":0,
      "id":1,
      "name":"villager",
      "properties":
      {
        "hp":"12"
      },
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
~~~~~~~~~~~~~~~~~

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

.. _json-tileset:

Tileset
-------

+------------------+----------+-----------------------------------------------------+
| Field            | Type     | Description                                         |
+==================+==========+=====================================================+
| columns          | int      | The number of tile columns in the tileset           |
+------------------+----------+-----------------------------------------------------+
| firstgid         | int      | GID corresponding to the first tile in the set      |
+------------------+----------+-----------------------------------------------------+
| grid             | object   | See :ref:`tmx-grid` (optional)                      |
+------------------+----------+-----------------------------------------------------+
| image            | string   | Image used for tiles in this set                    |
+------------------+----------+-----------------------------------------------------+
| imagewidth       | int      | Width of source image in pixels                     |
+------------------+----------+-----------------------------------------------------+
| imageheight      | int      | Height of source image in pixels                    |
+------------------+----------+-----------------------------------------------------+
| margin           | int      | Buffer between image edge and first tile (pixels)   |
+------------------+----------+-----------------------------------------------------+
| name             | string   | Name given to this tileset                          |
+------------------+----------+-----------------------------------------------------+
| properties       | object   | String key-value pairs                              |
+------------------+----------+-----------------------------------------------------+
| propertytypes    | object   | String key-value pairs                              |
+------------------+----------+-----------------------------------------------------+
| spacing          | int      | Spacing between adjacent tiles in image (pixels)    |
+------------------+----------+-----------------------------------------------------+
| terrains         | array    | Array of :ref:`terrains <json-terrain>` (optional)  |
+------------------+----------+-----------------------------------------------------+
| tilecount        | int      | The number of tiles in this tileset                 |
+------------------+----------+-----------------------------------------------------+
| tileheight       | int      | Maximum height of tiles in this set                 |
+------------------+----------+-----------------------------------------------------+
| tileoffset       | object   | See :ref:`tmx-tileoffset` (optional)                |
+------------------+----------+-----------------------------------------------------+
| tileproperties   | object   | Per-tile properties, indexed by gid as string       |
+------------------+----------+-----------------------------------------------------+
| tiles            | object   | Mapping from tile ID to :ref:`tile <json-tile>`     |
|                  |          | (optional)                                          |
+------------------+----------+-----------------------------------------------------+
| tilewidth        | int      | Maximum width of tiles in this set                  |
+------------------+----------+-----------------------------------------------------+
| type             | string   | ``tileset`` (for tileset files, since 1.0)          |
+------------------+----------+-----------------------------------------------------+

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
             "properties":
               {
                 "myProperty1":"myProperty1_value"
               },
             "propertytypes":
               {
                 "myProperty1":"string"
               },
             "spacing":1,
             "tilecount":266,
             "tileheight":32,
             "tilewidth":32
            }

.. _json-tile:

Tile (Definition)
~~~~~~~~~~~~~~~~~

+-----------+---------+--------------------------------------------+
| Field     | Type    | Description                                |
+===========+=========+============================================+
| terrain   | array   | index of terrain for each corner of tile   |
+-----------+---------+--------------------------------------------+

A tileset that associates information with each tile, like its image
path or terrain type, may include a "tiles" JSON object. Each key
is a local ID of a tile within the tileset.

For the terrain information, each value is a length-4 array where each
element is the index of a :ref:`terrain <json-terrain>` on one corner
of the tile. The order of indices is: top-left, top-right, bottom-left,
bottom-right.

Example:

.. code:: json

    "tiles":
    {
      "0":
      {
        "terrain":[0, 0, 0, 0]
      },
      "11":
      {
        "terrain":[0, 1, 0, 1]
      },
      "12":
      {
        "terrain":[1, 1, 1, 1]
      }
    }

.. _json-terrain:

Terrain
~~~~~~~

+---------+----------+-----------------------------------------+
| Field   | Type     | Description                             |
+=========+==========+=========================================+
| name    | string   | Name of terrain                         |
+---------+----------+-----------------------------------------+
| tile    | int      | Local ID of tile representing terrain   |
+---------+----------+-----------------------------------------+

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
