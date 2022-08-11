---
layout: post
title: Tiled 1.9.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch releases addresses a few serious issues in [Tiled 1.9][tiled-1.9],
including a crash when removing a custom property without having a project
loaded. Upgrading is highly recommended!

For Linux users, I'm sorry for breaking the "snap". Due to a lack of testing I
published a build that would not run at all. This release fixes that as well.

In general, I'm sorry for the wait on the patch release this time. Bad timing
to release a new major version just before leaving on holiday! I had intended
to finish 1.9 a little earlier, but next time I run out of time I should
probably delay the release.

Changelog
---------

*   Fixed properties-related crash when having no project loaded
*   Fixed loading of custom tile image rectangles ([#3405](https://github.com/mapeditor/tiled/issues/3405))
*   Fixed loading of member values for nested classes ([#3414](https://github.com/mapeditor/tiled/issues/3414))
*   Fixed visibility of "Move Object to Layer" sub-menu ([#3417](https://github.com/mapeditor/tiled/issues/3417))
*   Fixed shadow offset for other maps in a world ([#3429](https://github.com/mapeditor/tiled/issues/3429))
*   Fixed class dropdown to update immediately when changing custom types ([#3398](https://github.com/mapeditor/tiled/issues/3398))
*   Fixed deleting an overridden property to not make it disappear ([#3409](https://github.com/mapeditor/tiled/issues/3409))
*   Scripting: Added `TilesetsView.currentTilesetChanged`
*   JSON plugin: Fixed loading image layer "repeatx/y" properties (by Jene Litsch, [#3428](https://github.com/mapeditor/tiled/pull/3428))
*   snap: Fixed startup error due to missing libQt5Concurrent.so.5 ([#3408](https://github.com/mapeditor/tiled/issues/3408))
*   AppImage: Use custom AppRun that can call all binaries (by Philipp Seiler, [#3415](https://github.com/mapeditor/tiled/pull/3415))
*   AppImage: Updated to Sentry 0.5.0
*   Updated Chinese (Simplified) and Portuguese translations

### Thanks!

Many thanks to all who've reported issues or enabled the crash reporting
feature, and of course to everybody who supports Tiled development with a
donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.9]: {{ site.baseurl }}{% post_url 2022-06-25-tiled-1-9-released %}
