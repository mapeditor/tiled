Custom Export Formats
---------------------

Tiled provides several options for extending it with support for additional
file formats.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.3</div>

Using JavaScript
~~~~~~~~~~~~~~~~

Tiled is :doc:`extendable using JavaScript </reference/scripting>` and it is
possible to add custom export formats using :ref:`tiled.registerMapFormat
<script-registerMapFormat>` or :ref:`tiled.registerTilesetFormat
<script-registerTilesetFormat>`).

Using Python
~~~~~~~~~~~~

It is also possible to write :doc:`Python scripts <python>` to add
support for importing or exporting custom map formats.

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
