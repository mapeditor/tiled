---
layout: post
title: Tiled 1.11 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Tiled 1.11 brings many scripting API additions, automapping enhancements, export plugin improvements, and various quality-of-life changes and bug fixes.

### Scripting API Additions

Notable additions to the scripting API include support for [working with worlds](https://www.mapeditor.org/docs/scripting/classes/World.html), conveniently setting nested object properties, accessing tile images, creating [color values](https://www.mapeditor.org/docs/scripting/modules/tiled.html#color), and modifying tileset margin and spacing.

Additionally, a `--project` command-line parameter was added to allow project extensions to be loaded when using the `--export-map`, `--export-tileset` or `--evaluate` parameters.

### Usability Improvements

A host of small but useful usability improvements have been made. The "Move Object to Layer" menu now [displays group layer names](https://github.com/mapeditor/tiled/pull/3811). The <kbd>Ctrl+Shift+S</kbd> shortcut is now "Save As" by default, to match most other applications. Duplicating layers no longer adds "Copy" to the name by default.

To move rectangular areas of tiles, you can now hold <kbd>Shift</kbd> while right-click dragging [to "cut" the captured tile stamp](https://github.com/mapeditor/tiled/issues/3961). This can provide a quick alternative to switching to the tile selection or eraser tools.

When a read-only file is opened, a lock icon is now shown in its tab. This is particularly useful for people using file locking in the context of version control.

### Exporting Improvements

#### Godot 4 Export Plugin

The Godot 4 export plugin now supports [exporting objects](https://doc.mapeditor.org/en/stable/manual/export-tscn/#object-properties). Objects you wish to export need to have a custom `resPath` property set on them, in the form of `res://<object path>.tscn`, indicating the Godot object you wish to replace the object with. Also, the support for exporting flipped tiles has been updated to use the new [tile transformation flags](https://github.com/godotengine/godot/pull/80144) added in Godot 4.2. Finally, the positioning of collision objects was fixed.

If this export option doesn't fit your needs, the [Tiled Importer for Godot 4](https://github.com/Kiamo2/YATI) remains a good alternative with a very extensive feature set.

#### Python Plugin

The Python plugin now supports implementation of [custom tileset formats](https://doc.mapeditor.org/en/stable/manual/python/#tileset-plugins). Also, the Windows 10+ release of Tiled now uses Python 3.12 (previously 3.8 was used). It is however recommended to [extend Tiled with JavaScript](https://doc.mapeditor.org/en/stable/manual/scripting/) instead, which has more features and better availability.

#### GameMaker 2 Plugin

The positioning of objects on isometric maps when [exporting to GameMaker 2](https://doc.mapeditor.org/en/stable/manual/export-yy) was fixed.

#### tmxrasterizer

While not technically an exporter, the `tmxrasterizer` utility shipping with Tiled provides a way to quickly turn a map (or an entire world) into an image. For this release, it gained `--hide/show-object` arguments for fine-grained control over object visibility. It also gained `--frames` and `--frame-duration` arguments to [export animated maps](https://github.com/mapeditor/tiled/pull/3868) as multiple images. The `--hide/show-layer` arguments [now work on group layers](https://github.com/mapeditor/tiled/issues/3899).

Also, the loading of object templates in `.tx` format was fixed, allowing `tmxrasterizer` and `tmxviewer` to render template instances.

### Automapping Changes

The [Automapping feature][Automapping] has been made more convenient with three behavioral changes:

* **Ignore empty outputs per-rule**: This is useful when your rules have multiple output variations, but not all of them have the same amount of variations. Now, only those output indices with actual output for a given rule are considered. The [special "Ignore" tile](https://doc.mapeditor.org/en/stable/manual/automapping/#specialtiles) can be used if you need an empty variation.

* **Ignore flip flags**: There are now [per-input-layer options](https://doc.mapeditor.org/en/stable/manual/automapping/#layer-properties) to ignore flip flags, which can greatly reduce the amount of needed input patterns.

* **Always apply output sets with empty index**: When a rule has multiple variations, but part of its output should always be applied, now you can just leave out the "index" part of the relevant output layers. Previously this counted as a variation of its own.

Finally, the Automapping feature no longer fails add new tilesets used by applied changes.

### Noteworthy Bugfixes

The way maps are reloaded when they are changed externally was rewritten to fix issues with reloading maps in the context of worlds. Also, loaded maps and tilesets which are currently not opened for editing, are now also reloaded when they change on disk.

With the "Resolve object types and properties" [export option](https://doc.mapeditor.org/en/stable/manual/preferences/#export-options) enabled, members of custom classes used as properties are now also exported with their default values.

When changing the type of a [terrain set](https://doc.mapeditor.org/en/stable/manual/terrain/), remaining labels that are only relevant for a different type are now hidden and ignored by the Terrain Tool.

## Changelog

Many other small improvements could not be mentioned, so check out the full changelog below.

*   Added --project command-line parameter for use when exporting ([#3797](https://github.com/mapeditor/tiled/issues/3797))
*   Added group layer names in "Move Object to Layer" menu ([#3454](https://github.com/mapeditor/tiled/issues/3454))
*   Added lock icon to open tabs for which the file is read-only
*   Added Shift modifier to cut when capturing a tile stamp (by kdx2a, [#3961](https://github.com/mapeditor/tiled/issues/3961))
*   Made adding "Copy" when duplicating optional and disabled by default ([#3917](https://github.com/mapeditor/tiled/pull/3917))
*   Changed default shortcut for "Save As" to <kbd>Ctrl+Shift+S</kbd> and removed shortcut from "Save All" ([#3933](https://github.com/mapeditor/tiled/issues/3933))
*   Layer names are now trimmed when edited in the UI, to avoid accidental whitespace
*   Scripting: Added API for working with worlds ([#3539](https://github.com/mapeditor/tiled/issues/3539))
*   Scripting: Added `Object.setProperty` overload for setting nested values
*   Scripting: Added `Tile.image` for accessing a tile's image data
*   Scripting: Added `Image.copy` overload that takes a rectangle
*   Scripting: Added `Tileset.imageFileName` and `ImageLayer.imageFileName`
*   Scripting: Added `FilePath.localFile` and `FileEdit.fileName` (string alternatives to Qt.QUrl properties)
*   Scripting: Added `tiled.color` to create color values
*   Scripting: Made `Tileset.margin` and `Tileset.tileSpacing` writable
*   Scripting: Restored compatibility for MapObject.polygon ([#3845](https://github.com/mapeditor/tiled/issues/3845))
*   Scripting: Fixed issues with editing properties after setting class values from script
*   Scripting: Fixed setting/getting object reference values when nested as a class member
*   TMX format: Embedded images are now also supported on tilesets and image layers
*   JSON format: Fixed tile order when loading a tileset using the old format
*   Godot 4 plugin: Added support for exporting objects (by Rick Yorgason, [#3615](https://github.com/mapeditor/tiled/pull/3615))
*   Godot 4 plugin: Use Godot 4.2 tile transformation flags (by Rick Yorgason, [#3895](https://github.com/mapeditor/tiled/pull/3895))
*   Godot 4 plugin: Fixed positioning of tile collision shapes (by Ryan Petrie, [#3862](https://github.com/mapeditor/tiled/pull/3862))
*   GameMaker 2 plugin: Fixed positioning of objects on isometric maps
*   Python plugin: Added support for implementing tileset formats (with Pablo Duboue, [#3857](https://github.com/mapeditor/tiled/pull/3857))
*   Python plugin: Raised minimum Python version to 3.8
*   Python plugin: Now built against Python 3.12 for Windows 10+
*   tmxrasterizer: Added `--hide-object` and `--show-object` arguments (by Lars Luz, [#3819](https://github.com/mapeditor/tiled/pull/3819))
*   tmxrasterizer: Added `--frames` and `--frame-duration` arguments to export animated maps as multiple images ([#3868](https://github.com/mapeditor/tiled/pull/3868))
*   tmxrasterizer: Fixed `--hide/show-layer` to work on group layers ([#3899](https://github.com/mapeditor/tiled/issues/3899))
*   tmxviewer: Added support for viewing JSON maps ([#3866](https://github.com/mapeditor/tiled/issues/3866))
*   tmxrasterizer/viewer: Fixed loading of XML object templates (with Christian Schaadt, [#3977](https://github.com/mapeditor/tiled/pull/3977))
*   AutoMapping: Ignore empty outputs per-rule ([#3523](https://github.com/mapeditor/tiled/issues/3523))
*   AutoMapping: Added per-input-layer properties for ignoring flip flags ([#3803](https://github.com/mapeditor/tiled/issues/3803))
*   AutoMapping: Always apply output sets with empty index
*   AutoMapping: Fixed adding of new tilesets used by applied changes
*   Windows: Fixed the support for WebP images (updated to Qt 6.6.1, [#3661](https://github.com/mapeditor/tiled/issues/3661))
*   Fixed issues related to map and tileset reloading
*   Fixed possible crash after assigning to `tiled.activeAsset`
*   Fixed the option to resolve properties on export to also resolve class members ([#3411](https://github.com/mapeditor/tiled/issues/3411), [#3315](https://github.com/mapeditor/tiled/issues/3315))
*   Fixed terrain tool behavior and terrain overlays after changing terrain set type ([#3204](https://github.com/mapeditor/tiled/issues/3204), [#3260](https://github.com/mapeditor/tiled/issues/3260))
*   Fixed mouse handling issue when zooming while painting ([#3863](https://github.com/mapeditor/tiled/issues/3863))
*   Fixed possible crash after a scripted tool disappears while active
*   Fixed updating of used tilesets after resizing map ([#3884](https://github.com/mapeditor/tiled/issues/3884))
*   Fixed alignment of shortcuts in action search
*   Fixed object assignment buttons in tile collision editor ([#3399](https://github.com/mapeditor/tiled/issues/3399))
*   AppImage: Fixed ability to open paths with spaces from the CLI ([#3914](https://github.com/mapeditor/tiled/issues/3914))
*   AppImage: Updated to Sentry 0.7.6
*   Updated Bulgarian, Czech, French and Russian translations

## Support Tiled Development ❤️

Continued Tiled development is made possible by monthly donations through
[Patreon], [GitHub Sponsors][sponsors] and [OpenCollective] as well as many
people choosing to pay for [Tiled on itch.io][Itch]. This way, hundreds of
people have contributed to the release of Tiled 1.11, thank you!

If you haven't donated yet, please consider [setting up a small monthly
donation][donate] to support further improvements. Let's make this tool even better!

[Patreon]: https://www.patreon.com/bjorn
[OpenCollective]: https://opencollective.com/tiled
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[Tiled 1.9]: https://www.mapeditor.org/2022/06/25/tiled-1-9-released.html
