.. _godot4-export:

.. raw:: html

    <div class="new">New in Tiled 1.10</div>

Godot 4
=======

Godot 4 revamped its TileMap node, and Tiled ships with a plugin to export
maps in this format. For exporting to Godot 3, see the `Tiled To Godot Export
<https://github.com/mapeditor/tiled-to-godot-export>`__ extension.

The Godot 4 exporter assumes that the generated ``.tscn`` files and the tileset
artwork all share the same file hierarchy. The exporter will search for a
common parent folder containing a ``.godot`` project file and use that folder
as the ``res://`` root for the project. The exporter will search at least two
parent folders for a ``.godot`` file.

Layer Properties
~~~~~~~~~~~~~~~~

All layer types support the following custom properties:

* bool ``ySortEnabled`` (default: false)
* int ``zIndex`` (default: 0)
* bool ``noExport`` (default: false)
* bool ``tilesetOnly`` (default: blank)

The ``ySortEnabled`` property can be used to change the drawing order to allow
sprites to be drawn behind tiles based on their Y coordinate.

The ``zIndex`` property can be used to assign a specific depth value to a
layer.

The ``noExport`` property can be used to suppress exporting of an entire
layer, including any child layers. This is useful if you use a layer for
annotations (like adding background image or text objects) that you do not
want exported to Godot. Note that any views defined on this layer will
then also get ignored.

The ``tilesetOnly`` property can be used if you want to export all the tilesets
used in this layer, without actually exporting the layer itself. By default,
the exporter will only export tilesets which are actually used in the map, so
this property allows you to export tilesets that normally would otherwise get
skipped. This is most useful in combination with the :ref:`tilesetResPath
property <godot4-map-properties>`.

Tileset Properties
~~~~~~~~~~~~~~~~~~

Tilesets support the following property:

* bool ``exportAlternates`` (default: false)

The ``exportAlternates`` property is necessary when using flipped or rotated
tiles. This will create 7 alternate tiles for each tile, allowing all flipped
and rotation combinations.

Tile Properties
~~~~~~~~~~~~~~~

.. raw:: html

    <div class="new">New in Tiled 1.10.2</div>

All custom properties set on tiles will get exported as `Custom Data Layers
<https://docs.godotengine.org/en/stable/tutorials/2d/using_tilesets.html#assigning-custom-metadata-to-the-tileset-s-tiles>`__
of the Godot TileSet resource.

.. _godot4-map-properties:

Map Properties
~~~~~~~~~~~~~~

Maps support the following custom property:

* string ``tilesetResPath`` (default: blank)

The ``tilesetResPath`` property saves the tileset to an external .tres file,
allowing it to be shared between multiple maps more efficiently. This path 
must be in the form of 'res://<path>.tres'. The tileset file will be
overwritten every time the map is exported.

.. note::

    Only tilesets that are used in the current map will be exported. You
    must ensure that every map which uses the same ``.tres`` file also uses
    *all* of the same tilesets. You may wish to create a layer with the
    ``tilesetOnly`` property to ensure the correct tilesets are exported.

Object Properties
~~~~~~~~~~~~~~~~~

Objects support the following properties:

* string ``resPath`` (required)
* float ``originX`` (default: 0)
* float ``originY`` (default: 0)

The ``resPath`` property takes the form of 'res://<pbject path>.tscn' and must
be set to the path of the Godot object you wish to replace the object with.
Objects without this property set will not be exported.

The ``originX`` and ``originY`` properties provide an offset to the object
position. Tiled normally sets the origin to the lower-left corner of the
object, so if the origin of your object in Godot is different, you can use
these properties to ensure they get properly positioned.

Limitations
~~~~~~~~~~~

* The Godot 4 exporter does not currently support collection of images 
  tilesets or image layers.
* Godot's hexagonal maps only support :ref:`hex side lengths <tmx-map>`
  that are exactly half the tile height. So if, for example, your tile 
  height is 16, then your hex side length must be 8.
* Godot's hexagonal maps do not support 120° tile rotations.
* Animations frames must strictly go from left-to-right and top-to-bottom,
  without skipping any frames, and animation frames may not be used for
  anything else.
