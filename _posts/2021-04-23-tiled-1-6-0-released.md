---
layout: post
title: Tiled 1.6 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release makes a number of small improvements over [Tiled 1.5][Tiled-1-5] and fixes some regressions. It also adds crash reporting (for the Linux AppImage), object selection preview and other new features.

### Crash Reporting

To be able to understand the general stability of Tiled and to immediately get useful information (the call stack, mostly) as soon as a crash happens, I've now integrated [Sentry][sentry] as crash handler. You can read more about what information is sent out exactly on the [crash reporting](/crash-reporting) page.

![Crash Reporting](/img/posts/2021-04-crash-reporting.png)

Unfortunately this feature is currently only available for the Linux AppImage. In future releases I hope to make it available for Windows and macOS releases as well. Please consider enabling this feature to help me make Tiled more stable!

### Object Selection Improvements

While dragging the selection rectangle to select objects, the objects that would be selected are now highlighted (similarly to the hovered object). This helps avoid that short moment of disappointment, when you realize the objects that got selected were not what you had intended.

![Selection Behavior](/img/posts/2021-04-selection-behavior.gif)

At the same time, you can now toggle between selecting "touched" objects (the old behavior) and "enclosed" objects. The latter mode really helps if your goal is to select a bunch of smaller objects that overlap a large one. It can be toggled either persistently on the tool bar or temporarily with the Alt modifier.

Many more small improvements and fixes were made, so see the changelog below for the full list.

## Changelog

* Added object selection preview
* Added toggle to select enclosed rather than touched objects ([#3023](https://github.com/mapeditor/tiled/issues/3023))
* Added Sentry crash handler to Linux AppImage (disabled by default)
* Added %tileid variable for custom commands on tilesets ([#3026](https://github.com/mapeditor/tiled/issues/3026))
* Added option to lock the position of views and tool bars
* Added toggle to show/hide other maps in the same world ([#2859](https://github.com/mapeditor/tiled/issues/2859))
* Added a helpful text to Terrain Sets view when it is empty ([#3015](https://github.com/mapeditor/tiled/issues/3015))
* Allow opening projects from the File menu ([#3000](https://github.com/mapeditor/tiled/issues/3000))
* Made the terrains list in the Terrain Sets view not collapsible ([#3015](https://github.com/mapeditor/tiled/issues/3015))
* Automatically select the first terrain when selecting a Terrain Set ([#3015](https://github.com/mapeditor/tiled/issues/3015))
* When duplicating objects, place the duplicates next to the originals ([#2998](https://github.com/mapeditor/tiled/issues/2998))
* Tweaked selection outlines to be a little fatter and adjust to DPI
* Write `--export-formats` output to stdout instead of stderr ([#3002](https://github.com/mapeditor/tiled/issues/3002))
* Allow hiding objects in the Tile Collision Editor
* Scripting: Added missing Tileset.transparentColor property
* Fixed 'Detach templates' export option to add tilesets when needed
* Fixed Terrain Brush behavior on map edges
* Fixed Terrain Brush behavior for sets transitioning to nothing
* Fixed loss of edit focus when hovering tileset while assigning terrain ([#3015](https://github.com/mapeditor/tiled/issues/3015))
* Fixed shortcuts for flipping or rotating the current terrain pattern
* Fixed switching to Terrain Brush when clicked terrain is already selected ([#3015](https://github.com/mapeditor/tiled/issues/3015))
* Fixed state of "dynamic wrapping" toggle button on startup
* Fixed parallax layer positioning when reordering layers ([#3009](https://github.com/mapeditor/tiled/issues/3009))
* Windows: Fixed Swedish translation missing from installer
* Windows: Re-enabled code signing by SignPath (was missing for Tiled 1.5)
* snap: Added 'removable-media' plug, for accessing USB drives
* snap: "Open Containing Folder" action now also selects the file
* JSON plugin: Write out "version" property as string ([#3033](https://github.com/mapeditor/tiled/issues/3033))
* YY plugin: Fixed plugin loading issue for qmake builds
* libtiled-java: Optimized for multithreaded usage (by Samuel Manflame, [#3004](https://github.com/mapeditor/tiled/pull/3004))
* Updated Bulgarian, French, Portuguese (Portugal), Swedish and Turkish translations
* Added Thai translation (by Thanachart Monpassorn, currently at 54%)

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by over 300 [patrons][Patreon] and
[sponsors][sponsors] supporting me on a monthly basis as well as many people
cIoosing to pay for [Tiled on itch.io][Itch] and some who donated through
[Liberapay][Liberapay]. To ensure I will be able to keep developing Tiled at
this pace, please [set up a small monthly donation][donate]!

Your donation primarily enables me to work 3 full days/week on Tiled. With
additional funds I can spend more days on Tiled. Let's make this tool even
better!

[Tiled-1-5]: {{ site.baseurl }}{% post_url 2021-03-23-tiled-1-5-0-released %}
[Patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Liberapay]: https://liberapay.com/Tiled/
[sentry]: https://sentry.io/
