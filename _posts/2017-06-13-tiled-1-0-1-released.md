---
layout: post
title: Tiled 1.0.1 released
author: Thorbjørn Lindeijer
tags: release
---

It's been over two weeks since the big
[Tiled 1.0]({{ site.baseurl }}{% post_url 2017-05-24-tiled-1-0-0-released %})
release, and now it's time for the first patch release. No crashes fixed so far,
but upgrading is recommended especially due to the fixes to the Properties dock.

### Changelog

* Made the zoom level used in Tilesets view persistent
* Fixed mixed up polygon and polyline icons (by Ketan Gupta, [#1588](https://github.com/bjorn/tiled/pull/1588))
* Fixed reset of font size when using font dialog ([#1596](https://github.com/bjorn/tiled/issues/1596))
* Fixed several issues with the Properties dock ([#1583](https://github.com/bjorn/tiled/issues/1583), [#1611](https://github.com/bjorn/tiled/issues/1611))
* Fixed centering on object on layer with offset ([#1600](https://github.com/bjorn/tiled/issues/1600))
* Fixed handling of symbolic links in Recent Files menu and Maps view ([#1589](https://github.com/bjorn/tiled/issues/1589))
* Fixed labels for objects in grouped object layers
* Reverted the file format version back to "1.0" and added "tiledversion" attribute
* Lua plugin: Fixed group layers being exported with "imagelayer" type ([#1595](https://github.com/bjorn/tiled/issues/1595))
* Added Korean translation (by miru2533 and SshipSunBee, [#1604](https://github.com/bjorn/tiled/pull/1604))
* Updated Russian and Chinese translations

Thanks to everybody who contributed to this release with bug reports or patches!

### Download

As usual, you can [download Tiled at itch.io](https://thorbjorn.itch.io/tiled).
The best way to stay up-to-date is by installing Tiled through the
[itch.io app](https://itch.io/app). Install the **snapshot** version if you'd
like to stay bleeding-edge and keep an eye on
[the Devlog](https://thorbjorn.itch.io/tiled/devlog) for updates.
