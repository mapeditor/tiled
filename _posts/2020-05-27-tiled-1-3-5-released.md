---
layout: post
title: Tiled 1.3.5 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

A few more fixes were made since [Tiled 1.3.4][1], some of them quite important for some people. Hence, here is another patch release for Tiled 1.3!

Changelog
---------

*   Fixed initialization and restoring of map view ([#2779](https://github.com/bjorn/tiled/issues/2779))
*   Fixed skewed tile terrain/Wang overlays for non-square tiles ([#1943](https://github.com/bjorn/tiled/issues/1943))
*   Fixed link color on dark theme
*   Fixed small issue when right-clicking embedded tileset tab
*   Fixed Wang Sets toggle to also appear in the Tileset menu
*   Scripting: Fixed issue when closing/comitting BinaryFile ([#2801](https://github.com/bjorn/tiled/issues/2801))
*   Scripting: Fixed "Safe writing of files" when writing with TextFile
*   Updated Qt to 5.12.8 on all platforms
*   Small translation updates to Bulgarian, French and Portuguese

The next release will be the Tiled 1.4 Beta!

[1]: {{ site.baseurl }}{% post_url 2020-04-14-tiled-1-3-4-released %}
