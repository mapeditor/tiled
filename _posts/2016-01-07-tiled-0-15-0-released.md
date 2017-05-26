---
layout: post
title: Tiled 0.15.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1001
---

[Tiled 0.15.0](http://www.mapeditor.org/download) is out, so let's talk about what's new!

### Changing and Correcting External Links

Those who remember the [Tiled 0.14 release announcement](http://forum.mapeditor.org/t/tiled-0-14-0-released/769) may remember that I wanted the next release to be Tiled 1.0. We're not entirely there yet, but Tiled took a big step towards being more functionally complete.

The most important changes regarding this made sure that you can now load maps that refer to files that are for whatever reason no longer accessible. Previously, you'd then have to fall back to correcting the map file by hand. Now Tiled will tell you about the problems it found and allows you to fix them:

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/58f47c359c2f215c470bce39932e3012fe6dd8c6.png" width="690" height="406">

In addition, you can change any of these references through the Properties view after clicking the relevant items. And when changing a tileset image, you can also change its parameters like tile size, margin and spacing.

### Enable/Disable Plugins

Another important bit of functionality is that you can now choose which plugins are enabled. By default, only the generic plugins for exporting to Lua, JSON, CSV and for enabling Python import/export scripts are loaded. The project-specific plugins were often leading to confusion and now need to be explicitly enabled.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/859120859f5d70326b405d168fa31e53630a68fd.png" width="427" height="437">

Plugins can be enabled and disabled without restarting Tiled.

### Layer Combo Box

The status bar got a little more useful, since it now allows you to quickly switch the current layer. If you're not actively changing around your layer stack, this can entirely replace the Layers view:

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/7fd9602e6569e374f02d70bb2188fc0799feef75.png" width="690" height="80">

### Other Noteworthy Things

If you're using an image collection tileset, you can now choose how many tile columns it should have. Eventually I'd also like to add a dynamic wrapping display mode, but this should help in the meantime.

A Terrain Generator tool was added, which helps a lot with generating [a certain type of terrain tileset](https://github.com/tales/sourceoftales/blob/c04f712dfe4dde539e37f583b0f297c68c03ff83/tiles/terrain.png). But, I still need to write the usage instructions and I forgot to include it in the binary packages.

You can now go past the edges of the map when panning with the space bar, middle mouse button or the mini-map. This can be really helpful when you're editing things on the edge of the map.

### Change log

* Allow loading maps with broken external references
* Allow plugins to be enabled/disabled
* Allow changing tileset image parameters
* Allow changing the images of tiles in a collection tileset
* Allow changing external tileset references
* Allow panning over the edges of the map
* Added Terrain Generator tool
* Added column count property to image collection tilesets
* Added a combo box for changing the current layer to the status bar
* Moved the AutoMapping while drawing toggle into the menu
* Removing tiles from collection tilesets no longer changes tile IDs
* Unified layer offset handling
* Default tile layer data format changed to CSV
* Deprecated pure XML and Gzip-compressed tile layer data formats
* Fixed random tile picker for tiles with zero probability (by Henrik Heino)
* Fixed saving of alpha value of the map background color
* Fixed crash in tmxrasterizer and tmxviewer
* Fixed tmxrasterizer not reporting write errors
* Fixed isometric rendering bug with odd tile heights (by Ryan Schmitt)
* Updated Bulgarian, Dutch, French, German, Japanese, Russian and Spanish translations

Many thanks to all who contributed!

### A Look Ahead

With many of the [issues on the Tiled 1.0 milestone](https://github.com/bjorn/tiled/milestones/Tiled%201.0) resolved, I am confident that the next feature release will be 1.0. The most difficult issue still on the list there is to support editing of external tilesets, which is currently not possible due to the way Tiled is internally focused around map files.

Part of reaching 1.0 is also to finally get around to writing a manual, which is something I've long wanted to get around to, but for some reason improving Tiled itself has always felt more important.

Finally, it means to reach a stable basis upon which more cool functionality can be implemented. I want to add support for projects, simplifying tileset management, defining custom object types, make it possible to extend/automate Tiled with scripts, and many other things.

### Support Tiled Development <img src="https://cdn-standard.discourse.org/images/emoji/apple/heart.png?v=3" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

Thanks to the support from [over 120 patrons](https://www.patreon.com/bjorn?ty=h), [3 sponsors](http://www.mapeditor.org/) and the occasional donation, I can currently afford to spend little over a day/week on Tiled development. If you're enjoying Tiled, please consider to support me with [a small recurring donation](https://www.patreon.com/bePatron?u=90066) to help things go much faster!
