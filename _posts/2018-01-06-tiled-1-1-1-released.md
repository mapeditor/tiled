---
layout: post
title: Tiled 1.1.1 released
author: Thorbjørn Lindeijer
tags: release
---

Unfortunately, a crash was found in [Tiled 1.1.0][1] when loading a map
with a non-tile object template instance. Upgrading is recommended.

### Changelog

* Fixed crash on load for template instances of non-tile objects ([#1844](https://github.com/bjorn/tiled/issues/1844))
* Windows Installer: Include the Qt SVG image plugin ([#1847](https://github.com/bjorn/tiled/issues/1847))
* Linux AppImage: Updated from Qt 5.9.2 to Qt 5.9.3

[1]: {{ site.baseurl }}{% post_url 2018-01-03-tiled-1-1-0-released %}
