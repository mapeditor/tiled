---
layout: post
title: Tiled 1.4.2 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

In this second patch release for [Tiled 1.4][tiled-1-4] there are a number of important fixes and some small improvements.

Since Tiled 1.4.0 the default tile layer data format had switched to Zstandard compression by accident, which is not widely supported. It has been set back to CSV (though in general I'd recommend using a compressed format since it's more efficient, zlib is a good choice).

The Windows installer has been improved in that the creation of a desktop shortcut as well as the launching of Tiled at the end have been made optional. Both can be turned off with a checkbox now. Please let me know if you run into any issues or have additional requests, I feel like I'm slowly getting the hang of this!

Changelog
---------

Here's the full list of changes since [Tiled 1.4.1][tiled-1-4-1].

*   Reverted the default layer data format back to CSV (was changed to Zstd by accident in 1.4.0)
*   Added ability to draw lines using click+drag (in addition to click and click) when holding Shift
*   Improved positioning when adding maps to world via context menu
*   Disable instead of hide the "Save As Template" action when using embedded tilesets
*   Made Ctrl turn off snapping if Snap to Fine Grid is enabled ([#2061](https://github.com/bjorn/tiled/issues/2061))
*   Set minimum value of tile width and height to 1
*   Fixed Select Same Tile tool behavior for empty tiles
*   Fixed clickability of the dot in point objects
*   Fixed adjusting of terrain images when tileset width changes
*   Worlds: Fixed potential data loss when opening .world file
*   tmxrasterizer: Added `--show-layer` option (by Matthias Varnholt, [#2858](https://github.com/bjorn/tiled/pull/2858))
*   tmxrasterizer: Added parameter to advance animations (by Sean Ballew, [#2868](https://github.com/bjorn/tiled/pull/2868))
*   Scripting: Initialize tile layer size to map size upon add ([#2879](https://github.com/bjorn/tiled/issues/2879))
*   Windows installer: Made creation of the desktop shortcut optional
*   Windows installer: Made the launching of Tiled optional
*   Updated Qt to 5.12.9 on all platforms except Windows XP and snap releases
*   snap: Fixed issues with storing the default session ([#2852](https://github.com/bjorn/tiled/issues/2852))
*   snap: Enabled support for Zstandard ([#2850](https://github.com/bjorn/tiled/issues/2850))

### Thanks!

Many thanks to all who've reported issues and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). I depend on your support to be able to keep improving this tool!_

[tiled-1-4]: {{ site.baseurl }}{% post_url 2020-06-20-tiled-1-4-0-released %}
[tiled-1-4-1]: {{ site.baseurl }}{% post_url 2020-06-25-tiled-1-4-1-released %}
