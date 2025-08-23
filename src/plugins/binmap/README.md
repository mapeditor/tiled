Tiled BinMap Export Plugin
==========================

This is a [Tiled](https://mapeditor.org) plugin to dump tilemaps into a brainfuckingly simple binary format.

It ships BinMap as a plugin (which can be compiled into an .so file) as well as an extension (a .js file).

Installation
------------

You'll only need one of these, as they do exactly the same.

### JavaScript Extension

Copy `src/plugins/binmap/binmap-format.js` into `~/.config/tiled/extensions`.

### Tiled Plugin

1. Download the tiled source from [github](https://github.com/mapeditor/tiled).
2. Install all its dependencies (see its README for details).
3. Copy `src/plugins/binmap` directory into the downloaded repository.
4. Edit `src/plugins/plugins.qbs` and add `"binmap",` to the list.
5. In the repository's main directory, run `qbs`.

Documentation
-------------

The file format specification can be found [here](src/plugins/binmap/FORMAT.md). It is the simplest format possible,
just contains a fixed sized header and the uncompressed tile ids for each layer, that's all.
