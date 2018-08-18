---
layout: post
title: Tiled 1.1.6 released
author: Thorbjørn Lindeijer
tags: release
---

This is a bug-fixing release with mostly minor fixes and improvements that have been made since [Tiled 1.1.5][1].

### Changelog

* Fixed Terrain Brush issue on staggered isometric maps ([#1951](https://github.com/bjorn/tiled/issues/1951))
* Fixed objects to stay selected when moving them between layers
* Fixed small tab bar rendering issue on high DPI displays
* Fixed rendering of arrows on scroll bar buttons
* Fixed object labels to adjust properly to the font DPI
* Fixed resize handle locations for multiple zero-sized objects
* Fixed handling of arrow keys on focused layer combo box ([#1973](https://github.com/bjorn/tiled/issues/1973))
* Tile Collision Editor: Fixed handling of tile offset ([#1955](https://github.com/bjorn/tiled/issues/1955))
* Tile Collision Editor: Fixed potential crash on Undo ([#1965](https://github.com/bjorn/tiled/issues/1965))
* Python plugin: Added some missing API to the Cell class
* Windows and Linux: Downgraded builds to Qt 5.9 (fixes [#1928](https://github.com/bjorn/tiled/issues/1928))
* macOS: Fixed library loading issues for tmxrasterizer and terraingenerator
* macOS: Downgraded to Qt 5.6 (fixes resizing of undocked views and reduces minimum macOS version to 10.7)
* Updates to German, Hungarian, Norwegian Bokmål, Polish, Portuguese (Portugal), Russian and Ukrainian translations

### Recent Development

I'm currently spending most of my time finalizing Tiled 1.2. Recent improvements have included [highlighting of the hovered object][2] and showing an [object placement preview on hover][3]. A few more improvements remain to be done as well as polishing the multi-layer and world features. In the meantime, don't hesitate to install the development snapshot and give feedback!

Many thanks to all who submitted bug reports, as well as to those who [support me financially](https://www.patreon.com/bjorn) so that I can keep up Tiled development and maintenance!


[1]: {{ site.baseurl }}{% post_url 2018-04-25-tiled-1-1-5-released %}
[2]: https://thorbjorn.itch.io/tiled/devlog/38930/multi-layer-stamp-improvements-and-highlight-hovered-object
[3]: https://thorbjorn.itch.io/tiled/devlog/40807/object-placement-preview-on-hover
