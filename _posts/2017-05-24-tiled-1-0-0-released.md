---
layout: post
title: Tiled 1.0 released
author: Thorbjørn Lindeijer
tags: release
---

At long last I'm proud to announce the release of
[Tiled 1.0](https://thorbjorn.itch.io/tiled). This release is based on a branch of
development that started 1.5 years ago, with the goal of better supporting the
use of external tileset files alongside map files. In addition I wanted to
resolve all the loose ends that made Tiled feel incomplete. Last but not least,
a huge amount of other welcome improvements were made by a record number of
contributors!

Here are some of the major new features in this release:

### Working with External Tilesets

This is a change that will likely affect your workflow, unless you stick to
using embedded tilesets (which are still supported). With the large amount of
meta-information stored in a tileset, external tilesets are a great way to share
this information across maps. Until now, these files could not be properly
edited in Tiled.

Now you can open and save tileset files independently of map files and they can
be modified the same way that embedded tilesets can. In addition, all open
external tilesets are now visible in the Tilesets view, and they will be added
to your map automatically when you start using them.

<a href="/img/posts/2017-05-editing-external-tileset.png" class="thumbnail" style="margin: 10px;">
  <img src="/img/posts/2017-05-editing-external-tileset.png">
</a>

Visible changes include the tab bar being now on top of the tool bar, since
different tool bar actions and views are used when editing a tileset as opposed
to when editing a map. Also, in the tileset editor you'll now find that the
terrain information editor and tile collision editor are integrated as dockable
views.

<a href="/img/posts/2017-05-layer-hierarchy.png" class="thumbnail" style="margin: 0px 0px 10px 10px; float: right;">
  <img src="/img/posts/2017-05-layer-hierarchy.png" width="208" height="305">
</a>

### Layer Hierarchy

For those of you working with a large amount of map layers, there is now the
option to organize layers in a [hierarchy of groups](http://doc.mapeditor.org/manual/layers/#group-layers).
Properties like the opacity and visibility will apply recursively to all layers
in a group.

In addition, layers can now be reordered by using drag-n-drop.

### Typed Tiles and Tile Property Inheritance

Using tile objects to place entities on the map is a common use-case. Now, you
can also [associate a type with your tiles](http://doc.mapeditor.org/manual/custom-properties/#typed-tiles),
so that predefined custom properties are visible when selecting the tile or a tile object.
Basic custom properties added to a tile are also
[inherited by tile objects](http://doc.mapeditor.org/manual/custom-properties/#tile-property-inheritance).
These features can be a huge time-safer since you often no longer need to manually
fill in the object type or manually add the custom properties for each instance.

Also, cut/copy/paste actions for custom properties have been implemented.

### Text Objects

You can now [insert text objects](http://doc.mapeditor.org/manual/objects/#insert-text)
in your maps, either for visual or for annotation purposes. For each text
object, you can control its font, color, alignment and word wrapping properties.

<a href="/img/posts/2017-05-text-objects.png" class="thumbnail" style="margin: 10px;">
  <img src="/img/posts/2017-05-text-objects.png">
</a>

Apart from this new object type, other improvements have been made to handling
objects, including an improved selection rectangle, the ability to
[select objects below other objects](http://doc.mapeditor.org/manual/objects/#selecting-and-deselecting)
and synchronizing the current layer to the currently selected object.

### Fixes for Hexagonal Maps

The support for hexagonal and staggered maps has greatly improved. The fill tool
now works properly for these maps, the stamp brush no longer acts weird due to
the staggering and finally hexagonal rotation (in 60-degree increments) was
added.


### Changelog

Many more improvements were made, here is the full list:

* Added support for editing external tilesets ([#242](https://github.com/bjorn/tiled/issues/242))
* Added a text object with configurable font and wrapping ([#1429](https://github.com/bjorn/tiled/issues/1429))
* Added layer grouping ([#1038](https://github.com/bjorn/tiled/issues/1038))
* Added Tile.type and inherit tile object properties from the tile ([#436](https://github.com/bjorn/tiled/issues/436), [#1248](https://github.com/bjorn/tiled/issues/1248))
* Added a start page
* Added selection of underlying objects with Alt modifier (by Yuriy, [#1491](https://github.com/bjorn/tiled/issues/1491))
* Added an option to disable safe writing of files ([#1402](https://github.com/bjorn/tiled/issues/1402), [#1404](https://github.com/bjorn/tiled/issues/1404))
* Added invert selection action (by Leon Moctezuma, [#1423](https://github.com/bjorn/tiled/issues/1423))
* Added support for isometric terrain overlays and tile collision objects ([#419](https://github.com/bjorn/tiled/issues/419), [#757](https://github.com/bjorn/tiled/issues/757))
* Added 180-degree symmetry mode to terrain brush with Alt modifier
* Added short and consistent map format names to use with \--export-map (by Marce Coll, [#1382](https://github.com/bjorn/tiled/issues/1382))
* Added Swap Tiles action (by Alexander Münch, [#866](https://github.com/bjorn/tiled/issues/866))
* Added tileset background color property ([#227](https://github.com/bjorn/tiled/issues/227))
* Added 60 degree tile rotation support for hexagonal maps (by Victor Nicolaichuk, [#1447](https://github.com/bjorn/tiled/issues/1447))
* Added a check for duplicates when adding tiles (by Simião, [#1227](https://github.com/bjorn/tiled/issues/1227))
* Added option to run commands from menu as well as edit them (by Ketan Gupta, [#943](https://github.com/bjorn/tiled/issues/943))
* Added custom shortcuts for commands (by Ketan Gupta, [#1456](https://github.com/bjorn/tiled/issues/1456))
* Added optional ID and Position columns to objects view (by i-ka, [#1462](https://github.com/bjorn/tiled/issues/1462))
* Added an executable picker for custom commands (by Ketan Gupta, [#942](https://github.com/bjorn/tiled/issues/942))
* Added marching ants effect on selected objects (by Mohamed Thabet, [#1489](https://github.com/bjorn/tiled/issues/1489))
* Added all open tilesets to the Tilesets view
* Added auto-show/hide all views (Clear View) action (by erem2k, [#563](https://github.com/bjorn/tiled/issues/563))
* Added minimap in the resizing dialog (by Yuriy, [#1516](https://github.com/bjorn/tiled/issues/1516))
* Added drag-n-drop support in Layers view ([#178](https://github.com/bjorn/tiled/issues/178))
* Added support for storing object type definitions in JSON format ([#1313](https://github.com/bjorn/tiled/issues/1313))
* Added cut/copy/paste actions for custom properties ([#515](https://github.com/bjorn/tiled/issues/515))
* Allow changing the tile of tile objects (by Mohamed Thabet, [#409](https://github.com/bjorn/tiled/issues/409))
* Allow selecting a folder to fix multiple broken links at once
* Added support for dragging external tilesets into the Tilesets dock
* Added support for dragging images into image collection tilesets
* Write out Tiled version in TMX/JSON "version" attribute ([#1416](https://github.com/bjorn/tiled/issues/1416))
* Remember last view on map also for closed files ([#905](https://github.com/bjorn/tiled/issues/905))
* Remember tileset zoom level in the tileset editor (by Artem Sharganov, [#408](https://github.com/bjorn/tiled/issues/408))
* Change current layer depending on selected objects (by Glavak, [#1424](https://github.com/bjorn/tiled/issues/1424))
* Improved support for using Tiled on HiDpi screens
* Improved the behavior of the tile selection tool
* Made Ctrl+D duplicate objects instead of deleting them
* Use an eye icon instead of a checkbox for layer visibility (by Ketan Gupta, [#1127](https://github.com/bjorn/tiled/issues/1127))
* JSON tileset: Save width/height of individual tile images
* Linux: Added MIME type for tileset files
* Fixed hexagonal rotation of tile stamps (by Bdtrotte, [#1476](https://github.com/bjorn/tiled/issues/1476))
* Fixed handling of broken tile references, which now render as a red marker
* Fixed manual reloading of images for image collection tilesets
* Fixed Offset Layers tool to wait until mouse is moved
* Fixed current stamp to always update when a tile is clicked
* Fixed handling of pinch gestures ([#1305](https://github.com/bjorn/tiled/issues/1305))
* Fixed flipping a group of objects to work like expected (by Vitek1425, [#1475](https://github.com/bjorn/tiled/issues/1475))
* Fixed stamp brush to work better on staggered maps (by Bdtrotte)
* Fixed objects offsetting while resizing (by Acuion, [#1518](https://github.com/bjorn/tiled/issues/1518))
* Fixed fill tool for hexagonal maps ([#883](https://github.com/bjorn/tiled/issues/883))
* Fixed potential crash in Terrain Brush
* Windows: Fixed menus when using OpenGL in full screen mode ([#1576](https://github.com/bjorn/tiled/issues/1576))
* Windows: Added Sticker Knight and Python example scripts to installer ([#819](https://github.com/bjorn/tiled/issues/819))
* Windows: Fixed bringing existing Tiled window to foreground ([#1256](https://github.com/bjorn/tiled/issues/1256))
* AutoMapping: Fixed object groups always getting added
* AutoMapping: Improved map boundary handling (by Stefan Beller, [#1224](https://github.com/bjorn/tiled/issues/1224))
* AutoMapping: Apply custom properties set on output layers
* terraingenerator: Made the amount of columns configurable
* terraingenerator: Copy tile properties from the source tilesets
* Added Ukrainian translation (by Olexandr Nesterenko)
* Added Hungarian translation (by Balázs Úr)
* Added Finnish translation (by ekeimaja)
* Updated Bulgarian, Dutch, French, German, Russian, Spanish and Turkish translations

Thanks to everybody who contributed to this release with bug reports, suggestions or patches!

## A Look Ahead

With Tiled 1.0 out, I aim to focus a bit of time on completing the
[User Manual](http://doc.mapeditor.org) and also documenting the level of Tiled
map support by the various
[libraries and frameworks](http://doc.mapeditor.org/reference/support-for-tmx-maps/)
listed there.

Of course, feature development will continue as usual as well! On May 30th
[our Google Summer of Code students]({{ site.baseurl }}{% post_url 2017-05-05-announcing-our-google-summer-of-code-students %})
will officially start with their projects, which will likely form the basis
for Tiled 1.1.

## Support Tiled Development

Getting to this major release was only possible thanks to
[over 190 patrons](https://www.patreon.com/bjorn) supporting me on a monthly
basis as well as many people choosing to pay for
[Tiled on itch.io](https://thorbjorn.itch.io/tiled). As of this writing I still
need more support to reach my two-days-per-week funding goal.
[Please chip in](https://www.patreon.com/bePatron?u=90066) if
you can to ensure Tiled keeps getting better! <img src="https://cdn-standard.discourse.org/images/emoji/apple/heart.png?v=3" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">
