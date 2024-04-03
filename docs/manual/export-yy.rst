.. _gamemaker2-export:

.. raw:: html

    <div class="new new-prev">Since Tiled 1.5</div>

GameMaker Studio 2.3
====================

GameMaker Studio 2.3 uses a JSON-based format to store its rooms, and Tiled
ships with a plugin to export maps in this format.

This plugin will do its best to export the map as accurately as possible,
mapping Tiled's various features to the matching GameMaker features.
:ref:`Tile layers <yy-tile-layers>` get exported as tile layers when possible,
but will fall back to asset layers if necessary. :ref:`Objects
<yy-object-layers>` can get exported as instances, but also as tile graphics,
sprite graphics or views. :ref:`Image layers <yy-image-layers>` get exported
as background layers.

.. warning::

   Since GameMaker's "Add Existing" action doesn't work at this point in time
   (2.3.1) the easiest way to export a Tiled map to your GameMaker Project is
   to overwrite an already existing ``room.yy`` file.

   Starting with Tiled 1.8, it is no longer necessary to deactivate the "Use
   safe writing of files" option, since the GameMaker export now ignores it to
   avoid reload issues in GameMaker.

.. _yy-asset-references:

References to Existing Assets
-----------------------------

Since Tiled currently only exports a map as a GameMaker room, any sprites,
tilesets and objects used by the map are expected to be already available in
the GameMaker project.

For sprites, the sprite name is derived by looking for a ``*.yy`` file in the
directory of the image file and up to two parent directories. If such a file
is found, it is assumed to be the associated meta file and its name without
the file extension is used.
If no ``*.yy`` file can be found, the name of the image file without its file
extension is used.

If necessary, the sprite name can be explicitly specified using a custom
``sprite`` property (supported on tilesets, tiles from image collection
tilesets and image layers).

For tilesets, the tileset name entered in Tiled must match the name of the
tileset asset in GameMaker.

For object instances, the name of the object should be set in the *Class*
field.

Exporting a Tiled Map
---------------------

A Tiled map contains tile layers, object layers, image layers and group
layers. All these layer types are supported.

.. _yy-tile-layers:

Tile Layers
~~~~~~~~~~~

When possible, a tile layer will get exported as a tile layer.

When several tilesets are used on the same layer, the layer gets exported as a
group with a child tile layer for each tileset, since GameMaker supports only
one tileset per tile layer.

When the tile size of a tileset doesn't match the grid size of the map, or
when the map orientation is not orthogonal (for example, isometric or
hexagonal), the tiles will get exported to an asset layer instead. This layer
type is more flexible, though for tile graphics it does not support rotation.

When the layer includes tiles from a collection of images tileset, these will
get exported to an asset layer as sprite graphics.

.. _yy-object-layers:

Object Layers
~~~~~~~~~~~~~

Object layers in Tiled are very flexible since objects take so many forms. As
such the export looks at each object to see how it should be exported to the
GameMaker room.

When an object has a *Class*, it is exported as an instance on an instance
layer, where the class refers to the name of the object to instantiate. Except,
when the class is "view", the object is interpreted as :ref:`a view
<yy-views>`.

When an object has no Class, but it is a tile object, then it is exported as
either a tile graphic or a sprite graphic, depending on whether the tile is
from a tileset image or a collection of images.

The following custom properties can be set on objects to affect the exported
instance or sprite asset:

* color ``colour`` (default: based on layer tint color)
* float ``scaleX`` (default: derived from tile or 1.0)
* float ``scaleY`` (default: derived from tile or 1.0)
* bool ``inheritItemSettings`` (default: false)
* int ``originX`` (default: 0)
* int ``originY`` (default: 0)
* bool ``ignore`` (default: whether the object is hidden)

The ``scaleX`` and ``scaleY`` properties can be used to override the
scale of the instance. However, if the scale is relevant then it will
generally be easier to use a tile object, in which case it is
automatically derived from the tile size and the object size.

The ``originX`` and ``originY`` properties can be used to tell Tiled
about the origin of the sprite defined in GameMaker, as an offset from
the top-left. This origin is taken into account when determining the
position of the exported instance.

.. hint::

   Of course setting the class and/or the above properties manually for each
   instance will get old fast. Instead you can use tile objects with the class
   set on the tile or use :doc:`object templates <using-templates>`.

Object Instances
^^^^^^^^^^^^^^^^

The following additional custom properties can be set on objects that are
exported as object instances:

* bool ``hasCreationCode`` (default: false)
* int ``imageIndex`` (default: 0)
* float ``imageSpeed`` (default: 1.0)
* int ``creationOrder`` (default: 0)

The ``hasCreationCode`` property can be set to true. Refers to
"InstanceCreationCode_[inst_name].gml" in the room folder which you can create
inside GameMaker itself or with an external text editor.

By default the instance creation order is derived from the object positions
inside the layer and object hierarchy from Tiled. This can be changed by using
the custom property ``creationOrder``. Objects with lower values will be
created before objects with higher values (so objects with negative values
will be created before objects without a ``creationOrder`` property).

Additional custom properties that are not documented here can be used to
override the variable definitions that got set up inside GameMaker for the
object.

.. note::

    As of now only variable definitions of the object itself can be overridden.
    Overriding variable definitions of parent objects is not supported. As a
    workaround you can use the creation code to override variables of a parent
    object.

Tile Graphics
^^^^^^^^^^^^^

For objects exported as tile graphics (aka GMS 1.4 tiles), it should be noted
that rotation is not supported on asset layers.

When 90-degree rotation with grid-alignment suffices, these tiles should be
placed on tile layers instead. When free placement with rotation is required,
a collection of images tileset should be used, so that the objects can be
exported as sprite graphics instead.

Sprite Graphics
^^^^^^^^^^^^^^^

The following additional custom properties can be set on objects that are
exported as sprite graphics:

* float ``headPosition`` (default: 0.0)
* float ``animationSpeed`` (default: 1.0)

.. _yy-image-layers:

Image Layers
~~~~~~~~~~~~

Image layers are exported as background layers.

The file name of the source image is assumed to be the same as the name of the
corresponding sprite asset. Alternatively the custom property ``sprite`` can
be used to explicitly set the name of the sprite asset.

While not supported visually in Tiled, it is possible to create an image layer
without an image but with only a tint color. Such layers will get exported as
a background layer with just the color set.

The following custom properties can be set on image layers to affect the
exported background layers:

* string ``sprite`` (default: based on image filename)
* bool ``htiled`` (default: value of Repeat X property)
* bool ``vtiled`` (default: value of Repeat Y property)
* bool ``stretch`` (default: false)
* float ``hspeed`` (default: 0.0)
* float ``vspeed`` (default: 0.0)
* float ``animationFPS`` (default: 15.0)
* int ``animationSpeedtype`` (default: 0)

Even though the custom properties such as ``hspeed`` and ``vspeed`` have no
visual effect inside Tiled you will see the effect in the exported room inside
GameMaker.

Special Cases and Custom Properties
-----------------------------------

Rooms
~~~~~

If a ``Background Color`` is set in the map properties of Tiled an extra
background layer with the according color is exported as the bottommost layer.

The following custom properties can be set under *Map -> Map Properties*.

General
^^^^^^^

* string ``parent`` (default: "Rooms")
* bool ``inheritLayers`` (default: false)
* string ``tags`` (default: "")

The ``parent`` property is used to define the parent folder inside GameMakers
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

Sprite References
~~~~~~~~~~~~~~~~~

As :ref:`mentioned above <yy-asset-references>`, references to sprites
generally derive the name of the sprite asset from the image file name. The
following property can be set on tilesets, tiles from image collection
tilesets and image layers to explicitly specify the sprite name:

* string ``sprite`` (default: based on image filename)

.. _yy-paths:

Paths
~~~~~

.. warning::

    Paths are not supported yet, but it's planned to export polyline and
    polygon objects as paths on path layers in a future update.

.. _yy-views:

Views
~~~~~

Views can be defined using :ref:`rectangle objects <insert-rectangle-tool>`
where the *Class* has been set to "view". The position and size will be snapped
to pixels. Whether the view is visible when the room starts depends on whether
the object is visible. The use of views is automatically enabled when any
views are defined.

The following custom properties can be used to define the various other
properties of the view:

**General**

* bool ``inherit`` (default: false)

**Camera Properties**

The Camera Properties are automatically derived from the position and size of
the view object.

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
   as class in the :ref:`Custom Types Editor <custom-property-types>`,
   adding the above properties for ease of access. If you frequently use
   views with similar settings, you can set up
   :doc:`templates <using-templates>` for them.

Layers
~~~~~~

All layer types support the following custom properties:

* int ``depth`` (default: auto-assigned, like in GameMaker)
* bool ``visible`` (default: derived from layer)
* bool ``hierarchyFrozen`` (default: layer locked state)
* bool ``noExport`` (default: false)

The ``depth`` property can be used to assign a specific depth value to a
layer.

The ``visible`` property can be used to override the "Visible" state of the
layer if needed.

The ``hierarchyFrozen`` property can be used to override the "Locked" state of
the layer if needed.

The ``noExport`` property can be used to suppress exporting of an entire
layer, including any child layers. This is useful if you use a layer for
annotations (like adding background image or text objects) that you do not
want exported to GameMaker. Note that any views defined on this layer will
then also get ignored.
