---
layout: post
title: Tiled 1.8.5 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release fixes a number of small issues and updates the Chinese and
Portuguese translations.

Changelog
---------

*   Made expanded group layers persistent ([#3282](https://github.com/mapeditor/tiled/issues/3282))
*   Improved snapping behavior for scalable objects on staggered maps
*   Allow setting a shortcut on the 'Edit Tileset' action
*   Always select first entry while using the Open File in Project action
*   Improved Add Property dialog layout in case of long type names ([#3302](https://github.com/mapeditor/tiled/issues/3302))
*   Fixed restoring of window layout when maximized ([#590](https://github.com/mapeditor/tiled/issues/590))
*   Fixed snapping when dragging templates into a map ([#3326](https://github.com/mapeditor/tiled/issues/3326))
*   Fixed map selection rectangle in world for infinite maps ([#3340](https://github.com/mapeditor/tiled/issues/3340))
*   Fixed 'Merge Layer Down' action for infinite maps
*   Fixed several small issues in the image color picker ([#3348](https://github.com/mapeditor/tiled/issues/3348))
*   Fixed missing name for undo commands that add/remove maps from world
*   Fixed selection issues for tile objects with a non-zero tile offset
*   Fixed hover indicator sometimes overlapping selection indicator
*   Fixed removal of terrain info when removing tiles from a collection
*   Scripting: Fixed region.rects when compiled against Qt 5.9 to 5.13
*   Scripting: Layer.tintColor is now `#ffffff` when not set
*   macOS: Enabled support for loading SVGs
*   macOS: Show shortcuts in context menus when using Tiled Fusion style ([#1978](https://github.com/mapeditor/tiled/issues/1978))
*   AppImage: Updated to Sentry 0.4.17
*   Updated Chinese (Simplified) and Portuguese (Portugal) translations

### Thanks!

Many thanks to all who've reported issues or have made suggestions, and of
course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_
