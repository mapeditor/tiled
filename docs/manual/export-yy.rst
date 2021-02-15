.. _gamemaker2-export:

GameMaker Studio 2.3
--------------------

GameMaker Studio 2.3 uses a JSON-based format to store its rooms, and Tiled
ships with a plugin to export maps in this format.

Tiled itself works with tile layers, object layers, image layers and group
layers. All these layer types are supported and if needed Tiled will
automatically divide a layer into several sublayers on export as GameMaker
only supports one tileset per tile layer and differentiates between instance
and asset layers.

In general:

* Tiles from tilesets on tile layers will get exported as tiles on tile layers. 
* Tiles from image collections on tile layers will get exported as assets on asset layers.
* Tile objects with no type will get exported as assets on asset layers.
* Any objects with a type set on them will get exported as instances on instance layers.
* Image layers will get exported as background layers.

.. warning::

   Since GameMaker's "Add Existing" action doesn't work at this point in time
   (2.3.1) the easiest way to export a Tiled map to your GameMaker Project is
   to replace an already existing ``room.yy`` file. If you want to do this
   while GameMaker is running with the open project you'll need to deactivate
   "Use save writing of files" in the Tiled preferences (Under *Edit ->
   Preferences -> General -> Saving and Loading*). Otherwise GameMaker will
   detect that the room file got deleted and will propose to restore it from
   memory. Without "save writing" GameMaker will detect that the file got
   changed and propose to reload the updated one.

Tilesets
~~~~~~~~

A tileset in Tiled has to be named the same as the corresponding tileset asset
in the GameMaker project. The file name of the source image has to be the same
as the name of the corresponding sprite asset. Alternatively the custom
property ``sprite`` can be used to explicitly set the name of the sprite
asset.

* string ``sprite`` (default: based on image filename)

Based on Tileset Image
^^^^^^^^^^^^^^^^^^^^^^

This type of tilesets equals the tilesets in GameMaker. If the map orientation
is orthogonal and the size of the tiles matches the size of the map grid, tile
layers with this type of tiles will get exported as GameMaker tile layers.

Tiles that have a different size then the grid will get exported as assets on
a separate asset layer instead. If you use different tilesets on the same tile
layer inside Tiled they will get divided into sublayers on export as GameMaker
only supports one tileset per tile layer.

Collection of Images
^^^^^^^^^^^^^^^^^^^^

This type of tilesets can be used to place sprites on asset layers or
instances on instance layers. This can be done with the "Insert Tile"-Tool on
object layers.

Objects with no set ``Type`` will get exported as assets on asset layers while
objects with a set ``Type`` will get exported as instances on instance layers.

You can also place tiles from image collections on tile layers in Tiled but
they will get exported as assets on a separate asset layer instead.

Isometric and Hexagonal Grids
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Isometric and hexagonal map orientations are supported but the tile layers
will get exported as asset layers in this case since GameMaker Studio 2.3
supports only tile layers with an orthogonal grid.


Object Instances
~~~~~~~~~~~~~~~~

GameMaker object instances are created by putting the object name in the
``Type`` field of the object in Tiled. Rotation, flipping and scaling is
supported.

The following custom properties can be set on objects to affect the exported
instance:

* bool ``hasCreationCode`` (default: false)
* color ``colour`` (default: based on layer tint color)
* float ``scaleX`` (default: derived from tile or 1.0)
* float ``scaleY`` (default: derived from tile or 1.0)
* int ``imageIndex`` (default: 0)
* float ``imageSpeed`` (default: 1.0)
* bool ``ignore`` (default: whether the object is hidden)
* bool ``inheritItemSettings`` (default: false)
* int ``creationOrder`` (default: 0)
* int ``originX`` (default: 0)
* int ``originY`` (default: 0)

The ``hasCreationCode`` property can be set to true. Refers to
"InstanceCreationCode_[inst_name].gml" in the room folder which you can create
inside GameMaker itself or with an external text editor.

The ``scaleX`` and ``scaleY`` properties can be used to override the
scale of the instance. However, if the scale is relevant then it will
generally be easier to use a tile object, in which case it is
automatically derived from the tile size and the object size.

The ``originX`` and ``originY`` properties can be used to tell Tiled
about the origin of the sprite defined in GameMaker, as an offset from
the top-left. This origin is taken into account when determining the
position of the exported instance.

.. hint::

   Of course setting the type and/or the above properties manually for each
   instance will get old fast. Instead you can use tile objects with the type
   set on the tile or use :doc:`object templates <using-templates>`.


Assets
~~~~~~

GameMaker assets are created by leaving the ``Type`` field of a tile object in
Tiled empty. They can be placed on both the object and tile layers in Tiled
but will get exported always to GameMaker asset layers. Rotation, flipping and
scaling is supported.

Sprites
^^^^^^^

objeThe following custom properties can be set on sprites to affect the
exported sprites:

* float ``headPosition`` (default: 0.0)
* float ``rotation`` (default:0.0)
* float ``scaleX`` (default: derived from tile or 1.0)
* float ``scaleY`` (default: derived from tile or 1.0)
* float ``animationSpeed`` (default: 1.0)
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

GMRGraphic-Tiles
^^^^^^^^^^^^^^^^

Tiled supports placing single tiles from a tileset image outside the grid by
placing them on an object layer. In this case the individual tile assets will
get exported as "GMRGraphics" (aka GMS1.4 tiles) to an asset layer. These
"GMRGraphic"-tiles support horizontal and vertical flipping as well as color
tinting but no rotation. 

This type of tiles is also used to export Tiled maps with an isometric or
hexagonal map orientation.

Backgrounds
~~~~~~~~~~~

GameMaker background layers are created by using image layers in Tiled.

The file name of the source image has to be the same as the name of the
corresponding sprite asset. Alternatively the custom property ``sprite`` can
be used to explicitly set the name of the sprite asset.

If a ``Background Color`` is set in the map properties of Tiled an extra
background layer with the according color is exported as the bottommost layer.

The following custom properties can be set on image layers to affect the
exported background layers:

* string ``sprite`` (default: based on image filename)
* bool ``htiled`` (default: false)
* bool ``vtiled`` (default: false)
* bool ``stretch`` (default: false)
* float ``hspeed`` (default: 0.0)
* float ``vspeed`` (default: 0.0)
* float ``animationSpeed`` (default: 15.0)
* int ``animationSpeedtype`` (default: 0)
* int ``depth`` (default: 0 + N)

Even though the custom properties such as ``htiled`` and ``vtiled`` have no
visual effect inside Tiled you will see the effect in the exported room inside
GameMaker.

The ``depth`` property can be used to assign a specific depth value to the
layer.

Paths
~~~~~

.. warning::

    GameMaker Paths are not supported, yet. It's planned to export polyline
    and polygon objects as paths on path layers in a future update.

.. _yy-views:

Views
~~~~~

Views can be defined using :ref:`rectangle objects <insert-rectangle-tool>`
where the ``Type`` has been set to "view". The position and size will be
snapped to pixels. Whether the view is visible when the room starts
depends on whether the object is visible. The use of views is
automatically enabled when any views are defined. 

The following custom properties can be used to define the various other
properties of the view:

**General**

* bool ``inherit`` (default: false)

**Camera Properties**

The Camera Properties are automatically derived from the positions and sizes
of the view objects.

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

The following custom properties can be set under *Map -> Map Properties*.

General
^^^^^^^

* string ``path`` (default: "folders/Rooms.yy")
* bool ``inheritLayers`` (default: false)
* string ``tags`` (default: "")

The ``path`` property is used to define the parent folder inside GameMakers
asset browser.

The ``tags`` property is used to assign tags to the room. Multiple tags can be
separated by commas.

Room Settings
^^^^^^^^^^^^^

* bool ``inheritRoomSettings`` (default: false)
* bool ``persistent`` (default: false)
* bool ``clearDisplayBuffer`` (default: true)
* bool ``inheritCode`` (default: false)
* string ``creationCodeFile`` (default: "")

The ``creationCodeFile`` property is used to define the path of an existing
creation code file, e.g.: "${project_dir}/rooms/room_name/RoomCreationCode.gml".

Instance Creation Order
^^^^^^^^^^^^^^^^^^^^^^^

The instance creation order is derived from the object positions inside the
layer and object hierarchy from Tiled.

You can manipulate the order by using the custom property ``creationOrder``
inside objects. Objects with negative values will be created before objects
without a specified creationOrder value, while positive values will be created
after those unspecified objects.

Viewports and Cameras
^^^^^^^^^^^^^^^^^^^^^

**General**

* bool ``inheritViewSettings`` (default: false)
* bool ``enableViews`` (default: true when any "view" objects were found)
* bool ``clearViewBackground`` (default: false)

**Viewport 0 - Viewport 7**

You can configure up to 8 viewports by using view objects (see
:ref:`yy-views`).

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

* int ``depth`` (default: auto-assigned, like in GameMaker)

* int ``visible`` (default: derived from layer)

The ``depth`` property can be used to assign a specific depth value to a
layer.

The ``visible`` property can be used to overwrite the "Visible" state of the
layer inside Tiled if needed.
 
