---
layout: post
title: Tiled 1.8.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Two more crashes in [Tiled 1.8][tiled-1.8] were fixed and I've addressed a number of other issues. Stay safe and install this update!

Now I aim to focus on the next feature release again. One thing to note is that support for Qt versions older than 5.12 will be dropped, which means there will no longer be a Windows XP build.

Changelog
---------

*   Fixed deactivating of tools when no layer is selected (avoids crash)
*   Fixed `monospace` font option in multi-line text editor on macOS and Windows ([#3007](https://github.com/mapeditor/tiled/issues/3007))
*   Fixed ability to reset custom 'color' and 'object' properties ([#3270](https://github.com/mapeditor/tiled/issues/3270))
*   Fixed updating of layer positions when changing parallax factor of a group ([#3175](https://github.com/mapeditor/tiled/issues/3175))
*   Scripting: Fixed crash when assigning null to the MapObject.tile property
*   Scripting: Fixed adding of tilesets when adding layers to a loaded map ([#3268](https://github.com/mapeditor/tiled/issues/3268))
*   JSON format: Fixed layer locked status not getting saved ([#2877](https://github.com/mapeditor/tiled/issues/2877))
*   macOS: Fixed duplicate overwrite confirmation when using Export As ([#3152](https://github.com/mapeditor/tiled/issues/3152))
*   FreeBSD: Fixed compile due to missing include (by Dmitry Marakasov, [#3271](https://github.com/mapeditor/tiled/pull/3271))

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting
feature, and of course to everybody who supports Tiled development with a
donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.8]: {{ site.baseurl }}{% post_url 2022-02-07-tiled-1-8-0-released %}
