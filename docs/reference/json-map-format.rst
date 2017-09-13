JSON Map Format
===============

Tiled can also export maps as JSON files. To do so, simply select "File
> Export As" and select the JSON file type. You can export json from the
command line with the ``--export-map`` option.

The fields found in the JSON format differ slightly from those in the
:doc:`tmx-map-format`, but the meanings should remain the same.

The following fields can be found in a Tiled JSON file:

Map
---

+-------------------+----------+----------------------------------------------------------+
| Field             | Type     | Description                                              |
+===================+==========+==========================================================+
| width             | int      | Number of tile columns                                   |
+-------------------+----------+----------------------------------------------------------+
| height            | int      | Number of tile rows                                      |
+-------------------+----------+----------------------------------------------------------+
| tilewidth         | int      | Map grid width.                                          |
+-------------------+----------+----------------------------------------------------------+
| tileheight        | int      | Map grid height.                                         |
+-------------------+----------+----------------------------------------------------------+
| orientation       | string   | Orthogonal, isometric, or staggered                      |
+-------------------+----------+----------------------------------------------------------+
| layers            | array    | Array of `Layers <#layer>`__                             |
+-------------------+----------+----------------------------------------------------------+
| tilesets          | array    | Array of `Tilesets <#tileset>`__                         |
+-------------------+----------+----------------------------------------------------------+
| backgroundcolor   | string   | Hex-formatted color (#RRGGBB or #AARRGGBB) (Optional)    |
+-------------------+----------+----------------------------------------------------------+
| renderorder       | string   | Rendering direction (orthogonal maps only)               |
+-------------------+----------+----------------------------------------------------------+
| properties        | object   | String key-value pairs                                   |
+-------------------+----------+----------------------------------------------------------+
| nextobjectid      | int      | Auto-increments for each placed object                   |
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
      "width":4
    }

Layer
-----

+--------------+----------+---------------------------------------------------------+
| Field        | Type     | Description                                             |
+==============+==========+=========================================================+
| width        | int      | Column count. Same as map width in Tiled Qt.            |
+--------------+----------+---------------------------------------------------------+
| height       | int      | Row count. Same as map height in Tiled Qt.              |
+--------------+----------+---------------------------------------------------------+
| name         | string   | Name assigned to this layer                             |
+--------------+----------+---------------------------------------------------------+
| type         | string   | "tilelayer", "objectgroup", or "imagelayer"             |
+--------------+----------+---------------------------------------------------------+
| visible      | bool     | Whether layer is shown or hidden in editor              |
+--------------+----------+---------------------------------------------------------+
| x            | int      | Horizontal layer offset. Always 0 in Tiled Qt.          |
+--------------+----------+---------------------------------------------------------+
| y            | int      | Vertical layer offset. Always 0 in Tiled Qt.            |
+--------------+----------+---------------------------------------------------------+
| data         | int      | Array of GIDs. ``tilelayer`` only.                      |
+--------------+----------+---------------------------------------------------------+
| objects      | object   | Array of `Objects <#object>`__. ``objectgroup`` only.   |
+--------------+----------+---------------------------------------------------------+
| properties   | object   | string key-value pairs.                                 |
+--------------+----------+---------------------------------------------------------+
| opacity      | float    | Value between 0 and 1                                   |
+--------------+----------+---------------------------------------------------------+
| draworder    | string   | "topdown" (default) or "index". ``objectgroup`` only.   |
+--------------+----------+---------------------------------------------------------+

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

Object
------

+--------------+----------+----------------------------------------------+
| Field        | Type     | Description                                  |
+==============+==========+==============================================+
| id           | int      | Incremental id - unique across all objects   |
+--------------+----------+----------------------------------------------+
| width        | int      | Width in pixels. Ignored if using a gid.     |
+--------------+----------+----------------------------------------------+
| height       | int      | Height in pixels. Ignored if using a gid.    |
+--------------+----------+----------------------------------------------+
| name         | string   | String assigned to name field in editor      |
+--------------+----------+----------------------------------------------+
| type         | string   | String assigned to type field in editor      |
+--------------+----------+----------------------------------------------+
| properties   | object   | String key-value pairs                       |
+--------------+----------+----------------------------------------------+
| visible      | bool     | Whether object is shown in editor.           |
+--------------+----------+----------------------------------------------+
| x            | int      | x coordinate in pixels                       |
+--------------+----------+----------------------------------------------+
| y            | int      | y coordinate in pixels                       |
+--------------+----------+----------------------------------------------+
| rotation     | float    | Angle in degrees clockwise                   |
+--------------+----------+----------------------------------------------+
| gid          | int      | GID, only if object comes from a Tilemap     |
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

Tileset
-------

+------------------+----------+-----------------------------------------------------+
| Field            | Type     | Description                                         |
+==================+==========+=====================================================+
| firstgid         | int      | GID corresponding to the first tile in the set      |
+------------------+----------+-----------------------------------------------------+
| image            | string   | Image used for tiles in this set                    |
+------------------+----------+-----------------------------------------------------+
| name             | string   | Name given to this tileset                          |
+------------------+----------+-----------------------------------------------------+
| tilewidth        | int      | Maximum width of tiles in this set                  |
+------------------+----------+-----------------------------------------------------+
| tileheight       | int      | Maximum height of tiles in this set                 |
+------------------+----------+-----------------------------------------------------+
| imagewidth       | int      | Width of source image in pixels                     |
+------------------+----------+-----------------------------------------------------+
| imageheight      | int      | Height of source image in pixels                    |
+------------------+----------+-----------------------------------------------------+
| properties       | object   | String key-value pairs                              |
+------------------+----------+-----------------------------------------------------+
| margin           | int      | Buffer between image edge and first tile (pixels)   |
+------------------+----------+-----------------------------------------------------+
| spacing          | int      | Spacing between adjacent tiles in image (pixels)    |
+------------------+----------+-----------------------------------------------------+
| tileproperties   | object   | Per-tile properties, indexed by gid as string       |
+------------------+----------+-----------------------------------------------------+
| terrains         | array    | Array of `Terrains <#terrain>`__ (optional)         |
+------------------+----------+-----------------------------------------------------+
| tiles            | object   | Gid-indexed `Tiles <#tiles>`__ (optional)           |
+------------------+----------+-----------------------------------------------------+

Tileset Example
~~~~~~~~~~~~~~~

.. code:: json

            {
             "firstgid":1,
             "image":"..\/image\/terrain.png",
             "imageheight":192,
             "imagewidth":256,
             "margin":0,
             "name":"terrain",
             "properties":
                {

                },
             "spacing":0,
             "tilewidth":32
            }

Tiles
~~~~~

+-----------+---------+--------------------------------------------+
| Field     | Type    | Description                                |
+===========+=========+============================================+
| terrain   | array   | index of terrain for each corner of tile   |
+-----------+---------+--------------------------------------------+

A tilemap with terrain definitions may include a "tiles" JSON object.
Each key is a local ID of a tile within the tileset. Each value is an
length-4 array where each element is the index of a
`terrain <#terrain>`__ on one corner of the tile. The order of indices
is: top-left, top-right, bottom-left, bottom-right.

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
