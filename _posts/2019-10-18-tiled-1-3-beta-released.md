---
layout: post
title: Tiled 1.3 Beta released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

After more than a year since the [Tiled 1.2 release][tiled-12], it's almost
time for the next major update! This release took about twice as long as I had
expected, in part due to underestimating the work required for adding
scripting support and also due to the many additional improvements that have
been made during that time.

In the next two weeks, we'll be polishing up Tiled 1.3, potentially adding
some additional scripting API, fixing issues and updating the translations.
For this I need your feedback, so please give this beta release a try! The
[latest snapshot][snapshot] (version 2019.10.18) marks the start of the Tiled
1.3 Beta.

If you have been staying with the stable Tiled versions so far, this is the
perfect time to install the snapshot and try out the new features! By doing
this you have a chance to make sure they work fine in your project as well,
and to report any new issues before the final release.

### Changelog

* Added support for [extending Tiled with JavaScript][scripting] ([#949](https://github.com/bjorn/tiled/issues/949))
* Added error and warning counts to the status bar
* Added [Issues view][issues] where you can see warnings and errors and interact with them
* Added configuration of keyboard shortcuts ([#215](https://github.com/bjorn/tiled/issues/215))
* Added status bar notification on new releases (replacing Sparkle and WinSparkle)
* Added option to [show tile collision shapes on the map][collision] ([#799](https://github.com/bjorn/tiled/issues/799))
* Added switching current layer with [Ctrl + Right Click in map view][select-layer-by-clicking]
* Added search filter to the Objects view ([#1467](https://github.com/bjorn/tiled/issues/1467))
* Added icons to objects in the Objects view
* Added [dynamic wrapping mode][tileset-wrapping] to the tileset view ([#1241](https://github.com/bjorn/tiled/issues/1241))
* Added a \*.world file filter when opening a world file
* Added support for .world files in tmxrasterizer (by Samuel Magnan, [#2067](https://github.com/bjorn/tiled/pull/2067))
* Added synchronization of selected layers and tileset when switching between maps in a world (by JustinZhengBC, [#2087](https://github.com/bjorn/tiled/pull/2087))
* Added actions to show/hide and lock/unlock the selected layers
* Added toggle button for "Highlight Current Layer" action
* Added custom output chunk size option to map properties (by Markus, [#2130](https://github.com/bjorn/tiled/pull/2130))
* Added support for Zstandard compression and configurable compression level (with BRULE Herman and Michael de Lang, [#1888](https://github.com/bjorn/tiled/pull/1888))
* Added option to minimize output on export ([#944](https://github.com/bjorn/tiled/issues/944))
* Added export to Defold .collection files (by CodeSpartan, [#2084](https://github.com/bjorn/tiled/pull/2084))
* Added a warning when custom file properties point to non-existing files ([#2080](https://github.com/bjorn/tiled/issues/2080))
* Added shortcuts for next/previous tileset ([#1238](https://github.com/bjorn/tiled/issues/1238))
* Added saving of the last export target and format in the map/tileset file ([#1610](https://github.com/bjorn/tiled/issues/1610))
* Added option to repeat the last export on save ([#1610](https://github.com/bjorn/tiled/issues/1610))
* Tile Collision Editor: Added objects list view
* Changed the Type property from a text box to an editable combo box ([#823](https://github.com/bjorn/tiled/issues/823))
* Changed animation preview to follow zoom factor for tiles (by Ruslan Gainutdinov, [#2050](https://github.com/bjorn/tiled/pull/2050))
* Changed the shortcut for AutoMap from A to Ctrl+M
* AutoMapping: Added "OverflowBorder" and "WrapBorder" options (by João Baptista de Paula e Silva, [#2141](https://github.com/bjorn/tiled/pull/2141))
* AutoMapping: Allow any supported map format to be used for rule maps
* Python plugin: Added support for loading external tileset files (by Ruin0x11, [#2085](https://github.com/bjorn/tiled/pull/2085))
* Python plugin: Added Tile.type() and MapObject.effectiveType() (by Ruin0x11, [#2124](https://github.com/bjorn/tiled/pull/2124))
* Python plugin: Added Object.propertyType() (by Ruin0x11, [#2125](https://github.com/bjorn/tiled/pull/2125))
* Python plugin: Added Tileset.sharedPointer() function ([#2191](https://github.com/bjorn/tiled/issues/2191))
* tmxrasterizer: Load plugins to support additional map formats (by Nathan Tolbert, [#2152](https://github.com/bjorn/tiled/pull/2152))
* tmxrasterizer: Added rendering of object layers (by oncer, [#2187](https://github.com/bjorn/tiled/pull/2187))
* Fixed missing native styles when compiled against Qt 5.10 or later ([#1977](https://github.com/bjorn/tiled/issues/1977))
* Fixed file change notifications no longer triggering when file was replaced (by Nathan Tolbert, [#2158](https://github.com/bjorn/tiled/pull/2158))
* Fixed layer IDs getting re-assigned when resizing the map ([#2160](https://github.com/bjorn/tiled/issues/2160))
* Fixed performance issues when switching to a new map in a world with many maps (by Simon Parzer, [#2159](https://github.com/bjorn/tiled/pull/2159))
* Fixed restoring of expanded group layers in Objects view
* Fixed tileset view to keep position at mouse stable when zooming ([#2039](https://github.com/bjorn/tiled/issues/2039))
* libtiled-java: Added support for image layers and flipped tiles (by Sergey Savchuk, [#2006](https://github.com/bjorn/tiled/pull/2006))
* libtiled-java: Optimized map reader and fixed path separator issues (by Pavel Bondoronok, [#2006](https://github.com/bjorn/tiled/pull/2006))
* Updated builds on all platforms to Qt 5.12 (except snap release)
* Raised minimum supported Qt version from 5.5 to 5.6
* Raised minimum supported macOS version from 10.7 to 10.12
* Removed option to include a DTD in the saved files
* Removed the automappingconverter tool
* snap: Updated from Ubuntu 16.04 to 18.04 (core18, Qt 5.9)

### Thank You!

This release and all the work that led up to it would have never been possible without your continued support! I had committed to some really large changes, but I got them done thanks to the many full days I could work on Tiled.

Thanks to everybody who made this release possible! Patrons on [Patreon](https://www.patreon.com/bjorn) and [Liberapay](https://liberapay.com/bjorn) and everybody who bought [Tiled on itch.io](https://thorbjorn.itch.io/tiled) contributed to the time I could spend to implement these features. Thank you!

[snapshot]: https://thorbjorn.itch.io/tiled/devlog/105206/tiled-13-beta-released
[tiled-12]: {{ site.baseurl }}{% post_url 2018-09-18-tiled-1-2-0-released %}
[scripting]: https://doc.mapeditor.org/en/latest/reference/scripting/
[tileset-wrapping]: https://thorbjorn.itch.io/tiled/devlog/100642/dynamic-wrapping-tileset-view
[issues]: https://thorbjorn.itch.io/tiled/devlog/94974/new-issues-view-and-gamescom
[collision]: https://thorbjorn.itch.io/tiled/devlog/77202/show-tile-collision-shapes-on-the-map
[select-layer-by-clicking]: https://thorbjorn.itch.io/tiled/devlog/89540/select-tile-layers-by-clicking-them
[sponsors]: https://github.com/sponsors/bjorn
[matching-fund]: https://help.github.com/en/articles/about-github-sponsors#about-the-github-sponsors-matching-fund
