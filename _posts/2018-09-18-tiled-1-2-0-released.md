---
layout: post
title: Tiled 1.2 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
image: /img/posts/2018-09-world-view.png
---

The focus with Tiled 1.2 was generally on improving productivity. It
brings improvements to the object tools, introduces multi-layer
selection along with multi-tile layer editing, makes it possible to
quickly jump between maps in the same world, adds several export
options, and includes many smaller changes and fixes.

### Improved Object Tools

The most notable change will probably be the highlighting of the hovered
object, which really helps to know what you will be interacting with
when clicking. And when placing new objects, a preview of the object is
now always visible, consistent with the tile placement tools.

Polygon editing was improved in many ways. You can now click segments to
drag or select the points at both ends, or double-click them to split
the segment at that location. Quickly switch to the Edit Polygon tool by
double-clicking a polygon object, and switch back to the general
selection tool by pressing Escape. If there is a selection, Escape
clears it. And if you are currently making a change, Escape (or right-
click) aborts it.

<iframe class="thumbnail" width="560" height="315" src="https://www.youtube-nocookie.com/embed/elws59R9CrM?rel=0&amp;showinfo=0" frameborder="0" gesture="media" allow="encrypted-media" allowfullscreen style="margin: 20px auto; display: block; box-sizing: content-box;"></iframe>

What the above video doesn't demonstrate, is that you can now also
extend a polyline, join two polylines or turn a polyline into a polygon
by closing it.

### Multi-Layer Selection

Now you can finally select multiple layers at the same time! All
relevant actions have been adjusted to apply to all selected layers, and
you can change properties on all of them at once.

Probably best of all, the tile layer tools now work with multi-layer
stamps and you can copy/paste multiple layers at the same time as well!
The following video demonstrates the basic functionality of the tile
stamp tool working with multiple layers:

<iframe class="thumbnail" width="560" height="315" src="https://www.youtube-nocookie.com/embed/pu-yShBRCqM?rel=0&amp;showinfo=0" frameborder="0" gesture="media" allow="encrypted-media" allowfullscreen style="margin: 20px auto; display: block; box-sizing: content-box;"></iframe>

### Multi-Map World View

When your game features a large world, which you have split up in
multiple maps, it can be difficult to make sure these maps connect
properly and it can be time-consuming to search for the right map file.
This scenario is now directly supported by the new multi-map world view!

<a href="/img/posts/2018-09-world-view.png" class="thumbnail" style="margin: 20px 10px;">
  <img src="/img/posts/2018-09-world-view.png">
</a>

The above screenshot shows several maps from [Alchemic Cutie][alchemic].
Their world is split up for performance reasons as well as to allow
multiple developers to change the world at the same time.

All maps in the same world will be loaded together, and it is possible
to quickly switch between them while editing by simply clicking them.
For now, the world file needs to be written by hand, as defined in
[the documentation][worlds].

### New Export Options

Several features, like [tile property inheritance][inheritance], [object
templates][templates] and external tilesets, exist to make life easier
on the content development side. However, they of course can make it
more complicated to incorporate Tiled maps in your game. Developers may
need to deal with loading and parsing additional files or implement the
same logic used in Tiled to resolve property values.

For this reason several Export Options were added to the Preferences:

* **Embed tilesets** - For when you are exporting to JSON and loading an external tileset is not desired
* **Detach templates** - If you can't or don't want to load the external template object files
* **Resolve object types and properties** - Stores effective object type and properties with each object

They are applied each time the current map or tileset is exported
(without affecting the map itself), and they are also available as
options when exporting using the command-line.

### Compatibility Notes

The [JSON map format][json] was simplified a bit, which will require
adjustments in the map reader. See [its changelog][json-changelog] for
more information.

The [Python plugin][python] was updated from Python 2 to Python 3, which will
generally require changes to any custom Python plugins. Unfortunately,
you also need to install the same minor Python version as the one Tiled
was compiled against to be able to use this plugin, which may be a
problem especially on Linux. In addition, the Python plugin is not
available on macOS for now until we find out how to make it work, since
macOS only ships with Python 2.7. We'll try to reach broader
compatibility in the future.

### Changelog

Many smaller improvements and bugfixes were not mentioned, so here is
the full list:

* Added multi-layer selection, including multi-layer tile layer editing
* Added support for multi-map worlds ([#1669](https://github.com/bjorn/tiled/issues/1669))
* Added ability to extend existing polylines (with Ketan Gupta, [#1683](https://github.com/bjorn/tiled/issues/1683))
* Added option to highlight the hovered object ([#1190](https://github.com/bjorn/tiled/issues/1190))
* Added news from website to the status bar ([#1898](https://github.com/bjorn/tiled/issues/1898))
* Added option to show object labels for hovered objects
* Added option to embed tilesets on export ([#1850](https://github.com/bjorn/tiled/issues/1850))
* Added option to detach templates on export ([#1850](https://github.com/bjorn/tiled/issues/1850))
* Added option to resolve object types and properties on export ([#1850](https://github.com/bjorn/tiled/issues/1850))
* Added Escape for switching to the Select Objects tool and for clearing the selection
* Added Escape to cancel the current action in all object layer tools
* Added double-click on polygon objects to switch to Edit Polygons tool
* Added interaction with segments for polygons, for selection and dragging
* Added double-clicking a polygon segment for inserting a new point at that location
* Added action to lock/unlock all other layers (by kralle333, [#1883](https://github.com/bjorn/tiled/issues/1883))
* Added \--export-tileset command line argument (by Josh Bramlett, [#1872](https://github.com/bjorn/tiled/issues/1872))
* Added unique persistent layer IDs ([#1892](https://github.com/bjorn/tiled/issues/1892))
* Added 'version' and 'tiledversion' to external tileset files
* Added full paths to Recent Files menu as tool tips (by Gauthier Billot, [#1992](https://github.com/bjorn/tiled/issues/1992))
* Create Object Tools: Show preview already on hover ([#537](https://github.com/bjorn/tiled/issues/537))
* Objects view: Only center view on object on press or activation
* Objects view: When clicking a layer, make it the current one (by kralle333, [#1931](https://github.com/bjorn/tiled/issues/1931))
* Unified the Create Polygon and Create Polyline tools
* JSON plugin: Made the JSON format easier to parse (by saeedakhter, [#1868](https://github.com/bjorn/tiled/issues/1868))
* Tile Collision Editor: Allowed using object templates
* Templates view: Don't allow hiding the template object
* Python plugin: Updated to Python 3 (by Samuli Tuomola)
* Python plugin: Fixed startup messages not appearing in debug console
* Python plugin: Fixed file change watching for main script files
* Lua plugin: Include properties from templates ([#1901](https://github.com/bjorn/tiled/issues/1901))
* Lua plugin: Include tileset column count in export (by Matt Drollette, [#1969](https://github.com/bjorn/tiled/issues/1969))
* tBIN plugin: Don't ignore objects that aren't perfectly aligned ([#1985](https://github.com/bjorn/tiled/issues/1985))
* tBIN plugin: Fixed "Unsupported property type" error for newly added float properties
* Automapping: Report error when no output layers are found
* AutoMapping: Changed matching outside of map boundaries and added '[MatchOutsideMap](MatchOutsideMap)' option
* Linux: Modernized the appstream file (by Patrick Griffis)
* libtiled: Allow qrc-based tileset images ([#1947](https://github.com/bjorn/tiled/issues/1947))
* libtiled-java: Fixed loading maps with multiple external tilesets
* Make Ctrl+Q work for quitting also on Windows ([#1998](https://github.com/bjorn/tiled/issues/1998))
* Fixed performance issue when deleting many objects ([#1972](https://github.com/bjorn/tiled/issues/1972))
* Fixed randomizing of terrain, Wang tiles and stamp variations ([#1949](https://github.com/bjorn/tiled/issues/1949))
* Fixed tilesets getting added to maps when they shouldn't be ([#2002](https://github.com/bjorn/tiled/issues/2002))
* Fixed issue with default font size in combination with custom family ([#1994](https://github.com/bjorn/tiled/issues/1994))
* Fixed the tile grid to render below labels, handles and selection indicators
* Fixed confirming overwrite when exporting a tileset
* Fixed reading of infinite maps that don't use chunked layer data
* Updated Bulgarian, Dutch, French, German, Norwegian Bokmål, Portuguese (Portugal) and Turkish translations

Thanks to everybody who contributed to this release with bug reports,
suggestions, patches or translation updates!

## A Look Ahead

[The roadmap][roadmap] for Tiled 1.3 mentions the following major
features:

* Project Support ([#1665](https://github.com/bjorn/tiled/issues/1665))
* Add script API / make it scriptable ([#949](https://github.com/bjorn/tiled/issues/949))
* Configurable keyboard shortcuts ([#215](https://github.com/bjorn/tiled/issues/215))

Support for projects will mostly make it easier to switch between
multiple projects, as well as provide quicker access to the resources
within your project. Adding scripting to Tiled will make it customizable
and extendable in many ways. These are features I've long wanted to get
around to, and I'm eager to start working on it.

If you're missing something that you could really use in your project,
let us know or consider contributing to Tiled!

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by [nearly 300 patrons][patreon]
supporting me on a monthly basis as well as many people choosing to pay
for [Tiled on itch.io][itch] and some who donated through
[Liberapay][liberapay]. To ensure I will be able to keep developing
Tiled at this pace, [please chip in][patron]!

Your donation primarily enables me to work 2 full days/week on Tiled.
With additional funds I can place bounties [on
BountySource][bountysource] to encourage other contributors, or I could
spend some additional days on Tiled every once in a while. There's a lot
of work in front of us, but your support makes it doable!

[worlds]: https://doc.mapeditor.org/en/latest/manual/worlds/
[inheritance]: https://doc.mapeditor.org/en/stable/manual/custom-properties/#tile-property-inheritance
[templates]: https://doc.mapeditor.org/en/stable/manual/using-templates/
[alchemic]: https://alchemiccutie.com/
[json]: https://doc.mapeditor.org/en/latest/reference/json-map-format/
[json-changelog]: https://doc.mapeditor.org/en/stable/reference/json-map-format/#changelog
[python]: https://doc.mapeditor.org/en/stable/manual/python/
[roadmap]: https://github.com/bjorn/tiled/wiki/Roadmap
[patreon]: https://www.patreon.com/bjorn
[patron]: https://www.patreon.com/bePatron?u=90066
[itch]: https://thorbjorn.itch.io/tiled
[liberapay]: https://liberapay.com/Tiled/
[bountysource]: https://www.bountysource.com/teams/tiled
[MatchOutsideMap]: http://docs.mapeditor.org/en/latest/manual/automapping/#map-properties
