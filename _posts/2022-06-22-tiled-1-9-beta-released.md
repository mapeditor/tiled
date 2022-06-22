---
layout: post
title: Tiled 1.9 Release Candidate
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Here's another preview in preparation of the final Tiled 1.9 release, which will happen very soon if everything is alright. The downloads for this preview are [available on GitHub](https://github.com/mapeditor/tiled/releases/tag/v1.8.91).

Where the [Tiled 1.9 Alpha] was very focused on [AutoMapping](https://doc.mapeditor.org/de/latest/manual/automapping/), this release also improves a number of other areas, described below.

### Project Workflow

The project-related actions were simplified, with _File > New > New Project_ replacing the _Project > Save Project As_ action, and the "Recent Projects" and "Close Project" actions moving to the File menu as well.

In addition, all project-related settings are now only available after creating or loading a project.

### New Tile(set) Options

As a first step towards supporting sprite atlasses, the images in an Image Collection tileset can now refer to a sub-rectangle of their image. This sub-rectangle can be changed in the Tile Properties, or by scripts through the `Tile.imageRect` property.

In addition, the size at which all tiles in a tileset render can now be configured to be either the tile size (as before) or the map grid size. The latter is useful if you want to use tiles with a different resolution but scale them up or down to fit the grid.

Finally, and mainly as an option along with the previous one, the fill mode can be changed between "Stretch" (the default) or "Preserve Aspect Ratio". In the latter case, when a tile is not rendered at its native size, it is rendered at the maximum size that fits within the target rectangle, but keeping its aspect ratio.

### Built-in AutoMapping Rules Tileset

The first to use these new tile render options is the now built-in Automapping Rules Tileset, which can be added to a map by choosing _Map > Add Automapping Rules Tileset_. This tileset includes a tile for each special meaning supported in AutoMapping Rules. The icons have been tweaked to be easier to understand as well:

![automap-tiles](https://raw.githubusercontent.com/mapeditor/tiled/master/src/tiled/resources/automap-tiles.svg)

From left to right, these are Negate, Ignore, NonEmpty, Empty and Other.

Custom map, layer and object properties supported by the AutoMapping system are now displayed in the Properties view when it detects that the current map is an AutoMapping Rules map (the map needs to have at least one "input\*" and one "output\*" layer).

### Object Types Merged with Property Types

There was a lot of overlap between the Property Types, added in Tiled 1.8, and the Object Types. In this release, the Object Types have been merged into the Property Types, now simply called "Custom Types". This means custom classes can now have a color and can be used as "object type". If your project refers to an object types file, these types will be automatically imported as custom classes. If you used globally stored object types before, they can be manually imported to your project.

In addition, the "Type" property previously available only for objects and tiles is now available for all data types as the new "Class" property. For consistency, this value is written out as "class" also for objects and tiles, but a project-wide compatibility option is provided to make it still write out as "type":

![Compatibility Version](/img/posts/2022-06-compatibility-version.png)

## Changelog

These are the changes since [Tiled 1.9 Alpha]:

*   Added option to ignore transparent pixels when selecting tile objects ([#1477](https://github.com/mapeditor/tiled/issues/1477))
*   Added support for sub-images in image collection tilesets ([#1008](https://github.com/mapeditor/tiled/issues/1008))
*   Added Tile Render Size and Fill Mode options to Tileset
*   Added 'New Project' action, replacing 'Save Project As' ([#3279](https://github.com/mapeditor/tiled/issues/3279))
*   Added ability to load .tiled-session files from command-line
*   Added %worldfile variable for custom commands (by Pixel-Nori, [#3352](https://github.com/mapeditor/tiled/pull/3352))
*   Added "Class" field to all data types, referring to a custom class
*   Merged Object Types with Property Types
*   Don't scale point objects with the zoom level ([#3356](https://github.com/mapeditor/tiled/issues/3356))
*   Scripting: Added Tileset.columnCount property
*   Scripting: Added access to selected terrain in tileset editor
*   AutoMapping: Added built-in tileset with custom rule tiles
*   AutoMapping: Avoid additional undo commands after Erase and Delete
*   AutoMapping: Show relevant custom properties when a rules map is detected
*   Optimized rendering of tinted layers by caching tinted images
*   tmxrasterizer: Added options to hide certain layer types ([#3343](https://github.com/mapeditor/tiled/issues/3343))
*   macOS: Fixed layout of Custom Types Editor when using native style

The 1.9 RC also includes all the fixes that went into [Tiled 1.8.5] and [Tiled 1.8.6].

_All releases are now based on Qt 6.2, and require at least macOS 10.14, Ubuntu 20.04 (or equivalent distribution) or Windows 10. For compatibility with older systems (down to macOS 10.12, Ubuntu 18.04 and Windows 7), additional Qt 5 based packages are also available._

[Tiled 1.9 Alpha]: https://www.mapeditor.org/2022/04/08/tiled-1-9-alpha-released.html
[Tiled 1.8.5]: https://www.mapeditor.org/2022/05/17/tiled-1-8-5-released.html
[Tiled 1.8.6]: https://www.mapeditor.org/2022/06/15/tiled-1-8-6-released.html
