Working with Layers
===================

A Tiled map supports various sorts of content, and this content is
organized into various different layers. The most common layers are the
`Tile Layer <#tile-layers>`__ and the `Object Layer <#object-layers>`__.
There is also an `Image Layer <#image-layers>`__ for including simple
foreground or background graphics. The order of the layers determines
the rendering order of your content.

Layers can be hidden, made only partially visible and can be locked. Layers
also have an offset, which can be used to position them independently of each
other, for example to fake depth. Finally their contents can be tinted by
multiplying with a custom :ref:`tint color <tint-color>`.

.. figure:: images/layers/lock-visibility-toggle.png
   :alt: Layers View

   The eye and lock icon toggle the visibility and locked state of a
   layer respectively.

You use `Group Layers <#group-layers>`__ to organize the layers into a
hierarchy. This makes it more comfortable to work with a large amount of
layers.

Layer Types
-----------

.. _tile-layer-introduction:

Tile Layers
~~~~~~~~~~~

Tile layers provide an efficient way of storing a large area filled with
tile data. The data is a simple array of tile references and as such no
additional information can be stored for each location. The only extra
information stored are a few flags, that allow tile graphics to be
flipped vertically, horizontally or anti-diagonally (to support rotation
in 90-degree increments).

The information needed to render each tile layer is stored with the map,
which specifies the position and rendering order of the tiles based on
the orientation and various other properties.

Despite only being able to refer to tiles, tile layers can also be
useful for defining various bits of non-graphical information in your
level. Collision information can often be conveyed using a special
tileset, and any kind of object that does not need custom properties and
is always aligned to the grid can also be placed on a tile layer.

.. _object-layer-introduction:

Object Layers
~~~~~~~~~~~~~

Object layers are useful because they can store many kinds of
information that would not fit in a tile layer. Objects can be freely
positioned, resized and rotated. They can also have individual custom
properties. There are many kinds of objects:

-  **Rectangle** - for marking custom rectangular areas
-  **Ellipse** - for marking custom ellipse or circular areas
-  **Point** - for marking exact locations (since Tiled 1.1)
-  **Polygon** - for when a rectangle or ellipse doesn't cut it (often a
   collision area)
-  **Polyline** - can be a path to follow or a wall to collide with
-  **Tile** - for freely placing, scaling and rotating your tile
   graphics
-  **Text** - for custom text or notes (since Tiled 1.0)

All objects can be named, in which case their name will show up in a
label above them (by default only for selected objects). Objects can
also be given a *type*, which is useful since it can be used to
customize the color of their label and the available :ref:`custom
properties <predefining-properties>` for this
object type. For tile objects, the type can be :ref:`inherited from their
tile <tile-property-inheritance>`.

For most map types, objects are positioned in plain pixels. The only
exception to this are isometric maps (not isometric staggered). For
isometric maps, it was deemed useful to store their positions in a
projected coordinate space. For this, the isometric tiles are assumed to
represent projected squares with both sides equal to the *tile height*.
If you're using a different coordinate space for objects in your
isometric game, you'll need to convert these coordinates accordingly.

The object width and height is also mostly stored in pixels. For
isometric maps, all shape objects (rectangle, point, ellipse, polygon and
polyline) are projected into the same coordinate space described above.
This is based on the assumption that these objects are generally used to
mark areas on the map.

.. _image-layers:

Image Layers
~~~~~~~~~~~~

Image layers provide a way to quickly include a single image as
foreground or background of your map. They are currently not so useful,
because if you instead add the image as a Tileset and place it as a :ref:`Tile Object <insert-tile-tool>`,
you gain the ability to freely scale and rotate the image.

The only advantage of using an image layer is that it avoids selecting /
dragging the image while using the Select Objects tool. However, since Tiled
1.1 this can also be achieved by locking the object layer containing the tile
object you'd like to avoid interacting with.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.0</div>

.. _group-layers:

Group Layers
~~~~~~~~~~~~

Group layers work like folders and can be used for organizing the layers
into a hierarchy. This is mainly useful when your map contains a large
amount of layers.

The visibility, opacity, offset, lock and :ref:`tint color <tint-color>` of a
group layer affects all child layers.

Layers can be easily dragged in and out of groups with the mouse. The
Raise Layer / Lower Layer actions also allow moving layers in and out of
groups.

.. raw:: html

   <div class="new">New in Tiled 1.4</div>

.. _tint-color:

Tinting Layers
--------------

When you set the *Tint Color* property of a layer, this affects the way images
are rendered. This includes tiles, tile objects and the image of an
:ref:`Image Layer <image-layers>`.

Each pixel color value is multiplied by the tint color. This way you can
darken or colorize your graphics in various ways without needing to set up
separate images for it.

.. figure:: images/tint-color.png
   :alt: Tint Color Example

   A gray tileset rendered in a different color for each layer.

The tint color can also be set on a :ref:`Group Layer <group-layers>`, in
which case it is inherited by all layers in the group.


.. topic:: Future Extensions
   :class: future

   There are many ways in which the layers can be made more powerful:

   -  Ability to lock individual objects
      (`#828 <https://github.com/bjorn/tiled/issues/828>`__).
   -  Moving certain map-global properties to the Tile Layer
      (`#149 <https://github.com/bjorn/tiled/issues/149>`__). It would be
      useful if one map could accommodate layers of different tile sizes
      and maybe even of different orientation.

   If you like any of these plans, please help me getting around to it
   faster by `sponsoring Tiled development <https://www.mapeditor.org/donate>`__. The
   more support I receive the more time I can afford to spend improving
   Tiled!
