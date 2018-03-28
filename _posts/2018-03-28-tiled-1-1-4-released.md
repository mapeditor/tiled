---
layout: post
title: Tiled 1.1.4 released
author: Thorbjørn Lindeijer
tags: release
---

More bug fixes and small improvements have been made since [Tiled 1.1.3][1]. Upgrading is recommended!

### Changelog

* Fixed exporting of external tilesets to JSON or TSX formats
* Fixed problem with embedding or exporting tilesets with Wang sets
* Fixed tiles placed by the terrain tool being considered different ([#1913](https://github.com/bjorn/tiled/issues/1913))
* Fixed text alignment values appearing at random in Properties view ([#1767](https://github.com/bjorn/tiled/issues/1767))
* Re-enabled Space for toggling layer visibility
* Migrate properties set on tile collision layer to the tile ([#1912](https://github.com/bjorn/tiled/issues/1912))
* Don't reset stamp brush state when pressing Alt
* Automapping: Apply rules to selected area when there is one
* macOS: Fixed eye/lock icon display in Layers view
* Windows and Linux: Updated builds to Qt 5.10.1
* Linux: Indicate Tiled can open multiple files at once in desktop file
* Lowered the minimum supported version of Qt to 5.5

### Snap Package

For Linux users there is now a [Tiled snap package][snap] available, which provides a convenient way of installing Tiled and keeping it up to date. By default it will install the latest stable version of Tiled, but you can also choose to install the latest development version using the "edge" channel. At some point I hope to automatically publish the development snapshots to the "beta" channel ([#1920][2]).

### Current Development

Meanwhile, development also continues towards Tiled 1.2. The latest development snapshots have added support for [multi-layer editing][3] as well as further improvements to [polygon editing][4]. And soon I expect to merge the initial support for [showing adjacent maps][5].

Many thanks to all who submitted bug reports, as well as to those who [support me financially](https://www.patreon.com/bjorn) so that I can keep up Tiled development and maintenance!

[1]: {{ site.baseurl }}{% post_url 2018-03-06-tiled-1-1-3-released %}
[2]: https://github.com/bjorn/tiled/issues/1920
[3]: https://thorbjorn.itch.io/tiled/devlog/24167/multi-layer-editing-and-other-stuff
[4]: https://thorbjorn.itch.io/tiled/devlog/26226/extend-and-join-polylines-with-ease
[5]: https://github.com/bjorn/tiled/issues/1669#issuecomment-374933393
[snap]: https://snapcraft.io/tiled
