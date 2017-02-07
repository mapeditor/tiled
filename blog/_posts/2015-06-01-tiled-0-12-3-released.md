---
layout: post
title: Tiled 0.12.3 released
author: Thorbjørn Lindeijer
tags: release
topic_id: 557
---

As embarrassing as it was to make [two quick patch releases][1] for Tiled 0.12, unfortunately I needed to make another one since more critical updating issues were discovered.

The full list of fixes:

* Fixed updating of map view when rotating objects with Z key ([#966](https://github.com/bjorn/tiled/issues/966))
* Fixed updating of map view when joining, splitting or deleting polygon nodes ([#966](https://github.com/bjorn/tiled/issues/966))
* Fixed a crash when reading an invalid TMX file ([#960](https://github.com/bjorn/tiled/issues/960))
* Fixed live automapping updates when moving the mouse fast ([#958](https://github.com/bjorn/tiled/issues/958))
* Made Backspace work for deleting collision objects and animation frames ([topic][2])

Go get it from the [Download][3] page.

In the meantime, I've started to make progress on Tiled 0.13 by working on [tile stamp improvements][4]. If you want to help me progress faster and get around to more exciting features listed on the [Roadmap][5], please consider joining in to [support me through Patreon][6]. Thanks!


  [1]: http://forum.mapeditor.org/t/tiled-0-12-1-and-0-12-2-released/533
  [2]: http://forum.mapeditor.org/t/how-to-remove-animation-settings/545/6
  [3]: http://www.mapeditor.org/download
  [4]: https://github.com/bjorn/tiled/issues/969
  [5]: https://github.com/bjorn/tiled/wiki/Roadmap
  [6]: https://www.patreon.com/bjorn
