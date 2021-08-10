---
layout: post
title: Tiled 1.7.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This release fixes a bunch of small issues, including two possible crashes. I expect this to be the last stability update to [Tiled 1.7][tiled-1-7].

Changelog
---------

* Avoid automatically replacing external tilesets with "similar" ones
* Fixed copying and capturing stamps on staggered maps (with Alexander Dorogov, [#2874](https://github.com/mapeditor/tiled/issues/2874))
* Fixed possible crash in Tile Animation Editor
* Fixed data loss when saving maps with tilesets that failed to load ([#3106](https://github.com/mapeditor/tiled/issues/3106))
* Fixed creating multi-layer tile stamp from selection ([#2899](https://github.com/mapeditor/tiled/issues/2899))
* Scripting: Automatically reset object ID when adding to avoid duplicate IDs
* Linux: Possible workaround for crash in clipboard manager
* Updated to Sentry 0.4.12
* Updated Italian translation

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting feature, and of course to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1-7]: {{ site.baseurl }}{% post_url 2021-06-04-tiled-1-7-0-released %}
