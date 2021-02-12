.. _gamemaker2-export:

GameMaker: Studio 2.3
---------------------

GameMaker: Studio 2.3 uses a custom json-based format to store its rooms,
and Tiled ships with a plugin to export maps in this format. [[Currently
only orthogonal maps will export correctly.]]

Tiled itself works with tile layers, object layers, image layers and group layers. All layer types are supported and if needed Tiled will automatically divide a layer into several sublayers on export as GameMaker only supports one tileset per tile layer and differentiates between instance and asset layers.

In general:

* Tiles from tilesets on tile layers will get exported as tiles on tile layers. 
* Tiles from image collection on tile layers will get exported as assets on an asset layers.
* Tile objects with no type will get exported as assets on asset layers.
* Tile-, point- and polygon-objects with a set type will get exported as instances on instance layers.
* Image layers will get exported as background layers.


.. hint::

   The tilesets have to be named the same as the corresponding tileset assets in the GameMaker project. The file names of the source graphics of tilesets and assets should be the same as the corresponding sprites asset in the GameMaker project. Alternatively the custom string property ``sprite`` can be used to overwrite a file name. 

Tilesets
~~~~~~~~~~~

A tileset in Tiled has to be named the same as the corresponding tileset asset in the GameMaker project.
The file name of the source image has to be the same as the corresponding sprite asset. Alternatively the custom property ``sprite`` can be used to overwrite the file name.

* string ``sprite`` (default: filename)

Based on Tileset Image
^^^^^^^^^^^^^^^^^^^^^^
This type of tilesets equals the tilesets in GameMaker. If the map orientation is orthogonal tile layers with this type of tiles will get exported as GameMaker tile layers. Tiles that have a different size then the grid will get exported as assets on a separate asset layer instead.

Collection of Images
^^^^^^^^^^^^^^^^^^^^
This type of tilesets can be used to place sprites on asset layers or instances on instance layers. This can be done with the "Insert Tile"-Tool on object layers.
Objects with no set Type will get exported as assets on asset layers while objects with a set type will get exported as instances on instance layers.
You can place the tiles from image collections on the tile layers in Tiled but they will get exported as assets on a separate asset layer instead.

Isometric and Hexagonal Grids
^^^^^^^^^^^^^^^^^^^^

Isometric and hexagonal map orientations are also supported but the tile layers will get exported as asset layers in this case since GameMaker Studio 2 supports only tile layers with an orthogonal grid.



Object Instances
~~~~~~~~~~~~~~~~

GameMaker object instances are created by putting the object name in the ``Type`` field of the object in Tiled. Rotation, flipping and scaling is supported.

The following custom properties can be set on objects to affect the exported instance:

* bool ``hasCreationCode`` (default: false)
* color ``colour`` (default: white)
* float ``scaleX`` (default: derived from tile or 1.0)
* float ``scaleY`` (default: derived from tile or 1.0)
* int ``imageIndex`` (default: 0)
* float ``imageSpeed` (default: 1.0)
* bool ``ignore`` (default: derived from tile (!Visible) or false)
* bool ``inheritItemSettings`` (default: false)
* int ``creationOrder`` (default: 0)
* int ``originX`` (default: 0)
* int ``originY`` (default: 0)

The ``hasCreationCode`` property can be set to true. Refers to "InstanceCreationCode_[inst_name].gml" in the room folder which you can create inside GameMaker itself or with an external text editor.

The ``scaleX`` and ``scaleY`` properties can be used to override the
scale of the instance. However, if the scale is relevant then it will
generally be easier to use a tile object, in which case it is
automatically derived from the tile size and the object size.

The ``originX`` and ``originY`` properties can be used to tell Tiled
about the origin of the sprite defined in GameMaker, as an offset from
the top-left. This origin is taken into account when determining the
position of the exported instance.

.. hint::

   Of course setting the type and/or the above properties manually for
   each instance will get old fast. Instead you can use tile objects with the type set on the tile or use :doc:`object templates <using-templates>`.


Assets
~~~~~~

GameMaker assets are created by leaving the ``Type`` field of a tile object in Tiled empty. They can be placed on both the object and tile layers in Tiled but will get exported always to GameMaker asset layers. Rotation, flipping and scaling is supported.

Tiled also supports placing single tiles from a tileset image outside the grid by playing them on an object layer. In this case the single tile assets will get exported as "GMRGraphics" (aka GMS1.4 Tiles) to an asset layer. These "GMRGraphic"-tiles support horizontal and vertical flipping as well as tinting but no rotation.

The following custom properties can be set on assets to affect the
exported assets:

* float ``headPosition`` (default: 0.0)
* float ``rotation`` (default:0.0)
* float ``scaleX`` (default: derived from tile or 1.0)
* float ``scaleY`` (default: derived from tile or 1.0)
* float ``animationSpeed` (default: 1.0)
* color ``colour`` (default: white)
* bool ``ignore`` (default: derived from tile (!Visible) or false)
* bool ``inheritItemSettings`` (default: false)
* int ``creationOrder`` (default: 0)
* int ``originX`` (default: 0)
* int ``originY`` (default: 0)

The ``scaleX`` and ``scaleY`` properties can be used to override the
scale of the asset. However, if the scale is relevant then it will
generally be easier to use a tile object, in which case it is
automatically derived from the tile size and the object size.

The ``originX`` and ``originY`` properties can be used to tell Tiled
about the origin of the sprite defined in GameMaker, as an offset from
the top-left. This origin is taken into account when determining the
position of the exported assets.

Backgrounds
~~~~~~~~~~~
GameMaker background layers are created by using image layers in Tiled. 
The filename of the source image has to be the same as the corresponding sprite asset. Alternatively the custom property ``sprite`` can be used to overwrite the sprite name.

If a background color is set in the map properties of Tiled an extra background layer with the according color is exported as the bottommost layer.

The following custom properties can be set on image layers to affect the
exported background layers:

* bool ``htiled`` (default: false)
* bool ``vtiled`` (default: false)
* bool ``stretch`` (default: false)
* float ``hspeed`` (default: 0.0)
* float ``vspeed`` (default: 0.0)
* float ``animationSpeed` (default: 15.0)
* int ``animationSpeedtype`` (default: 0)
* int ``depth`` (default: 0 + N)

Even though the custom properties such as ``htiled``and ``vtiled`` have no visual effect inside Tiled you will see the effect in the exported room inside GameMaker.

The ``depth`` property can be used to assign a specific depth value to a layer.

Paths
~~~~~


.. warning::
    GameMaker Paths are not supported, yet.
    But it's planned to export polyline and polygon objects as paths on path layers in a future update.


Views
~~~~~

.. figure:: images/gamemaker-view-settings.png
   :alt: GameMaker View Settings
   :align: right

Views can be defined using :ref:`rectangle objects <insert-rectangle-tool>`
where the Type has been set to ``view``. The position and size will be
snapped to pixels. Whether the view is visible when the room starts
depends on whether the object is visible. The use of views is
automatically enabled when any views are defined. 

The following custom properties can be used to define the various other
properties of the view:

**General**

* bool ``inherit`` (default: false)

**Camera Properties**

The Camera Properties are automatically derived from the positions and sizes of the view objects.

**Viewport Properties**

* int ``xport`` (default: 0)
* int ``yport`` (default: 0)
* int ``wport`` (default: 1366)
* int ``hport`` (default: 768)

**Object following**

* string ``objectId`` 
* int ``hborder`` (default: 32)
* int ``vborder`` (default: 32)
* int ``hspeed`` (default: -1)
* int ``vspeed`` (default: -1)

.. hint::

   When you're defining views in Tiled, it is useful to add ``view``
   as object type in the :ref:`Object Types Editor <predefining-properties>`,
   adding the above properties for ease of access. If you frequently use
   views with similar settings, you can set up
   :doc:`templates <using-templates>` for them.

Room Properties
~~~~~~~~~~~~~~~

The following custom properties can be set under Map -> Map Properties.

General
^^^^^^^

* string ``path`` (default: "folders/Rooms.yy")
* bool ``inheritLayers"`` (default: false)
* string ``tags`` (default: "")

The ``path`` property is used to define the room location inside GameMakers asset browser.

The ``tags`` property is used to assign tags to the room. Multiple tags can be separated by commas.

Room Settings
^^^^^^^^^^^^^

* bool ``inheritRoomSettings`` (default: false)
* bool ``persistent`` (default: false)
* bool ``clearDisplayBuffer`` (default: true)
* bool ``inheritCode`` (default: false)
* string ``creationCodeFile`` (default: "")

The ``creationCodeFile`` property is used to define the path of the creation code file, e.g.: "${project_dir}/rooms/room_name/RoomCreationCode.gml".

Instance Creation Order
^^^^^^^^^^^^^^^^^^^^^^^

The instance creation order is derived from the object positions inside the layer and object hierarchy from Tiled.
You can manipulate the order by using the custom property ``creationOrder` inside objects. Objects with negative values will be sorted in before objects without a specified creationOrder value, while positive values will be sorted in after those unspecified objects.

Viewports and Cameras
^^^^^^^^^^^^^^^^^^^^^

* bool ``inheritViewSettings`` (default: false)
* bool ``enableViews`` (default: false)
* bool ``clearViewBackground`` (default: false)

**Viewport 0 - Viewport 7**

You can configure up to 8 viewports by using view objects (see Views).


Physics
^^^^^^^

* bool ``inheritPhysicsSettings`` (default: false)
* bool ``PhysicsWorld`` (default: false)
* float ``PhysicsWorldGravityX`` (default: 0.0)
* float ``PhysicsWorldGravityY`` (default: 10.0)
* float ``PhysicsWorldPixToMeters`` (default: 0.1)

Layer Properties
~~~~~~~~~~~~~~~~

All layer types inside Tiled support the following custom properties:

* int ``visible`` (default: derived from layer or true)
* int ``depth`` (default: 0 + N)

The ``depth`` property can be used to assign a specific depth value to a layer.
The ``visible`` property can be used to overwrite the "Visible" state of the layer inside Tiled if needed.
 
