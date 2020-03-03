---
layout: post
title: Tiled 1.3.3 released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch release addresses several important issues and brings an updated Bulgarian translation. Many crashes were fixed in the scripting API, compatibility with GameMaker: Studio 1.4.9999 was fixed (no more need for re-saving in the room editor) and setting the compression level to the default for older maps was fixed.

_**Note:** Due to broken loading of the compression level, maps created with Tiled versions older than 1.3.0 would start using the worst compression level (level 0) when saved with Tiled 1.3. If you're using compressed layer data (any format), make sure the compressionlevel attribute is not set to 0. Tiled 1.3.3 and beyond will not write the compression level when it is left on the default (-1)._

Changelog
---------

* Fixed loading of compression level ([#2753](https://github.com/bjorn/tiled/issues/2753))
* Fixed default value for Hex Side Length property
* Fixed hiding of status bar text for some tools
* Fixed removing of object labels when removing a group layer
* GmxPlugin: Fixed compatibility with GameMaker: Studio 1.4.9999
* Scripting: Made TextFile.commit and BinaryFile.commit close as well
* Scripting: Fixed crashes when modifying certain new objects ([#2747](https://github.com/bjorn/tiled/issues/2747))
* Scripting: Fixed potential crash in Asset.macro/undo/redo/isModified
* Scripting: Fixed potential crash when accessing Tool.preview
* Scripting: Fixed loading of images from extensions folder
* Scripting: Reload extensions also when files are added/removed
* Updated Bulgarian translation (by Любомир Василев)

Many thanks to all who've reported issues and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, be aware that I depend on your support to be able to keep improving this tool. Please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). Thanks!_
