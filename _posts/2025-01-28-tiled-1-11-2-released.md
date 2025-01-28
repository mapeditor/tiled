---
layout: post
title: Tiled 1.11.2 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

Another small bugfix release for [Tiled 1.11][tiled-1.11]! Upgrading is
recommended since it fixes the most common crashes reported through Sentry.
It also ships with an updated plugin for exporting to [GameMaker][].

### Crashes Fixed

Most people will never run into these, but some managed to trigger them
repeatedly:

* Fixed crash while handling file reloads without any files opened. This can
  happen when a world file or an external tileset is reloaded while no map is
  open.

* Fixed crash when closing the last file with multiple custom properties selected.

These crashes were relatively easy to fix thanks to the crash reports sent
through Sentry. Unfortunately Sentry is for now still only enabled in the Linux
AppImage, but it has already been very helpful, so a big thanks to those who
have enabled [crash reporting](https://www.mapeditor.org/crash-reporting)!

### GameMaker Exporter

Sometime last year, [GameMaker][] (previously known as "GameMaker Studio 2.3")
made some changes to their file format, mostly related to the order of the
properties, which it is now very strict about. This release ships with an
updated [export plugin](https://doc.mapeditor.org/en/latest/manual/export-yy/)
that works with the latest version of GameMaker.

### Thanks!

Many thanks to all who've reported issues, and of course to everybody who
supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small
monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other
platforms](https://www.mapeditor.org/donate). With your support I can keep
improving this tool!_

[tiled-1.11]: {{ site.baseurl }}{% post_url 2024-06-27-tiled-1-11-released %}
[GameMaker]: https://gamemaker.io/
