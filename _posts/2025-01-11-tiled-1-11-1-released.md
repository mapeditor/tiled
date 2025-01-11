---
layout: post
title: Tiled 1.11.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Apart from fixing a number of issues in [Tiled 1.11][tiled-1.11], this release
adds support for loading Aseprite images, brings a few scripting API additions
as well as a small AutoMapping improvement.

### Aseprite Image Support

Tiled now supports loading [Aseprite](https://aseprite.org/) images directly.
Aseprite is a popular editor for pixel art. This means you can now use `.ase` /
`.aseprite` files in your tilesets, without needing to export them to PNG or
other common formats first.

This is made possible by [qaseprite](https://github.com/mapeditor/qaseprite), a
new Qt image format plugin that loads Aseprite images based on the libraries
provided by the Aseprite project.

Of course exporting your images may still be necessary for your game engine or
other tools, but in some cases even that can be avoided, for example using
[MonoGame.Aseprite](https://github.com/AristurtleDev/monogame-aseprite), [Unity
aseprite-importer](https://github.com/negi0109/unity-aseprite-importer) or
[Godot Aseprite Wizard](https://github.com/viniciusgerevini/godot-aseprite-wizard).
For now it remains to be seen how such importers can be used alongside the
various Tiled support libraries for these frameworks.

### AutoMapping Improvement

The [AutoMapping feature](https://doc.mapeditor.org/en/stable/manual/automapping/)
now ignores rules with empty input or output regions. This is especially useful
while setting up rules, since incomplete rules that did not have their inputs
defined yet would previously cause the output to be applied everywhere at an
offset.

Changelog
---------

All changes in this release are listed below:

*   Releases now ship with support for loading Aseprite images ([#4109](https://github.com/mapeditor/tiled/issues/4109))
*   Scripting: Added `FileFormat.nameFilter`
*   Scripting: Added `MapEditor.currentBrushChanged` signal
*   Scripting: Added `tiled.cursor` to create mouse cursor values
*   Scripting: Added `Tileset.transformationFlags` ([#3753](https://github.com/mapeditor/tiled/issues/3753))
*   Scripting: Added `Dialog.addRadioButtonGroup` for selecting one of a list of mutually exclusive options ([#4107](https://github.com/mapeditor/tiled/pull/4107))
*   Scripting: Made `currentWangSet` and `currentWangColorIndex` properties writeable ([#4105](https://github.com/mapeditor/tiled/pull/4105))
*   AutoMapping: Ignore rules with empty input or output regions ([#3834](https://github.com/mapeditor/tiled/issues/3834))
*   Fixed saving/loading of custom properties set on worlds ([#4025](https://github.com/mapeditor/tiled/issues/4025))
*   Fixed issue with placing tile objects after switching maps ([#3497](https://github.com/mapeditor/tiled/issues/3497))
*   Fixed crash when accessing a world through a symlink ([#4042](https://github.com/mapeditor/tiled/issues/4042))
*   Fixed performance issue when tinting tiles from large tilesets
*   Fixed error reporting when exporting on the command-line (by Shuhei Nagasawa, [#4015](https://github.com/mapeditor/tiled/pull/4015))
*   Fixed updating of object label when text changes without changing size
*   Fixed minimum value of spinbox in Tile Animation Editor
*   Fixed loading of custom property types in tilesets referenced by tile stamps ([#4044](https://github.com/mapeditor/tiled/pull/4044))
*   Fixed compile against Qt 6.8 and updated releases to Qt 6.8.1
*   snap: Updated to core24
*   AppImage: Updated to Sentry 0.7.13

### Thanks!

Many thanks to all who've reported issues, and of course to everybody who
supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.11]: {{ site.baseurl }}{% post_url 2024-06-27-tiled-1-11-released %}
