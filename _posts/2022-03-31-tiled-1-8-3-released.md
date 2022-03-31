---
layout: post
title: Tiled 1.8.3 and 1.8.4 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release fixes a huge amount of small issues and contains some initial
[AutoMapping](https://doc.mapeditor.org/en/stable/manual/automapping/)
optimizations. Upgrading is highly recommended.

With the more serious bugs already addressed in the [Tiled 1.8.1][tiled-1.8.1]
and [Tiled 1.8.2][tiled-1.8.2] releases, in the past month I've taken some time
to address smaller issues and some long-standing ones. Most notably the
"Horizontal Flip" action for objects will no longer [rotate them by 180
degrees](https://github.com/mapeditor/tiled/issues/1704)!

Also notable is that the loading of AutoMapping rule maps is about twice as
fast now, and applying of rules which do not use an "inputnot" layer is about
3x as fast as before. These small speed-ups are back-ported from the next
feature release, which is focused on [AutoMapping
improvements][tiled-1.9-roadmap].

*Unfortunately, Tiled 1.8.3 introduced a crash upon trying to create new
objects. This was fixed in Tiled 1.8.4, released on the same day!*

Changelog
---------

*   Improved rendering quality of the Mini-map when it's small ([#1431](https://github.com/mapeditor/tiled/pull/1431))
*   Fixed automatic tool switching after deleting layers
*   Fixed rendering of arrows for object references in class members ([#3306](https://github.com/mapeditor/tiled/issues/3306))
*   Fixed image layer repeat settings not copied to duplicates ([#3307](https://github.com/mapeditor/tiled/issues/3307))
*   Fixed map bounding rectangle for infinite isometric maps
*   Fixed tile selection to not get removed when deleting ([#3281](https://github.com/mapeditor/tiled/issues/3281))
*   Fixed custom types not being usable without opening a project ([#3295](https://github.com/mapeditor/tiled/issues/3295))
*   Fixed use of custom property types in global object types file ([#3301](https://github.com/mapeditor/tiled/issues/3301))
*   Fixed parallax layer positions for other maps in a world
*   Fixed crash when rendering invalid polygon objects
*   Fixed sticky Bucket Fill preview when hovering same tile
*   Fixed automatically reloaded map becoming the active document
*   Fixed "Map format '%s' not found" error
*   Fixed updating of "Unload/Save World" menu enabled state
*   Fixed flipping horizontally to not rotate objects by 180 degrees ([#1704](https://github.com/mapeditor/tiled/issues/1704))
*   Fixed displacement when flipping horizontally on isometric maps ([#2660](https://github.com/mapeditor/tiled/issues/2660))
*   Fixed offset of tile collision shapes on isometric maps ([#3138](https://github.com/mapeditor/tiled/issues/3138))
*   Mark world as modified when map size changes ([#3020](https://github.com/mapeditor/tiled/issues/3020))
*   Prevent unsaved maps from being added to a world ([#3317](https://github.com/mapeditor/tiled/issues/3317))
*   Hide "Move Objects to Layer" menu when there's only one object layer
*   Scripting: Avoid possible crash due to garbage collection ([#3290](https://github.com/mapeditor/tiled/issues/3290))
*   Scripting: Fixed missing null check in Tileset.loadFromImage and Tile.setImage
*   Scripting: Initialize tile layer size also when added as part of a group layer ([#3291](https://github.com/mapeditor/tiled/issues/3291))
*   AutoMapping: Applying rules without "inputnot" layers is now much faster
*   AutoMapping: Optimized calculation of each rule's input/output region
*   AutoMapping: Fixed compatibility with "RegionsInput" / "RegionsOutput" layers
*   AutoMapping: Fixed ability to AutoMap using project rules in unsaved maps
*   CSV plugin: Improved error message and replace reserved characters ([#3309](https://github.com/mapeditor/tiled/issues/3309))
*   terraingenerator: Fixed crash when source terrain doesn't have an image ([#3299](https://github.com/mapeditor/tiled/issues/3299))
*   macOS: Fixed main window expanding with many open files ([#1047](https://github.com/mapeditor/tiled/issues/1047))
*   JSON plugin: Wrap arrays at the map width or chunk width
*   Qt 6: Fixed captured or erased area when dragging backwards
*   Updated Finnish translation (by Tuomas Lähteenmäki)

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting
feature, and of course to everybody who supports Tiled development with a
donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.8.1]: {{ site.baseurl }}{% post_url 2022-02-11-tiled-1-8-1-released %}
[tiled-1.8.2]: {{ site.baseurl }}{% post_url 2022-02-17-tiled-1-8-2-released %}
[tiled-1.9-roadmap]: https://github.com/orgs/mapeditor/projects/4
