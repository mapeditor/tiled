---
layout: post
title: Tiled 1.4 Beta released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

That's right, today's [development snapshot][snapshot] is the Tiled 1.4 Beta which all the adventurous are encouraged to give a try. Now is the best time to check out all the new features and to provide feedback to help make sure Tiled 1.4 will be a solid release!

So what's new? Well, for those who don't have time to read through the 10 [devlogs](https://thorbjorn.itch.io/tiled/devlog) about the development snapshots that have led up to this release the full list of relevant changes is summarized below. I've tried to put the major changes at the top.

Changelog
---------

*   Added [support for projects](https://thorbjorn.itch.io/tiled/devlog/113135/introducing-projects) ([#1665](https://github.com/bjorn/tiled/issues/1665))
*   Added object reference property type (with Steve Le Roy Harris and Phlosioneer, [#707](https://github.com/bjorn/tiled/issues/707))
*   Added [world editing tool](https://doc.mapeditor.org/en/latest/manual/worlds/#editing-worlds) for adding/removing and moving around maps in a world (with Nils Kübler, [#2208](https://github.com/bjorn/tiled/pull/2208))
*   Added a quick "Open file in Project" (Ctrl+P) action
*   Added new Object Alignment property to Tileset (with Phlosioneer, [#91](https://github.com/bjorn/tiled/issues/91))
*   Added [layer tint color](https://thorbjorn.itch.io/tiled/devlog/120663/tinting-alignment-and-object-references) (by Gnumaru, [#2687](https://github.com/bjorn/tiled/pull/2687))
*   Added support for [using maps as images](https://thorbjorn.itch.io/tiled/devlog/124453/use-maps-as-tilesets-and-show-object-references) (with Phlosioneer, [#2708](https://github.com/bjorn/tiled/pull/2708))
*   Added 'Open with System Editor' action for custom file properties ([#2172](https://github.com/bjorn/tiled/issues/2172))
*   Added option to render object names when exporting as image ([#2216](https://github.com/bjorn/tiled/issues/2216))
*   Added 'Replace Tileset' action to Tilesets view
*   Added shortcut to tooltips for all registered actions
*   Added automatic reloading of object templates (by Phlosioneer, [#2699](https://github.com/bjorn/tiled/pull/2699))
*   Added 'Clear Console' button and context menu action ([#2220](https://github.com/bjorn/tiled/issues/2220))
*   Added 'Reopen Closed File' (Ctrl+Shift+T) action
*   Added status bar button to toggle the Console view
*   Added a border around the tile selection highlight
*   Switch current tileset tab if all selected tiles are from the same tileset (by Mitch Curtis, [#2792](https://github.com/bjorn/tiled/pull/2792))
*   Made tileset dynamic wrapping toggle persistent
*   Properties view: Added action for adding a property to context menu ([#2796](https://github.com/bjorn/tiled/issues/2796))
*   Optimized loading of CSV tile layer data (by Phlosioneer, [#2701](https://github.com/bjorn/tiled/pull/2701))
*   Improved map positioning when toggling 'Clear View'
*   Remember the preferred format used for saving
*   Removed the Maps view (replaced by Project view)
*   Removed file system hierarchy from Templates view (replaced by Project view)
*   Fixed the status bar placement to be always at the bottom of the window
*   Fixed issue where image layer images cannot be loaded from Qt resource files (by obeezzy, [#2711](https://github.com/bjorn/tiled/pull/2711))
*   GmxPlugin: Added support for layer tint color
*   Scripting: Assign global variables to console script evaluations (by Phlosioneer, [#2724](https://github.com/bjorn/tiled/pull/2724))
*   Scripting: Added coordinate conversion to TileMap
*   Scripting: Added support for custom "file" properties
*   Scripting: Added checks for nullptr arguments (by Phlosioneer, [#2736](https://github.com/bjorn/tiled/pull/2736))
*   Scripting: Added some missing tileset related properties
*   Scripting: Provide access to registered file formats (by Phlosioneer, [#2716](https://github.com/bjorn/tiled/pull/2716))
*   Scripting: Enabled scripted formats to be used on the command-line
*   Scripting: Introduced \_\_filename global value (with konsumer)
*   docs: Clarify "can contain" documentation and error handling (by Phlosioneer, [#2702](https://github.com/bjorn/tiled/pull/2702))
*   docs: Document all optional attributes, update some docs (by Phlosioneer, [#2705](https://github.com/bjorn/tiled/pull/2705))
*   docs: Alphabetize scripting API reference (by Phlosioneer, [#2720](https://github.com/bjorn/tiled/pull/2720))
*   docs: Added missing BinaryFile constructor docs (by Phlosioneer, [#2732](https://github.com/bjorn/tiled/pull/2732))
*   docs: Enabled Algolia powered search
*   libtiled-java: Big update to support newer TMX attributes (by Mike Thomas, [#1925](https://github.com/bjorn/tiled/pull/1925))
*   libtiled-java: Fixed writing of the tile type (by Phlosioneer, [#2704](https://github.com/bjorn/tiled/pull/2704))

String Freeze
-------------

This beta release also marks the string freeze, which is when I do my best to no longer change any translatable strings. This means now is a good time to start translating Tiled to your preferred language. Of course, a lot of translations already exist and for some we have quite dedicated translators, so please always get in contact if you want to join this effort so we can coordinate it.

For those interested, see the [instructions about translating Tiled](https://github.com/bjorn/tiled/wiki/Translating-Tiled) and the current [translations listed on Weblate](https://hosted.weblate.org/projects/tiled/translations/).

Remaining Work
--------------

Apart from updating translations, there is still a bunch of work remaining. Some scripting APIs remain to be added (most importantly, access to the current project, but also some file system functionality), a number of bugs should still be looked into and last but not least [the manual](https://doc.mapeditor.org/en/latest/) still needs to be updated with some of the major new features. I expect it to be about two weeks until the final release.

_Looking forward to the next new feature release? Please consider supporting me on [GitHub Sponsors](https://github.com/sponsors/bjorn), [Patreon](https://www.patreon.com/bjorn) or [Liberapay](https://liberapay.com/Tiled). I depend on your support to keep improving Tiled! Thank you!_

[snapshot]: https://thorbjorn.itch.io/tiled/devlog/149768/tiled-14-beta-released
