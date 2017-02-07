---
layout: post
title: Tiled 0.14.1 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 788
---

One very important fix was made after the [Tiled 0.14 release][1] so here's Tiled 0.14.1.

The problem was that when you right-clicked to capture a tile from the map view, the Properties view would update to show the correct ID of that tile, but it would not show the correct properties. Even worse, when you tried to edit its properties, you could be editing the properties of other tiles (which you had previously selected in the tileset view) without realizing. Now, when you capture tiles from the map, the Properties view should allow you to edit the properties of all the captured tiles. Thanks to @Hop for [reporting][2] this issue!

So here's the full changelog:

* Added missing 'renderorder' property to the Lua export ([#1096][3])
* Fixed editing of properties of tiles captured from the map

Go get it from the [Download page][4].


  [1]: http://discourse.mapeditor.org/t/tiled-0-14-0-released/769
  [2]: http://discourse.mapeditor.org/t/eyedropper-tool/755/12?u=bjorn
  [3]: https://github.com/bjorn/tiled/issues/1096
  [4]: http://www.mapeditor.org/download
