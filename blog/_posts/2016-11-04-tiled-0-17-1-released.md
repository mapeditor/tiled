---
layout: post
title: Tiled 0.17.1 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 1791
---

I've just released [Tiled 0.17.1](https://thorbjorn.itch.io/tiled). This release fixes a number of bugs, so upgrading is definitely recommended. It also brings updates to the Chinese and French translations. On Windows, the installer now includes previously left out image format plugins and the experimental export plugin for [Defold](http://www.defold.com/).

The big change for GNU/Linux users is the immediate availability of an [AppImage](http://appimage.org/). This convenient format should run on most GNU/Linux distributions. I've set these up to build automatically on [Travis CI](https://travis-ci.org/bjorn/tiled), so they will be included in every release from now on.

**Changelog**

* Fixed wrong alpha value when opening the color picker dialog
* Fixed saving of object group color alpha value
* Fixed tile id adjustment for newly added tilesets
* Fixed "Object Properties" entry in the context menu to be always enabled (by Erik Schilling)
* Fixed out-of-sync tile selection during layer offset change (by nykm)
* Fixed hidden objects becoming visible when offsetting the map (by ranjak)
* Fixed problems with using predefined file properties
* Lua plugin: Fixed type of animation frame properties
* OS X: Use standard shortcut for toggling full screen
* OS X: Fixed compile when pkg-config is present
* Windows: Include the Defold plugin
* Windows: Added support for DDS, TGA, WBMP and WEBP image formats
* Linux: Added 64-bit AppImage (with help from Simon Peter)
* Chinese translation updates (by endlesstravel and buckle2000)
* French translation updated (by Yohann Ferreira)

In the meantime, work on the next feature release is progressing well. The main change will be much improved support for working with external tileset files (usually `.tsx` files), making it easier to take advantage of sharing all tileset-associated information between maps.

As always, thanks a lot for the support [through Patreon](https://www.patreon.com/bjorn) and on [itch.io](https://thorbjorn.itch.io/tiled)! I'm only able to keep improving Tiled because of this support. If you are not supporting me yet, [please chip in](https://www.patreon.com/bePatron?u=90066) towards my next goal of being able to actually afford working on Tiled two full days per week! Thanks! :heart:
