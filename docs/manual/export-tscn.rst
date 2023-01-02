.. _godot4-export:

.. raw:: html

    <div class="new new-prev">Since Tiled 1.10</div>

Godot 4
=======

Godot 4.0 revamped its tilemap node, and Tiled ships with a plugin to export
maps in this format. For previous version of Godot, see
`Tiled To Godot Export <https://github.com/MikeMnD/tiled-to-godot-export>`__.

The Godot 4 exporter assumes that the generated .tscn files and the tileset
artwork all share the same file hierarchy. The exporter will search for a
common parent folder containing a .godot project file and use that folder
as the res:// root for the project. The exporter will search at least two
parent folders for a .godot file.

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
    must ensure that every map which uses the same .tres file also uses *all*
    of the same tilesets. You may wish to create a layer with the
    ``tilesetOnly`` property to ensure the correct tilesets are exported.

Limitations
~~~~~~~~~~~

* The Godot 4 exporter does not currently support collection of images 
  tilesets, object layers, or image layers.
* Godot's hexagonal maps only support :ref:`hex side lengths <tmx-map>`
  that are exactly half the tile height. So if, for example, your tile 
  height is 16, then your hex side length must be 8.
* Godot's hexagonal maps do not support 120° tile rotations.
* Animations frames must strictly go from left-to-right and top-to-bottom,
  without skipping any frames, and animation frames may not be used for
  anything else.