---
layout: post
title: Tiled 1.2.5 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release fixes a few issues various people have run into.

### Changelog

* Fixed exporting to a file name containing multiple dots ([#2149](https://github.com/bjorn/tiled/issues/2149))
* Fixed possible crash in AutoMapper ([#2157](https://github.com/bjorn/tiled/issues/2157))
* Fixed crash when unloading certain plugins
* Fixed duplicated entries in Objects view after grouping layers
* Fixed adjacent maps within a world not being properly clickable
* Fixed empty maps within a world not being clickable
* Fixed handling of negative multiplierX/Y in a world file

### Tiled 1.3 Beta Coming Soon

Next week I plan to announce the Tiled 1.3 Beta, since this version is now
pretty much feature complete (or feature overloaded, one could say). It's been
way too long since the last new feature release, so again I'll be trying to
shorten the time between them! In the meantime, you can already try it by
installing the [latest development snapshot][snapshot]!

[snapshot]: https://thorbjorn.itch.io/tiled/devlog/103543/script-map-readers-and-binary-formats
