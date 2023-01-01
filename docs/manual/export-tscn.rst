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

Layers
~~~~~~

All layer types support the following custom properties:

* bool ``ySortEnabled`` (default: false)
* int ``zIndex`` (default: 0)
* bool ``noExport`` (default: false)

The ``ySortEnabled`` property can be used to change the drawing order to allow
sprites to be drawn behind tiles based on their Y coordinate.

The ``zIndex`` property can be used to assign a specific depth value to a
layer.

The ``noExport`` property can be used to suppress exporting of an entire
layer, including any child layers. This is useful if you use a layer for
annotations (like adding background image or text objects) that you do not
want exported to Godot. Note that any views defined on this layer will
then also get ignored.

Limitations
~~~~~~~~~~~

* The Godot 4 exporter does not currently support collection of images 
  tilesets, animations, object layers, or image layers.
* Godot's hexagonal maps only support :ref:`hex side lengths <tmx-map>`
  that are exactly half the tile height. So if, for example, your tile 
  height is 16, then your hex side length must be 8.