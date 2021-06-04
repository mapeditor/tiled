---
layout: post
title: Tiled 1.5 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Tiled 1.5 brings a large amount of improvements, including a more powerful
Terrain Brush, parallax scrolling and GameMaker Studio 2.3 export!

### Unified Wang and Terrain tools

Since the addition of Wang tiles in [Tiled 1.1][Tiled-1-1] there were two very
similar tools, each with their own set of features and limitations. Now, these
tools have been unified into [one powerful tool][terrain] for automatic corner
and edge-based tile placement. The new "Terrain Brush" even supports the [blob
tileset][blob], a much requested feature that was previously not supported by
either tool.

#### Painting with an edge-based tileset

<div style="text-align: center;">
<img src="/img/posts/2021-03-edge-set.png" style="height: 180px; margin: 0.5em;">
<img src="/img/posts/2021-03-edge-painting.gif" style="height: 180px; margin: 0.5em; border-radius: 5px;">
</div>

#### Painting with a blob tileset

<div style="text-align: center;">
<img src="/img/posts/2021-03-blob-set.png" style="height: 180px; margin: 0.5em;">
<img src="/img/posts/2021-03-blob-painting.gif" style="height: 180px; margin: 0.5em; border-radius: 5px;">
</div>

The unified tool also supports [flipping or rotating tiles][transformations],
either to create versions that otherwise don't exist in a tileset or for more
variation.

***Compatibility Note:** The [storage format][docs-wangset] has changed such
that older Tiled versions will not be able to read terrain information saved
by Tiled 1.5.*

### Parallax Scrolling Factor

While custom parallax scrolling factors could previously be set using custom
properties, this way it is not possible to see a level like it would appear in
the game while editing, which can be important for prop placement. Now Tiled
features a built-in property for controlling the [parallax scrolling
factor][docs-parallax] per layer, along with an optional live preview.

<iframe class="video" width="560" height="315" src="https://www.youtube-nocookie.com/embed/s6Yb5v96l04" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

*The level shown above is from the game [Super Catboy][Super Catboy], still in
development.*

### New Export Formats

<img src="/img/posts/2021-03-gamemaker-studio.png" style="width: 192px; margin: 0; float: right;">

Tiled can now [export maps to GameMaker Studio 2.3][GameMaker Export], which
uses an entirely different file format than the still supported GameMaker:
Studio 1.4. The export even covers tiles placed on isometric and hexagonal
grids, which are normally not supported by GameMaker. It does this by
exporting the tile layer to an Asset layer.

<img src="/img/posts/2021-03-maptool.webp" style="width: 64px; margin: 0; margin-left: 1em; margin-bottom: 1em; float: right;">

Another tool you can now export maps to is RPTools [MapTool][MapTool], though
for now only tile layers using image collection tilesets are supported. It
does support tile flipping and rotation, which is needed when using "ProBono's
6x6 Mega Tile Pack V2 - Grid50" included with MapTool. A terrain set
definition for this tileset is available at
[cpetig/tilesets][cpetig/tilesets].

### Customization Options

It is now possible to make the "Select Object" tool prefer picking objects
from the selected layer, or to only pick objects from selected layers. This
can really help when dealing with overlapping objects or when group-selecting
multiple objects where the region includes objects from other layers that you
don't want to select. When "Highlight Current Layer" is enabled, objects from
the selected layers are automatically preferred.

![New Options](/img/posts/2021-03-new-options.png)

The panning behavior can now be customized. For the middle mouse button
there's an option to use auto-scrolling (used in the video demonstrating
parallax above). When using the arrow keys, the view will now scroll smoothly
by default, instead of in steps based on key repetition.

### Scripting

A few major additions have been made to the scripting API, which can now
[load, save and modify images][script-image] and [launch external
processes][script-process]. Scripted actions can now also be added to most
context menus and they get a default icon to make them stand out from the
built-in actions.

## Changelog

Many small improvements could not be mentioned above. Here's the full summary of the changes.

* Unified Wang and Terrain tools (backwards incompatible change!)
* Added support for a per-layer parallax scrolling factor ([#2951](https://github.com/mapeditor/tiled/pull/2951))
* Added export to GameMaker Studio 2.3 ([#1642](https://github.com/mapeditor/tiled/issues/1642))
* Added option to change object selection behavior ([#2865](https://github.com/mapeditor/tiled/pull/2865))
* Added Monospace option to the multi-line text editor
* Added option to auto-scroll on middle click
* Added smooth scrolling option for arrow keys
* Added a 'Convert to Polygon' action for rectangle objects
* Added support for drawing with a blob tileset
* Added 'Duplicate Terrain Set' action
* Added Terrain Set type (Corner, Edge or Mixed)
* Added support for rotating and flipping Terrain tiles (by Christof Petig, [#2912](https://github.com/mapeditor/tiled/pull/2912))
* Added support for exporting to RPTools [MapTool](https://www.rptools.net/toolbox/maptool/) RpMap files (by Christof Petig, [#2926](https://github.com/mapeditor/tiled/pull/2926))
* Added Ctrl+Shift to toggle Snap to Fine Grid (by sverx, [#2895](https://github.com/bjorn/tiled/pull/2895))
* Eraser: Added Shift to erase on all layers (by Michael Aganier, [#2897](https://github.com/bjorn/tiled/pull/2897))
* Automatically add .world extension to new World files
* Shape Fill Tool now displays the size of the current shape ([#2808](https://github.com/mapeditor/tiled/issues/2808))
* Tile Collision Editor: Added action to add an auto-detected bounding box collision rectangle (by Robin Macharg, [#1960](https://github.com/bjorn/tiled/pull/1960))
* Tile Collision Editor: Added context menu action to copy selected collision objects to all other selected tiles (by Robin Macharg, [#1960](https://github.com/bjorn/tiled/pull/1960))
* Tilesets view: Added "Edit Tileset" action to tab context menu
* Tilesets view: Added "Add External Tileset" action to tilesets menu
* Scripting: Added initial API for creating and modifying Terrain Sets
* Scripting: Added API for working with images ([#2787](https://github.com/mapeditor/tiled/pull/2787))
* Scripting: Added API for launching other processes ([#2783](https://github.com/mapeditor/tiled/issues/2783))
* Scripting: Added MapView.center property
* Scripting: Added missing Layer.id and Layer.parentLayer properties
* Scripting: Enable extending most context menus
* Scripting: Fixed reset of file formats on script reload ([#2911](https://github.com/mapeditor/tiled/issues/2911))
* Scripting: Fixed missing GroupLayer and ImageLayer constructors
* Scripting: Added default icon for scripted actions
* Enabled high-DPI scaling on Linux and changed rounding policy
* Remember last file dialog locations in the session instead of globally
* Fixed loading extension path from project config (by Peter Ruibal, [#2956](https://github.com/mapeditor/tiled/pull/2956))
* Fixed performance issues when using a lot of custom properties
* Fixed storing template instance size when overriding the tile ([#2889](https://github.com/mapeditor/tiled/issues/2889))
* Fixed removal of object reference arrow when deleting target object ([#2944](https://github.com/mapeditor/tiled/issues/2944))
* Fixed updating of object references when layer visibility changes
* Fixed map positioning issues in the World Tool ([#2970](https://github.com/mapeditor/tiled/issues/2970))
* Fixed handling of Shift modifiers in Bucket and Shape Fill tools ([#2883](https://github.com/mapeditor/tiled/issues/2883))
* Fixed scrolling speed in Tileset view when holding Ctrl
* Fixed issue causing export.target to get written out as "."
* Fixed "Repeat last export on save" when using Save All ([#2969](https://github.com/mapeditor/tiled/issues/2969))
* Fixed interaction shape for rectangle objects to be more precise ([#2999](https://github.com/mapeditor/tiled/issues/2999))
* Fixed "AutoMap While Drawing" not applying when using Cut/Delete
* Fixed path in AutoMap error message when rules file doesn't exist
* Lua plugin: Don't embed external tilesets, unless enabled as export option ([#2120](https://github.com/mapeditor/tiled/issues/2120))
* Python plugin: Added missing values to MapObject.Shape enum ([#2898](https://github.com/bjorn/tiled/issues/2898))
* Python plugin: Fixed linking issue when compiling against Python 3.8
* CSV plugin: Include flipping flags in exported tile IDs
* GMX plugin: Take tile object alignment into account
* Linux: "Open Containing Folder" action now also selects the file
* libtiled-java: Many updates (by Henri Viitanen, [#2207](https://github.com/bjorn/tiled/pull/2207))
* Ported Tiled to Qt 6 (releases still use 5.15 for now)
* Updated Bulgarian, Chinese (Simplified), Czech, Finnish, French, Portuguese, Portuguese (Portugal), Russian, Swedish and Turkish translations

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by over 300 [patrons][Patreon] and
[sponsors][sponsors] supporting me on a monthly basis as well as many people
choosing to pay for [Tiled on itch.io][Itch] and some who donated through
[Liberapay][Liberapay]. To ensure I will be able to keep developing Tiled at
this pace, please [set up a small monthly donation][donate]!

Your donation primarily enables me to work 2 full days/week on Tiled. With
additional funds I can spend more days on Tiled. Let's make this tool even
better!

[Tiled-1-1]: {{ site.baseurl }}{% post_url 2018-01-03-tiled-1-1-0-released %}
[Tiled-1-4]: {{ site.baseurl }}{% post_url 2020-06-20-tiled-1-4-0-released %}
[Patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Liberapay]: https://liberapay.com/Tiled/
[packages-workflow]: https://github.com/mapeditor/tiled/actions/workflows/packages.yml
[MapTool]: https://www.rptools.net/toolbox/maptool/
[GameMaker Export]: https://doc.mapeditor.org/en/latest/manual/export-yy/
[terrain]: https://doc.mapeditor.org/en/latest/manual/terrain/
[blob]: http://www.cr31.co.uk/stagecast/wang/blob.html
[transformations]: https://doc.mapeditor.org/en/latest/manual/terrain/#tile-transformations
[Super Catboy]: https://store.steampowered.com/app/1376910/Super_Catboy/
[MapTool]: https://www.rptools.net/toolbox/maptool/
[cpetig/tilesets]: https://github.com/cpetig/tilesets
[docs-wangset]: https://doc.mapeditor.org/en/latest/reference/tmx-map-format/#wangset
[docs-parallax]: https://doc.mapeditor.org/en/latest/manual/layers/#parallax-factor
[script-image]: https://doc.mapeditor.org/en/latest/reference/scripting/#image
[script-process]: https://doc.mapeditor.org/en/latest/reference/scripting/#process
