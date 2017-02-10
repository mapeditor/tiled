---
layout: post
title: Tiled 0.16.2 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1519
---

While [work on the next feature release](http://discourse.mapeditor.org/c/development) of Tiled continues, also some bugfixes have accumulated. In addition, the releases are now built against Qt 5.6 (previously Qt 5.5), which fixed some issues as well. Here's the full list:

* JSON plugin: Fixed loading of custom properties on terrains
* Lua plugin: Fixed missing export of object layer drawing order ([#1289](https://github.com/bjorn/tiled/issues/1289))
* Fixed tile index adjustment when tileset image changes width ([#1242](https://github.com/bjorn/tiled/issues/1242))
* Fixed `--export-map [format]` option ([#1307](https://github.com/bjorn/tiled/pull/1307))
* Fixed shortcuts for some tools when language is set to Dutch ([#1280](https://github.com/bjorn/tiled/issues/1280))
* Fixed a painting related bug affecting the top edge after AutoMapping ([#1308](https://github.com/bjorn/tiled/issues/1308))
* Fixed issues when compiling against Qt 5.6 on OS X and Windows ([#1309](https://github.com/bjorn/tiled/issues/1309))
* Fixed crash on maximizing with Maps view open on Windows ([#1153](https://github.com/bjorn/tiled/issues/1153), [#1268](https://github.com/bjorn/tiled/issues/1268), Qt 5.6.1)
* Fixed focus issue while typing predefined object types ([#1244](https://github.com/bjorn/tiled/issues/1244), Qt 5.6)
* Fixed silent fail when saving to restricted location on Windows ([#965](https://github.com/bjorn/tiled/issues/965), Qt 5.6)

Download [Tiled 0.16.2](https://thorbjorn.itch.io/tiled) through itch.io. Enjoy!
