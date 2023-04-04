---
layout: post
title: Tiled 1.10.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Apart from fixing a number of issues in [Tiled 1.10][tiled-1.10], this release
adds read-only access to parts of the loaded project and improves the
interaction of panning with <kbd>Space</kbd>. The latter now requires the left
mouse button to be pressed as well, and now also works in the tileset view.

For Linux users, note that the Qt 5 AppImage no longer works on Ubuntu 18.04
since it is now built on Ubuntu 20.04 and requires Glibc 2.30. Users of Ubuntu
18.04 can still install the latest Tiled through [snap][snap] or
[Flatpak][Flatpak]. The snap has now also been updated to Qt 5.15 and the
Flatpak for 1.10.1 will use Qt 6.4.

Changelog
---------

*   Make panning with Space require pressing a mouse button as well ([#3626](https://github.com/mapeditor/tiled/issues/3626))
*   Scripting: Added read-only access to Project properties (by dogboydog, [#3622](https://github.com/mapeditor/tiled/pull/3622))
*   Scripting: Fixed behavior of `Dialog.SameWidgetRows` ([#3607](https://github.com/mapeditor/tiled/issues/3607))
*   Fixed object labels to adjust to application font changes
*   Fixed grid rendering for odd Hex Side Length values ([#3623](https://github.com/mapeditor/tiled/issues/3623))
*   Fixed tile stamp getting messed up on staggered maps in some cases ([#3431](https://github.com/mapeditor/tiled/issues/3431))
*   JSON plugin: Fixed loading of empty tilesets created by script ([#3542](https://github.com/mapeditor/tiled/issues/3542))
*   Godot 4 plugin: Removed depth limit for `.godot` project file ([#3612](https://github.com/mapeditor/tiled/issues/3612))
*   Improved Terrain Brush for Hexagonal (Staggered) maps with side length 0 ([#3617](https://github.com/mapeditor/tiled/issues/3617))
*   Removed "Add Folder to Project" button from the startup page
*   Qt 6: Increased the image allocation limit from 128 MB to 1 GB ([#3616](https://github.com/mapeditor/tiled/issues/3616))
*   Qt 6 / Linux: Fixed long startup time for some icon themes
*   snap: Updated from core20 to core22 (now uses Qt 5.15)
*   Qbs: Added `projects.Tiled.libDir` option ([#3613](https://github.com/mapeditor/tiled/issues/3613))

### Thanks!

Many thanks to all who've reported issues, and of course to everybody who
supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.10]: {{ site.baseurl }}{% post_url 2023-03-10-tiled-1-10-released %}
[snap]: https://snapcraft.io/tiled
[Flatpak]: https://flathub.org/apps/details/org.mapeditor.Tiled
