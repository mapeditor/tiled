---
layout: post
title: Tiled 1.2 Release Candidate released
author: Thorbjørn Lindeijer
tags: release
---

The Tiled 1.2 Release Candidate is now available as the [latest development snapshot](https://thorbjorn.itch.io/tiled), bringing fixes, last-minute features as well as some updated translations. Please give it a go and provide feedback! I expect to make the final release within a week from now.

## Changelog

Here are the changes since the [Tiled 1.2 Beta][tiled-1-2-beta]:

### Gauthier Billot

* Display full path in recent files menu tool tips ([#1992](https://github.com/bjorn/tiled/pull/1992))

### Kyle Filz

* Lua plugin: Include properties from templates ([#1901](https://github.com/bjorn/tiled/pull/1901))

### Thorbjørn Lindeijer

* tBIN plugin: Several improvements ([#1985](https://github.com/bjorn/tiled/issues/1985))
* AutoMapping: Changed matching outside of map boundaries (added [MatchOutsideMap][1] property)
* Properties view now applies changes to all selected layers
* Made "Merge Layer Down" work on all selected layers
* Made "Offset Layers" tool work on all selected layers
* Make Ctrl+Q work for quitting on all platforms ([#1998](https://github.com/bjorn/tiled/issues/1998))
* Qbs: Fixed warning messages in Python probe
* Fixed reading of infinite maps that don't use chunked layer data
* Fixed randomizing of terrain, Wang tiles and stamp variations ([#1949](https://github.com/bjorn/tiled/issues/1949))
* Fixed tilesets getting added to maps when they shouldn't be ([#2002](https://github.com/bjorn/tiled/issues/2002))

### Translation Updates

* Norwegian Bokmål (by Allan Nordhøy)
* German (by Erik Schilling)
* Portuguese (Portugal) (by João Lopes)
* Polish (by Michał Wawrzynowicz)
* Russian (by Rafael Osipov)
* French (by Rhenaud Dubois)
* Dutch (by Thorbjørn Lindeijer)
* Turkish (by monolifed)
* Bulgarian (by Любомир Василев)

Thanks to everybody who is helping by reporting issues, providing patches or updating translations, to make sure that Tiled 1.2 will be a great release!

[tiled-1-2-beta]: {{ site.baseurl }}{% post_url 2018-08-22-tiled-1-2-beta-released %}
[1]: http://docs.mapeditor.org/en/latest/manual/automapping/#map-properties
