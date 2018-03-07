---
layout: post
title: Tiled 1.1.3 released
author: Thorbjørn Lindeijer
tags: release
---

This release bring a host of bug fixes and some translation updates. Upgrading is recommended!

### Changelog

* Fixed crash when removing a tileset referenced by multiple objects
* Fixed crash on paste when it introduced more than one new tileset
* Fixed Invert Selection for non-infinite maps
* Fixed Select All to not select objects on locked layers
* Fixed logic determining the tilesets used by a tile layer
* Fixed copy/paste changing object order ([#1896](https://github.com/bjorn/tiled/issues/1896))
* Fixed tileset getting loaded twice when used by the map and a template
* Fixed repainting issues on undo/redo for new maps ([#1887](https://github.com/bjorn/tiled/issues/1887))
* JSON plugin: Fixed loading of infinite maps using CSV tile layer format ([#1878](https://github.com/bjorn/tiled/issues/1878))
* Linux: Updated AppImage to Qt 5.9.4
* Updated Hungarian, Japanese, Norwegian Bokmål, Portuguese and Ukrainian translations

Many thanks to everybody who submitted bug reports and translation updates, as well as to all who [support me financially](https://www.patreon.com/bjorn) so that I can keep up Tiled development and maintenance!
