---
layout: post
title: Tiled 0.14.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 769
---

[Tiled 0.14][1] is out now, with several nice new features and enhancements! Here are the highlights, followed by the full change log below.

### Layer Offsets

This feature was [requested a long time ago][2] and came up many times since then. Now, each layer can be set at a custom offset, which primarily enables emulating height and stacking of tiles.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/ec895e9061c89ccc120c63a7c7e3e412d23ed277.png" width="690" height="295">

<a class="attachment" href="http://discourse.mapeditor.org/uploads/default/original/1X/4a03b7a3566038a78838f0c8a6f3e7c72772d4fb.webm">Tiled-Layer-Offset.webm</a> provides a short video of editing with this feature.

The graphics I've used for these examples are [by Yar][3] (CC-BY 3.0).

### Custom Tileset Formats

While it was always possible for plugins to add custom map formats, the same wasn't true for tileset formats. This was especially unfortunate for those wanting to use external tilesets in combination with the JSON map format, which necessarily always embedded the tileset in the map file. Now, the JSON plugin adds a JSON tileset format and will no longer embed tileset information in the map file for external tilesets.

### New Object Name Labels

In [Tiled 0.12][4] the object name labels went missing since they interfered with the resizing improvements and did not work well with rotation either. They also did not display for polygons and tile objects. Now, the object names labels are back:

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/a6a31e8222b41a88df3b840c3ade5f1d9c65c56b.png" width="690" height="236">

You can choose to disable them, to have them only for the selected objects or to show them always. They won't be elided, scaled or rotated like the old labels, making sure they are always readable.

In the future I'd like to make it possible to configure what it shows on the label, but for now it will just show the name of the object.

### Efficiency

The selection of the tile palette now synchronizes when tiles are captured from the map. This also provides quick access to tile properties by right-clicking them on the map. Similarly, when selecting a tile object (on an object layer), the tile animation and collision editors will update to show the associated tile.

The active brush is no longer reset when switching between maps, so the right-click capture feature of the stamp brush is now also a quick way to copy/paste an area from one map to another.

### Change log

* Added support for custom external tileset formats (JSON format added)
* Added support for shifting layers by some distance in pixels
* Added back object name labels in a much improved form
* Added tile stamp variation support to the fill tool
* Synchronize tileset selection when capturing tiles from the map
* Change tile in collision and animation editors based on selected tile object
* Keep the active brush when switching maps
* Python plugins can now add export-only map formats
* Fixed updating of current tile when changing map
* Fixed animated tile overlay to look less odd in some cases
* Fixed Save As dialog popping up when saving fails
* Fixed tilesets view collapsing when switching maps on OS X
* Updated Russian, Spanish, Czech, French, Japanese, German, Dutch and Bulgarian translations

Many thanks to all who contributed!

### A Look Ahead to Tiled 1.0

That's right, I'd like the next version of Tiled to be 1.0! It'll probably take a bit longer than this version since there are a [large number of issues][5] to address before I'm ready to call it "complete". But I want to finish it this year, which is possible because of many awesome people [supporting me with a donation][6] each month. Thanks for that!

Of course, this is not to be seen as the end, but rather a beginning. This milestone is about providing a solid base upon which to build future changes. For changes planned beyond 1.0, check out the [Tiled roadmap][7].


  [1]: http://www.mapeditor.org/download
  [2]: https://github.com/bjorn/tiled/issues/4
  [3]: http://opengameart.org/content/isometric-64x64-outside-tileset
  [4]: http://forum.mapeditor.org/t/tiled-0-12-0-released/519
  [5]: https://github.com/bjorn/tiled/milestones/Tiled%201.0
  [6]: https://www.patreon.com/bjorn
  [7]: https://trello.com/b/yl3PAtN0/tiled-roadmap
