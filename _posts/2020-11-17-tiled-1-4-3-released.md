---
layout: post
title: Tiled 1.4.3 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This [Tiled 1.4][tiled-1-4] patch release makes sure Tiled runs fine on macOS 11 (Big Sur), but a number of other fixes are included as well.

Changelog
---------

Here's the full list of changes since [Tiled 1.4.2][tiled-1-4-2].

*   Fixed running Tiled on macOS Big Sur ([#2845](https://github.com/bjorn/tiled/issues/2845))
*   Fixed opening of files in already open instance of Tiled
*   Fixed crash in Edit Commands dialog ([#2914](https://github.com/bjorn/tiled/issues/2914))
*   Fixed Object Alignment not getting set when reloading a tileset
*   Tile Collision Editor: Fixed invisible tile for isometric oriented tileset ([#2892](https://github.com/bjorn/tiled/issues/2892))
*   Improved error message when adding external tileset
*   Ignore attempts to replace a tileset with itself
*   qmake: Support linking to system Zstd on all UNIX-like systems
*   Updated Qt to 5.12.10 on all platforms

### Thanks!

Many thanks to all who've reported issues and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). I depend on your support to be able to keep improving this tool!_

[tiled-1-4]: {{ site.baseurl }}{% post_url 2020-06-20-tiled-1-4-0-released %}
[tiled-1-4-2]: {{ site.baseurl }}{% post_url 2020-08-05-tiled-1-4-2-released %}
