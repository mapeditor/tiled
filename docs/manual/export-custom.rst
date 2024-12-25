Custom Export Formats
---------------------

Tiled provides several options for extending it with support for additional
file formats.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.3</div>

Using JavaScript
~~~~~~~~~~~~~~~~

Tiled is :doc:`extendable using JavaScript </manual/scripting>` and it is
possible to add custom export formats using `tiled.registerMapFormat
<https://www.mapeditor.org/docs/scripting/modules/tiled.html#registerMapFormat>`__ or `tiled.registerTilesetFormat
<https://www.mapeditor.org/docs/scripting/modules/tiled.html#registerTilesetFormat>`__.

This is the recommended way to add support for custom map or tileset formats.

Using Python
~~~~~~~~~~~~

On some platforms, it is also possible to write :doc:`Python scripts <python>`
to add support for importing or exporting custom map and tileset formats.

.. warning::

    Python scripting is not supported by the macOS release nor the Tiled snap
    release for Ubuntu. The plugin is also very specific in the supported
    Python version. Hence, its use is not recommend.

Using C++
~~~~~~~~~

Currently all export options shipping with Tiled are written as C++ Tiled
plugins. The API for such plugins is not documented (apart from Doxygen-style
comments in the ``libtiled`` source code), but there are over a dozen examples
you can look at.

.. note::

    For binary compatibility reasons, a C++ plugin needs to be compiled for
    the same platform, by the same compiler and with the same versions of Qt
    and Tiled that the plugin is supposed to support. Generally, the easiest
    way to achieve this is by compiling the plugin along with Tiled, which is
    what all current plugins do. If you write a C++ plugin that could be
    useful for others, it is recommended you open a pull request to have it
    shipped with Tiled.
