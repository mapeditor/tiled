---
layout: post
title: Tiled 1.7.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

As a pure stability update, this release fixes several crashes and other small issues. Upgrading is highly recommended!

Changelog
---------

* Don't save export target and format to exported files
* Fixed crashes resulting from the Tile Animation Editor
* Fixed possible crash when pasting multi-layer stamp ([#3097](https://github.com/mapeditor/tiled/issues/3097))
* Fixed possible crash when restoring expanded layers in Objects view
* Fixed parallax factor getting lost when layer is cloned ([#3077](https://github.com/mapeditor/tiled/issues/3077))
* Fixed an issue with synchronizing selected tiles to current stamp ([#3095](https://github.com/mapeditor/tiled/issues/3095))
* Commands: Fixed possible crash in Edit Commands window
* Commands: Automatically quote the command executable
* Commands: Improved starting directory for executable file chooser
* Commands: Fixed the 'Clear' button to reset the shortcut
* Updated Sentry crash reporter to 0.4.11
* Updated French translation

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting feature, and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_
