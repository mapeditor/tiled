---
layout: post
title: Tiled 1.3.1 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

A number of issues have been reported since the release of [Tiled 1.3.0][1], so here's the first patch release! Most importantly, it fixes the new version notification for the Windows release, which was broken due to shipping the wrong OpenSSL DLLs. A few other other important functionality improvements have been made as well.

### Changelog

* Added reloading of object types when changed externally (by Jacob Coughenour, [#2674](https://github.com/bjorn/tiled/pull/2674))
* Added a status bar to the startup screen
* Made the shortcuts for the tools configurable ([#2666](https://github.com/bjorn/tiled/issues/2666))
* Made Undo/Redo shortcuts configurable ([#2669](https://github.com/bjorn/tiled/issues/2669))
* Fixed importing of keyboard settings (.kms files) ([#2671](https://github.com/bjorn/tiled/issues/2671))
* Fixed small window showing up on startup for a split second
* Windows: Fixed the shipped version of OpenSSL (fixes new version notification)
* Tiled Quick: Don't compile/install by default ([#2673](https://github.com/bjorn/tiled/issues/2673))

### Thanks!

Thanks to everybody who contributed to this patch release! That includes Jacob Coughenour for his nice improvement as well as those who've reported issues!

It also, obviously, includes everybody who's making it possible for me to continue working on Tiled. This patch release took me about 16 hours, yet it still comes just a week after the 1.3.0 release since I can afford to spend two full days/week on Tiled. Thank you for your support!

[1]: {{ site.baseurl }}{% post_url 2019-11-13-tiled-1-3-0-released %}
