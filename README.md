Tiled Map Editor - http://www.mapeditor.org/

About Tiled
-------------------------------------------------------------------------------

Tiled is a general purpose tile map editor. It is meant to be used for editing
maps of any tile-based game, be it an RPG, a platformer or a Breakout clone.

Tiled is very flexible, for example there are no restrictions on map size, tile
size or the number of layers or tiles. Also, it allows arbitrary properties to
be set on the map, its layers, the tiles or on the objects. Its map format
(TMX) is relatively easy to understand and allows a map to use multiple
tilesets while also allowing each tileset to grow or shrink as necessary later.

About the Qt Version
-------------------------------------------------------------------------------

Tiled was originally written in Java. In 2008 the Qt version was started with
the goal to replace the Java version with a faster, better looking and even
easier to use map editor. Qt offered many opportunities to improve the
performance and usability of the user interface, and has a more extensive
feature set than the standard Java libraries.

Compiling
-------------------------------------------------------------------------------

Make sure the Qt (>= 4.6) development libraries are installed:

* In Ubuntu/Debian: `sudo apt-get install libqt4-dev libqt4-opengl-dev zlib1g-dev`
* In Fedora:        `yum install qt-devel`
* In Arch Linux:    `pacman -S qt`

Now you can compile by running:

    $ qmake (or qmake-qt4 on some systems, like Fedora)
    $ make

To do a shadow build, you can run qmake from a different directory and refer
it to tiled.pro, for example:

    $ mkdir build
    $ cd build
    $ qmake ../tiled.pro
    $ make

You can now simply run Tiled using bin/tiled.

Installing
-------------------------------------------------------------------------------

For installing Tiled you can run 'make install'. By default Tiled will install
to /usr/local. You can change this prefix when running qmake, and/or you can
change the install root when running make install, as follows:

Use /usr instead of /usr/local:

    $ qmake -r PREFIX=/usr

(Recursive needed when it's not the first time that you're running qmake, since
this affects nested pro files)

Install to some packaging directory:

    $ make install INSTALL_ROOT=/tmp/tiled-pkg

