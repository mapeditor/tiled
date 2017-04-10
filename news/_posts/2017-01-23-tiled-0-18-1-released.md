---
layout: post
title: Tiled 0.18.1 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1981
---

Today I've released [Tiled 0.18.1](https://thorbjorn.itch.io/tiled), a recommended upgrade since it fixes some crashes and several other issues. It also makes a host of improvements to the GameMaker 1.4 export.

Thanks to everybody who contributed to this release with bug reports, suggestions or patches!

**A note for Windows users upgrading from previous versions using the installer:** Please first _uninstall_ Tiled manually and then reinstall using the installer. Due to fixing the installer to work properly system-wide, it will fail to upgrade versions before 0.18.1 and leave behind a broken Tiled.

### Changelog

* Fixed terrain brush for isometric staggered maps ([#427](https://github.com/bjorn/tiled/issues/427), by Clyde)
* Fixed crash when resizing map causes objects to get removed
* Fixed crash when duplicating an object layer
* Fixed position of image layer after Resize or Offset Map ([#1418](https://github.com/bjorn/tiled/issues/1418))
* Fixed the quality of the minimap on HiDpi displays
* Fixed Alt-drag behavior to not override resize handles ([#1425](https://github.com/bjorn/tiled/issues/1425))
* When adding a new layer, insert it above the current one
* GmxPlugin: Fixed positioning for non-tile objects and support scaling ([#1426](https://github.com/bjorn/tiled/issues/1426))
* GmxPlugin: Export tile objects without a type as tiles
* GmxPlugin: Support horizontal and vertical flipping
* Windows: Fixed encoding problems with command-line output ([#1381](https://github.com/bjorn/tiled/issues/1381))
* Windows: Fixed the architecture of shipped MSVC DLLs ([#1338](https://github.com/bjorn/tiled/issues/1338))
* Updated Chinese translation ([#1432](https://github.com/bjorn/tiled/pull/1432), by Clyde)

### Support Tiled Development

These fixes and improvements were possible thanks to your support [through Patreon](https://www.patreon.com/bjorn) and on [itch.io](https://thorbjorn.itch.io/tiled)! If you are not supporting me yet, [please chip in](https://www.patreon.com/bePatron?u=90066) to ensure they can keep coming! Thanks! :heart:
