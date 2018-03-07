---
layout: post
title: Tiled 1.1.2 released
author: Thorbjørn Lindeijer
tags: release
---

A number of fixes have been made since [Tiled 1.1.1][1], among which a
potential crash and the embarrassing inability to load infinite maps
stored in JSON format. Upgrading is recommended.


### Changelog

* Fixed possible crash while editing polygons
* Fixed hang when loading map file with empty compressed layer data
* Fixed selection of tile stamp to work on mouse click
* Fixed tools not being up to date on modifier keys after activation
* Fixed "Offset Map" action for infinite maps ([#1866](https://github.com/bjorn/tiled/issues/1866))
* Templates view: Keep template centered when resizing view
* Tile Collision Editor: Keep tile centered when resizing view
* Tile Collision Editor: Display tool info text in status bar
* JSON plugin: Fixed reading of infinite maps ([#1858](https://github.com/bjorn/tiled/issues/1858))
* libtiled-java: Fixed some bugs (by Henry Wang, [#1840](https://github.com/bjorn/tiled/pull/1840))
* libtiled-java: Fixed tile offset value not being considered (by digitalhoax, [#1863](https://github.com/bjorn/tiled/pull/1863))

[1]: {{ site.baseurl }}{% post_url 2018-01-06-tiled-1-1-1-released %}
