---
layout: post
title: Tiled 1.0.2 released
author: Thorbjørn Lindeijer
tags: release
---

More bugs were fixed since
[Tiled 1.0.1]({{ site.baseurl }}{% post_url 2017-06-13-tiled-1-0-1-released %}),
including a hang when pasting a tile stamp and then using the fill tool, so
upgrading is recommended! Also, by popular demand the properties of selected
tiles and terrains are once again visible while editing maps, albeit read-only.

### Changelog

* Added read-only tile and terrain properties in map editor ([#1615](https://github.com/bjorn/tiled/issues/1615))
* Fixed Terrains view to display all tilesets with terrain
* Fixed hang when trying to fill with a pasted stamp ([#1617](https://github.com/bjorn/tiled/issues/1617), [#1624](https://github.com/bjorn/tiled/issues/1624))
* Fixed crash when editing collision when tile image wasn't loaded
* Fixed rendering of tile objects when the image couldn't be loaded
* Fixed rendering of tile object outlines for resized objects
* Fixed labels shown on objects hidden via a group layer
* Fixed updating of label positions when moving a group layer
* GmxPlugin: Fixed tile type inheritance for tile objects
* Restored Ctrl+N shortcut on "New Map" action

Thanks to everybody who submitted bug reports!

### Download

As usual, you can [download Tiled at itch.io](https://thorbjorn.itch.io/tiled).
The best way to stay up-to-date is by installing Tiled through the
[itch.io app](https://itch.io/app). Install the **snapshot** version if you'd
like to stay bleeding-edge and keep an eye on
[the Devlog](https://thorbjorn.itch.io/tiled/devlog) for updates.
