---
layout: post
title: Tiled 1.12.1 Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This patch release for [Tiled 1.12][tiled-1.12] mostly fixes one critical issue on macOS, where it
was no longer possible to choose the type when adding a custom property or class member. A few
smaller issues have also been resolved.

Properties View Flicker
-----------------------

Regarding this flickering, for those who noticed, it's something I've been annoyed by for months
while working on [the new Properties framework](https://github.com/mapeditor/tiled/pull/4045). But I
couldn't so far find a good way to avoid it.

When switching between files or objects, the Properties widget hierarchy is reconstructed, then its
layout is updated and then it is repainted. But due to part of this process happening through queued
events, a repaint could happen before the layout update, followed by another repaint once the layout
was done, which resulted in a flicker.

Qt has a mechanism for avoiding this, the `QWidget` has an
[`updatesEnabled`](https://doc.qt.io/qt-6/qwidget.html#updatesEnabled-prop) property which can be
used to suppress repaints in such situations. I had tried to use it exactly as documented:

```c++
setUpdatesEnabled(false);
bigVisualChanges();
setUpdatesEnabled(true);
```

But it hadn't done the trick. I had tried many other hacks, like forcing the layout calculations in
a bunch of places, but nothing had worked reliably.

Turns out the above code didn't work because it enables updates long before the queued layout update
is done, so it didn't actually suppress the problematic repaint. To get it to enable updates only
after layout, I've now applied the following trick:

```c++
QTimer::singleShot(0, propertiesView, [propertiesView] {
    propertiesView->setUpdatesEnabled(true);
});
```

This way, we enqueue the enabling of updates after the queued layout update. Unfortunately, that
still wasn't enough in case scroll bars were involved, which can cause another delayed layout
update. So I settled on [this solution](https://github.com/mapeditor/tiled/pull/4460):

```c++
QTimer::singleShot(0, mWidget, [w = mWidget] {
    QTimer::singleShot(0, w, [w] {
        w->setUpdatesEnabled(true);
    });
});
```

Let me know if you still notice the Properties view flickering sometimes! (or if you know a better
solution)

Changelog
---------

*   Fixed Properties view flicker when switching between objects or files ([#4460](https://github.com/mapeditor/tiled/pull/4460))
*   Fixed selection mode indicator to not toggle on Alt when it would move objects (by kunal649, [#4434](https://github.com/mapeditor/tiled/pull/4434))
*   Fixed status bar pixel coords being rounded instead of floored (by kunal649, [#4426](https://github.com/mapeditor/tiled/pull/4426))
*   macOS: Fix ability to choose type when adding properties ([#4459](https://github.com/mapeditor/tiled/pull/4459))

### Thanks!

Many thanks to those who've reported issues, and of course to everybody who
supports Tiled development with a donation!

_If you enjoy using Tiled and have not donated yet, please consider a small monthly donation [through GitHub](https://github.com/sponsors/bjorn) or [other platforms](https://www.mapeditor.org/donate). With your support I can keep improving this tool!_

[tiled-1.12]: {{ site.baseurl }}{% post_url 2026-03-13-tiled-1-12-released %}
