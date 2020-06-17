---
layout: post
title: Tiled 1.3 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

It's been over a year after the release of [Tiled 1.2][tiled-1-2] and a lot has happened since then. I'm happy to announce the immediately availability of Tiled 1.3 "Make It Yours", extensible with JavaScript and including many new helpful features.

### Scripted Extensions

The biggest change in this release is the introduction of [the scripting API](https://doc.mapeditor.org/en/latest/reference/scripting/), which allows you to extend the functionality of Tiled with JavaScript. Scripts can implement custom actions, custom editing tools and add support for additional map or tileset formats.

Almost everything that can be modified through the UI can be changed through a script as well. Scripts can also connect to certain events to automate actions, for example on loading or saving an asset. Any changes made by scripts automatically create appropriate undo commands, which can be grouped together using the [`Asset.macro`](https://doc.mapeditor.org/en/latest/reference/scripting/#id4) function.

Scripts can be grouped in folders to make it easier to share them with others, for example by cloning a git repository into the [extensions folder](https://doc.mapeditor.org/en/latest/reference/scripting/#scripted-extensions). Tiled automatically reloads the scripts when it detects a change to any loaded script file.

### Issues View

A new "Issues" view was added, where reported warnings and errors are displayed persistently and can be searched. Many of the issues reported here can also be double-clicked to jump to the relevant location for fixing the issue. The error and warning counts are displayed on the status bar to make sure they don't go unnoticed.

<a href="/img/posts/2019-11-issues-view.png" class="thumbnail" style="margin: 20px 40px;">
  <img src="/img/posts/2019-11-issues-view.png">
</a>

While Tiled may encounter many issues of itself, for example when AutoMapping or exporting to certain formats, issues can also be reported through the scripting API. This could be used to add sanity checks to make sure your map won't trigger an error in your game.

### Configurable Keyboard Shortcuts

The keyboard shortcuts of most actions can now be changed from the new Keyboard tab in the Preferences. Shortcut schemes can be imported and exported and potential conflicts are marked in red.

### New Update Notifications

Tiled now features a native up-to-date check, which displays an unobtrusive notification in the status bar whenever it detects that a newer version is available. This replaces the previously used 3rd-party solutions Sparkle and WinSparkle. For those who don't want it, it can be turned off in the Preferences, in which case you can still manually check for a new version by opening the "About Tiled" dialog.

The new system does not automatically download & install the new package. For automatic updates, I recommend installing Tiled through [the itch.io app](https://itch.io/app).

### Many Other Improvements

<center>
<a href="/img/posts/2019-11-view-tile-collisions.png" class="thumbnail" style="margin: 10px; display: inline-block;">
	<img src="/img/posts/2019-11-view-tile-collisions.png" style="height: 170px;">
</a>
<a href="/img/posts/2019-11-dynamic-wrapping.png" class="thumbnail" style="margin: 10px; display: inline-block;">
	<img src="/img/posts/2019-11-dynamic-wrapping.png" style="height: 170px;">
</a>
</center>

It's been a long time since the last new features release, so it's hard to summarize the large amount of improvements and new features. To name a few, there is a new option to [display tile collision shapes on the map](https://thorbjorn.itch.io/tiled/devlog/77202/show-tile-collision-shapes-on-the-map), you can [switch layers by Ctrl+clicking the map view](https://thorbjorn.itch.io/tiled/devlog/89540/select-tile-layers-by-clicking-them), a [dynamic wrapping mode](https://thorbjorn.itch.io/tiled/devlog/100642/dynamic-wrapping-tileset-view) was added to the Tilesets view, the *tmxrasterizer* learned how to render entire worlds, the last export parameters are now remembered and export can be automatically triggered when saving, newly opened maps are now scaled to fit the view, etc.

### Changelog

Many smaller improvements and bugfixes could not be mentioned. Here is the full list:

- Added support for extending Tiled with JavaScript ([#949](https://github.com/bjorn/tiled/issues/949))
- Added error and warning counts to the status bar
- Added Issues view where you can see warnings and errors and interact with them
- Added configuration of keyboard shortcuts ([#215](https://github.com/bjorn/tiled/issues/215))
- Added status bar notification on new releases (replacing Sparkle and WinSparkle)
- Added option to show tile collision shapes on the map ([#799](https://github.com/bjorn/tiled/issues/799))
- Added switching current layer with Ctrl + Right Click in map view
- Added search filter to the Objects view ([#1467](https://github.com/bjorn/tiled/issues/1467))
- Added icons to objects in the Objects view
- Added dynamic wrapping mode to the tileset view ([#1241](https://github.com/bjorn/tiled/issues/1241))
- Added a \*.world file filter when opening a world file
- Added support for .world files in tmxrasterizer (by Samuel Magnan, [#2067](https://github.com/bjorn/tiled/pull/2067))
- Added synchronization of selected layers and tileset when switching between maps in a world (by JustinZhengBC, [#2087](https://github.com/bjorn/tiled/pull/2087))
- Added actions to show/hide and lock/unlock the selected layers
- Added toggle button for “Highlight Current Layer” action
- Added custom output chunk size option to map properties (by Markus, [#2130](https://github.com/bjorn/tiled/pull/2130))
- Added support for Zstandard compression and configurable compression level (with BRULE Herman and Michael de Lang, [#1888](https://github.com/bjorn/tiled/pull/1888))
- Added option to minimize output on export ([#944](https://github.com/bjorn/tiled/issues/944))
- Added export to Defold .collection files (by CodeSpartan, [#2084](https://github.com/bjorn/tiled/pull/2084))
- Added a warning when custom file properties point to non-existing files ([#2080](https://github.com/bjorn/tiled/issues/2080))
- Added shortcuts for next/previous tileset ([#1238](https://github.com/bjorn/tiled/issues/1238))
- Added saving of the last export target and format in the map/tileset file ([#1610](https://github.com/bjorn/tiled/issues/1610))
- Added option to repeat the last export on save ([#1610](https://github.com/bjorn/tiled/issues/1610))
- Added Fit Map in View action (by Mateo de Mayo, [#2206](https://github.com/bjorn/tiled/issues/2206))
- Tile Collision Editor: Added objects list view
- Changed the Type property from a text box to an editable combo box ([#823](https://github.com/bjorn/tiled/issues/823))
- Changed animation preview to follow zoom factor for tiles (by Ruslan Gainutdinov, [#2050](https://github.com/bjorn/tiled/pull/2050))
- Changed the shortcut for AutoMap from A to Ctrl+M
- AutoMapping: Added “OverflowBorder” and “WrapBorder” options (by João Baptista de Paula e Silva, [#2141](https://github.com/bjorn/tiled/pull/2141))
- AutoMapping: Allow any supported map format to be used for rule maps
- Python plugin: Added support for loading external tileset files (by Ruin0x11, [#2085](https://github.com/bjorn/tiled/pull/2085))
- Python plugin: Added Tile.type() and MapObject.effectiveType() (by Ruin0x11, [#2124](https://github.com/bjorn/tiled/pull/2124))
- Python plugin: Added Object.propertyType() (by Ruin0x11, [#2125](https://github.com/bjorn/tiled/pull/2125))
- Python plugin: Added Tileset.sharedPointer() function ([#2191](https://github.com/bjorn/tiled/issues/2191))
- tmxrasterizer: Load plugins to support additional map formats (by Nathan Tolbert, [#2152](https://github.com/bjorn/tiled/pull/2152))
- tmxrasterizer: Added rendering of object layers (by oncer, [#2187](https://github.com/bjorn/tiled/pull/2187))
- Fixed missing native styles when compiled against Qt 5.10 or later ([#1977](https://github.com/bjorn/tiled/issues/1977))
- Fixed file change notifications no longer triggering when file was replaced (by Nathan Tolbert, [#2158](https://github.com/bjorn/tiled/pull/2158))
- Fixed layer IDs getting re-assigned when resizing the map ([#2160](https://github.com/bjorn/tiled/issues/2160))
- Fixed performance issues when switching to a new map in a world with many maps (by Simon Parzer, [#2159](https://github.com/bjorn/tiled/pull/2159))
- Fixed restoring of expanded group layers in Objects view
- Fixed tileset view to keep position at mouse stable when zooming ([#2039](https://github.com/bjorn/tiled/issues/2039))
- libtiled-java: Added support for image layers and flipped tiles (by Sergey Savchuk, [#2006](https://github.com/bjorn/tiled/pull/2006))
- libtiled-java: Optimized map reader and fixed path separator issues (by Pavel Bondoronok, [#2006](https://github.com/bjorn/tiled/pull/2006))
- Updated builds on all platforms to Qt 5.12 (except snap release)
- Raised minimum supported Qt version from 5.5 to 5.6
- Raised minimum supported macOS version from 10.7 to 10.12
- Removed option to include a DTD in the saved files
- Removed the automappingconverter tool
- snap: Updated from Ubuntu 16.04 to 18.04 (core18, Qt 5.9)
- Updated Chinese, Portuguese (Portugal), Turkish and Ukrainian translations

Thanks to everybody who contributed to this release with bug reports, suggestions, patches or translation updates!

## A Look Ahead

[The roadmap][roadmap] for Tiled 1.4 mentions the following major features:

* Project Support ([#1665](https://github.com/bjorn/tiled/issues/1665)), dropped from Tiled 1.3
* Connecting Objects ([#707](https://github.com/bjorn/tiled/issues/707))

With 1.3 done I plan to go full steam ahead on support for projects, which was dropped from 1.3 to avoid further delaying the release. I scheduled the 1.4 release in Q1 2020 with the aim to reduce the time between feature releases but feel confident that I can also add support for connections between objects.

Other than those items, I expect we'll also want to make further extensions to the scripting API, to make it even more powerful and add further UI interactions, which are currently very limited. But first I want to see what people will do with the scripts and what functionality they may be missing, so be sure to provide feedback!

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by about 300 [patrons][patreon] and [sponsors][sponsors] supporting me on a monthly basis as well as many people choosing to pay for [Tiled on itch.io][itch] and some who donated through [Liberapay][liberapay]. To ensure I will be able to keep developing Tiled at this pace, please [set up a small monthly donation][donate]!

Your donation primarily enables me to work 2 full days/week on Tiled. With additional funds I can spend more time on Tiled every once in a while. Let's make this tool even better!

[tiled-1-2]: {{ site.baseurl }}{% post_url 2018-09-18-tiled-1-2-0-released %}
[roadmap]: https://github.com/bjorn/tiled/wiki/Roadmap
[patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[itch]: https://thorbjorn.itch.io/tiled
[liberapay]: https://liberapay.com/Tiled/
