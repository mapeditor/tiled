---
layout: post
title: Tiled 1.1.5 released
author: Thorbjørn Lindeijer
tags: release
---

The bugfixing continues in this release, to keep Tiled 1.1 from misbehaving as I try to make progress on the eventual release of Tiled 1.2. Unfortunately, one of the fixes was a regression introduced in [Tiled 1.1.4][1]. Sorry about that!

### Changelog

* Fixed erasing mode of the Terrain Brush (broken in 1.1.4)
* Fixed crash after editing a template
* Fixed rendering of eye/lock icons in Layers view (now for all platforms)
* Fixed object index when undoing Move Object to Layer action ([#1932](https://github.com/bjorn/tiled/issues/1932))
* Fixed shortcuts for flipping and rotating objects ([#1926](https://github.com/bjorn/tiled/issues/1926))
* Fixed dynamic retranslation of tools and tool actions
* Fixed possible crash when undoing/redoing Wang color changes
* Fixed handling of sub-properties in Object Type Editor ([#1936](https://github.com/bjorn/tiled/issues/1936))
* Fixed crash when deleting an object right before dragging it ([#1933](https://github.com/bjorn/tiled/issues/1933))
* Adjust Wang tile data when tileset column count changes ([#1851](https://github.com/bjorn/tiled/issues/1851))
* Improved fill behavior in case of selection on infinite map ([#1921](https://github.com/bjorn/tiled/issues/1921))
* Removed ability to hide tile collision objects ([#1929](https://github.com/bjorn/tiled/issues/1929))
* Remove tile collision layer along with the last object ([#1230](https://github.com/bjorn/tiled/issues/1230))
* JSON plugin: Made the reader more strict about object types ([#1922](https://github.com/bjorn/tiled/issues/1922))
* JSON plugin: Added support for Wang sets

Many thanks to all who submitted bug reports, as well as to those who [support me financially](https://www.patreon.com/bjorn) so that I can keep up Tiled development and maintenance!

[1]: {{ site.baseurl }}{% post_url 2018-03-28-tiled-1-1-4-released %}
