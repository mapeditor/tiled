Generic File Formats
====================

Tiled supports exporting to several generic file formats which are not
targeting any specific framework.

JSON
----

:doc:`The JSON format </reference/json-map-format>` is the most common
additional file format supported by Tiled. It can be used instead of TMX
since Tiled can also open JSON maps and tilesets and the format supports
all Tiled features. Especially in the browser and when using JavaScript
in general, the JSON format is easier to load.

.. _lua-export:

Lua
---

Maps and tilesets can be exported to Lua code. This export option supports
most of Tiled's features and is useful when using a Lua-based framework like
`LÖVE`_ (with `Simple Tiled Implementation`_), `Solar2D`_ (with
`ponytiled`_ or `Dusk Engine`_) or `Defold`_.

Currently not included are the type of custom properties (though the
type does affect how a property value is exported) and information
related to :doc:`object templates <using-templates>`.

The tiles are referenced using :doc:`/reference/global-tile-ids`, as done in
the :doc:`TMX </reference/tmx-map-format>` and :doc:`JSON
</reference/json-map-format>` formats.

CSV
---

The CSV export only supports :doc:`tile layers <editing-tile-layers>`.
Maps containing multiple tile layers will export as multiple files,
called ``base_<layer-name>.csv``.

Each tile is written out by its ID, unless the tile has a custom
property called ``name``, in which case its value is used to write out
the tile. Using multiple tilesets will lead to ambiguous IDs, unless the
custom ``name`` property is used. Empty cells get the value ``-1``.

When tiles are flipped horizontally, vertically or diagonally, these states
are exported using bitflags in the ID, in the same way as done in the
:doc:`/reference/tmx-map-format`.

.. _LÖVE: https://love2d.org/
.. _Solar2D: https://solar2d.com/
.. _Defold: https://www.defold.com/
.. _Simple Tiled Implementation: https://github.com/karai17/Simple-Tiled-Implementation
.. _ponytiled: https://github.com/ponywolf/ponytiled
.. _Dusk Engine: https://github.com/GymbylCoding/Dusk-Engine
