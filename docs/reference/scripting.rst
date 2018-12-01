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

The currently selected asset (map or tileset), or ``null`` if no file is
open.

The returned value is an instance of a class that provides read and
write access to the properties of the asset. Changing a property will
create an undo command for that action.

**tiled.openAssets**
~~~~~~~~~~~~~~~~~~~~

The list of currently opened assets (can contain maps and tilesets).

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
