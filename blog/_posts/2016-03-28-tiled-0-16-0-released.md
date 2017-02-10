---
layout: post
title: Tiled 0.16.0 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1206
---

I've just released [Tiled 0.16](https://thorbjorn.itch.io/tiled), bringing all the new features developed this year since [Tiled 0.15](http://discourse.mapeditor.org/t/tiled-0-15-0-released/1001). Here are the highlights!

## Improved Custom Properties

Many improvements have been made related to [custom properties](http://doc.mapeditor.org/manual/custom-properties/). First off, if you were annoyed that you had to add each property individually to each object, you will really enjoy the new Object Types Editor, which allows you to predefine a set of properties for each object type.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/27763a0b286d522d994e06fa0a1d25bd6057f758.png" width="690" height="305">

Once defined, you only need to specify the type of the object and Tiled will display these properties in the familiar "Custom Properties" section, ready for setting their value.

Also, when adding custom properties you are now not only prompted for their name, but also their type. Initially, Tiled supports the basic types "string", "int", "float" and "bool". Choosing a type will help with editing the value later, since a custom editor will be used. It also helps to avoid quoting of the values in the JSON and Lua export formats.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/8b291ed6bcdc8aab6eea93fedce13161368b1f2c.png" width="327" height="118">

Finally, the Properties view is now also available in the Tile Collision Editor. This allows you to set properties on the collision shapes, like density, friction and restitution.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/5c8dc3d4c257b82a52df0d603d5d9090f74a5d79.png" width="690" height="410">

The [usage of custom properties is now also documented](http://doc.mapeditor.org/manual/custom-properties/), being one of the first basic features in Tiled to see a proper section in the manual.

## Auto-Updates and New Installer

On Windows and OS X, Tiled will now automatically check for updates periodically (no more than once a day or week), and notify you when there is a newer version. Of course, you can opt out of this if you want, and there is also a button to check for updates manually.

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/18adad3eaba660fe03daada885c5be9d61939bc1.png" width="427" height="178">

When an update is found, it'll show the release notes and offer to automatically download and install the update. This should be really helpful in staying up-to-date in the future!

Also, the Tiled installer for Windows is now based on MSI. This new installer should offer a better overall experience, especially regarding upgrades or switching between versions.

## Other Things Worth Mentioning

It is now possible to reset color properties, for example clearing the Color value on an Object Layer:

<img src="http://discourse.mapeditor.org/uploads/default/original/1X/d1db2ab3427954a394f98b487556e3ac965f54dd.png" width="369" height="210">

An [eraser mode](http://discourse.mapeditor.org/t/february-2016-development-updates/1104/3?u=bjorn) was added to the Terrain Brush, accessible via a button at the bottom of the Terrains view.

When you try to open map files through the file manager, it will now try to reuse an existing Tiled instance rather than opening a new Tiled for each file. This also works with multiple files or for opening files from the command prompt.


## Change log

* Added checking for updates, based on Sparkle and WinSparkle
* Added default property definitions to object types (with Michael Bickel)
* Added types to custom properties: string, float, int, boolean (with CaptainFrog)
* Added Properties view to the Tile Collision Editor (by Seanba)
* Added a reset button for color properties
* Added eraser mode to Terrain Brush and fixed some small issues
* Reuse existing Tiled instance when opening maps from the file manager (with Will Luongo)
* Allow setting tile probability for multiple tiles (by Henrik Heino)
* New MSI based installer for Windows
* Optimized selection of many objects
* libtiled-java: Fixed loading of maps with CSV layer data that are not square (by Zachary Jia)
* Fixed potential crash when having Terrain Brush selected and switching maps
* Updated Dutch, French, German, Japanese, Russian and Spanish translations


## A Look Ahead

When Tiled 0.15 was released, I was confident the next release could be Tiled 1.0. A lot of work has already been done on [editing of external tilesets](http://discourse.mapeditor.org/t/march-2016-development-updates/1164), but I feel like it may have been just the beginning. That's why I wanted to get the above features out first. Now, I will continue to work towards making Tiled ready for its 1.0 release!

## Support Tiled Development :heart:

With your support I can spend more time developing Tiled, bringing you frequently requested features faster! Please consider joining the [almost 140 patrons](https://www.patreon.com/bjorn?ty=h) who are spending a small amount each month to make this possible. In return I will be keeping you up to date with Tiled developments! Even a [small $1/month donation](https://www.patreon.com/bePatron?u=90066) keeps you involved and helps a lot, thanks!
