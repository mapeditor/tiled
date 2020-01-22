---
layout: post
title: Tiled 1.3.2 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch release addresses many smaller issues in [Tiled 1.3][1], the current stable release.

Also, it is the first release where the Windows installer and the contained binaries are signed, though not by me but by [SignPath](https://about.signpath.io/?). Hopefully this will help making Windows Defender Smartscreen happy after a few people have downloaded Tiled!

Changelog
---------

* Fixed initialization of selected layers ([#2719](https://github.com/bjorn/tiled/issues/2719))
* Fixed stamp action shortcuts not being configurable ([#2684](https://github.com/bjorn/tiled/issues/2684))
* Fixed the tileset view to respect the 'wheel zooms by default' preference
* Fixed insertion position when using drag-n-drop to rearrange layers
* Fixed displayed layer data format in Properties
* Fixed repeating of export when map is saved by a custom command ([#2709](https://github.com/bjorn/tiled/issues/2709))
* Fixed issue when multiple worlds are loaded that use pattern matching
* Issues view can now be hidden by clicking the status bar counters
* macOS: Fixed black toolbar when enabling OpenGL rendering ([#1839](https://github.com/bjorn/tiled/issues/1839))
* Windows: Fixed context menus activating first item on release ([#2693](https://github.com/bjorn/tiled/issues/2693))
* Windows installer: Include the 'defoldcollection' plugin ([#2677](https://github.com/bjorn/tiled/issues/2677))
* Windows installer: Signed by [SignPath](https://about.signpath.io/?)
* libtiled: Avoid inheriting Properties from QVariantMap ([#2679](https://github.com/bjorn/tiled/pull/2679))
* docs: Added some notes to Python and JavaScript pages ([#2725](https://github.com/bjorn/tiled/pull/2725))
* Updated Qt from 5.12.5 to 5.12.6
* Updated Finnish translation (by Tuomas Lähteenmäki and odamite)
* Updated part of Italian translation (by Katia Piazza)

Many thanks to all who've reported issues and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, be aware that I depend on your support to be able to keep improving this tool. Please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). Thanks!_

[1]: {{ site.baseurl }}{% post_url 2019-11-13-tiled-1-3-0-released %}
