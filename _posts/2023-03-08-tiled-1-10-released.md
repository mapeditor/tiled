---
layout: post
title: Tiled 1.10 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Restored file format compatibility, quick action search, Godot 4 export and many scripting API additions are just some of the many small improvements in this release!

### Restored Compatibility with Tiled 1.8

In [Tiled 1.9][], the `type` attribute was renamed to `class` as part of [unifying custom types][], mostly for consistency reasons. This compatibility breakage continued to cause trouble months after that release, even with an option available in the Project Properties to save files using the Tiled 1.8 format. This is why this change has been reverted in Tiled 1.10. Of course, there is now a "Tiled 1.9" compatibility version for those who's project relies solely on the `class` attribute.

### Action Search Pop-up

A handy addition is the new "Search Actions" pop-up (aka. "Command Palette" in other apps), which can be triggered with <kbd>Ctrl+Shift+P</kbd>. It makes it easy to find most available menu and tool bar actions and makes them quickly available using just the keyboard.

![Action Search Pop-up](/img/posts/2023-03-action-search.png)

Thanks to [@dogboydog](https://github.com/dogboydog/) for getting this feature started!

### Godot 4 Export Plug-in

[Godot 4.0](https://godotengine.org/article/godot-4-0-sets-sail/) just released and [@Skrapion](https://github.com/Skrapion) has been working on a new Tiled plug-in that can export a map to a Godot `.tscn` file. It currently supports all map orientations, tile flipping, collisions and animations, but still lacks support for object layers and image collection tilesets.

Alternatively, there is also a [Tiled Importer for Godot 4](https://github.com/Kiamo2/YATI) available, with a very extensive feature set.

### Automapping Changes

In Automapping rules, it is possible to define multiple output options for randomization purposes. So far, these options each had equal probability. It is now possible to specify the probability of an output index using a custom [`Probability`][automapping probability] property on one of its output layers.

Locked layers are no longer affected by Automapping rules by default. The new [`IgnoreLock`][IgnoreLock] option can be used to make rules that ignore layer locks.

To improve compatibility with old Automapping rules, the presence of any "regions" layer now makes [`MatchInOrder`][MatchInOrder] default to `true`.

Finally, the [Automapping documentation](https://doc.mapeditor.org/en/stable/manual/automapping/) got a big update thanks to [eishiya](http://eishiya.com/), with new examples based on the improvements shipped in [Tiled 1.9][] and 1.10. 

### Object Fill and Application Font

A new custom class option was added to disable drawing the fill for certain shape objects. Useful for when the fill is getting in the way, for example when marking large areas or view boundaries.

![Demonstration of the new Draw Fill option](/img/posts/2023-03-draw-fill.png)

An option has been added to the Theme preferences to set a custom application font.

![Custom interface font setting in Preferences dialog](/img/posts/2023-03-application-font.png)

### Scripting API Additions

Based on continuous feedback from people developing [Tiled extensions](https://doc.mapeditor.org/en/stable/reference/scripting/), this release ships with many small additions. For example, it is now possible to render a map to an image using [`TileMap.toImage`][TileMap.toImage] and there are functions for handling [compression][] and [Base64 encoding][Base64]. You can also easily check the Tiled version using [`tiled.versionLessThan`][tiled.versionLessThan]. Scripts now also have initial access to the project through [`tiled.projectFilePath`][tiled.projectFilePath].

### Upgrade to Qt 6

After fixing a number of remaining issues with Tiled compiled against Qt 6, the releases for all platforms have now been updated from Qt 5.15.2 to Qt 6.4.2. This brings Tiled back to a supported Qt version and means some bugs may be fixed and some new ones could appear.

It also means the main downloads now require at least macOS 10.14, Ubuntu 20.04 (or equivalent distribution) or Windows 10. For compatibility with older systems (down to macOS 10.12, Ubuntu 18.04 and Windows 7), additional Qt 5 based packages are [available on GitHub][github releases].

## Changelog

Many other small improvements could not be mentioned, so check out the full changelog below.

*   Restored Tiled 1.8 file format compatibility by default ([#3560](https://github.com/mapeditor/tiled/pull/3560))
*   Added action search popup on <kbd>Ctrl+Shift+P</kbd> (with dogboydog, [#3449](https://github.com/mapeditor/tiled/pull/3449))
*   Added Godot 4 export plugin (by Rick Yorgason, [#3550](https://github.com/mapeditor/tiled/pull/3550))
*   Added file system actions also for tileset image based tilesets ([#3448](https://github.com/mapeditor/tiled/issues/3448))
*   Added custom class option to disable drawing fill for objects (with dogboydog, [#3312](https://github.com/mapeditor/tiled/issues/3312))
*   Added option to choose a custom interface font ([#3589](https://github.com/mapeditor/tiled/pull/3589))
*   Implemented rendering of major grid lines for staggered / hexagonal maps ([#3583](https://github.com/mapeditor/tiled/issues/3583))
*   Fixed new layer names to be always unique (by Logan Higinbotham, [#3452](https://github.com/mapeditor/tiled/issues/3452))
*   Fixed broken tile images after importing/exporting a tileset
*   AutoMapping: Added support for output set probability ([#3179](https://github.com/mapeditor/tiled/issues/3179))
*   AutoMapping: When input regions are defined, match in order by default ([#3559](https://github.com/mapeditor/tiled/issues/3559))
*   AutoMapping: Skip locked layers when applying rules ([#3544](https://github.com/mapeditor/tiled/issues/3544))
*   AutoMapping: Fixed NoOverlappingOutput in case of multiple output indices ([#3551](https://github.com/mapeditor/tiled/issues/3551))
*   AutoMapping: Fixed automatic output regions for object output ([#3473](https://github.com/mapeditor/tiled/issues/3473))
*   AutoMapping: Fixed crash on undo when output layers have properties
*   Scripting: Added `Object.setColorProperty` and `Object.setFloatProperty` ([#3423](https://github.com/mapeditor/tiled/issues/3423))
*   Scripting: Added `tiled.projectFilePath`
*   Scripting: Added `tiled.versionLessThan`
*   Scripting: Added `TileMap.toImage` ([#3519](https://github.com/mapeditor/tiled/issues/3519))
*   Scripting: Added `Tool.targetLayerType` ([#3248](https://github.com/mapeditor/tiled/issues/3248))
*   Scripting: Added `region.contiguousRegions()` ([#3576](https://github.com/mapeditor/tiled/pull/3576))
*   Scripting: Added `tiled.compress` and `tiled.decompress` ([#3153](https://github.com/mapeditor/tiled/issues/3153))
*   Scripting: Added Base64 encoding and decoding API ([#3153](https://github.com/mapeditor/tiled/issues/3153))
*   Scripting: Allow assigning `null` to `Tile.objectGroup` (by Logan Higinbotham, [#3495](https://github.com/mapeditor/tiled/issues/3495))
*   Scripting: Allow changing the items in a combo box added to a dialog
*   Scripting: Fixed painting issues after changing TileLayer size ([#3481](https://github.com/mapeditor/tiled/issues/3481))
*   Scripting: Renamed `Tileset.collection` to `Tileset.isCollection` ([#3543](https://github.com/mapeditor/tiled/issues/3543))
*   Defold plugin: Allow overriding z value also when exporting to `.collection` ([#3214](https://github.com/mapeditor/tiled/issues/3214))
*   Qt 6: Fixed invisible tileset tabs when only a single tileset is open
*   Qt 6: Fixed behavior of "Class of" selection popup
*   Qt 6: Fixed tile rendering when OpenGL is enabled ([#3578](https://github.com/mapeditor/tiled/issues/3578))
*   Fixed positioning of point object name labels (by Logan Higinbotham, [#3400](https://github.com/mapeditor/tiled/issues/3400))
*   Fixed slight drift when zooming the map view in/out
*   Fixed remaining lag after switching off hardware acceleration ([#3584](https://github.com/mapeditor/tiled/issues/3584))
*   Fixed point object hover highlight position ([#3571](https://github.com/mapeditor/tiled/issues/3571))
*   Fixed drawing lines with stamps having differently sized variations ([#3533](https://github.com/mapeditor/tiled/issues/3533))
*   Fixed compile against Qt 6.4
*   snap: Added Wayland platform plugin and additional image format plugins
*   AppImage: Updated to Sentry 0.6.0
*   Updated Bulgarian, French, German, Hungarian, Russian and Swedish translations

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

Continued Tiled development is made possible by monthly donations through
[Patreon][Patreon] and [GitHub Sponsors][sponsors] as well as many people
choosing to pay for [Tiled on itch.io][Itch]. This way, hundreds of people
have contributed to the release of Tiled 1.10, thank you!

If you haven't donated yet, please consider [setting up a small monthly
donation][donate] to support further improvements. Let's make this tool even better!

[Patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Liberapay]: https://liberapay.com/Tiled/
[Automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[Tiled 1.9]: https://www.mapeditor.org/2022/06/25/tiled-1-9-released.html
[unifying custom types]: https://www.mapeditor.org/2022/06/25/tiled-1-9-released.html#unified-custom-types
[automapping probability]: https://doc.mapeditor.org/en/stable/manual/automapping/#layer-properties
[custom dialogs]: https://doc.mapeditor.org/docs/scripting/classes/Dialog.html
[TileMap.toImage]: https://www.mapeditor.org/docs/scripting/classes/TileMap.html#toImage
[tiled.versionLessThan]: https://www.mapeditor.org/docs/scripting/modules/tiled.html#versionLessThan
[tiled.projectFilePath]: https://www.mapeditor.org/docs/scripting/modules/tiled.html#projectFilePath
[MatchInOrder]: https://doc.mapeditor.org/en/stable/manual/automapping/#matchinorder
[IgnoreLock]: https://doc.mapeditor.org/en/stable/manual/automapping/#ignorelock
[github releases]: https://github.com/mapeditor/tiled/releases
[Base64]: https://www.mapeditor.org/docs/scripting/modules/Base64.html
[compression]: https://www.mapeditor.org/docs/scripting/modules/tiled.html#compress
