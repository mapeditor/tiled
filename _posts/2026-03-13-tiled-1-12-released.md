---
layout: post
title: Tiled 1.12 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Tiled 1.12 brings a rewritten Properties view with direct widget interaction, support for list-valued custom properties, a new Oblique map orientation, layer blending modes, capsule objects, and many usability and scripting improvements.

### Rewritten Properties View

The Properties view has been rewritten to make editing properties more direct and convenient. Rather than creating widgets when clicking values, the values can now be interacted with immediately. This especially improves the workflow when working with checkboxes, comboboxes, colors, file paths, object references, and for included buttons.

![The rewritten Properties view, showing the new widgets used for Map, Layer, Object and Tileset properties, directly editable in-place](/img/posts/2026-03-properties-view.png)

This rewrite also layed the groundwork for supporting lists in custom properties.

### List Values in Custom Properties

Custom properties can now store lists of values. This much requested feature is useful when a single value is not sufficient. Need multiple references, fill containers with multiple items or maybe just have a list of tags? Now you no longer have to resort to comma-separated strings or numbered property names.

![A list-valued custom property being edited in Tiled, showing a list of instances of a custom "Drop" class](/img/posts/2026-03-list-properties.png)

Of course list values are also supported in the scripting API, where they map to/from JS arrays:

```js
tiled.activeAsset.selectedObjects[0].setProperty("drops", [
  tiled.propertyValue("Drop", { "item": "MaggotSlime", "probability": 0.5 }),
  tiled.propertyValue("Drop", { "item": "Knife", "probability": 0.01 }),
]);
```

### Oblique Maps, Layer Blending, and Capsules

This release adds support for an **Oblique** map orientation, allowing the X and/or Y axis to be skewed. Here are two examples of oblique maps rendered by Tiled, using tiles provided by **sxdxs** and **wayfu** on [the original GitHub issue](https://github.com/mapeditor/tiled/issues/2917):

![Two oblique maps as rendered by Tiled](/img/posts/2026-03-oblique-orientation.png)

Layers gained support for SVG 1.2 / CSS **blending modes** (Normal, Multiply, Screen, Overlay, Darken, Lighten, Color Dodge, Color Burn, Hard Light, Soft Light, Difference, and Exclusion). Here's a comparison of Normal and Multiply blending modes in a level by Tom Happ:

![Comparison of Normal and Multiply blending modes](/img/posts/2026-03-blending-modes.png)

For object layers, there is now a **capsule object shape**, which should be a nice addition when your physics engine supports it. In addition, object opacity can now be adjusted per object.

![A map shown in Tiled with a number of capsule objects](/img/posts/2026-03-capsule-objects.png)

### Many Workflow Improvements

A large part of this release is focused on smaller quality-of-life improvements that add up in day-to-day use. The tileset dock can now be filtered by tileset name, and number inputs now support expressions, so you can type a calculation to set a value.

Several tile editing tools became more convenient as well. The Rectangular Select tool gained square selection and expand-from-center behavior. The Stamp Brush, Terrain Brush and Eraser now show status information for their various modes. It is also now possible to cancel tile-related operations with <kbd>Escape</kbd> or right-click. Finally, the Magic Wand, Bucket Fill and Select Same Tile tools now allow their area to be expanded by dragging across multiple tiles.

Other usability updates include a "Go to Tile" action to jump to specific coordinates, a "World > World Properties" action that makes World properties accessible, switching back to the previous tool by repeating the shortcut of the current tool, plus a button to toggle the Terrain Brush to full tile mode.

### Exporting, Scripting and Automation

The tBIN plugin gained support for the tIDE XML format. And there is a new export plugin for [Remixed Dungeon](https://github.com/NYRDS/remixed-dungeon).

On the scripting side, this release adds API for accessing and modifying the [custom property types of the project](/docs/scripting/classes/Project.html#propertyTypes), access to session properties through [`tiled.session`](/docs/scripting/modules/tiled.html#session), [`MapEditor.selectedTool`](/docs/scripting/interfaces/MapEditor.html#selectedTool) and [`MapEditor.tool`](/docs/scripting/interfaces/MapEditor.html#tool), as well as [`TileMap.chunkSize`](/docs/scripting/classes/TileMap.html#chunkSize) and [`TileMap.compressionLevel`](/docs/scripting/classes/TileMap.html#compressionLevel).

Automapping was improved by no longer matching rules based on empty input indexes, while reloading of rule maps was optimized and rule maps are now loaded on demand.

### Translation Updates Welcome

I'd like to improve the state of translations for Tiled 1.12 and future releases. If you're interested in helping update translations, please get in touch. More information about translating Tiled can be found on the [Translating Tiled wiki page](https://github.com/mapeditor/tiled/wiki/Translating-Tiled). I plan to do patch releases for Tiled 1.12 to ship updated translations.

## Changelog

Many other small improvements could not be mentioned, so check out the full changelog below.

*   Rewritten Properties view to enable direct widget interaction ([#4045](https://github.com/mapeditor/tiled/pull/4045))
*   Added support for lists in custom properties ([#1493](https://github.com/mapeditor/tiled/issues/1493))
*   Added capsule object shape (by Jocelyn, [#2153](https://github.com/mapeditor/tiled/issues/2153))
*   Added Oblique map orientation, skewing X and/or Y axis ([#2917](https://github.com/mapeditor/tiled/issues/2917))
*   Added support for per-object opacity (by jcbk101, [#4031](https://github.com/mapeditor/tiled/pull/4031))
*   Allow filtering tilesets by name in the tileset dock (with dogboydog, [#4239](https://github.com/mapeditor/tiled/pull/4239))
*   Allow changing the values of number inputs using expressions (with dogboydog, [#4234](https://github.com/mapeditor/tiled/pull/4234))
*   Added support for SVG 1.2 / CSS blending modes to layers ([#3932](https://github.com/mapeditor/tiled/issues/3932))
*   Added support for natural sorting of project files (by Edgar Jr. San Martin, [#4284](https://github.com/mapeditor/tiled/pull/4284))
*   Added button to toggle Terrain Brush to full tile mode (by Finlay Pearson, [#3407](https://github.com/mapeditor/tiled/pull/3407))
*   Added square selection and expand-from-center to Rectangular Select tool ([#4201](https://github.com/mapeditor/tiled/issues/4201))
*   Added status info for various Stamp Brush, Terrain Brush and Eraser modes ([#3092](https://github.com/mapeditor/tiled/issues/3092), [#4201](https://github.com/mapeditor/tiled/issues/4201))
*   Added Escape to clear tile selection when any tile related tool is selected ([#4243](https://github.com/mapeditor/tiled/issues/4243))
*   Added Escape to cancel tile selection and shape drawing operations
*   Added Backspace to remove the previously added point while creating or extending polygons and polylines ([#4372](https://github.com/mapeditor/tiled/pull/4372))
*   Added a "Go to Tile" action to jump to specific coordinates (by PoonamMehan, [#4348](https://github.com/mapeditor/tiled/pull/4348))
*   Made the shortcut for current tool switch to previous tool ([#4280](https://github.com/mapeditor/tiled/pull/4280))
*   Allow canceling Select Same Tile, Magic Wand and Bucket Fill operations with right-click and Escape
*   Allow dragging over multiple tiles with Select Same Tile, Magic Wand and Bucket Fill tools ([#4276](https://github.com/mapeditor/tiled/pull/4276))
*   Allow zooming in on areas outside the map bounds (by kunal649, [#3860](https://github.com/mapeditor/tiled/issues/3860))
*   Don't switch to Edit Polygons tool on double-click with Alt pressed
*   Adjust world map position when resizing a map with offset ([#4270](https://github.com/mapeditor/tiled/issues/4270))
*   Added export plugin for Remixed Dungeon (by Mikhael Danilov, [#4158](https://github.com/mapeditor/tiled/pull/4158))
*   Added "World > World Properties" menu action (with dogboydog, [#4190](https://github.com/mapeditor/tiled/pull/4190))
*   Added Delete shortcut to Remove Tiles action by default and avoid ambiguity ([#4201](https://github.com/mapeditor/tiled/issues/4201))
*   Fixed selection to be preserved when toggling dynamic wrapping (by Mollah Hamza, [#4385](https://github.com/mapeditor/tiled/pull/4385))
*   Fixed tileset tabs to fall back to filename in case of unnamed tilesets (by Sid, [#4360](https://github.com/mapeditor/tiled/pull/4360))
*   Fixed alpha component of tint color not applying correctly to opaque images (by Roland Helmerichs, [#4310](https://github.com/mapeditor/tiled/pull/4310))
*   Fixed panning with space bar not always working on first click (with Oval, [#4338](https://github.com/mapeditor/tiled/pull/4338))
*   Fixed undo behavior after resizing objects certain ways (by Kanishka, [#4339](https://github.com/mapeditor/tiled/pull/4339))
*   Fixed suggesting filename with trailing dot when export filter is unset (by Sid, [#4368](https://github.com/mapeditor/tiled/pull/4368))
*   Fixed snapping mode sync across instances (by Sid, [#4364](https://github.com/mapeditor/tiled/pull/4364))
*   Fixed missing error message when 'Export as Image' fails (by kunal649, [#4397](https://github.com/mapeditor/tiled/pull/4397))
*   Scripting: Added API for custom property types (with dogboydog, [#3971](https://github.com/mapeditor/tiled/pull/3971))
*   Scripting: Added `TileMap.chunkSize` and `TileMap.compressionLevel` properties
*   Scripting: Added optional defaultValue and toolTip params to `Dialog` add widget methods (by Oval, [#4358](https://github.com/mapeditor/tiled/pull/4358))
*   Scripting: Added `tiled.session` to read and write session properties (by Kanishka, [#4345](https://github.com/mapeditor/tiled/pull/4345))
*   Scripting: Added `MapEditor.selectedTool` and `MapEditor.tool` ([#4330](https://github.com/mapeditor/tiled/pull/4330))
*   Scripting: Fixed the `fileName` property of map/tileset passed to `FileFormat.write` (by Shuvam Pal, [#4359](https://github.com/mapeditor/tiled/pull/4359))
*   AutoMapping: Don't match rules based on empty input indexes
*   AutoMapping: Optimized reloading of rule maps and load rule maps on-demand
*   tBIN plugin: Added support for the tIDE XML format (by Casey Warrington, [#4308](https://github.com/mapeditor/tiled/pull/4308))
*   Windows: Fixed issue with opening Tile Animation Editor ([#4223](https://github.com/mapeditor/tiled/issues/4223))
*   macOS: Add <kbd>Cmd+Shift+[</kbd> and <kbd>Cmd+Shift+]</kbd> shortcuts to switch tabs (by Oval, [#4344](https://github.com/mapeditor/tiled/pull/4344))
*   macOS: Fixed crash when JS code is JIT-compiled ([#4218](https://github.com/mapeditor/tiled/issues/4218))
*   Workaround tileset view layout regression in Qt 6.9
*   Raised minimum supported Qt version from 5.12 to 5.15.2
*   AppImage: Updated to Sentry 0.12.8
*   Updated Bulgarian translation

Thanks to all contributors who made this release possible! Many smaller improvements were made in the last weeks by students interested in participating in the [Google Summer of Code][GSoC] with us this year, and many more are pending review for upcoming releases.

## Support Tiled Development ❤️

Continued Tiled development is made possible by monthly donations through
[Patreon], [GitHub Sponsors][sponsors] and [OpenCollective] as well as many
people choosing to pay for [Tiled on itch.io][Itch]. This way, hundreds of
people have contributed to the release of Tiled 1.12, thank you!

If you haven't donated yet, please consider [setting up a small monthly
donation][donate] to support further improvements. Let's make this tool even better!

[Patreon]: https://www.patreon.com/bjorn
[OpenCollective]: https://opencollective.com/tiled
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[Itch]: https://thorbjorn.itch.io/tiled
[Automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[GSoC]: {{ site.baseurl }}{% post_url 2026-02-26-google-summer-of-code-2026 %}
