---
layout: post
title: Tiled 0.18.2 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 2064
---

[Tiled 0.18.2](https://thorbjorn.itch.io/tiled) is out now, bringing some bugfixes and minor improvements.

**A note for Windows users upgrading from 0.18.0 or older versions using the installer:** Please first uninstall Tiled manually and then reinstall using the installer. Due to fixing the installer to work properly system-wide, it will fail to upgrade versions before 0.18.1 and leave behind a broken Tiled.

### Changelog

* Fixed crash when deleting multiple selected objects
* Fixed crash when moving multiple selected objects to another object layer
* Fixed updating of values displayed in Objects and Layers views
* GmxPlugin: Added support for image collection tilesets
* Object Types Editor: Improved behavior when adding new types ([#1448](https://github.com/bjorn/tiled/issues/1448))
* Linux: Fixed shipping of image format plugins in AppImage releases ([#1444](https://github.com/bjorn/tiled/issues/1444))

Thanks to everybody who contributed to this release with bug reports, suggestions or patches!
