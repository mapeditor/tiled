---
layout: post
title: Tiled 1.2.3 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This is a tiny bugfixing release, fixing a few important issues. Most
importantly, copy/paste works again in the Tile Collision Editor, a feature
that broke in [Tiled 1.2.1][1].

### Changelog

* Fixed cut/copy in Tile Collision Editor ([#2075](https://github.com/bjorn/tiled/issues/2075))
* Fixed crash when trying to add Wang colors without a selected Wang set ([#2083](https://github.com/bjorn/tiled/issues/2083))
* tBIN plugin: Fixed hang when locating missing tileset image ([#2068](https://github.com/bjorn/tiled/issues/2068))
* CSV plugin: Fixed exporting of grouped tile layers

Many thanks to those who brought up these issues!

[1]: {{ site.baseurl }}{% post_url 2018-11-14-tiled-1-2-1-released %}
