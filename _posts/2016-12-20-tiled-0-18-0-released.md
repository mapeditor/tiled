---
layout: post
title: Tiled 0.18.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1897
---

Before merging the major changes scheduled for Tiled 1.0, it was time to release [Tiled 0.18](https://thorbjorn.itch.io/tiled), bringing a host of small enhancements!

## New Convenience Actions

Actions were added to create a new layer based on the current selection, either through copying or moving (_Layer > New > Layer via Cut/Copy_). In addition, the _Paste in Place_ action now works for tile layers, pasting a section of tiles at exactly the same spot as it was copied from.

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/85ec5a011d892ccdf0d833be9a42d1c552c4df12.png" width="690" height="191">

A new context menu for custom properties includes actions to change the type of the custom property to another, compatible type.

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/337ef259560eab69e722c5cec4566ff79c5c6787.png" width="620" height="160">

A context menu action was added to reset the size of tile objects. Useful when you scaled them by accident, or when you want to adjust them after the size of the tile image changed.

Also convenient should be that the mouse wheel can now also scroll past the map boundaries and the added _Snap to Pixels_ mode.

## Reordering of Objects

While it was already possible to manually change the drawing order of objects for a long time (by setting the _Drawing Order_ of their object layer to _Manual_ and using the object context menu), now there are also buttons for this in the Objects view, regardless of the _Drawing Order_ setting.

The display order of the objects in the Objects view is now reversed, to match the ordering of the layers. This especially makes sense when using manual drawing order, because now the top-most (last drawn) object will be at the top of the list.

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/41908d9743082eb715f9e7361461bae811cf5c4a.png" width="690" height="265">

The new [Sticker Knight example](https://github.com/bjorn/tiled/tree/master/examples/sticker-knight) demonstrates how Tiled can be used to build levels without using any tile layer.

## GameMaker: Studio export plugin

It is now possible to export maps to [GameMaker: Studio 1.4](https://www.yoyogames.com/gamemaker/features), with support for both tile and object layers (objects get translated to instances).

## Map Thumbnails on Linux

The `tmxrasterizer` tool shipping with Tiled was improved a bit and is now set up as a thumbnailer for TMX files. When Tiled is installed through the system's package management, this enables thumbnails in supporting file managers like GNOME's Nautilus and PCManFM.

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/6dba9d0cd1ea1ebc5d175dc376e8a650000a9764.png" width="690" height="348">

(the maps shown here are from [The Mana World](https://www.themanaworld.org/))

## Change log

Many smaller changes have been made as well. Here's the full list:

* Added Layer via Copy/Cut actions
* Added support for Paste in Place action for tile layers
* Added context menu to change custom property type (by Dmitry Hrabrov)
* Added support for higher precision for custom floating point properties
* Added %mappath variable to commands (by Jack Roper)
* Added snapping to pixels (by Mamed Ibrahimov)
* Added right-click to clear the tile selection
* Added a context menu action to reset the size of tile objects
* Added exporter for Game Maker Studio room files (by Jones Blunt)
* Added Move Up/Down buttons to Objects view (by iskolbin)
* Added pixel coordinates to status bar for object tools (by iskolbin)
* Added Sticker Knight platformer example (by Ponywolf)
* tmxrasterizer: Added --size argument and support local file URLs
* tmxrasterizer: Use smooth pixmap transform by default
* Linux: Register tmxrasterizer as thumbnail generator for TMX files
* Allow scrolling past map edges with mouse wheel
* Enabled HiDpi scaling and improved the quality of some icons
* Reversed the order of the objects in the Objects view
* JSON plugin: Added Node.js support to the JavaScript export
* Updated TMX schema definition (by assofohdz)
* Fixed unfinished objects getting saved
* Fixed OpenGL rendering mode when application is scaled (HiDpi screens)
* Fixed Remove and Rename actions for predefined properties
* Windows: Fixed console output
* libtiled-java: Use Maven, deploy to OSSRH and code updates (by Mike Thomas)
* libtiled-java: Added a basic isometric renderer (by Mike Thomas)
* Updated Brazilian Portuguese, Chinese, Czech, Dutch, Hebrew, Norwegian Bokmål and Spanish translations


## A Look Ahead

In the meantime, a major change has been made to Tiled on the `master` branch to allow external tileset files to be opened and edited alongside map files. This was the biggest task remaining for Tiled 1.0. You can already try out the new Tiled with rearranged interface by installing a development snapshot. I can use all the feedback you have to make sure Tiled 1.0 will be a great release!

## Support Tiled Development :heart:

We can enjoy this new release and look forward to Tiled 1.0 thanks to [170 patrons supporting me on a monthly basis](https://www.patreon.com/bjorn) as well as many people choosing to [pay for Tiled on itch.io](https://thorbjorn.itch.io/tiled/purchase). I'm close to reaching my funding goal for spending _two full days_ per week on Tiled, but I still need your help to get there!
