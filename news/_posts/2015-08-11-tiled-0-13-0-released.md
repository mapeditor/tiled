---
layout: post
title: Tiled 0.13.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 708
---

I'm happy to announce that Tiled 0.13 is [now available][1]!

Since I can [spend each Monday fully on Tiled][2], working on new features is much easier and more enjoyable. It also leads to a faster release cycle, though another factor is that I've decided to focus on very few major features at a time. I'll highlight some of the more noticeable changes.

### Persistent Tile Stamps

The main new feature I've been working on was to add support for persistent tile stamps. The *Stamp Brush* and *Bucket Fill Tool* both work with tile stamps, and until now it was not easily possible to save a stamp and use it again later. There was a "quick stamp" feature where you could store the stamp under a number key when holding Ctrl, but it was not persistent and limited to 9 slots.

A new "Tile Stamps" view has been added where you can save an unlimited number of stamps. The quick stamp feature still works (and is now persistent), but you can also add arbitrary named stamps. It also displays a small preview of each stamp to make them easier to find back. You can choose the folder it uses to store the stamps, for example to use different sets for different projects or to share them with team members.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/facf3ec11ec946c46802778f0b2a40c49cd1c762.png" width="690" height="326">

In addition, tile stamps can have variations. Each time a stamp is used, a random variation will be chosen based on the probability. A random mode was available before, but it only worked for single-tile randomness. Stamp variations do the same thing, but also work for multi-tile pieces.


### Select Same Tile Tool

A new contributor, [Mamed Ibrahimov][3], has made several nice improvements and the Select Same Tile tool is one of them. It allows to quickly select all occurrences of the same tile, for example to then use the fill tool to replace them with something else (holding Shift to make it apply to the whole selection).

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/34d71f3a1309fbb19c48fc0843254ff654e0e65b.png" width="575" height="41">

### Object Selection Improvements

The object selection outlines are now always drawn on top, rather than being overlapped by other objects. Also, improvements were made to the mouse cursor showing when you can drag an object around. Finally, newly added objects are now automatically selected.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/b8d8b42efd651c865afb98c45126304a5ebb729d.png" width="690" height="191">

These are all more like bug fixes of course, but it wasn't until recently that I would actually have the time to care about these details.

### File Format Changes

The Lua and JSON map formats now also support tile layer data compression. Previously, they always used the native array notation. Using zlib compression (the default for new maps) generally yields much smaller file sizes. You can change the layer data format in the *Map Properties*.

The TMX, JSON and Lua map formats now feature a `tilecount` property for tilesets. This can be helpful when determining how much memory should be allocated.

Thanks to Alejandro Cámara, the CSV (comma-separated values) export plugin now supports exporting maps containing multiple tile layers, creating a numbered file for each layer.

### Change log

Many more changes were made, so here's a more complete list.

* Added persistent Tile Stamps with support for variations ([#969][4])
* Added Select Same Tile tool (by Mamed Ibrahimov)
* Added option to disable opening of last files on startup (by Mamed Ibrahimov)
* Added tilecount property to TMX, JSON and Lua map formats ([#806][5])
* Added tileset properties to Properties view, as read-only (by Mamed Ibrahimov)
* Added Save All action (by Mamed Ibrahimov)
* Added translation of command line messages (by Mamed Ibrahimov)
* Added menu item linking to online documentation
* Object selection outlines are now drawn on top of everything
* Select new objects after they have been created
* Made the starting point for polylines and polygons visible
* Use the tile probability property also in random mode
* Ungrouped position and size properties ([#892][6])
* CSV plugin: Extended to export all tile layers (by Alejandro Cámara)
* Lua and JSON plugins: Added support for layer data compression
* Fixed crash when changing flipping flag for multiple objects (by Mamed Ibrahimov)
* Fixed Ctrl+T causing a crash when no maps are open
* Fixed availability of 'Execute in Terminal' command on Linux with Qt 5
* Fixed drag object mouse cursor to appear only when it should
* Fixed selected file format when doing Save As with a non-TMX map
* Fixed problems with infinate scaling factors when resizing objects
* Require at least Qt 5.1.0
* Require compiler support for C++11
* Updated Russian, German, Czech and Italian translations

Many thanks to all who contributed!

### A Look Ahead to Tiled 0.14

Check out the [Tiled roadmap][7] board on Trello for an outlook to future releases. You can try to influence the planning by voting on issues, or reminding me about things you consider missing from the roadmap.

If you want to help shape the next release, please give feedback, contribute patches or make translation updates. To help me get through the roadmap faster, please consider making [a small monthly donation][9] through Patreon. Thanks!


  [1]: http://www.mapeditor.org/download
  [2]: https://www.patreon.com/bjorn
  [3]: https://github.com/IMMZ
  [4]: https://github.com/bjorn/tiled/issues/969
  [5]: https://github.com/bjorn/tiled/issues/806
  [6]: https://github.com/bjorn/tiled/issues/892
  [7]: https://trello.com/b/yl3PAtN0/tiled-roadmap
  [8]: http://doc.mapeditor.org
  [9]: https://www.patreon.com/bjorn
