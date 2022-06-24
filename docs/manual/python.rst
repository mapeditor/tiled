Python Scripts
==============

.. note::

    Since Tiled 1.3, Tiled can be :doc:`extended using JavaScript
    </reference/scripting>`. The JavaScript API provides a lot more
    opportunity for extending Tiled's functionality than just adding custom
    map formats. It is fully documented and works out of the box on all
    platforms. It should be preferred over the Python plugin when possible.

Tiled ships with a plugin that enables you to use Python 3 to add
support for custom map formats. This is nice especially since you don't
need to compile Tiled yourself and the scripts are easy to deploy to any
platform.

For the scripts to get loaded, they should be placed in ``~/.tiled``.
Tiled watches this directory for changes, so there is no need to restart
Tiled after adding or changing scripts (though the directory needs to
exist when you start Tiled).

There are several `example scripts`_ available in the repository.

.. note::

    To create the ``~/.tiled`` folder on Windows, open command prompt
    (``cmd.exe``), which should start in your home folder by default, then
    type ``mkdir .tiled`` to create the folder.

    On Linux, folders starting with a dot are hidden by default. In most file
    managers you can toggle showing of hidden files using ``Ctrl+H``.

.. note::

    Since Tiled 1.2.4, the Python plugin is disabled by default, because
    depending on which Python version is installed on the system the loading of
    this plugin may cause a crash (`#2091`_). To use the Python plugin, first
    enable it in the Preferences.

.. warning::

    On Windows, Python is not installed by default. For the Tiled Python
    plugin to work, you'll need to install Python 3.8 (get it from
    https://www.python.org/). You will also need to check the box "Add Python
    3.8 to PATH" in the installer:

    .. figure:: images/python-windows.png

    On Linux you will also need to install the appropriate package.
    However, currently Linux builds are done on Ubuntu 20.04 against
    Python 3.8, and you'd need to install the same version somehow.

    The Python plugin is currently not available for macOS releases.

Example Export Plugin
---------------------

Suppose you'd like to have a map exported in the following format:

.. code::

    29,29,29,29,29,29,32,-1,34,29,29,29,29,29,29,
    29,29,29,29,29,29,32,-1,34,29,29,29,29,29,29,
    29,29,29,29,29,29,32,-1,34,29,29,29,29,29,29,
    29,29,29,29,29,29,32,-1,34,29,29,29,29,29,29,
    25,25,25,25,25,25,44,-1,34,29,29,29,29,29,29,
    -1,-1,-1,-1,-1,-1,-1,-1,34,29,29,29,29,29,29,
    41,41,41,41,41,41,41,41,42,29,29,24,25,25,25,
    29,29,29,29,29,29,29,29,29,29,29,32,-1,-1,-1,
    29,29,29,29,29,29,39,29,29,29,29,32,-1,35,41,
    29,29,29,29,29,29,29,29,29,29,29,32,-1,34,29,
    29,29,29,29,29,29,29,29,37,29,29,32,-1,34,29;


You can achieve this by saving the following ``example.py`` script in
the scripts directory:

.. code:: python

    from tiled import *

    class Example(Plugin):
        @classmethod
        def nameFilter(cls):
            return "Example files (*.example)"

        @classmethod
        def shortName(cls):
            return "example"

        @classmethod
        def write(cls, tileMap, fileName):
            with open(fileName, 'w') as fileHandle:
                for i in range(tileMap.layerCount()):
                    if isTileLayerAt(tileMap, i):
                        tileLayer = tileLayerAt(tileMap, i)
                        for y in range(tileLayer.height()):
                            tiles = []
                            for x in range(tileLayer.width()):
                                if tileLayer.cellAt(x, y).tile() != None:
                                    tiles.append(str(tileLayer.cellAt(x, y).tile().id()))
                                else:
                                    tiles.append(str(-1))
                            line = ','.join(tiles)
                            if y == tileLayer.height() - 1:
                                line += ';'
                            else:
                                line += ','
                            print(line, file=fileHandle)


            return True

Then you should see an "Example files" entry in the type dropdown when
going to *File > Export*, which allows you to export the map using the
above script.

.. note::

    This example does not support the use of group layers.

Debugging Your Script
---------------------

Any errors that happen while parsing or running the script are printed
to the Console, which can be enabled in *View > Views and Toolbars
> Console*.

API Reference
-------------

It would be nice to have the full API reference documented here, but for
now please check out the `source file`_ for available classes and
methods.

.. note::

    Any help with maintaining the Python plugin would be very appreciated. See
    `open issues related to Python support`_

.. _example scripts: https://github.com/bjorn/tiled/tree/master/src/plugins/python/scripts
.. _source file: https://github.com/bjorn/tiled/blob/master/src/plugins/python/tiledbinding.py
.. _open issues related to Python support: https://github.com/bjorn/tiled/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+python+in%3Atitle
.. _#2091: https://github.com/bjorn/tiled/issues/2091
