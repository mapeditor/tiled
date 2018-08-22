---
layout: post
title: Tiled 1.2 Beta released
author: Thorbjørn Lindeijer
tags: release
---

At long last it is almost time for Tiled 1.2! But before we get there,
we will still need some time for fixing bugs, updating translations and
acting on last-minute feedback from those using the development
snapshots. The [latest snapshot][snapshot] (version 2018.08.22) is
considered a Tiled 1.2 Beta release.

If you have been staying with the stable Tiled versions so far, this is
the perfect time to install the snapshot and try out the new features!
By doing this you have a chance to make sure they work fine in your
project as well, and to report any new issues before the final release.

Here is a quick summary of the major updates since [Tiled 1.1][tiled-11]:

* Enhancements to [object tools](https://thorbjorn.itch.io/tiled/devlog/21137/improvements-to-object-tools-and-more) and [polygon editing tools](https://thorbjorn.itch.io/tiled/devlog/21850/polygon-editing-improvements), including [extending and joining polylines](https://thorbjorn.itch.io/tiled/devlog/26226/extend-and-join-polylines-with-ease).
* Editing [multiple tile layers at the same time](https://thorbjorn.itch.io/tiled/devlog/24167/multi-layer-editing-and-other-stuff)
* A [multi-map world view](https://thorbjorn.itch.io/tiled/devlog/35054/multi-map-world-view-python-3-and-raising-funds) to provide an overview and quick navigation between maps.
* [Highlighting of the hovered object](https://thorbjorn.itch.io/tiled/devlog/38930/multi-layer-stamp-improvements-and-highlight-hovered-object) and an improved [preview when placing objects](https://thorbjorn.itch.io/tiled/devlog/40807/object-placement-preview-on-hover).
* [Several export options](https://thorbjorn.itch.io/tiled/devlog/43780/export-options-and-polishing) that can simplify your map loading code.
* Integration of the news on the website, so you'll never miss an important update like this!

In terms of compatibility it is important to note:

* The [JSON format](http://doc.mapeditor.org/en/latest/reference/json-map-format/) was simplified a bit. Loaders will need to be adjusted, but the adjustments should be pleasant.
* The Python plugin was updated to Python 3. Currently there are still compatibility issues since you'll need to use the same minor version of Python 3 that Tiled was built against for the plugin to load.

I hope you will have fun trying out the beta and I'm looking forward to
hear your feedback!

Thanks to everybody who made this release possible! There was a big
focus on usability and productivity in these past months, thanks in
large part to the priority requests from several major patrons. Patrons on [Patreon](https://www.patreon.com/bjorn) and [Liberapay](https://liberapay.com/bjorn) and everybody who bought [Tiled on itch.io](https://thorbjorn.itch.io/tiled) contributed to the time I could spend to implement these features. Thank you!


[snapshot]: https://thorbjorn.itch.io/tiled/devlog/45718/tiled-12-beta
[tiled-11]: {{ site.baseurl }}{% post_url 2018-01-03-tiled-1-1-0-released %}
