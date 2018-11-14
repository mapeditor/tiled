---
layout: post
title: Tiled 1.2.1 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This long-due patch release fixes several issues present in [Tiled 1.2.0][1],
so upgrading is recommended! The snapshot builds have been updated as
well and minor updates have been made to [the documentation][documentation].

### Changelog

* Fixed JSON templates not being visible in Templates view ([#2009](https://github.com/bjorn/tiled/issues/2009))
* Fixed Maps view to show all readable map formats
* Fixed crash when deleting a command using the context menu (by Robert Lewicki, [#2014](https://github.com/bjorn/tiled/issues/2014))
* Fixed crash after a world file failed to load
* Fixed Select None action to be enabled when there is any selection
* Fixed disappearing of tile types on export/import of a tileset ([#2023](https://github.com/bjorn/tiled/issues/2023))
* Fixed tool shortcuts when using Spanish translation
* Fixed saving of the "Justify" alignment option for text objects ([#2026](https://github.com/bjorn/tiled/issues/2026))
* Changed Cut, Copy and Delete actions to apply based on selected layer types
* Windows: Updated builds to Qt 5.9.7
* Updated Russian translation (by Rafael Osipov, [#2017](https://github.com/bjorn/tiled/pull/2017))

Thanks to everybody who contributed to this patch release!

### Current Development

I recently took [a one-month break][break] from Tiled development to
focus on renovating a house we'd like to move into soon. While that's
far from done, in the meantime I'm [back on Tiled development][back].
I've started looking into making Tiled scriptable, one of the major
features on [the roadmap][roadmap] for the next Tiled release.

[1]: {{ site.baseurl }}{% post_url 2018-09-18-tiled-1-2-0-released %}
[documentation]: https://doc.mapeditor.org/
[break]: https://www.patreon.com/posts/beta-release-1-2-21597744
[back]: https://www.patreon.com/posts/getting-back-to-22600061
[roadmap]: https://github.com/bjorn/tiled/wiki/Roadmap
