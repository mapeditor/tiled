Scripting
=========

Introduction
------------

Initial scripting capabilities have been added to Tiled. The API is
still incomplete, but many actions can be automated either by
interacting with any open assets or by triggering UI actions. Scripts
can be run from the Console view and there is a script loaded on
startup.

Startup Script
^^^^^^^^^^^^^^

If present, a :file:`startup.js` script is evaluated on startup. This
script could define functions that can be called from the Console or can
connect to signals to add functionality.

The location of the startup script depends on the platform. The file
:file:`startup.js` is searched for in the following locations:

+-------------+-----------------------------------------------------------------+
| **Windows** | | :file:`C:/Users/<USER>/AppData/Local/Tiled/startup.js`        |
|             | | :file:`C:/ProgramData/Tiled/startup.js`                       |
+-------------+-----------------------------------------------------------------+
| **macOS**   | | :file:`~/Library/Preferences/Tiled/startup.js`                |
+-------------+-----------------------------------------------------------------+
| **Linux**   | | :file:`~/.config/tiled/startup.js`                            |
|             | | :file:`/etc/xdg/tiled/startup.js`                             |
+-------------+-----------------------------------------------------------------+

Any file that exists will be evaluated. Changes made to startup scripts
only take effect when restarting Tiled.

Console View
^^^^^^^^^^^^

In the Console view (*View > Views and Toolbars > Console*) you will
find a text entry where you can write or paste scripts to evaluate them.

You can use the Up/Down keys to navigate through previously entered
script expressions.

Connecting to Signals
^^^^^^^^^^^^^^^^^^^^^

The script API provides signals to which functions can be connected.
Currently, the tiled module has the most useful :ref:`set of signals <script-tiled-signals>`.

Properties usually will have related signals which can be used to detect
changes to that property, but most of those are currently not
implemented.

To connect to a signal, call its ``connect`` function and pass in a
function object. In the following example, newly created maps
automatically get their first tile layer removed:

.. code:: js

    tiled.assetCreated.connect(function(asset) {
        if (asset.layerCount > 0) {
            asset.removeLayerAt(0)
            tiled.log("assetCreated: Removed automatically added tile layer.")
        }
    })

In some cases it will be necessary to later disconnect the function from
the signal again. This can be done by defining the function separately
and passing it into the ``disconnect`` function:

.. code:: js

    function onAssetCreated(asset) {
        // Do something...
    }

    tiled.assetCreated.connect(onAssetCreated)
    // ...
    tiled.assetCreated.disconnect(onAssetCreated)

API Reference
-------------

tiled module
^^^^^^^^^^^^

The ``tiled`` module is the main entry point and provides properties,
functions and signals which are documented below.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **version** : string |ro|, Currently used version of Tiled.
    **platform** : string |ro|, "Operating system. One of ``windows``, ``macos``, ``linux`` or ``unix``
    (for any other UNIX-like system)."
    **arch** : string |ro|, "Processor architecture. One of ``x64``, ``x86`` or ``unknown``."
    **activeAsset** : :ref:`script-asset`, "Currently selected asset, or ``null`` if no file is open. Can be assigned
    any open asset in order to change the active asset."
    **openAssets** : array |ro|, "List of currently opened :ref:`assets <script-asset>`."

Functions
~~~~~~~~~

tiled.trigger(action : string) : void
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

tiled.alert(text : string [, title : string]) : void
    Shows a modal warning dialog to the user with the given text and
    optional title.

tiled.confirm(text : string [, title : string]) : bool
    Shows a yes/no dialog to the user with the given text and optional
    title. Returns ``true`` or ``false``.

tiled.prompt(label : string [, text : string [, title : string]]) : string
    Shows a dialog that asks the user to enter some text, along with the
    given label and optional title. The optional ``text`` parameter provides
    the initial value of the text. Returns the entered text.

tiled.log(text : string) : void
    Outputs the given text in the Console window as regular text.

tiled.error(text : string) : void
    Outputs the given text in the Console window as error message (automatically
    gets "Error: " prepended).

.. _script-registerMapFormat:

tiled.registerMapFormat(shortName : string, mapFormat : object) : void
    Registers a new map format that can then be used to export maps to.

    If a map format is already registered with the same ``shortName``,
    the existing format is replaced. The short name can also be used to
    specify the format when using ``--export-map`` on the command-line,
    in case the file extension is ambiguous or a different one should be
    used.

    The ``mapFormat`` object is expected to have the following members:

    .. csv-table::
        :widths: 1, 2

        **name** : string, Name of format as shown in the file dialog.
        **extension** : string, The file extension used by the format.
        "**toString** : function(map : :ref:`script-map`, fileName : string) : string", "A function
        that returns the string representation of the given map, when
        saved to the given file name (useful for relative references)."

    Example that produces a simple JSON representation of a map:

    .. code:: javascript

        var customMapFormat = {
            name: "Custom map format",
            extension: "custom",

            toString: function(map, fileName) {
                var m = {
                    width: map.width,
                    height: map.height,
                    layers: []
                };

                for (var i = 0; i < map.layerCount; ++i) {
                    var layer = map.layerAt(i);
                    if (layer.isTileLayer) {
                        var rows = [];
                        for (y = 0; y < layer.height; ++y) {
                            var row = [];
                            for (x = 0; x < layer.width; ++x)
                                row.push(layer.cellAt(x, y).tileId);
                            rows.push(row);
                        }
                        m.layers.push(rows);
                    }
                }

                return JSON.stringify(m);
            },
        }

        tiled.registerMapFormat("custom", customMapFormat)


.. _script-tiled-signals:

Signals
~~~~~~~

tiled.assetCreated(asset : :ref:`script-asset`)
    A new asset has been created.

tiled.assetOpened(asset : :ref:`script-asset`)
    An asset has been opened.

tiled.assetAboutToBeSaved(asset : :ref:`script-asset`)
    An asset is about to be saved. Can be used to make last-minute changes.

tiled.assetSaved(asset : :ref:`script-asset`)
    An asset has been saved.

tiled.assetAboutToBeClosed(asset : :ref:`script-asset`)
    An asset is about to be closed.

tiled.activeAssetChanged(asset : :ref:`script-asset`)
    The currently active asset has changed.


.. _script-object:

Object
^^^^^^

The base of most data types in Tiled. Provides the ability to associate custom
properties with the data.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **asset** : :ref:`script-asset` |ro|, "The asset this object is part of, or ``null``."
    **readOnly** : bool |ro|, Whether the object is read-only.

Functions
~~~~~~~~~

.. _script-object-property:

Object.property(name : string) : variant
    Returns the value of the custom property with the given name, or
    ``undefined`` if no such property is set on the object.

    *Note:* Currently it is not possible to inspect the value of ``file`` properties.

.. _script-object-setProperty:

Object.setProperty(name : string, value : variant) : void
    Sets the value of the custom property with the given name. Supported types
    are ``bool``, ``number`` and ``string``. When setting a ``number``, the
    property type will be set to either ``int`` or ``float``, depending on
    whether it is a whole number.

    *Note:* Support for ``color`` and ``file`` properties is currently missing.

.. _script-object-properties:

Object.properties() : object
    Returns all custom properties set on this object. Modifications to the
    properties will not affect the original object.

.. _script-object-setProperties:

Object.setProperties(properties : object) : void
    Replaces all currently set custom properties with a new set of properties.

.. _script-object-removeProperty:

Object.removeProperty(name : string) : void
    Removes the custom property with the given name.

.. _script-asset:

Asset
^^^^^

Inherits :ref:`script-object`.

Represents any top-level data type that can be saved to a file. Currently
either a :ref:`script-map` or a :ref:`script-tileset`.

All modifications made to assets and their contained parts create undo
commands. This includes both modifying functions that are called as well as
simply assigning to a writable property.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **fileName** : string |ro|, File name of the asset.
    **modified** : bool |ro|, Whether the asset was modified after it was saved or loaded.

.. _script-map:

TileMap
^^^^^^^

Inherits :ref:`script-asset`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **width** : int |ro|, Width of the map in tiles (only relevant for non-infinite maps). Use :ref:`resize <script-map-resize>` to change it.
    **height** : int |ro|, Height of the map in tiles (only relevant for non-infinite maps). Use :ref:`resize <script-map-resize>` to change it.
    **size** : size |ro|, Size of the map in tiles (only relevant for non-infinite maps). Use :ref:`resize <script-map-resize>` to change it.
    **tileWidth** : int, Tile width (used by tile layers).
    **tileHeight**: int, Tile height (used by tile layers).
    **infinite** : bool, Whether this map is infinite.
    **hexSideLength** : int, Length of the side of a hexagonal tile (used by tile layers on hexagonal maps).
    **staggerAxis** : int, "For staggered and hexagonal maps, determines which axis (X or Y) is staggered: 0 (X), 1 (Y)."
    **orientation** : int, "General map orientation: 0 (Unknown), 1 (Orthogonal), 2 (Isometric), 3 (Staggered), 4 (Hexagonal)"
    **renderOrder** : int, "Tile rendering order (only implemented for orthogonal maps): 0 (RightDown), 1 (RightUp), 2 (LeftDown), 3 (LeftUp)"
    **staggerIndex** : int, "For staggered and hexagonal maps, determines whether the even or odd indexes along the staggered axis are shifted. 0 (Odd), 1 (Even)."
    **backgroundColor** : color, Background color of the map.
    **layerDataFormat** : int, "The format in which the layer data is stored, taken into account by TMX, JSON and Lua map formats: 0 (XML), 1 (Base64), 2 (Base64Gzip), 3 (Base64Zlib), 4 (CSV)"
    **selectedArea** : :ref:`SelectionArea <script-selectedarea>`, The selected area of tiles.
    **layerCount** : int |ro|, Number of top-level layers the map has.

Functions
~~~~~~~~~

new TileMap()
    Constructs a new map.

.. _script-map-layerAt:

TileMap.layerAt(index : int) : :ref:`script-layer`
    Returns a reference to the top-level layer at the given index. When the
    layer gets removed from the map, the reference changes to a standalone
    copy of the layer.

.. _script-map-removeLayerAt:

TileMap.removeLayerAt(index : int) : void
    Removes the top-level layer at the given index. When a reference to the
    layer still exists, that reference becomes a standalone copy of the layer.

.. _script-map-removeLayer:

TileMap.removeLayer(layer : :ref:`script-layer`) : void
    Removes the given layer from the map. The reference to the layer becomes
    a standalone copy.

.. _script-map-insertLayerAt:

TileMap.insertLayerAt(index : int, layer : :ref:`script-layer`) : void
    Inserts the layer at the given index. The layer can't already be part of
    a map.

.. _script-map-addLayer:

TileMap.addLayer(layer : :ref:`script-layer`) : void
    Adds the layer to the map, above all existing layers. The layer can't
    already be part of a map.

.. _script-map-resize:

TileMap.resize(size : size [, offset : point [, removeObjects : bool = false]]) : void
    Resizes the map to the given size, optionally applying an offset (in tiles)

.. _script-layer:

Layer
^^^^^

Inherits :ref:`script-object`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **name** : string, Name of the layer.
    **opacity** : number, "Opacity of the layer, from 0 (fully transparent) to 1 (fully opaque)."
    **visible** : bool, Whether the layer is visible (affects child layer visibility for group layers).
    **locked** : bool, Whether the layer is locked (affects whether child layers are locked for group layers).
    **offset** : point, Offset in pixels that is applied when this layer is rendered.
    **map** : :ref:`script-map`, Map that this layer is part of (or ``null`` in case of a standalone layer).
    **isTileLayer** : bool |ro|, Whether this layer is a :ref:`script-tilelayer`.
    **isObjectGroup** : bool |ro|, Whether this layer is an :ref:`script-objectgroup`.
    **isGroupLayer** : bool |ro|, Whether this layer is a group layer.
    **isImageLayer** : bool |ro|, Whether this layer is an image layer.

.. _script-tilelayer:

TileLayer
^^^^^^^^^

Inherits :ref:`script-layer`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **width** : int |ro|, Width of the layer in tiles (only relevant for non-infinite maps).
    **height** : int |ro|, Height of the layer in tiles (only relevant for non-infinite maps).
    **size** : size |ro|, Size of the layer in tiles (has ``width`` and ``height`` members) (only relevant for non-infinite maps).

Functions
~~~~~~~~~

new TileLayer([name : string])
    Constructs a new tile layer, which can be added to a :ref:`script-map`.

TileLayer.region() : region
    Returns the region of the layer that is covered with tiles.

TileLayer.cellAt(x : int, y : int) : cell
    Returns the value of the cell at the given position.

.. _script-objectgroup:

ObjectGroup
^^^^^^^^^^^

Inherits :ref:`script-layer`.

The "ObjectGroup" is a type of layer that can contain objects. It will
henceforth be referred to as a layer.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **objectCount** : int |ro|, Number of objects on this layer.
    **color** : color, Color of shape and point objects on this layer (when not set by object type).

Functions
~~~~~~~~~

new ObjectGroup([name : string])
    Constructs a new object layer, which can be added to a :ref:`script-map`.

ObjectGroup.objectAt(index : int) : :ref:`script-mapobject`
    Returns a reference to the object at the given index. When the object is
    removed, the reference turns into a standalone copy of the object.

ObjectGroup.removeObjectAt(index : int) : void
    Removes the object at the given index.

ObjectGroup.removeObject(object : :ref:`script-mapobject`) : void
    Removes the given object from this layer. The object reference turns into
    a standalone copy of the object.

ObjectGroup.insertObjectAt(index : int, object : :ref:`script-mapobject`) : void
    Inserts the object at the given index. The object can't already be part
    of a layer.

ObjectGroup.addObject(object : :ref:`script-mapobject`) : void
    Adds the given object to the layer. The object can't already be part of
    a layer.

.. _script-mapobject:

MapObject
^^^^^^^^^

Inherits :ref:`script-object`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **id** : int |ro|, Unique (map-wide) ID of the object.
    **name** : string, Name of the object.
    **type** : string, Type of the object.
    **x** : number, X coordinate of the object in pixels.
    **y** : number, Y coordinate of the object in pixels.
    **pos** : point, Position of the object in pixels (has ``x`` and ``y`` members).
    **width** : number, Width of the object in pixels.
    **height** : number, Height of the object in pixels.
    **size** : size, Size of the object in pixels (has ``width`` and ``height`` members).
    **rotation** : number, Rotation of the object in degrees clockwise.
    **visible** : bool, Whether the object is visible.
    **layer** : :ref:`script-objectgroup` |ro|, Layer this object is part of (or ``null`` in case of a standalone object).
    **map** : :ref:`script-map` |ro|, Map this object is part of (or ``null`` in case of a standalone object).

Functions
~~~~~~~~~

new MapObject([name : string])
    Constructs a new map object, which can be added to an :ref:`script-objectgroup`.

.. _script-tileset:

Tileset
^^^^^^^

Inherits :ref:`script-asset`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **name** : string, Name of the tileset.
    **tileWidth** : int |ro|, Tile width for tiles in this tileset in pixels.
    **tileHeight** : int |ro|, Tile Height for tiles in this tileset in pixels.
    **tileSize** : size |ro|, Tile size for tiles in this tileset in pixels (has ``width`` and ``height`` members).
    **tileSpacing** : int |ro|, Spacing between tiles in this tileset in pixels.
    **margin** : int |ro|, Margin around the tileset in pixels (only used at the top and left sides of the tileset image).
    **tileOffset** : point, Offset in pixels that is applied when tiles from this tileset are rendered.
    **backgroundColor** : color, Background color for this tileset in the *Tilesets* view.

Functions
~~~~~~~~~

new Tileset([name : string])
    Constructs a new tileset.

Tileset.tile(id : int) : :ref:`script-tile`
    Returns a reference to the tile with the given ID. Raises an error if no
    such tile exists. When the tile gets removed from the tileset, the
    reference changes to a standalone copy of the tile.

Tileset.tiles() : [:ref:`script-tile`]
    Returns an array containing all tiles in the tileset.

.. _script-tile:

Tile
^^^^

Inherits :ref:`script-object`.

Properties
~~~~~~~~~~

.. csv-table::
    :widths: 1, 2

    **id** : int |ro|, ID of this tile within its tileset.
    **width** : int |ro|, Width of the tile in pixels.
    **height** : int |ro|, Height of the tile in pixels.
    **size** : size |ro|, Size of the tile in pixels (has ``width`` and ``height`` members).
    **type** : string, Type of the tile.
    **probability** : number, Probability that the tile gets chosen relative to other tiles.

.. _script-selectedarea:

SelectedArea
^^^^^^^^^^^^

Functions
~~~~~~~~~

SelectedArea.set(rect : rect) : void
    Sets the selected area to the given rectangle.

SelectedArea.set(region : region) : void
    Sets the selected area to the given region.

SelectedArea.add(rect : rect) : void
    Adds the given rectangle to the selected area.

SelectedArea.add(region : region) : void
    Adds the given region to the selected area.

SelectedArea.subtract(rect : rect) : void
    Subtracts the given rectangle from the selected area.

SelectedArea.subtract(region : region) : void
    Subtracts the given region from the selected area.

SelectedArea.intersect(rect : rect) : void
    Sets the selected area to the intersection of the current selected area and the given rectangle.

SelectedArea.intersect(region : region) : void
    Sets the selected area to the intersection of the current selected area and the given region.


.. |ro| replace:: *[read-only]*
