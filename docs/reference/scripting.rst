Scripting
=========

Introduction
------------

Initial scripting capabilities have been added to Tiled, which can be
used from the Console view (*View > Views and Toolbars > Console*).

In the Console you will find a text entry where you can write or paste
scripts to automate certain actions. The available API is documented
below.

API Reference
-------------

tiled module
^^^^^^^^^^^^

The ``tiled`` module is the main entry point and provides a few
functions and properties which are documented below.

**tiled.version**
~~~~~~~~~~~~~~~~~

The currently used version of Tiled.

**tiled.platform**
~~~~~~~~~~~~~~~~~~

Operating system. One of ``windows``, ``macos``, ``linux`` or ``unix``
(for any other UNIX-like system).

**tiled.arch**
~~~~~~~~~~~~~~

Processor architecture. One of ``x64``, ``x86`` or ``unknown``.

**tiled.activeAsset**
~~~~~~~~~~~~~~~~~~~~~

The currently selected asset (:ref:`script-map` or
:ref:`script-tileset`), or ``null`` if no file is open. Can be assigned
any open asset in order to change the active asset.

The value is an instance of a class that provides read and write access
to the properties of the asset. Changing a property will create an undo
command for that action.

**tiled.openAssets**
~~~~~~~~~~~~~~~~~~~~

The list of currently opened assets (can contain
:ref:`Maps <script-map>` and :ref:`Tilesets <script-tileset>`).

**tiled.trigger(action)**
~~~~~~~~~~~~~~~~~~~~~~~~~

This function can be used to trigger any registered action. This
includes most actions you would normally trigger through the menu or by
using their shortcut.

The following actions are currently available:

+---------------------------+
| Action                    |
+===========================+
| About                     |
+---------------------------+
| AddExternalTileset        |
+---------------------------+
| AutoMap                   |
+---------------------------+
| AutoMapWhileDrawing       |
+---------------------------+
| BecomePatron              |
+---------------------------+
| ClearRecentFiles          |
+---------------------------+
| ClearView                 |
+---------------------------+
| Close                     |
+---------------------------+
| CloseAll                  |
+---------------------------+
| Copy                      |
+---------------------------+
| Cut                       |
+---------------------------+
| Delete                    |
+---------------------------+
| Documentation             |
+---------------------------+
| EditCommands              |
+---------------------------+
| Export                    |
+---------------------------+
| ExportAs                  |
+---------------------------+
| ExportAsImage             |
+---------------------------+
| FullScreen                |
+---------------------------+
| HighlightCurrentLayer     |
+---------------------------+
| HighlightHoveredObject    |
+---------------------------+
| LabelForHoveredObject     |
+---------------------------+
| LabelsForAllObjects       |
+---------------------------+
| LabelsForSelectedObjects  |
+---------------------------+
| LoadWorld                 |
+---------------------------+
| MapProperties             |
+---------------------------+
| NewMap                    |
+---------------------------+
| NewTileset                |
+---------------------------+
| NoLabels                  |
+---------------------------+
| OffsetMap                 |
+---------------------------+
| Open                      |
+---------------------------+
| Paste                     |
+---------------------------+
| PasteInPlace              |
+---------------------------+
| Preferences               |
+---------------------------+
| Quit                      |
+---------------------------+
| Reload                    |
+---------------------------+
| ResizeMap                 |
+---------------------------+
| Save                      |
+---------------------------+
| SaveAll                   |
+---------------------------+
| SaveAs                    |
+---------------------------+
| ShowGrid                  |
+---------------------------+
| ShowTileAnimations        |
+---------------------------+
| ShowTileObjectOutlines    |
+---------------------------+
| SnapNothing               |
+---------------------------+
| SnapToFineGrid            |
+---------------------------+
| SnapToGrid                |
+---------------------------+
| SnapToPixels              |
+---------------------------+
| TilesetProperties         |
+---------------------------+
| ZoomIn                    |
+---------------------------+
| ZoomNormal                |
+---------------------------+
| ZoomOut                   |
+---------------------------+
| SelectAll                 |
+---------------------------+
| SelectInverse             |
+---------------------------+
| SelectNone                |
+---------------------------+
| CropToSelection           |
+---------------------------+
| Autocrop                  |
+---------------------------+
| AddTileLayer              |
+---------------------------+
| AddObjectLayer            |
+---------------------------+
| AddImageLayer             |
+---------------------------+
| AddGroupLayer             |
+---------------------------+
| LayerViaCopy              |
+---------------------------+
| LayerViaCut               |
+---------------------------+
| GroupLayers               |
+---------------------------+
| UngroupLayers             |
+---------------------------+
| DuplicateLayers           |
+---------------------------+
| MergeLayersDown           |
+---------------------------+
| SelectPreviousLayer       |
+---------------------------+
| SelectNextLayer           |
+---------------------------+
| RemoveLayers              |
+---------------------------+
| MoveLayersUp              |
+---------------------------+
| MoveLayersDown            |
+---------------------------+
| ToggleOtherLayers         |
+---------------------------+
| ToggleLockOtherLayers     |
+---------------------------+
| LayerProperties           |
+---------------------------+
| DuplicateObjects          |
+---------------------------+
| RemoveObjects             |
+---------------------------+

Actions that are checkable will toggle when triggered.

**tiled.alert(text [, title])**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Shows a modal warning dialog to the user with the given text and
optional title.

**tiled.confirm(text [, title])**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Shows a yes/no dialog to the user with the given text and optional
title. Returns ``true`` or ``false``.

**tiled.prompt(label [, text [, title]])**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Shows a dialog that asks the user to enter some text, along with the
given label and optional title. The optional ``text`` parameter provides
the initial value of the text. Returns the entered text.

.. _script-asset:

Asset
^^^^^

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **fileName** : String *[read-only]*, File name of the asset.
    **modified** : bool *[read-only]*, Whether the asset was modified after it was saved or loaded.

.. _script-map:

Map
^^^

Inherits :ref:`script-asset`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **width** : int *[read-only]*, Width of the map in tiles. Use :ref:`resize <script-map-resize>` to change it.
    **height** : int *[read-only]*, Height of the map in tiles. Use :ref:`resize <script-map-resize>` to change it.
    **size** : Size *[read-only]*, Size of the map in tiles (has ``width`` and ``height`` members). Use :ref:`resize <script-map-resize>` to change it.
    **tileWidth** : int, Tile width (used by tile layers).
    **tileHeight**: int, Tile height (used by tile layers).
    **infinite** : bool, Whether this map is infinite.
    **hexSideLength** : int, Length of the side of a hexagonal tile (used by tile layers on hexagonal maps).
    **staggerAxis** : int, "For staggered and hexagonal maps, determines which axis (X or Y) is staggered: 0 (X), 1 (Y)."
    **orientation** : int, "General map orientation: 0 (Unknown), 1 (Orthogonal), 2 (Isometric), 3 (Staggered), 4 (Hexagonal)"
    **renderOrder** : int, "Tile rendering order (only implemented for orthogonal maps): 0 (RightDown), 1 (RightUp), 2 (LeftDown), 3 (LeftUp)"
    **staggerIndex** : int, "For staggered and hexagonal maps, determines whether the even or odd indexes along the staggered axis are shifted. 0 (Odd), 1 (Even)."
    **backgroundColor** : Color, Background color of the map.
    **layerDataFormat** : int, "The format in which the layer data is stored, taken into account by TMX, JSON and Lua map formats: 0 (XML), 1 (Base64), 2 (Base64Gzip), 3 (Base64Zlib), 4 (CSV)"
    **selectedArea** : :ref:`SelectionArea <script-selectedarea>`, The selected area of tiles.
    **layerCount** : int [read-only], Number of top-level layers the map has.

Functions
~~~~~~~~~

.. _script-map-layerAt:

**Map.layerAt(index : int)** : Layer
    Returns the top-level layer at the given index.

.. _script-map-resize:

**Map.resize(size : Size [, offset : Point [, removeObjects : bool = false]])** : void
    Resizes the map to the given size, optionally applying an offset (in tiles)

*todo*

.. _script-layer:

Layer
^^^^^

Properties
~~~~~~~~~~

*todo*

.. _script-tilelayer:

TileLayer
^^^^^^^^^

Inherits :ref:`script-layer`.

Properties
~~~~~~~~~~

*todo*

Functions
~~~~~~~~~

*todo*


.. _script-objectgroup:

ObjectGroup
^^^^^^^^^^^

Inherits :ref:`script-layer`.

Properties
~~~~~~~~~~

*todo*

Functions
~~~~~~~~~

*todo*

.. _script-mapobject:

MapObject
^^^^^^^^^^^^^^^

Properties
~~~~~~~~~~

*todo*

.. _script-tileset:

Tileset
^^^^^^^

Inherits :ref:`script-asset`.

Properties
~~~~~~~~~~

*todo*

.. _script-selectedarea:

SelectedArea
^^^^^^^^^^^^

Functions
~~~~~~~~~

**SelectedArea.set(rect : Rect)** : void
    Sets the selected area to the given rectangle.

**SelectedArea.set(region : Region)** : void
    Sets the selected area to the given region.

**SelectedArea.add(rect : Rect)** : void
    Adds the given rectangle to the selected area.

**SelectedArea.add(region : Region)** : void
    Adds the given region to the selected area.

**SelectedArea.subtract(rect : Rect)** : void
    Subtracts the given rectangle from the selected area.

**SelectedArea.subtract(region : Region)** : void
    Subtracts the given region from the selected area.

**SelectedArea.intersect(rect : Rect)** : void
    Sets the selected area to the intersection of the current selected area and the given rectangle.

**SelectedArea.intersect(region : Region)** : void
    Sets the selected area to the intersection of the current selected area and the given region.
