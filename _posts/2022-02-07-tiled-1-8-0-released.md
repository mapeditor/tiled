---
layout: post
title: Tiled 1.8 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Tiled 1.8 is here, bringing custom property types, configurable parallax
origin, repeatable image layers, many scripting and AutoMapping improvements
and a huge amount of fixes!

## Custom Property Types

Tiled has always had a pretty flexible custom property system, allowing you to
associate custom data with any of its data types. In this release, you can
define your own types for these properties. They come in two forms for now:
custom enums and custom classes.

### Custom Enums

Custom enums allow predefining a fixed set of valid values for a property. You
can choose whether their value is stored as string or number, and you can
choose whether to allow selecting a single value or multiple. When allowing
multiple values and storing the value as number, bitflags will be used.

Suppose for example, that the "enemies" in your game can be either Neutral,
Friendly or Aggressive. Now you can set up this enum in your project and you
no longer have to remember the numbers or type those values!

![Custom Enum](/img/posts/2022-02-custom-enum.png)

### Custom Classes

Custom classes allow predefining a set of properties, with custom default
values, which are always added as a unit. Maybe some of your objects have
physics enabled, and when they do, you want to see all the relevant
properties. Now you can!

![Custom Class](/img/posts/2022-02-custom-class.png)

As an added bonus, custom classes can use other custom classes as their
members, any change you make to your custom enums or classes is immediately
visible in the Properties view and changed properties are shown in bold and
easy to unset again.

## Parallax Origin and Image Layer Repeat

In Tiled 1.5, support was added for a parallax scrolling factor. In this
release, we added a [Parallax Origin][parallax-origin] property to each map,
which can be used to set the camera location at which the parallax offset is
0\. This can really help to match what you see in Tiled with what you see in
your game. Also the handling of the [parallax factor on group
layers][parallax-group-layer-fix] was fixed.

Somewhat related to parallax is the new support for repeating image layers.
They provide a convenient way of set up an infinite repeating background. Of
course, they're also perfect for adding entirely custom grids. Talking about
grids, the major grid can now be configured separately in the X and Y
directions.

![Repeating Image Layers](/img/posts/2022-02-repeating-image-layers.gif)

_(demonstrating repeating image layers using [Magic Cliffs Environment](https://ansimuz.com/site/portfolio/magic-cliffs-environment/) by Ansimuz)_

## Scripting Extensions

Support for loading [JavaScript modules][scripting-modules] has been added
(recognized by their `.mjs` extension), which allows splitting up your
extension in multiple files without polluting the global scope.

There's a new [File interface][scripting-file], providing various file system
operations. It is now also possible to open a map or tileset created from a
script in Tiled without first saving it to a file, by assigning it to
[`tiled.activeAsset`][scripting-activeAsset]. The [`region`][scripting-region]
type got a lot more useful. 

These and many other small additions are included in the updated
[`tiled-api`][tiled-api] package, providing auto-completion and type checking
for Tiled extensions.

## AutoMapping Improvements

A number of quality-of-life improvements have been made to the [AutoMapping
system][automapping].

It is now possible to organize your rule maps using group layers, and using
group layers in your target maps also no longer breaks AutoMapping. The
`regions` layer can now be used alongside `region_input/output` layers for
defining shared regions. Layers in the rule map can be disabled by prefixing
their name with "//".

Finally, a rule map can now be referenced directly from a Tiled project or
used by [`TileMap.autoMap`][scripting-automap], without needing a `rules.txt`
file.

## Other Noteworthy Stuff

The multi-layer painting behavior was much improved, such that it now also
works when [multiple layers have the same
name](https://github.com/mapeditor/tiled/issues/3094). In the common case
where you have the same number of layers selected as you are painting, it now
only uses the layer order and not their names to determine which layers to
modify. When it does need to use the names, it still also takes the order into
account and will not paint more than once to the same layer.

On the tool side, when placing a tile object it can now be flipped or rotated
[before placing it](https://github.com/mapeditor/tiled/issues/3091). Also, the
object context menu is now available while using the object creation tools.
Finally, it is now much less likely to use the "Offset Layer" tool
accidentally, thanks to improved logic for automatic switching of tools.

The undo behavior was improved such that tile selection changes no longer mark
a file as modified. Also, subsequent edits to the same property no longer
create multiple undo commands. And in many cases, undo commands are now
automatically removed when they become "obsolete", for example when making a
layer visible and then invisible again.

## Changelog

Many other improvements and fixes could not be mentioned above. Here's the full summary of the changes.

* Added support for custom enum properties (with svipal, [#2941](https://github.com/mapeditor/tiled/pull/2941))
* Added support for custom class properties ([#489](https://github.com/mapeditor/tiled/issues/489))
* Added parallax origin property to the map (with krukai, [#3209](https://github.com/mapeditor/tiled/pull/3209))
* Added Repeat X/Y properties to Image Layers (with krukai, [#3205](https://github.com/mapeditor/tiled/pull/3205))
* Added an action for selecting all layers (Ctrl+Alt+A) ([#3081](https://github.com/mapeditor/tiled/pull/3081))
* Added actions to select or add tilesets to Project view context menu
* Added cut/copy/paste actions to Tile Animation Editor
* Improved undo behavior by merging sequential edits to the same property ([#3103](https://github.com/mapeditor/tiled/issues/3103))
* Improved multi-layer painting behavior ([#3094](https://github.com/mapeditor/tiled/issues/3094))
* Separated the X and Y components of the major grid option ([#3208](https://github.com/mapeditor/tiled/issues/3208))
* Added automatic fading out of the grid when zooming out a lot
* AutoMapping: Made it find layers within groups ([#1771](https://github.com/mapeditor/tiled/issues/1771))
* AutoMapping: regions layer can now be used alongside region_input/output layers
* AutoMapping: Recognize "//" layer name prefix for ignoring layers ([#3262](https://github.com/mapeditor/tiled/issues/3262))
* AutoMapping: Allow setting a rule map as project rules file ([#3221](https://github.com/mapeditor/tiled/issues/3221))
* Tweaked focus behavior in the Template Editor
* Changed the default Terrain Brush shortcut back to T
* Reset tile animations when disabling playback and when exporting as image
* Don't require saving maps upon creation ([#1902](https://github.com/mapeditor/tiled/issues/1902))
* Apply transformation actions to the preview while placing tiles ([#3091](https://github.com/mapeditor/tiled/issues/3091))
* Allow using object context menu in object creation tools
* Reduced the step size for the parallax factor property
* Improved the logic for automatically switching tools ([#2807](https://github.com/mapeditor/tiled/issues/2807))
* Ignore selection changes when marking a file as modified ([#3194](https://github.com/mapeditor/tiled/issues/3194))
* Use the tileset background color in the collision editor (with Benja Appel, [#3163](https://github.com/mapeditor/tiled/pull/3163))
* Show the read error when using --export-map/tileset
* Avoid deselecting all layers when clicking empty area in Layers view ([#2806](https://github.com/mapeditor/tiled/issues/2806))
* Scripting: Added File API
* Scripting: Added tiled.applicationDirPath property
* Scripting: Added tiled.extensionsPath property ([#3139](https://github.com/mapeditor/tiled/issues/3139))
* Scripting: Added missing Layer.tintColor property
* Scripting: Added missing ObjectGroup.drawOrder property ([#3147](https://github.com/mapeditor/tiled/issues/3147))
* Scripting: Added TileMap.removeObjects ([#3149](https://github.com/mapeditor/tiled/issues/3149))
* Scripting: Added TileMap.regionEdited signal
* Scripting: Added TileMap.layers and GroupLayer.layers properties, for convenience
* Scripting: Added region.rects property and region.contains(x,y)
* Scripting: Treat custom format extensions as case-insensitive ([#3141](https://github.com/mapeditor/tiled/issues/3141))
* Scripting: Allow tools to stay active when tiles or a terrain type are selected ([#3201](https://github.com/mapeditor/tiled/issues/3201))
* Scripting: Extended the terrain related API ([#2663](https://github.com/mapeditor/tiled/issues/2663))
* Scripting: tiled.activeAsset can be assigned asset created in script ([#3160](https://github.com/mapeditor/tiled/issues/3160))
* Scripting: Fixed possible crash after creating tilesets from script ([#3229](https://github.com/mapeditor/tiled/issues/3229))
* Scripting: Fixed possible crash in TileMap.autoMap
* Scripting: Fixed dialog window titles to show on macOS ([#2910](https://github.com/mapeditor/tiled/issues/2910))
* Scripting: Fixed tileset or tile references for maps loaded from script
* Scripting: Avoid crash when script reload happens during popup ([#2991](https://github.com/mapeditor/tiled/issues/2991))
* Fixed the logic for handling group layer parallax factors (with LilithSilver, [#3125](https://github.com/mapeditor/tiled/pull/3125))
* Fixed keyboard modifiers getting stuck for Terrain Brush ([#2678](https://github.com/mapeditor/tiled/issues/2678))
* Fixed debug messages showing in the Console and Issues views
* Fixed enabled state of File > Export action for tilesets ([#3177](https://github.com/mapeditor/tiled/issues/3177))
* Fixed Snap to Grid for hexagonal maps
* Fixed AutoMapping rules file to update after changing project properties ([#3176](https://github.com/mapeditor/tiled/issues/3176))
* Fixed 'Detect Bounding Box' action missing in Keyboard settings
* Fixed toggling "Clear View" on & off shifting the map
* Fixed command-line output not showing on Windows ([#2688](https://github.com/mapeditor/tiled/issues/2688))
* Fixed "Select object on map" when no object layer is selected ([#3207](https://github.com/mapeditor/tiled/issues/3207))
* Fixed adjusting of tile types when tileset width changed (by Albert Vaca Cintora, [#3237](https://github.com/mapeditor/tiled/pull/3237))
* Fixed missing Qt translations for Linux AppImage
* Fixed minimap viewport position when layers are offset ([#3211](https://github.com/mapeditor/tiled/issues/3211))
* Fixed "Highlight Current Layer" getting confused ([#3223](https://github.com/mapeditor/tiled/issues/3223))
* Fixed Terrain Set type property to be disabled when appropriate (avoids crash)
* Fixed saving broken references to files loaded using "ext:" prefix ([#3185](https://github.com/mapeditor/tiled/issues/3185))
* Fixed performance issue in Project view related to file icons
* Fixed dynamic wrapping when adding tiles to a collection ([#3076](https://github.com/mapeditor/tiled/issues/3076))
* Fixed potential crash when changing a WangSet from script
* Tiled Manual is now available in French
* JSON plugin: Added "tmj", "tsj" and "tj" as accepted file extensions
* YY plugin: Don't use safe writing of files
* YY plugin: Write out custom "object" properties as instance name (instead of the ID)
* YY plugin: Determine sprite names by looking for meta files (by krukai, [#3213](https://github.com/mapeditor/tiled/pull/3213))
* CSV plugin: Improved handling of infinite maps
* RpMap plugin: Fixed hardcoded exported tile size ([#3184](https://github.com/mapeditor/tiled/discussions/3184))
* libtiled-java: Introduced TilesetCache interface (by Samuel Manflame, [#3117](https://github.com/mapeditor/tiled/pull/3117))
* Added Ukrainian translation to Windows installer ([#3132](https://github.com/mapeditor/tiled/issues/3132))
* Updated to Sentry 0.4.14
* Updated Bulgarian, Chinese (Simplified), French, Korean, Portuguese (Brasil), Portuguese (Portugal), Russian, Swedish and Turkish translations

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
[parallax-origin]: https://doc.mapeditor.org/en/stable/manual/layers/#parallax-reference-point
[scripting-api]: https://www.mapeditor.org/docs/scripting
[scripting-file]: https://www.mapeditor.org/docs/scripting/interfaces/File.html
[scripting-activeAsset]: https://www.mapeditor.org/docs/scripting/modules/tiled.html#activeAsset
[scripting-region]: https://www.mapeditor.org/docs/scripting/interfaces/region.html
[tiled-api]: https://www.npmjs.com/package/@mapeditor/tiled-api
[automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[scripting-automap]: https://www.mapeditor.org/docs/scripting/classes/TileMap.html#autoMap
[parallax-group-layer-fix]: https://github.com/mapeditor/tiled/issues/3124
[scripting-modules]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Modules
