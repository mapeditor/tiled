---
layout: post
title: Tiled 1.9.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch release addresses further issues in [Tiled 1.9][tiled-1.9], mostly
related to custom types. It also includes a number of small improvements.
Upgrading is highly recommended!

The [Defold export
plugins](https://doc.mapeditor.org/en/stable/manual/export-defold/) (`defold`
and `defoldcollection`) now both support setting the tilesource using custom
properties and the `defold` plugin now also automatically assigns Z-values to
the exported layers. Just in time for the [#MadeWithDefold Jam
2022](https://itch.io/jam/madewithdefold-jam-2)!

Changelog
---------

*   Allow adding maps to image collection tilesets ([#3447](https://github.com/mapeditor/tiled/issues/3447))
*   Auto-detect JSON file format when importing custom types ([#3472](https://github.com/mapeditor/tiled/issues/3472))
*   Added file system actions to the tile context menu ([#3448](https://github.com/mapeditor/tiled/issues/3448))
*   Fixed possible crash in Custom Types Editor ([#3465](https://github.com/mapeditor/tiled/issues/3465))
*   Fixed display of overridden values from a nested class
*   Fixed ability to reset nested string and file properties ([#3409](https://github.com/mapeditor/tiled/issues/3409))
*   Fixed changing nested property values for multiple objects ([#3344](https://github.com/mapeditor/tiled/issues/3344))
*   Fixed resolving of class properties on export to affect all data types ([#3470](https://github.com/mapeditor/tiled/issues/3470))
*   Fixed possible duplication of Automapping Rules Tileset ([#3462](https://github.com/mapeditor/tiled/issues/3462))
*   Fixed case where object labels could become visible for hidden layer ([#3442](https://github.com/mapeditor/tiled/issues/3442))
*   Fixed updating of custom property colors when changing style
*   Scripting: Added Tileset.findTile
*   AutoMapping: Fixed applying of rule probability ([#3425](https://github.com/mapeditor/tiled/issues/3425))
*   Defold plugin: Assign incrementing z values and allow specifying tile_set ([#3214](https://github.com/mapeditor/tiled/issues/3214))
*   Updates to German translation (by Christian Pervoelz)

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting
feature, and of course to everybody who supports Tiled development with a
donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.9]: {{ site.baseurl }}{% post_url 2022-06-25-tiled-1-9-released %}
