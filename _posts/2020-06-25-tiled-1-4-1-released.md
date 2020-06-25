---
layout: post
title: Tiled 1.4.1 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

The [Tiled 1.4][tiled-1-4] release came with a lot of big improvements, but unfortunately it also had a number of bugs and regressions. Upgrading is highly recommended!

Apart from the bugfixes, two changes have been made for convenience. It is now possible to see .world files in the Project view, and opening them will load that world and open their first map, when available. Similarly, when you double-click a template file or choose it from the "Open File in Project" action, it will open in the Template Editor view instead of popping up an error.

Changelog
---------

*   When opening a .world file, load the world and open its first map
*   When opening an object template, show it in the Template Editor
*   Fixed crash on trying to export using the command-line ([#2842](https://github.com/bjorn/tiled/issues/2842))
*   Fixed crash when deleting multiple objects with manual drawing order ([#2844](https://github.com/bjorn/tiled/issues/2844))
*   Fixed potential crash when removing a tileset
*   Fixed potential scaling happening for maps used as tilesets ([#2843](https://github.com/bjorn/tiled/issues/2843))
*   Fixed positioning of map view when switching between maps in a world
*   Fixed file dialog start location
*   Scripting: Fixed issues with absolute file paths on Windows ([#2841](https://github.com/bjorn/tiled/issues/2841))
*   Lua plugin: Fixed syntax used for object properties ([#2839](https://github.com/bjorn/tiled/issues/2839))

### Thanks!

Many thanks to all who've reported issues and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). I depend on your support to be able to keep improving this tool!_

[tiled-1-4]: {{ site.baseurl }}{% post_url 2020-06-17-tiled-1-4-0-released %}
