---
layout: post
title: Tiled 1.0.3 released
author: Thorbjørn Lindeijer
tags: release
---

It was found that Tiled 1.0.0 to 1.0.2 could crash on reloading a map,
so upgrading to Tiled 1.0.3 is very much recommended. This release
includes a bunch of other fixes as well, outlined below.

### Changelog

* Fixed crash on reload map ([#1659](https://github.com/bjorn/tiled/issues/1659), [#1694](https://github.com/bjorn/tiled/issues/1694))
* Fixed possible crash on undo/redo in collision editor ([#1695](https://github.com/bjorn/tiled/issues/1695))
* Fixed tile replacement to add tileset when needed (by Mohamed Thabet, [#1641](https://github.com/bjorn/tiled/pull/1641))
* Fixed the display of the image source property for tilesets
* Fixed shortcut for 'Copy tile coordinates' (Alt+C) in Portuguese translation (by olueiro)
* JSON plugin: Fixed reading of tileset column count
* JSON plugin: Fixed reading of custom properties on tile collision object group

Thanks to everybody who submitted bug reports and fixes!

### Download

As usual, you can [download Tiled at itch.io](https://thorbjorn.itch.io/tiled).
The best way to stay up-to-date is by installing Tiled through the
[itch.io app](https://itch.io/app). Install the **snapshot** version if you'd
like to stay bleeding-edge and keep an eye on
[the Devlog](https://thorbjorn.itch.io/tiled/devlog) for updates.
