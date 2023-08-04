---
layout: post
title: Tiled 1.10.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This late patch release addresses a large number of issues and adds several
new features. It is planned to be the last update to [Tiled 1.10][tiled-1.10],
with the focus now shifting towards the next release.

### Custom Project Properties

It is now possible to set custom properties on the project, in the Project
Properties dialog. This can come in handy when extensions need to be
configured with parameters.

### Scripting API Additions

Several noteworthy scripting API additions were made. Scripts can now [open
file dialogs][promptOpenFile] and [edit tile layers using
terrain][TileLayerWangEdit]. Also, a [Geometry interface][Geometry] was added
with functions to obtain points on a line or ellipse, or creating an ellipse
region.

### Godot 4

The [Godot 4 export plug-in][export-tscn] was extended with support for
exporting custom tile properties as "[Custom Data Layers][Custom Data
Layers]". If the exporter still doesn't do everything you expect from it, be
sure to also check out [YATI][YATI] (Yet Another Tiled Importer) for Godot 4.

### Universal macOS Binary

The macOS 10.14+ release is now a Universal Binary. This does double its size,
but it should improve performance for people on Apple Silicon. There have also
been [reports](https://discourse.mapeditor.org/t/it-cant-work-well-on-apple-m1-pro-monterey/5814)
of Tiled freezing on M1 and M2 chips and the hope is that this might resolve
that problem. Please be sure to let us know whether it does!

Changelog
---------

*   Added support for setting custom properties on the project (with dogboydog, [#2903](https://github.com/mapeditor/tiled/issues/2903))
*   Added feedback when Terrain Brush and Terrain Fill Mode can't find a tile
*   Removed Space and Ctrl+Space shortcuts from Layers view to avoid conflict with panning ([#3672](https://github.com/mapeditor/tiled/issues/3672))
*   Display the image base name for unnamed tile objects referring to single images
*   Scripting: Added API for editing tile layers using terrain sets (with a-morphous, [#3758](https://github.com/mapeditor/tiled/pull/3758))
*   Scripting: Added file dialog API (with dogboydog, [#3782](https://github.com/mapeditor/tiled/pull/3782))
*   Scripting: Support erasing tiles in Tool.preview and TileMap.merge
*   Scripting: Added Geometry interface with line and ellipse helpers
*   Scripting: Added WangSet.effectiveTypeForColor
*   Fixed crash when changing file property of custom class ([#3783](https://github.com/mapeditor/tiled/issues/3783))
*   Fixed loading of invalid color properties ([#3793](https://github.com/mapeditor/tiled/issues/3793))
*   Fixed handling of enum values with 31 flags and fixed the applied limit ([#3658](https://github.com/mapeditor/tiled/issues/3658))
*   Fixed object preview position with parallax factor on group layer ([#3669](https://github.com/mapeditor/tiled/issues/3669))
*   Fixed hover highlight rendering with active parallax factor ([#3669](https://github.com/mapeditor/tiled/issues/3669))
*   Fixed updating of object selection outlines when changing parallax factor ([#3669](https://github.com/mapeditor/tiled/issues/3669))
*   Fixed "Offset Map" action to offset all objects when choosing "Whole Map" as bounds
*   Fixed several issues with drawing ellipses ([#3776](https://github.com/mapeditor/tiled/pull/3776))
*   Fixed Terrain Fill Mode for sets containing transitions to empty ([#3774](https://github.com/mapeditor/tiled/pull/3774))
*   Godot 4 plugin: Export custom tile properties as Custom Data Layers (with Kevin Harrison, [#3653](https://github.com/mapeditor/tiled/pull/3653))
*   AppImage: Updated to Sentry 0.6.5
*   Qt 6: Increased the image allocation limit from 1 GB to 4 GB ([#3616](https://github.com/mapeditor/tiled/issues/3616))
*   macOS: The macOS 10.14+ build is now a Universal macOS Binary ([#3707](https://github.com/mapeditor/tiled/issues/3707))

### Thanks!

Many thanks to all who've reported issues, and of course to everybody who
supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.10]: {{ site.baseurl }}{% post_url 2023-03-10-tiled-1-10-released %}
[promptOpenFile]: https://www.mapeditor.org/docs/scripting/modules/tiled.html#promptOpenFile
[TileLayerWangEdit]: https://www.mapeditor.org/docs/scripting/interfaces/TileLayerWangEdit.html
[Geometry]: https://www.mapeditor.org/docs/scripting/modules/Geometry.html
[export-tscn]: https://doc.mapeditor.org/en/stable/manual/export-tscn/
[Custom Data Layers]: https://docs.godotengine.org/en/stable/tutorials/2d/using_tilesets.html#assigning-custom-metadata-to-the-tileset-s-tiles
[YATI]: https://github.com/Kiamo2/YATI
