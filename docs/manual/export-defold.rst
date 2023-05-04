Defold
------

Tiled can export to `Defold <https://defold.com/>`__ using one of the two
supplied plugins. Both are disabled by default.

defold
~~~~~~

This plugin exports a map to a `Defold Tile Map <https://www.defold.com/manuals/tilemap/>`__ (\*.tilemap).
It only supports tile layers and only a single tileset may be used.

.. raw:: html

   <div class="new">New in Tiled 1.9.2</div>

Custom Properties
^^^^^^^^^^^^^^^^^

The ``tile_set`` property of the Tile Map can be set by adding a custom
string property to the map named "tile_set" (case sensitive). If left empty,
it will need to be set up in Defold after each export.

A custom float property named "z" can be added to set the ``z`` value for each
tile layer. By default, the layers will be exported with incrementing z values,
so you only need to set this property in case you need to customize the
rendering order.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.3</div>

defoldcollection
~~~~~~~~~~~~~~~~

This plugin exports a map to a `Defold Collection
<https://www.defold.com/manuals/building-blocks/>`__ (\*.collection), while
also creating multiple .tilemap files.

It supports:

* Group layers (**only top-level group layers are supported, not nested ones!**)
* Multiple Tilesets per Tilemap

The plugin automatically assigns a Z-index to each layer ranging between 0 and
0.1. It supports the use of 9999 Group Layers and 9999 Tile Layers per Group
Layer.

When any additional information from the map is needed, the map can be
exported in :ref:`Lua format <lua-export>` and loaded as Defold script.

Custom Properties
^^^^^^^^^^^^^^^^^

* The ``tile_set`` property of each tilemap may need to be set up manually in
  Defold after each export. However, Tiled will attempt to find the
  .tilesource file corresponding with the name your Tileset in Tiled in your
  project's ``/tilesources/`` directory. If one is found, manual adjustments
  won't be necessary.

  Alternatively, a custom string property called "tilesource" (case-sensitive)
  can be added to the *tileset*, which will then be used instead (since Tiled
  1.9.2).

* If you create custom properties on your map called ``x-offset`` and
  ``y-offset``, these values will be used as coordinates for your top-level
  GameObject in the Collection. This is useful when working with :doc:`Worlds
  <worlds>`.

.. raw:: html

   <div class="new">New in Tiled 1.10</div>

* A custom float property named "z" can be added to tile layers to manually
  specify their ``z`` value.
