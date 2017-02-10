---
layout: post
title: Tiled 0.17.2 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1848
---

[Tiled 0.17.2](https://thorbjorn.itch.io/tiled) is just a small release with a few bug fixes. Potentially most importantly, it downgrades Qt from 5.7 to 5.6.2 for the OS X release because of crashes reported when Tiled is compiled against Qt 5.7.

**Changelog**

* Fixed bug with editing type and name for multiple objects
* Fixed ability to change the image of a tile in an image collection tileset
* Fixed wrong layer name getting edited when switching maps ([#1396](https://github.com/bjorn/tiled/issues/1396))
* Fixed possible crash when missing tileset images and using tile animations ([#1393](https://github.com/bjorn/tiled/issues/1393))
* Compiled against Qt 5.6.2 on macOS to avoid crashes with Qt 5.7 ([#1354](https://github.com/bjorn/tiled/issues/1354))

Thanks a lot for the support [through Patreon](https://www.patreon.com/bjorn) and on [itch.io](https://thorbjorn.itch.io/tiled)! I'm only able to keep improving Tiled because of this support. If you are not supporting me yet, [please chip in](https://www.patreon.com/bePatron?u=90066) towards my next goal of being able to actually afford working on Tiled two full days per week! Thanks! :heart:
