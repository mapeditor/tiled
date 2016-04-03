# Introduction

## About Tiled

**Tiled is a 2D level editor that helps you develop the content of your game. Its primary feature is to edit tile maps of various forms, but it also supports free image placement as well as powerful ways to annotate your level with extra information used by the game. Tiled focuses on general flexiblity while trying to stay intuitive.**

In terms of tile maps, it supports straight rectangular tile layers, but also projected isometric, staggered isometric and staggered hexagonal layers. A tileset can be either a single image containing many tiles, or it can be a collection of individual images. In order to support certain depth faking techniques, tiles and layers can be offset by a custom distance and their rendering order can be configured.

The primary tool for editing tile layers is a stamp brush that allows efficient painting and copying of tile areas. It also supports drawing lines and circles. In addition, there are several selection tools and a tool that does [automatic terrain transitions](using-the-terrain-tool.md). Finally, it can apply changes based on [pattern-matching](https://github.com/bjorn/tiled/wiki/Automapping) to automate parts of your work.

Tiled also supports object layers, which traditionally were only for annotating your map with information but more recently they can also be used to place images. You can add rectangle, ellipse, polygon, polyline and tile objects. Object placement is not limited to the tile grid and objects can also be scaled or rotated. Object layers offer a lot of flexibility to add almost any information to your level that your game needs.

Other things worth mentioning are the support for adding custom map or tileset formats through plugins, the tile stamp memory, tile animation support and the tile collision editor.

## Getting Started

*Most of the manual still needs to be written, including this section. Fortunately, there is a very nice [Tiled Map Editor Tutorial Series](http://www.gamefromscratch.com/post/2015/10/14/Tiled-Map-Editor-Tutorial-Series.aspx) on GamesFromScratch.com. In addition, the support for Tiled in various [engines and frameworks](../reference/support-for-tmx-maps.md) often comes with some usage information.*
