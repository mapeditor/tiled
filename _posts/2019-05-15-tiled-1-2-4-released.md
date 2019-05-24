---
layout: post
title: Tiled 1.2.4 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release fixes various issues that have been discovered. It also adds a
Swedish translation.

### Changelog

* Fixed view boundaries to take into account layer offsets ([#2090](https://github.com/bjorn/tiled/issues/2090))
* Fixed map size when switching infinite off ([#2051](https://github.com/bjorn/tiled/issues/2051))
* Fixed the image cache to check file modification time ([#2081](https://github.com/bjorn/tiled/issues/2081))
* Fixed updating a few things when changing tileset drawing offset
* Fixed position of tile object outline on isometric maps
* Fixed saving of tile stamps when using the Shape Fill Tool
* tBIN plugin: Fixed loading of some tilesets on Linux
* tBIN plugin: Fixed possible crash when images can't be found ([#2106](https://github.com/bjorn/tiled/issues/2106))
* Python plugin: Disable this plugin by default, to avoid crashes on startup ([#2091](https://github.com/bjorn/tiled/issues/2091))
* JSON plugin: Fixed writing of position for objects without ID
* Added Swedish translation (by Anton Regnander)

### Next Release

In the meantime, the development towards Tiled 1.3 is progressing nicely. Its
main feature is to be [extensible using JavaScript][scripting], but it will also
[render collision shapes on the map][collision] and [allows the keyboard shortcuts to be changed][shortcuts].
Consider giving it a try, your feedback is welcome!

[scripting]: https://doc.mapeditor.org/en/latest/reference/scripting/
[collision]: https://thorbjorn.itch.io/tiled/devlog/77202/show-tile-collision-shapes-on-the-map
[shortcuts]: https://thorbjorn.itch.io/tiled/devlog/78207/configurable-keyboard-shortcuts
