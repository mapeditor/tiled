---
layout: post
title: Tiled 1.8.6 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Connections between objects are now maintained when pasting or duplicating
objects, the Defold export plugin now supports rotated tiles and the German
translation was updated.

A number of other fixes and small improvements were made as well, as detailed
in the below changelog.

In the meantime, we're also getting very close to the Tiled 1.9 release, so
this is likely going to be the last patch release to [Tiled 1.8][tiled-1-8].
Many changes have followed since the [Tiled 1.9 Alpha][tiled-1-9-alpha], so the
plan is to make a 1.9 Beta release soon.

### Changelog

*   Keep references between objects when copy/pasting or duplicating ([#3361](https://github.com/mapeditor/tiled/issues/3361))
*   Improved default translation used in case of multiple options
*   Terrain Brush: Update preview on mouse release ([#3381](https://github.com/mapeditor/tiled/issues/3381))
*   Fixed 'Add Variation' action in Tile Stamps context menu ([#3362](https://github.com/mapeditor/tiled/issues/3362))
*   Fixed importing of removed shortcuts ([#3367](https://github.com/mapeditor/tiled/issues/3367))
*   Fixed breaking of alternative shortcuts on import or reset ([#3367](https://github.com/mapeditor/tiled/issues/3367))
*   Fixed conflict detection to handle alternative shortcuts ([#3368](https://github.com/mapeditor/tiled/issues/3368))
*   Fixed locking up UI on property type name conflict ([#3380](https://github.com/mapeditor/tiled/issues/3380))
*   Scripting: Fixed possible crash when accessing Layer.map
*   Defold plugins: Added support for rotated tiles ([#3369](https://github.com/mapeditor/tiled/issues/3369))
*   Updates to German translation (by Ettore Atalan)

### Thanks!

Many thanks to all who've reported issues or have made suggestions, and of
course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1-8]: {{ site.baseurl }}{% post_url 2022-02-07-tiled-1-8-0-released %}
[tiled-1-9-alpha]: {{ site.baseurl }}{% post_url 2022-04-08-tiled-1-9-alpha-released %}
