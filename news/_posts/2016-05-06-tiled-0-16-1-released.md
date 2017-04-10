---
layout: post
title: Tiled 0.16.1 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1333
---

Today I noticed that the Windows packages for [Tiled 0.16.0](http://discourse.mapeditor.org/t/tiled-0-16-0-released/1206) did not actually have the auto-updater enabled. This bugfix release resolves that issue as well as fixing several small issues. It also includes an updated Turkish translation and a Norwegian Bokmål translation was added.

* Fixed auto-updater not enabled for Windows release ([f2bb1725](https://github.com/bjorn/tiled/commit/f2bb17256d81dc10f3580c196e394926af2c278b))
* Fixed saving of object IDs assigned to tile collision shapes ([#1052](https://github.com/bjorn/tiled/issues/1052))
* Fixed crash when pressing Backspace with Custom Properties section selected ([#1245](https://github.com/bjorn/tiled/issues/1245))
* Fixed crash on exit when leaving the Tile Collision Editor open ([#1253](https://github.com/bjorn/tiled/issues/1253))
* Added Norwegian Bokmål translation (by Peter André Johansen, [#1261](https://github.com/bjorn/tiled/pull/1261))
* Updated Turkish translation ([#1254](https://github.com/bjorn/tiled/pull/1254))

Download [Tiled 0.16.1](https://thorbjorn.itch.io/tiled) through itch.io.
