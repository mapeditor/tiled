---
layout: post
title: Tiled 1.9 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release comes with major improvements to Automapping, custom dialogs in scripts, a simplified project setup and a unified custom type system, among many other improvements!

### Let Automapping Work For You

[Automapping][], or pattern-based tile placement, was originally introduced in [Tiled 0.6][] to simplify editing of maps for [The Mana World][]. In this release the algorithm has been almost entirely rewritten, making it much faster, easier to set up and adding several new options.

![AutoMapping Demonstration by eishiya](/img/posts/2022-04-automapping-by-eishiya.gif)

_(a demonstration of AutoMapping by eishiya, where tile variations are chosen based on a "light" layer)_

#### Performance and Usability

As a result of optimizations and taking advantage of multi-core CPUs, applying Automapping rules is now 10-30x faster and generally no longer causes noticeable delays. In addition, the "AutoMap While Drawing" option no longer creates separate undo steps, instead seamlessly integrating any changes with the paint operation.

To apply a certain set of rules only to some of your maps, you can now use [filename filters][] in the ``rules.txt`` file.

#### Simplified Setup

The input and output region of each rule are now automatically determined. To still enable the matching of "empty tile" or "non-empty tile" and other special cases, a built-in "Automapping Rules Tileset" was introduced, which can be added to your rules map from the _Map_ menu. This tileset currently provides the following [special tiles][]:

![automap-tiles](/img/posts/2022-06-automap-tiles.svg)

From left to right, these are Empty, Ignore, NonEmpty, Other and Negate. It is also possible to set up your own tiles for these special cases.

#### Per-Rule Options

A number of [per-rule options][] can be set by placing a rectangle object on a "rule_options" layer that covers all the rules the options should apply to.

Any custom map, layer and object properties supported by Automapping are now displayed in the Properties view when it detects that the current map is an Automapping Rules map (the map needs to have at least one "input\*" and one "output\*" layer).

### Custom Scripted Dialogs

Thanks to [@dogboydog](https://github.com/dogboydog), scripted extensions can now build their own [custom dialogs][]. This can greatly simplify the configuration of extensions, which previously might have relied on multiple prompts or reading custom properties.

![custom-dialog](/img/posts/2022-06-custom-dialog.png)

Talking about scripts, it is now possible to trigger a script from the command-line using the new `--evaluate <file>` argument.

### Simplified Project Setup

The project-related actions were simplified, with _File > New > New Project_ replacing the _Project > Save Project As_ action, and the "Recent Projects" and "Close Project" actions moving to the File menu as well.

In addition, all project-related settings are now only available after creating or loading a project.

### Unified Custom Types

Object Types have been merged into the Property Types, now simply called "Custom Types". This means custom classes can now have a color and can be used as "object type". If your project refers to an object types file, these types will be automatically imported as custom classes. If you used globally stored object types before, they can be manually imported to your project.

In addition, the "Type" property previously available only for objects and tiles is now available for all data types as the new "Class" property. For consistency, this value is written out as "class" also for objects and tiles, but a project-wide compatibility option is provided to make it still write out as "type":

![Compatibility Version](/img/posts/2022-06-compatibility-version.png)

### New Tileset Options

The size at which all tiles in a tileset render can now be configured to be either the tile size (as before) or the map grid size. The latter is useful if you want to use tiles with a different resolution but scale them up or down to fit the grid.

When a tile is not rendered at its native size, it used to always get stretched. A new "Fill Mode" option was added to the tileset that can be either "Stretch" or "Preserve Aspect Ratio".

As a first step towards supporting sprite atlasses, the images in an Image Collection tileset can now refer to a sub-rectangle of their image. This sub-rectangle can be changed in the Tile Properties, or by scripts through the `Tile.imageRect` property.

## Changelog

Many other small improvements could not be mentioned, so check out the full changelog below.

*   Added option to ignore transparent pixels when selecting tile objects ([#1477](https://github.com/mapeditor/tiled/issues/1477))
*   Added support for sub-images in image collection tilesets ([#1008](https://github.com/mapeditor/tiled/issues/1008))
*   Added "Class" field to all data types, referring to a custom class
*   Added Tile Render Size and Fill Mode options to Tileset
*   Added %worldfile variable for custom commands (by Pixel-Nori, [#3352](https://github.com/mapeditor/tiled/pull/3352))
*   Added 'New Project' action, replacing 'Save Project As' ([#3279](https://github.com/mapeditor/tiled/issues/3279))
*   Added ability to load .tiled-session files from command-line
*   Merged Object Types with Property Types
*   Don't scale point objects with the zoom level ([#3356](https://github.com/mapeditor/tiled/issues/3356))
*   Take into account image layer content when determining visual map size ([#3386](https://github.com/mapeditor/tiled/issues/3386))
*   Scripting: Added Dialog API for building custom UI (by tileboydog, [#3384](https://github.com/mapeditor/tiled/pull/3384))
*   Scripting: Added -e,--evaluate to run a script from command-line
*   Scripting: Added Tool.toolBarActions property ([#3318](https://github.com/mapeditor/tiled/issues/3318))
*   Scripting: Added Tileset.columnCount property
*   Scripting: Added ImageLayer.image property
*   Scripting: Added access to selected terrain in tileset editor
*   AutoMapping: Applying rules is now 10-30x faster
*   AutoMapping: Explicit "regions" layers are no longer needed and have been deprecated ([#1918](https://github.com/mapeditor/tiled/issues/1918))
*   AutoMapping: "AutoMap While Drawing" no longer creates separate undo steps ([#2166](https://github.com/mapeditor/tiled/issues/2166))
*   AutoMapping: Custom tiles can now match "Empty", "Non-Empty" and "Other" tiles through a "MatchType" property ([#3100](https://github.com/mapeditor/tiled/issues/3100))
*   AutoMapping: A custom tile with "MatchType" set to "Negate" can be used instead of "inputnot" layers
*   AutoMapping: Added built-in tileset with these custom rule tiles
*   AutoMapping: Added a number of per-rule options which can be set using rectangle objects
*   AutoMapping: Erase tiles by placing tiles with "MatchType" set to "Empty" on output layers ([#3100](https://github.com/mapeditor/tiled/issues/3100))
*   AutoMapping: Accumulate touched layers in AutoMap While Drawing ([#3313](https://github.com/mapeditor/tiled/issues/3313))
*   AutoMapping: Support map name filters in rules.txt ([#3014](https://github.com/mapeditor/tiled/issues/3014))
*   AutoMapping: Show relevant custom properties when a rules map is detected
*   Optimized rendering of tinted layers by caching tinted images
*   tmxrasterizer: Added options to hide certain layer types ([#3343](https://github.com/mapeditor/tiled/issues/3343))
*   Raised minimum supported Qt version from 5.6 to 5.12 (drops Windows XP support)
*   Raised minimum C++ version to C++17
*   Removed qmake project files (only Qbs supported now)
*   macOS: Fixed layout of Custom Types Editor when using native style
*   AppImage: Updated to Sentry 0.4.18
*   Python plugin: Now built against Python 3.8 on Windows and Linux
*   Updated Bulgarian, Czech, French and Russian translations

_In contrast to the [Tiled 1.9 Release Candidate][], these builds are once again based on Qt 5.15. There is still an additional compatibility release for macOS 10.12, built against Qt 5.12._

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by almost 300 [patrons][Patreon] and
[sponsors][sponsors] supporting me on a monthly basis as well as many people
choosing to pay for [Tiled on itch.io][Itch] and some who donated through
[Liberapay][Liberapay].

Your donations enable me to work full-time on Tiled! If you're not donating
yet, please consider [setting up a small monthly donation][donate] to keep
this sustainable. Let's make this tool even better!

[Patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Liberapay]: https://liberapay.com/Tiled/
[Automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[Tiled 1.9 Release Candidate]: https://www.mapeditor.org/2022/06/22/tiled-1-9-release-candidate.html
[The Mana World]: https://www.themanaworld.org/
[Tiled 0.6]: https://www.mapeditor.org/2011/01/26/tiled-qt-060-released.html
[special tiles]: https://doc.mapeditor.org/en/stable/manual/automapping/#matching-special-cases
[rules file]: https://doc.mapeditor.org/en/stable/manual/automapping/#setting-up-the-rules-file
[filename filters]: https://doc.mapeditor.org/en/stable/manual/automapping/#setting-up-the-rules-file
[per-rule options]: https://doc.mapeditor.org/en/stable/manual/automapping/#object-properties
[custom dialogs]: https://doc.mapeditor.org/docs/scripting/classes/Dialog.html
