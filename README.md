Tiled Map Editor - https://www.mapeditor.org/

About Tiled
-------------------------------------------------------------------------------

Tiled is a general purpose tile map editor for all tile-based games, such as
RPGs, platformers or Breakout clones.

Tiled is highly flexible. It can be used to create maps of any size, with no
restrictions on tile size, or the number of layers or tiles that can be used.
Maps, layers, tiles, and objects can all be assigned arbitrary properties.
Tiled's map format (TMX) is easy to understand and allows multiple tilesets to
be used in any map. Tilesets can be modified at any time.

[![Build Status](https://api.travis-ci.com/mapeditor/tiled.svg?branch=master&status=passed)](https://travis-ci.com/github/mapeditor/tiled)
[![Build status](https://ci.appveyor.com/api/projects/status/ceb79jn5cf99y3qd/branch/master?svg=true)](https://ci.appveyor.com/project/bjorn/tiled/branch/master)
[![Snap](https://snapcraft.io/tiled/badge.svg)](https://snapcraft.io/tiled)
[![Bountysource](https://www.bountysource.com/badge/tracker?tracker_id=52019)](https://www.bountysource.com/trackers/52019-tiled?utm_source=52019&utm_medium=shield&utm_campaign=TRACKER_BADGE)
[![Translation status](https://hosted.weblate.org/widgets/tiled/-/shields-badge.svg)](https://hosted.weblate.org/engage/tiled/?utm_source=widget)
[![Open Source Helpers](https://www.codetriage.com/mapeditor/tiled/badges/users.svg)](https://www.codetriage.com/mapeditor/tiled)

About the Qt Version
-------------------------------------------------------------------------------

Tiled was originally written in Java. In 2008, work began to develop a faster,
better looking, and easier-to-use version of Tiled based on the Qt framework.
This decision was made as the Qt framework has a greater feature set than is
offered by the standard Java libraries.


Compiling
-------------------------------------------------------------------------------

Before you can compile Tiled, you must ensure the Qt (>= 5.6) development
libraries have been installed as well as the Qbs build tool:

* On Ubuntu/Debian: `sudo apt install qt5-default libqt5svg5 qttools5-dev-tools zlib1g-dev qtdeclarative5-dev qtdeclarative5-private-dev qbs`
* On Fedora:        `sudo dnf builddep tiled`
* On Arch Linux:    `sudo pacman -S qt qt5-tools qbs`
* On macOS with [Homebrew](https://brew.sh/):
  + `brew install qbs`
  + `brew link qt5 --force`

If you want to build the Python plugin, you additionally need to install the
Python 3 development libraries:

* On Ubuntu/Debian: `sudo apt install python3-dev`
* On Windows: https://www.python.org/downloads/windows/

Alternatively, you can [download Qt here](https://www.qt.io/download-qt-installer).
You will still need to install a development environment alongside and some
libraries depending on your system, for example:

* On Ubuntu/Debian: `sudo apt install build-essential zlib1g-dev libgl1-mesa-dev`
* On Windows:       Choose "MinGW" Qt version, or install Visual Studio
* On macOS:         Install Xcode

The easiest way to compile and run Tiled is to open `tiled.qbs` in Qt Creator
and run the project from there.

From the command-line, you may need to set up Qbs before you can build Tiled
(you will also need to make sure the version of Qt you want to use is in your
path):

    qbs setup-toolchains --detect     # setup toolchains
    qbs setup-qt --detect             # setup Qt (not needed since Qbs 1.13)
    qbs                               # build Tiled

You can now run Tiled as follows:

    qbs run -p tiled

Qt 6
-------------------------------------------------------------------------------

For compiling libtiledquick you'll need to install the Vulkan headers:

* On Unbuntu/Debian: `sudo apt install libvulkan-dev`

### Working with Visual Studio 2017

Once Qbs is set up (see previous instructions), it is possible to generate a
Visual Studio 2017 project with it that allows you to code, compile and run
using that IDE. This can be done with the following command:

    qbs generate -g visualstudio2017

Installing
-------------------------------------------------------------------------------

To install Tiled, run `qbs install` from the terminal. By default, Tiled will
be installed to `<build-dir>/install-root`.

The installation prefix can be changed when building Tiled. For example, to use
an installation prefix of  `/usr`:

    qbs qbs.installPrefix:"/usr"

To install Tiled to a packaging directory:

    qbs install --install-root /tmp/tiled-pkg

By default, Tiled and its plugins are compiled with an Rpath that allows them
to find the shared *libtiled* library immediately after being compiled. When
packaging Tiled for distribution, the Rpath should be disabled by appending
`projects.Tiled.useRPaths:false` to the qbs command.
