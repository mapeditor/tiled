---
layout: post
title: Tiled 1.12.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

A larger patch release for [Tiled 1.12][tiled-1.12], with follow-up fixes for
the new list-typed custom properties, a Properties view regression from
[1.12.1][tiled-1.12.1], and a few long-standing bugs.

List Properties
---------------

Copy/pasting a [list-typed custom property][lists-intro] dropped its items,
saving them as `type="std::nullptr_t"` in TMX. The same bug affected lists nested
inside class properties, and list properties stored on projects or worlds
([#4514](https://github.com/mapeditor/tiled/pull/4514)).

Separately, the export helper forgot to recurse into list values when
resolving class member defaults, so a list of class-typed items could miss
inherited values on export
([#4525](https://github.com/mapeditor/tiled/pull/4525)).

Properties View
---------------

The [flicker fix][tiled-1.12.1-flicker] in Tiled 1.12.1 introduced a helper that
toggled `QWidget::updatesEnabled` around layout updates, then restored the
previous state. However, `updatesEnabled()` returns `false` not only when
the widget itself was explicitly disabled, but also when a parent widget has
updates disabled. So in some cases, like switching from a map to the tile
collision editor and back with an object selected, the helper would leave the
Properties view stuck with updates disabled. Fixed by skipping the toggle when
updates are already effectively disabled
([#4506](https://github.com/mapeditor/tiled/pull/4506)).

Object Properties can also again be edited even after deselecting the current
object ([#4440](https://github.com/mapeditor/tiled/pull/4440)), and custom
property names are now trimmed of surrounding whitespace
([#4486](https://github.com/mapeditor/tiled/pull/4486)).

Long-Standing Bugs
------------------

Three issues that had been open for a while:

*   The Tile Animation Editor didn't refresh after reloading the tileset
    image ([#3923](https://github.com/mapeditor/tiled/issues/3923), since 2024).

*   In the Tilesets view with dynamic wrapping, empty grid cells below the
    last row of tiles could be selected as if they were tiles
    ([#3498](https://github.com/mapeditor/tiled/issues/3498), since 2022).

*   Some editor widgets and models didn't refresh their UI strings when
    changing the editor language, so parts of the UI kept showing the
    previous language until Tiled was restarted
    ([#3443](https://github.com/mapeditor/tiled/issues/3443), since 2022).

Changelog
---------

*   Added whitespace trimming for custom property names (with Praagya26, [#4486](https://github.com/mapeditor/tiled/pull/4486))
*   Reduced animated tile marker opacity during terrain editing (by Huy Vũ, [#4449](https://github.com/mapeditor/tiled/pull/4449))
*   Fixed ability to change properties after deselecting the current object ([#4440](https://github.com/mapeditor/tiled/pull/4440))
*   Fixed Properties view getting stuck with updates disabled ([#4506](https://github.com/mapeditor/tiled/pull/4506))
*   Fixed copy/paste of list properties losing item types ([#4514](https://github.com/mapeditor/tiled/pull/4514))
*   Fixed resolving of class members values in lists on export ([#4525](https://github.com/mapeditor/tiled/pull/4525))
*   Fixed locale-aware parsing of numbers in expression-capable spin boxes ([#4507](https://github.com/mapeditor/tiled/pull/4507))
*   Fixed Tile Animation Editor update after tileset image reload ([#3923](https://github.com/mapeditor/tiled/issues/3923))
*   Fixed runtime language switching in many editor widgets and models (by SIDDHAARTHAA, [#4411](https://github.com/mapeditor/tiled/pull/4411))
*   Fixed multi-object selection bounding box for point objects (by Mollah Hamza, [#4401](https://github.com/mapeditor/tiled/pull/4401))
*   Fixed non-tile locations in wrapping Tilesets view being selectable ([#3498](https://github.com/mapeditor/tiled/issues/3498))

### Thanks!

Many thanks to all who've reported issues and submitted patches, and of course
to everybody who supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small
monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other
platforms](https://www.mapeditor.org/donate). With your support I can keep
improving this tool!_

[tiled-1.12]: {{ site.baseurl }}{% post_url 2026-03-13-tiled-1-12-released %}
[tiled-1.12.1]: {{ site.baseurl }}{% post_url 2026-03-25-tiled-1-12-1-released %}
[tiled-1.12.1-flicker]: {{ site.baseurl }}{% post_url 2026-03-25-tiled-1-12-1-released %}#properties-view-flicker
[lists-intro]: {{ site.baseurl }}{% post_url 2026-03-13-tiled-1-12-released %}#list-values-in-custom-properties
