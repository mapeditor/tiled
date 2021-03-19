---
layout: post
title: Tiled 1.1 released
author: Thorbjørn Lindeijer
tags: release
---

This release got a little out of hand. After reaching [the 1.0
milestone][1], I really wanted to make smaller incremental updates. For
several reasons, not the least our [successful participation][2] in the
Google Summer of Code, the amount of changes in this release is huge!
Below I'll try to highlight some of the most significant ones.

### Mapping Without Boundaries

When you start a new map, you may not know in advance how large it will
need to be. Of course, that's why we had the Resize Map action. Now
though, you can just select "Infinite" in the New Map dialog. This is
possible, because tile layers now only allocate memory for the parts
you're actually using. All the tools have been adjusted to work on
[infinite maps][infinite].

<iframe class="thumbnail" width="560" height="315" src="https://www.youtube-nocookie.com/embed/RUNW3QgaEgU?rel=0&amp;showinfo=0" frameborder="0" gesture="media" allow="encrypted-media" allowfullscreen style="margin: 20px auto; display: block; box-sizing: content-box;"></iframe>

On large maps, zooming in and out is an efficient way of navigating.
From now on the wheel zooms by default, without the need to hold
Control.

### Wang Tiles

<img src="/img/posts/wangtile.png" style="width: 32px; height: 32px; margin: 10px 20px; float: right;">

Long ago, Hao Wang introduced the concept of [Wang tiles][4]. A Wang
tile has a color associated with each of its four sides, such that in a
set of Wang tiles those colors determine which side fits to which other
sides. While mathematically interesting, we use this concept to automate
certain editing operations, like drawing of roads and advanced random
filling.

The existing [Terrain Brush][5] already provided similar features, but
[the Wang tiles][6] take the functionality a little further. When you
define the colors of your tile, you can choose to work with either the
corners or the sides (or both). This makes Wang tiles suitable for roads
and fences. In addition, all tile painting tools now have a Wang fill-
mode, which is like the random mode but takes into account the sides of
the tiles.

On the other hand, this feature is still new and the [Wang
Brush][wangbrush] is not yet as convenient as it could. Your feedback is
welcome!

### Object Templates

If your game is like most games, then your maps are filled with
repeating objects. Usually directly placing tiles from a tileset works
fine, but sometimes the objects carry a lot of custom properties or they
may have specific polygon shapes, such that your workflow may include a
lot of copy/pasting. And then what if later on you want to change
a property in all those copies? For this use-case, Tiled now supports
[object templates][templates].

<a href="/img/posts/2018-01-templates-overview.png" class="thumbnail" style="margin: 20px 10px;">
  <img src="/img/posts/2018-01-templates-overview.png">
</a>

An object template is stored in its own file and each instance of that
template on a map just refers to it. Instances can override properties
of the template, at which point that aspect is no longer affected by
changes to the template. Templates can be edited in a dedicated window
in the Templates view.

Related to objects, a new [point object][point] type was added that is
different from all other types in that it can't be rotated or resized.
This is useful for marking locations.

### Context Sensitive Toolbar

Most tools have additional options that are not immediately obvious. To
help with this, a context-sensitive tool bar was introduced that can
display tool-specific options and actions. It is currently implemented
for the tile painting and selection tools, hosting the drawing modes,
stamp transformation actions and selection operations.

<p style="text-align: center;">
<a href="/img/posts/2018-01-shape-fill-toolbar.png" class="thumbnail" style="display: inline-block; margin: 10px;">
  <img src="/img/posts/2018-01-shape-fill-toolbar.png">
</a>
<a href="/img/posts/2018-01-selection-toolbar.png" class="thumbnail" style="display: inline-block; margin: 10px;">
  <img src="/img/posts/2018-01-selection-toolbar.png">
</a>
</p>

The left version is shown when the new [Shape Fill][shapefill] tool is
selected, which provides a quick way to fill rectangles or ellipses.
Noteworthy is also that you can now capture a stamp from the map while
using any of the tile painting tools.

### New Exporting Options

Tilesets can now be exported as well, which allows them to be saved in
[Lua format][lua].

The [GameMaker: Studio 1.4 export][gmx] was improved. It is now possible
to set the scale, origin and creation code for instances. Also views can
now be defined in Tiled. Finally the default depth for tile layers now
matches the default of GameMaker.

Support was added for the [tBIN map format][tbin], which simplifies
the workflow of [modding Stardew Valley maps][sv].

### Major Updates to the Manual

The last thing I'll mention here is not a change to Tiled, but regards
the [User Manual][manual]. Since the last release, the manual has
transitioned to [Read the Docs][rtd], was translated from Markdown to
reStructuredText (now using [Sphinx][sphinx]) and was vastly expanded.

As you may have already noticed many of the links in this post went to
the manual. Apart from pages covering all the new features, new sections
have also been added about [editing tilesets][tileset], the [user
preferences][preferences], the [export formats][export] and about
writing [Python import/export scripts][python]. Also the pages about
[Automapping][automapping] and the [JSON format][json] have been ported
over from the wiki.

Going forward, we'll host multiple versions of the manual, so that
you're not confused by features developed for Tiled 1.2 while using 1.1.
Read the Docs also makes it easy to host translations of the manual. You
can help translate the manual (and Tiled itself) on [Weblate][weblate].
But that's quite a lot of text to translate!

### Changelog

Many improvements could not be mentioned, so here is the full list:

* Added support for infinite maps (by Ketan Gupta, [#260](https://github.com/bjorn/tiled/issues/260))
* Added support for Wang tiles and related tools (by Benjamin Trotter)
* Added support for reusable object templates (by Mohamed Thabet)
* Added working directory setting for custom commands (by Ketan Gupta, [#1580](https://github.com/bjorn/tiled/issues/1580))
* Added output of custom commands in Debug Console (by Ketan Gupta, [#1552](https://github.com/bjorn/tiled/issues/1552))
* Added autocrop action based on tile layers (by Ketan Gupta, [#642](https://github.com/bjorn/tiled/issues/642))
* Added tool bar with tool-specific actions and settings (by Ketan Gupta, [#1084](https://github.com/bjorn/tiled/issues/1084))
* Added shape fill tool for filling rectangles or circles (by Benjamin Trotter, [#1272](https://github.com/bjorn/tiled/issues/1272))
* Added option to lock/unlock a layer (by Ketan Gupta, [#734](https://github.com/bjorn/tiled/issues/734))
* Added .xml as possible file extension for TMX files
* Added keyboard shortcut for Save All (by Thomas ten Cate)
* Added actions to remove a segment from polygon or to split a polyline (by Ketan Gupta, [#1685](https://github.com/bjorn/tiled/issues/1685))
* Added icon for animation editor in the tileset editor (by Ketan Gupta, [#1706](https://github.com/bjorn/tiled/issues/1706))
* Added display of flip bits for hovered tile in status bar ([#1707](https://github.com/bjorn/tiled/issues/1707))
* Added ability to capture tiles while using fill tools ([#790](https://github.com/bjorn/tiled/issues/790))
* Added option to have mouse wheel zoom by default ([#1472](https://github.com/bjorn/tiled/issues/1472))
* Added tab closing actions to context menu, and close by middle-click (by Justin Jacobs, [#1720](https://github.com/bjorn/tiled/issues/1720))
* Added ability to reorder terrain types (by Justin Jacobs, [#1603](https://github.com/bjorn/tiled/issues/1603))
* Added a point object for marking locations (by Antoine Gersant, [#1325](https://github.com/bjorn/tiled/issues/1325))
* Added 'New Tileset' button when no tileset is opened (by Rhenaud Dubois, [#1789](https://github.com/bjorn/tiled/issues/1789))
* Added 'Open File' button when no file opened (by Rhenaud Dubois, [#1818](https://github.com/bjorn/tiled/issues/1818))
* Added support for custom input formats and TMX output to the --export-map command-line option
* Added island RPG example based on Beach tileset by finalbossblues
* Added file-related context menu actions to tileset tabs
* Added action to reset to default window layout (by Keshav Sharma, [#1794](https://github.com/bjorn/tiled/issues/1794))
* Added support for exporting tilesets, including to Lua format (by Conrad Mercer, [#1213](https://github.com/bjorn/tiled/issues/1213))
* Keep object types sorted alphabetically (by Antoine Gersant, [#1679](https://github.com/bjorn/tiled/issues/1679))
* Improved polygon node handles and drag behavior
* Fixed %executablepath variable for executables found in PATH ([#1648](https://github.com/bjorn/tiled/issues/1648))
* Fixed Delete key to delete selected polygon nodes when appropriate (by Ketan Gupta, [#1555](https://github.com/bjorn/tiled/issues/1555))
* Fixed Terrain Brush going wild in some scenarios ([#1632](https://github.com/bjorn/tiled/issues/1632))
* Fixed the "Embed in Map" checkbox to be persistent ([#1664](https://github.com/bjorn/tiled/issues/1664))
* Fixed crash when saving two new maps using the same file name ([#1734](https://github.com/bjorn/tiled/issues/1734))
* Fixed issues caused by paths not being cleaned ([#1713](https://github.com/bjorn/tiled/issues/1713))
* Fixed suggested file name for tilesets to match the tileset name (by killerasus, [#1783](https://github.com/bjorn/tiled/issues/1783))
* Fixed selection rectangle's shadow offset when zooming (by Antoine Gersant, [#1796](https://github.com/bjorn/tiled/issues/1796))
* Fixed save dialog to reopen after heeding the file extension warning (by Antoine Gersant, [#1782](https://github.com/bjorn/tiled/issues/1782))
* Fixed potential crash when zooming out too much ([#1824](https://github.com/bjorn/tiled/issues/1824))
* Fixed potential crash after deleting object or group layers
* Fixed Object Selection tool clearing selection on double-click
* Enabled building with Qbs on macOS, including the Python plugin (by Jake Petroules)
* Automapping: Don't fail if an input/inputnot layer isn't found
* Automapping: Added a "StrictEmpty" flag to input layers
* GMX plugin: Added support for defining views with objects (by William Taylor, [#1621](https://github.com/bjorn/tiled/issues/1621))
* GMX plugin: Added support for setting scale and origin for instances ([#1427](https://github.com/bjorn/tiled/issues/1427))
* GMX plugin: Added support for setting the creation code for instances and the map
* GMX plugin: Start counting default tile layer depth from 1000000 ([#1814](https://github.com/bjorn/tiled/issues/1814))
* tBIN plugin: Added read/write support for the tBIN map format (by Chase Warrington, [#1560](https://github.com/bjorn/tiled/issues/1560))
* libtiled-java: Generate classes from XSD, some fixes and build with Maven (by Mike Thomas, [#1637](https://github.com/bjorn/tiled/issues/1637))
* libtiled-java: Added support for manipulating non-consecutive tile IDs in a tileset (by Stéphane Seng)
* Python plugin: Adjusted example scripts to API changes (by spiiin, [#1769](https://github.com/bjorn/tiled/issues/1769))
* Flare plugin: Various changes (by Justin Jacobs, [#1781](https://github.com/bjorn/tiled/issues/1781))
* TMW plugin: Removed since it is no longer needed
* Updated Dutch, Bulgarian, English, French, German, Korean, Norwegian Bokmål, Spanish and Turkish translations

Thanks to everybody who contributed to this release with bug reports, suggestions or patches!

## A Look Ahead

While doing the final preparations for this release, I've already been
busy working on several highly requested features for the next release,
Tiled 1.2. These include:

* Ability to edit multiple tile layers at the same time (see [this demonstration video](https://www.youtube.com/watch?v=pu-yShBRCqM))
* Option to show multiple maps in the same view (see [screenshot](https://github.com/bjorn/tiled/issues/1669#issuecomment-352011751))
* Polygon editing improvements (some slipped into 1.1)

What else should be done? Help us decide by voicing your opinion or
maybe even contributing to the development!

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

Making all these improvements (and guiding others to contribute) was
only possible thanks to [over 200 patrons][patreon] supporting me on a
monthly basis as well as many people choosing to pay for [Tiled on
itch.io][itch] and some who donate through [Liberapay][liberapay]. To
ensure I will be able to keep developing Tiled at this pace, [please
chip in][patron]!

Your donation primarily enables me to work 2 full days/week on Tiled.
With additional funds I can place bounties [on
BountySource][bountysource] to encourage other contributors, and I would
like to rent a small office room so I can concentrate better on my work.


[1]: {{ site.baseurl }}{% post_url 2017-05-24-tiled-1-0-0-released %}
[2]: {{ site.baseurl }}{% post_url 2017-09-07-google-summer-of-code-2017-results %}
[infinite]: https://doc.mapeditor.org/en/latest/manual/using-infinite-maps/
[4]: https://en.wikipedia.org/wiki/Wang_tile
[5]: https://doc.mapeditor.org/en/latest/manual/using-the-terrain-tool/
[6]: https://doc.mapeditor.org/en/latest/manual/using-wang-tiles/
[wangbrush]: https://doc.mapeditor.org/en/latest/manual/editing-tile-layers/#wang-brush
[templates]: https://doc.mapeditor.org/en/latest/manual/using-templates/
[point]: https://doc.mapeditor.org/en/latest/manual/objects/#insert-point
[shapefill]: https://doc.mapeditor.org/en/latest/manual/editing-tile-layers/#shape-fill-tool
[lua]: https://doc.mapeditor.org/en/latest/manual/export/#lua
[gmx]: https://doc.mapeditor.org/en/latest/manual/export/#gamemaker-studio-1-4
[tbin]: https://doc.mapeditor.org/en/latest/manual/export/#tbin
[sv]: https://stardewvalleywiki.com/Modding:Maps
[manual]: https://doc.mapeditor.org/en/latest/
[rtd]: https://readthedocs.org/
[sphinx]: http://www.sphinx-doc.org/en/stable/
[tileset]: https://doc.mapeditor.org/en/latest/manual/editing-tilesets/
[export]: https://doc.mapeditor.org/en/latest/manual/export/
[preferences]: https://doc.mapeditor.org/en/latest/manual/preferences/
[python]: https://doc.mapeditor.org/en/latest/manual/python/
[automapping]: https://doc.mapeditor.org/en/latest/manual/automapping/
[json]: https://doc.mapeditor.org/en/latest/reference/json-map-format/
[weblate]: https://hosted.weblate.org/engage/tiled/
[patreon]: https://www.patreon.com/bjorn
[itch]: https://thorbjorn.itch.io/tiled
[liberapay]: https://liberapay.com/Tiled/
[patron]: https://www.patreon.com/bePatron?u=90066
[bountysource]: https://www.bountysource.com/teams/tiled
