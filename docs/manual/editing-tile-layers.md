# Editing Tile Layers

[Tile Layers](layers.md#tile-layers) are what makes Tiled a _tile map editor_.
Although not as flexible as [Object Layers](layers.md#object-layers), they
provide efficient data storage and good rendering performance as well as
efficient content creation. Every new map gets one by default, though feel free
to delete it when you're not going to use it.

## Stamp Brush

Shortcut: `B`

The primary tool for editing tile layers is the Stamp Brush. It can be used to
paint single tiles as well as larger "stamps", which is where it gets its name
from. Using the right mouse button, it can also quickly capture tile stamps
from the currently active layer. A tile stamp is commonly created by selecting
one or more tiles in the Tilesets view.

The Stamp Brush has some extra features:

* While holding `Shift`, click any two points to draw a line between them.

* While holding `Ctrl+Shift`, click any two points two draw a circle or ellipse
  centered on the first point.

* Activate the _Random Mode_ using the dice button on the tool bar to have the
  Stamp Brush paint with random tiles from the tile stamp. The probability of
  each tile depends on how often it occurred on the tile stamp, as well as
  the probability set on each tile in the _Tileset Editor_.

* In combination with the _Tile Stamps_ view, it can also place randomly from a
  set of predefined tile stamps. This can be more useful than the _Random Mode_,
  which randomly places individual tiles.

## Terrain Brush

Shortcut: `T`

The Terrain Brush allows for efficient editing with a certain type of
corner-based terrain transitions. Setting it up requires associating terrain
information with your tiles, which is described in detail in
[Using the Terrain Tool](using-the-terrain-tool.md).

Similarly to the [Stamp Brush](#stamp-brush), you can draw lines by holding
`Shift`. When holding `Ctrl`, the size of the editing area is reduced to one
corner (this currently doesn't work well in combination with drawing lines).

<div class="new">New in Tiled 1.0</div>

When holding `Alt`, the editing operations are also applied at a 180 degree
rotation. This is especially useful when editing strategic maps where two sides
need to have equal opportunities. The modifier works well in combination with
either `Shift` for drawing lines or `Ctrl` for reducing the edited area.

## Bucket Fill Tool

Shortcut: `F`

The Bucket Fill Tool provides a quick way of filling empty areas or areas
covered with the same tiles. The currently active tile stamp will be repeated in
the filled area. It can also be used in combination with the _Random Mode_.

When holding `Shift`, the tool fills the currently selected area regardless of
its contents. This is useful for filling custom areas that have been selected
previously using one or more [Selection Tools](#selection-tools).

## Eraser

Shortcut: `E`

A simple eraser tool. Left click erases single tiles and right click can be used
to quickly erase rectangular areas.

## Selection Tools

There are various tile selection tools that all work in similar fashion:

* **Rectangular Select** allows selection of rectangular areas (shortcut: `R`)

* **Magic Wand** allows selection of connected areas filled with the same
  tile (shortcut: `W`)

* **Select Same Tile** allows selection of same-tiles across the entire layer
  (shortcut: `S`)

By default, each of these tools replaces the currently selected area. The
following modifiers can be used to change this behavior:

* Holding `Shift` expands the current selection with the new area
* Holding `Ctrl` subtracts the new area from the current selection
* Holding `Ctrl` and `Shift` selects the intersection of the new area with the
  current selection

## Managing Tile Stamps

It can often be useful to store the current tile stamp somewhere to use it again
later. The following shortcuts work for this purpose:

* `Ctrl + 1-9` - Store current tile stamp (similar to `Ctrl + C`)
* `1-9` - Recall the stamp stored at this location (similar to `Ctrl + V`)

Tile stamps can also be stored by name and extended with variations using the
_Tile Stamps_ view.
