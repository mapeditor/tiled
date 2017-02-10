---
layout: post
title: Tiled 0.17.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1574
---

[Tiled 0.17](https://thorbjorn.itch.io/tiled) is out now, containing almost 5 months of improvements since [Tiled 0.16](http://discourse.mapeditor.org/t/tiled-0-16-0-released/1206)! Here's a summary of the changes, followed by the full change log.

## Tiled Fusion Theme

The most visible change is surely the customized cross-platform theme, which can also be used in dark mode and can be colored to your liking. A lot of effort went into automatically adapting to the chosen level of brightness and providing enough contrast where needed.

Here we have default gray and customized dark:

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/6a4f575e753fab490883d68181e286be41e9a5fa.png" width="690" height="449">

But you can also choose a slight color to change the mood:

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/f026159bf845e935d36098e528a654b0f500151b.png" width="690" height="449">

This theme is now the default on Windows and Linux. While I think it already looks awesome, any suggestions for further improvements are very welcome!

## Custom Properties

Continueing the improvements to custom properties introduced in Tiled 0.16, there are now two new custom property types: color and file. Using these types makes setting their values easier by providing a color picker and a file chooser respectively, and it also standardizes the way their values are stored. File properties will store as a relative path from the map file.

In addition, string properties can now be multi-line. This makes it much easier to use them for including lists or short scripts.

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/9bb5f01d892ab8bde777066d124ee95ec6427343.png" width="690" height="214">

## Editing Tools

You can now right-click drag with the Eraser to quickly erase large areas. Creating new rectangle/ellipse objects is now possible by dragging in any direction. And for the object selection tool, the behavior on resize, move and rotate has been improved to be more responsive and to avoid the small jump at the start.

## Other Noteworthy Improvements

Often you need to quickly access the current file in another application, or access related files stored in the same folder. This is especially the case for poor people who need to work with Perforce. I've added "Copy File Path" and "Open Containing Folder" actions to the tab context menu, to make this easier:

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/ad8eaaa7dbcb3737e6e75a173811b0b8ec7f7b9e.png" width="330" height="82">

The New Tileset dialog now features a color picker for choosing the transparent color:

<img src="http://discourse.mapeditor.org/uploads/mapeditor/original/1X/65408a190550c9d3721a0059bc839edbd6f34255.png" width="561" height="373">

Finally, it is now possible to drag the tileset view with the middle mouse button as well, a much requested feature.

## Change log

* Added a platform-independent theme, which can be dark ([#786](https://github.com/bjorn/tiled/issues/786))
* Added Paste in Place action for objects ([#1257](https://github.com/bjorn/tiled/issues/1257))
* Added custom property type 'color' ([#1275](https://github.com/bjorn/tiled/issues/1275))
* Added custom property type 'file' ([#1278](https://github.com/bjorn/tiled/issues/1278))
* Added option for removing invisible objects in resize dialog ([#1032](https://github.com/bjorn/tiled/issues/1032), by Mamed Ibrahimov)
* Added support for editing multi-line string properties ([#205](https://github.com/bjorn/tiled/issues/205))
* Added %layername and %objectid to available command variables
* Added support for scrolling in tileset view with middle mouse button ([#1050](https://github.com/bjorn/tiled/issues/1050), with Will Luongo)
* Added a rectangle erase mode to the eraser ([#1297](https://github.com/bjorn/tiled/issues/1297))
* Added export to Defold .tilemap files ([#1316](https://github.com/bjorn/tiled/pull/1316), by Nikita Razdobreev)
* Added simple full screen mode
* Added "Copy File Path" and "Open Containing Folder" actions to tab context menu
* Added warning when saving with the wrong file extension
* Added color picker for setting transparent color of a tileset ([#1173](https://github.com/bjorn/tiled/issues/1173), by Ava Brumfield)
* Various object selection tool improvements
* Allow creating rectangle/ellipse objects in any direction ([#1300](https://github.com/bjorn/tiled/issues/1300))
* Enabled nested views and grouped dragging for stacked views ([#1291](https://github.com/bjorn/tiled/issues/1291))
* Fixed updating object drag cursor when exiting resize handles ([#1277](https://github.com/bjorn/tiled/issues/1277))
* Fixed tile animations to stay in sync when changing them ([#1288](https://github.com/bjorn/tiled/issues/1288))
* Fixed preservation of tile meta-data when tileset width is changed ([#1315](https://github.com/bjorn/tiled/issues/1315))
* Windows and OS X releases now built against Qt 5.7
* Updated Bulgarian, Dutch, German, Norwegian Bokmål, Russian, Spanish and Turkish translations

## A Look Ahead

Those following the releases of the past year know that actually I am trying hard to get to a Tiled 1.0 release that I would consider "feature complete" and that is well documented, and which would provide a basis upon which to continue building cool and useful features. Tiled 0.17 became packed with new features but isn't that release yet. So I will continue working towards making it happen!

## Support Tiled Development :heart:

I am only able to keep developing Tiled at this pace thanks to the support from [almost 160 patrons](https://www.patreon.com/bjorn?ty=h) and many people choosing to [pay for Tiled on itch.io](https://thorbjorn.itch.io/tiled/purchase) when downloading it. I'm already spending _two full days per week_ on Tiled, but I did _not yet_ reach my funding goal for this so please join in if you're liking the recent developments and would like to see more!

In return I will be keeping you up to date with Tiled developments. Even a [small $1/month donation](https://www.patreon.com/bePatron?u=90066&rid=147403&exp=1&patAmt=1.0) keeps you involved and helps a lot, thanks!
