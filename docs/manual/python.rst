Python Scripts
==============

Tiled ships with a plugin that enables you to use Python 2.7 to add
support for custom map formats. This is nice especially since you don't
need to compile Tiled yourself and the scripts are easy to deploy to any
platform.

For the scripts to get loaded, they should be placed in ``~/.tiled``.
Tiled watches this directory for changes, so there is no need to restart
Tiled after adding or changing scripts (though the directory needs to
exist when you start Tiled).

There are several `example scripts`_ available in the repository.

.. warning::

    On Windows, Python is not installed by default. For the Tiled Python
    plugin to work, you'll need to install Python 2.7 (get it from
    https://www.python.org/). On Linux you may also need to install the
    appropriate package.


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
                            print >>fileHandle, line

            return True

Then you should see an "Example files" entry in the type dropdown when
going to *File > Export*, which allows you to export the map using the
above script.

.. note::

    This example does not support the use of group layers, and in fact
    the script API doesn't support this yet either. Any help with
    maintaining the Python plugin would be very appreciated. See
    `open issues related to Python support`_.

Debugging Your Script
---------------------

Any errors that happen while parsing or running the script are printed
to the Debug Console, which can be enabled in *View > Views and Toolbars
> Debug Console*.

API Reference
-------------

It would be nice to have the full API reference documented here, but for
now please check out the `source file`_ for available classes and
methods.


.. _example scripts: https://github.com/bjorn/tiled/tree/master/src/plugins/python/scripts
.. _source file: https://github.com/bjorn/tiled/blob/master/src/plugins/python/tiledbinding.py
.. _open issues related to Python support: https://github.com/bjorn/tiled/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+python+in%3Atitle
