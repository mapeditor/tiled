---
layout: post
title: Tiled 1.3.4 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This is a small bugfixing release, addressing some important issues for those affected.

Changelog
---------

* Fixed automatic reload issues when editing object types (regression in 1.3.1, [#2768](https://github.com/bjorn/tiled/issues/2768))
* Scripting: Added methods to get tileset's image size (backported from 1.4, [#2733](https://github.com/bjorn/tiled/issues/2733))
* Scripting: Fixed map.tilesets when 'Embed tilesets' is enabled
* Fixed the "Fix Tileset" button in the Template Editor
* macOS: Disabled unified tool bar to avoid repainting issues ([#2667](https://github.com/bjorn/tiled/issues/2667))
* macOS and Linux: Updated Qt from 5.12.6 to 5.12.7

Many thanks to those who've reported issues and of course to everybody who supports Tiled development with a donation!

Update on Tiled 1.4
-------------------

I don't expect there to be another Tiled 1.3 release, since I'm now fully focused on finishing up Tiled 1.4. Check out the "#snapshot" posts to see what's new and consider installing a development snapshot to try it out. Now is the best time to provide feedback on the new features!

_Please note that I depend on your support to be able to keep improving this tool. If you have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). Thanks!_
