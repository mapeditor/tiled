---
layout: post
title: Tiled 1.2.2 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch release brings an assortment of small fixes and improvements. Maybe most importantly, it enables backwards compatibility of the JSON export option with older versions of Tiled. Those who need this can enable the new "json1" plugin (and optionally disable the standard "json" plugin, to avoid ambiguity).

### Changelog

* Added 'json1' plugin that exports to the old JSON format ([#2058](https://github.com/bjorn/tiled/issues/2058))
* Enable the adding of point objects in Tile Collision Editor ([#2043](https://github.com/bjorn/tiled/issues/2043))
* Reload AutoMapping rules when they have changed on disk (by Justin Zheng, [#1997](https://github.com/bjorn/tiled/issues/1997))
* Fixed remembering of last used export filter
* Fixed label color to update when object layer color is changed (by Justin Zheng, [#1976](https://github.com/bjorn/tiled/issues/1976))
* Fixed stamp and fill tools to adjust when tile probability is changed (by Justin Zheng, [#1996](https://github.com/bjorn/tiled/issues/1996))
* Fixed misbehavior when trying to open non-existing files
* Fixed mini-map bounds when layer offsets are used in combination with group layers
* Fixed Templates view missing from the Views menu ([#2054](https://github.com/bjorn/tiled/issues/2054))
* Fixed Copy Path / Open Folder actions for embedded tilesets ([#2059](https://github.com/bjorn/tiled/issues/2059))
* Python plugin: Made the API more complete ([#1867](https://github.com/bjorn/tiled/pull/1867))
* Linux: Updated to Qt 5.9.7 and Python 3.5
* Updated Chinese, German, Korean, Norwegian Bokmål, Portuguese (Portugal) and Ukrainian translations

Thanks to everybody who contributed to this patch release!

### Current Development

In the meantime I'm making steady progress on [making Tiled scriptable](https://github.com/bjorn/tiled/issues/949), but the task is turning out to include more open questions and challenges than I had anticipated. Nevertheless I expect to release an updated development snapshot soon with more of the desired functionality available.
