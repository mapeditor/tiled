---
layout: post
title: Tiled 1.8.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Unfortunately a number of potential crashes and some other issues in the new
[Tiled 1.8][tiled-1.8] were not discovered during testing. Upgrading is highly
recommended!

For Linux users who are using "snap" to install Tiled, it has finally moved to
the "core20" base, so now it is built against Qt 5.12. This most importantly
brings a significant upgrade to the JavaScript engine and the available API.

Changelog
---------

* Fixed pasted objects not getting selected if a tile layer was also copied
* Fixed possible crash when trying to determine whether OpenGL is used
* Fixed possible crash when using the Insert Tile tool
* Fixed possible crash in tile stamp preview
* AutoMapping: Fixed crash when an input layer does not exist ([#3269](https://github.com/mapeditor/tiled/issues/3269))
* Scripting: Automatically add tilesets to the map where needed ([#3268](https://github.com/mapeditor/tiled/issues/3268))
* snap: Updated from core18 to core20 (now uses Qt 5.12)
* AppImage: Updated to Sentry 0.4.15

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting
feature, and of course to everybody who supports Tiled development with a
donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.8]: {{ site.baseurl }}{% post_url 2022-02-07-tiled-1-8-0-released %}
