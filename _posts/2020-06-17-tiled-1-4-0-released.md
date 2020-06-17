---
layout: post
title: Tiled 1.4 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

At long last I can announce the immediate release of Tiled 1.4.0! The biggest
change in this release is the support for projects and sessions, but there
have been many other welcome improvements as well.

### Projects and Sessions

Until now, Tiled has been pretty comfortable to work with on a single project
at a time. It remembered the tile size or export format you used last time,
reopened the files you had open, etc. Unfortunately, all of that is rarely
helpful as soon you're doing multiple projects. There was also a Maps view
that shows you any maps in a certain folder, but you'd have to navigate to
another folder when switching projects. This is why for a long time I wanted
to add support for projects, and now they are finally there!

Once you add a [project file], it's suddenly easier to collaborate. Teammates
can open the project and will immediately get a new session and can see the
maps and tilesets in your project in the new *Project* view. The last used
settings, open files and loaded worlds, all of that is now stored in the
session that is created alongside your project. And you can quickly open any
file in the project using the new *Open File in Project* (``Ctrl+P``) action.

<a href="/img/posts/2020-06-open-file-in-project.png" class="thumbnail" style="margin: 20px 40px;">
  <img src="/img/posts/2020-06-open-file-in-project.png" alt="Open File in Project Action">
</a>

The project can also come with its own [extensions], [Automapping] rules, [object
types] and [custom commands]. And we'll surely be adding more to this list in the
future!

With the addition of the *Project* view, the *Maps* view is now removed, as is
the file system view that was part of the *Templates* view (now called
*Template Editor*, which works together with the files shown in the *Project*
view).

### Connections Between Objects

Tiled can now render [connections between objects] on your map. To enable this,
there is a new type of custom property called "object", which is stored as the
unique ID of the object that is referenced. The object reference can be set
using a search dialog or by directly picking the target object on the map.

<a href="/img/posts/2020-06-connections-between-objects.png" class="thumbnail" style="margin: 20px 40px;">
  <img src="/img/posts/2020-06-connections-between-objects.png" alt="Connections Between Objects">
</a>

Each connection is rendered as an arrow matching the color of the target
object. A "Go to Object" context menu action was added, which is useful if you
have the connections turned off or the target object is far away.

### World Editing

The [worlds feature](https://doc.mapeditor.org/en/stable/manual/worlds/) added
in [Tiled 1.2][tiled-1-2] so far required manually editing the JSON file,
which was especially annoying if you were positioning each map individually.
For this case, there is now a new [*World
Tool*](https://doc.mapeditor.org/en/stable/manual/worlds/#editing-worlds)
which allows easily adding, removing and moving around maps within the world.

There's now also a *Map -> New World...* action that makes it easy to start
using this feature.

### Tile Object Alignment

The inconsistent alignment for tile objects (usually bottom-left) vs.
rectangle objects (top-left) has been a [long-standing frustration][#91] to
many. To resolve this problem, a new tileset property *Object Alignment* was
added, which controls the alignment for any tile objects referring to that
tileset.

By setting this property to *Top Left*, tile objects align consistently with
rectangles. But it can also be used for example to have your sprites
bottom-center aligned or to center them at their position.

### Scripting

The support for extending Tiled with JavaScript, which was added in [Tiled
1.3][tiled-1-3], also saw a number of improvements. The *Console* now
automatically assigns numbered variables to the result of evaluations. APIs
were added for [coordinate conversion], working with [file paths] and
accessing [inherited custom properties]. Also, scripts can now access and use
any registered file formats and scripted file formats are now available when
exporting via the command-line.

### Other New Features

Maps can now be used as images, for example as the source image of a tileset.
This is useful to create a kind of virtual tilesets where you combine smaller
tiles into larger tiles (also known as metatiles).

Layers can now also be [tinted with a custom color], which is also supported
by the [GameMaker: Studio 1.4 export].

<a href="/img/posts/2020-06-tinting-layers.png" class="thumbnail" style="margin: 20px 40px;">
  <img src="/img/posts/2020-06-tinting-layers.png" alt="Connections Between Objects">
</a>

A number of usability improvements were made as well. All buttons now show
their shortcuts in their tool tips, there's a new action to reopen the last
closed file (`Ctrl+Shift+T`), the status bar gained a toggle button for the
Console, etc. Check out the changelog below for the full list!

### Changelog

- Added [support for projects][project file] ([#1665])
- Added [object reference property][connections between objects] type (with Steve Le Roy Harris and Phlosioneer, [#707])
- Added world editing tool for adding/removing and moving around maps in a world (with Nils Kübler, [#2208])
- Added a quick "Open file in Project" (`Ctrl+P`) action
- Added new Object Alignment property to Tileset (with Phlosioneer, [#91])
- Added [layer tint color][tinted with a custom color] (by Gnumaru, [#2687])
- Added support for using maps as images (with Phlosioneer, [#2708])
- Added 'Open with System Editor' action for custom file properties ([#2172])
- Added option to render object names when exporting as image ([#2216])
- Added 'Replace Tileset' action to Tilesets view
- Added shortcut to tooltips for all registered actions
- Added automatic reloading of object templates (by Phlosioneer, [#2699])
- Added 'Clear Console' button and context menu action ([#2220])
- Added 'Reopen Closed File' (`Ctrl+Shift+T`) action
- Added status bar button to toggle the Console view
- Added a border around the tile selection highlight
- Switch current tileset tab if all selected tiles are from the same tileset (by Mitch Curtis, [#2792])
- Made tileset dynamic wrapping toggle persistent
- Properties view: Added action for adding a property to context menu ([#2796])
- Optimized loading of CSV tile layer data (by Phlosioneer, [#2701])
- Improved map positioning when toggling 'Clear View'
- Remember the preferred format used for saving
- Normalize rotation values when rotating objects ([#2775])
- Removed the Maps view (replaced by Project view)
- Removed file system hierarchy from Templates view (replaced by Project view)
- Fixed potential crash when triggering AutoMap ([#2766])
- Fixed the status bar placement to be always at the bottom of the window
- Fixed potential issue with automatic reloading of files ([#1904])
- Fixed issue where image layer images cannot be loaded from Qt resource files (by obeezzy, [#2711])
- GmxPlugin: Added support for layer tint color
- Scripting: Assign global variables to console script evaluations (by Phlosioneer, [#2724])
- Scripting: Added [coordinate conversion] to TileMap
- Scripting: Added support for custom "file" properties
- Scripting: Added checks for nullptr arguments (by Phlosioneer, [#2736])
- Scripting: Added some missing tileset related properties
- Scripting: Added [FileInfo API][file paths] with various file path operations (with David Konsumer, [#2822])
- Scripting: Provide access to registered file formats (by Phlosioneer, [#2716])
- Scripting: Enabled scripted formats to be used on the command-line
- Scripting: Added functions to [access inherited properties][inherited custom properties] (by Bill Clark, [#2813])
- Scripting: Introduced `__filename` global value (with konsumer)
- Scripting: Fixed `ObjectGroup.insertObjectAt` to use the index
- docs: Many updates for Tiled 1.4
- docs: Clarify "can contain" documentation and error handling (by Phlosioneer, [#2702])
- docs: Document all optional attributes, update some docs (by Phlosioneer, [#2705])
- docs: Alphabetize scripting API reference (by Phlosioneer, [#2720])
- docs: Added missing BinaryFile constructor docs (by Phlosioneer, [#2732])
- docs: Enabled Algolia powered search
- libtiled-java: Big update to support newer TMX attributes (by Mike Thomas, [#1925])
- libtiled-java: Fixed writing of the tile type (by Phlosioneer, [#2704])
- libtiled-java: Enable loading of maps from jar files (by Adam Hornáček, [#2829])
- Updated Bulgarian, Chinese (Simplified), Czech, Finnish, French, Norwegian Bokmål, Portuguese (Portugal) and Turkish translations

Thanks to everybody who contributed to this release with bug reports,
suggestions, patches or translation updates!

## A Look Ahead

The 1.4 release started with a small plan that included only two major
features. The idea was that this would shorten the development time on the
release. While those features implied more work than I had anticipated, the
release was also delayed because of the huge amount of other things that got
done.

I'd like to do new feature releases more often, for example every three
months. My idea is to do this by always keeping the main branch in a
releasable state. The main branch will be the stable branch and development
(including testing and documentation) of new features will happen on temporary
branches. If this works out, we can expect Tiled 1.5 around September,
including whatever got done until then.

On the list of things I'd like to work on are currently to extend the custom
property system to include custom types, compounds, arrays and enums. There
are also some scripting APIs still pending and I'd like to try a number of
visual improvements, including making a unified and scalable icon set. In the
end though, I'm driven mostly by feedback, especially from those who are
supporting me, so let me know what you'd like to see!

## Support Tiled Development <img src="/img/heart.png" style="width: 1em;" title=":heart:" class="emoji" alt=":heart:">

This new release was made possible by about 300 [patrons][patreon] and
[sponsors][sponsors] supporting me on a monthly basis as well as many people
choosing to pay for [Tiled on itch.io][itch] and some who donated through
[Liberapay][liberapay]. To ensure I will be able to keep developing Tiled at
this pace, please [set up a small monthly donation][donate]!

Your donation primarily enables me to work 2 full days/week on Tiled. With
additional funds I can spend more time on Tiled every once in a while. Let's
make this tool even better!

[tiled-1-2]: {{ site.baseurl }}{% post_url 2018-09-18-tiled-1-2-0-released %}
[tiled-1-3]: {{ site.baseurl }}{% post_url 2019-11-13-tiled-1-3-0-released %}
[milestone]: https://github.com/bjorn/tiled/milestone/18
[#1665]: https://github.com/bjorn/tiled/issues/1665
[#707]: https://github.com/bjorn/tiled/issues/707
[#2208]: https://github.com/bjorn/tiled/pull/2208
[#91]: https://github.com/bjorn/tiled/issues/91
[#2687]: https://github.com/bjorn/tiled/pull/2687
[#2708]: https://github.com/bjorn/tiled/pull/2708
[#2172]: https://github.com/bjorn/tiled/issues/2172
[#2216]: https://github.com/bjorn/tiled/issues/2216
[#2699]: https://github.com/bjorn/tiled/pull/2699
[#2220]: https://github.com/bjorn/tiled/issues/2220
[#2792]: https://github.com/bjorn/tiled/pull/2792
[#2796]: https://github.com/bjorn/tiled/issues/2796
[#2701]: https://github.com/bjorn/tiled/pull/2701
[#2775]: https://github.com/bjorn/tiled/issues/2775
[#2766]: https://github.com/bjorn/tiled/issues/2766
[#1904]: https://github.com/bjorn/tiled/issues/1904
[#2711]: https://github.com/bjorn/tiled/pull/2711
[#2724]: https://github.com/bjorn/tiled/pull/2724
[#2736]: https://github.com/bjorn/tiled/pull/2736
[#2822]: https://github.com/bjorn/tiled/pull/2822
[#2716]: https://github.com/bjorn/tiled/pull/2716
[#2813]: https://github.com/bjorn/tiled/pull/2813
[#2702]: https://github.com/bjorn/tiled/pull/2702
[#2705]: https://github.com/bjorn/tiled/pull/2705
[#2720]: https://github.com/bjorn/tiled/pull/2720
[#2732]: https://github.com/bjorn/tiled/pull/2732
[#1925]: https://github.com/bjorn/tiled/pull/1925
[#2704]: https://github.com/bjorn/tiled/pull/2704
[#2829]: https://github.com/bjorn/tiled/pull/2829
[patreon]: https://www.patreon.com/bjorn
[sponsors]: https://github.com/sponsors/bjorn
[donate]: https://www.mapeditor.org/donate
[itch]: https://thorbjorn.itch.io/tiled
[liberapay]: https://liberapay.com/Tiled/
[project file]: https://doc.mapeditor.org/en/stable/manual/projects/
[coordinate conversion]: https://doc.mapeditor.org/en/stable/reference/scripting/#script-map-screentotile
[tinted with a custom color]: https://doc.mapeditor.org/en/stable/manual/layers/#tinting-layers
[GameMaker: Studio 1.4 export]: https://doc.mapeditor.org/en/stable/manual/export/#gamemaker-studio-1-4
[file paths]: https://doc.mapeditor.org/en/stable/reference/scripting/#fileinfo
[inherited custom properties]: https://doc.mapeditor.org/en/stable/reference/scripting/#script-object-resolvedproperty
[extensions]: https://doc.mapeditor.org/en/stable/reference/scripting/
[Automapping]: https://doc.mapeditor.org/en/stable/manual/automapping/
[object types]: https://doc.mapeditor.org/en/stable/manual/custom-properties/#predefining-properties
[custom commands]: https://doc.mapeditor.org/en/stable/manual/using-commands/#
[connections between objects]: https://doc.mapeditor.org/en/stable/manual/objects/#connecting-objects
