---
layout: post
title: Tiled 1.9 Alpha Released
author:
  name: Thorbjørn Lindeijer
  twitter: thorbjorn81
tags: release
---

This is the first preview release of Tiled 1.9, which is focused on the [AutoMapping](https://doc.mapeditor.org/de/latest/manual/automapping/) feature. The downloads for this preview are [available on GitHub](https://github.com/mapeditor/tiled/releases/tag/v1.8.90).

AutoMapping has always been a rather hidden feature with a steep learning curve, but it was appreciated by those who dug into it and has saved many projects an enormous amount of tedious tile placement. In the upcoming Tiled 1.9, the goal is to make AutoMapping more accessible, more powerful and a lot faster.

_Since the documentation is still largely outdated, read this post for the changes!_

![AutoMapping Demonstration by eishiya](/img/posts/2022-04-automapping-by-eishiya.gif)

_(a demonstration of AutoMapping by eishiya, where tile variations are chosen based on a "light" layer)_

### Performance and Usability

The process of applying AutoMapping rules has been entirely rewritten, while mostly maintaining compatibility. It is about 10-30x faster compared to Tiled 1.8.4 (where it had already been made 3x faster compared to Tiled 1.8.2), depending on your CPU since it can now match rules in parallel.

The increased speed makes AutoMapping more pleasant to use on large maps or when using hundreds of rules, but where it really makes a difference is when "AutoMap While Drawing" is enabled. Another big improvement is that the "AutoMap While Drawing" mode no longer creates separate undo steps for applying the AutoMapping rules. Instead, it seamlessly integrates with the paint operation\*.

_*) This doesn't work yet when erasing or when AutoMapping places or removes objects._

### Easier to Set Up

#### No More `regions` Layers

The rules are still defined in a special map, called a "rule map", but it is no longer needed to set up "regions", "regions_input" and/or "regions_output" layers that define where each rule is in the map. Instead, these regions are now derived from the "input", "inputnot" and "output" layers that you need to create anyway.

Explicit use of "regions" layers is hereby deprecated and discouraged, especially since using them will bring back the confusing behavior described in the next section, for compatibility reasons.

#### Explicit "Empty", "NonEmpty" and "Other" Tiles

There used to be a special meaning for empty tiles within an input region, depending on other factors like whether the "StrictEmpty" option was set on that layer, whether there was any other non-empty tile defined for that location and whether there was a matching "inputnot" layer. This was a source of much confusion.

Now, there is a "MatchType" tile property, which can be used to mark certain tiles as matching with "Empty" tiles, "NonEmpty" tiles or "Other" tiles (tiles not used on this layer for this rule). There is also an "Ignore" match type, which is useful if you need to connect different parts of a rule without anything in between (which you'd previously need to resolve with a regions layer).

As a potential replacement for using "inputnot" layers, there is also a "MatchType" called "Forbid", which can be used to effectively negate the condition at a certain location. Instead of creating an "inputnot" layer, you would create another "input" layer and place the "Forbid" tile where you want to reverse the conditions for a target layer.

![automap-tiles](https://raw.githubusercontent.com/mapeditor/tiled/accee950b4a9b84249038496afe778cf791283d7/src/tiled/images/scalable/automap-tiles.svg)

A [preliminary tileset](https://github.com/mapeditor/tiled/blob/accee950b4a9b84249038496afe778cf791283d7/tests/automapping/automap-tiles.tsx) is provided with tiles of each of these types. Feedback on the icons is welcome!

### More Powerful

#### Per-Rule Options

A number of per-rule options have been added, which can be set by placing a rectangle object on a "rule_options" layer that covers all the rules the options should apply to. The currently supported options are:

* **ModX**: Only apply a rule every N tiles on the X axis (defaults to 1).
* **ModY**: Only apply a rule every N tiles on the Y axis (defaults to 1).
* **OffsetX**: An offset applied in combination with ModX (defaults to 0).
* **OffsetY**: An offset applied in combination with ModY (defaults to 0).
* **Probability**: The probability that the rule will be applied (value from 0-1, defaults to 1).
* **Disabled**: A convenient way to (temporarily) disable some rules (defaults to false).
* **NoOverlappingOutput**: Same as the old *NoOverlappingRules* option on rule-maps, but per-rule (defaults to false).

All these options can also be set on the rule map itself, in which case they change the defaults for all rules in the map. There are likely additional options that would be nice to add, so again feedback is welcome.

#### Filename Filters in `rules.txt`

While it was already possible to use multiple directories for map files so they could have different `rules.txt` files, this wasn't very convenient. Now, it is possible to define a filename filter to make sure certain sets of rules are only applied to certain maps. For example:

```ini
globalRules.tmx

[town*]
townRules.tmx

[dungeon*]
dungeonRules.tmx
iceDungeonRules.tmx
lavaDungeonRules.tmx

[fakeTownDungeon.tmx]
//a dungeon that has elements of a town in it
dungeonRules.tmx
townRules.tmx

[*]
globalRulesThatRunAfterEverythingElse.tmx
```

Doing this is not only useful if you need to limit some rules to only some maps, but it is of course also good for performance, if the AutoMapping is still not fast enough for you!

## Changelog

*   Scripting: Added -e,--evaluate to run a script from command-line
*   Scripting: Added Tool.toolBarActions property ([#3318](https://github.com/mapeditor/tiled/issues/3318))
*   AutoMapping: Applying rules is now 10-30x faster
*   AutoMapping: "AutoMap While Drawing" no longer creates separate undo steps ([#2166](https://github.com/mapeditor/tiled/issues/2166))
*   AutoMapping: Explicit "regions" layers are no longer needed and have been deprecated ([#1918](https://github.com/mapeditor/tiled/issues/1918))
*   AutoMapping: Custom tiles can now match "Empty", "Non-Empty" and "Other" tiles through a "MatchType" property ([#3100](https://github.com/mapeditor/tiled/issues/3100))
*   AutoMapping: A custom tile with "MatchType" set to "Forbid" can be used instead of "inputnot" layers
*   AutoMapping: Added a number of per-rule options which can be set using rectangle objects
*   AutoMapping: Erase tiles by placing tiles with "MatchType" set to "Empty" on output layers ([#3100](https://github.com/mapeditor/tiled/issues/3100))
*   AutoMapping: Accumulate touched layers in AutoMap While Drawing ([#3313](https://github.com/mapeditor/tiled/issues/3313))
*   AutoMapping: Support map name filters in rules.txt ([#3014](https://github.com/mapeditor/tiled/issues/3014))
*   Split up object types file type selection
*   Raised minimum supported Qt version from 5.6 to 5.12 (drops Windows XP support)
*   Raised minimum C++ version to C++17
*   Removed qmake project files (only Qbs supported now)

#### Included Changes for Future 1.8 Patch

* Made expanded group layers persistent
* Scripting: Fixed region.rects when compiled against Qt 5.9 to 5.13

_The 64-bit Windows installer is now based on Qt 6.2 and needs at least Windows 10 (the 32-bit installer still supports Windows 7)_
