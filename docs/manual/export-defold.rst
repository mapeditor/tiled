Defold
------

Tiled can export to `Defold <https://defold.com/>`__ using one of the two
supplied plugins. Both are disabled by default.

defold
~~~~~~

This plugin exports a map to a `Defold Tile Map <https://www.defold.com/manuals/tilemap/>`__ (\*.tilemap).
It only supports tile layers and only a single tileset may be used.

Upon export, the ``tile_set`` property of the Tile Map is left empty, so it
will need to be set up in Defold after each export.

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

Upon export:

* The ``Path`` property of each Tileset may need to be set up manually in
  Defold after each export. However, Tiled will attempt to find the
  .tilesource file corresponding with the name your Tileset in Tiled in your
  project's ``/tilesources/`` directory. If one is found, manual adjustments
  won't be necessary.

* If you create custom properties on your map called ``x-offset`` and
  ``y-offset``, these values will be used as coordinates for your top-level
  GameObject in the Collection. This is useful when working with :doc:`Worlds
  <worlds>`.

All layers of a Tilemap will have Z-index property assigned with values
ranging between 0 and 0.1. The plugin supports the use of 9999 Group Layers
and 9999 Tile Layers per Group Layer.

When any additional information from the map is needed, the map can be
exported in :ref:`Lua format <lua-export>` and loaded as Defold script.
