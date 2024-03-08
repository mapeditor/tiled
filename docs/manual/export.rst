Export Formats
==============

While there are many :doc:`libraries and frameworks
</reference/support-for-tmx-maps>` that work directly with Tiled maps, Tiled
also supports a number of additional file and export formats, as well as
:ref:`exporting a map to an image <export-as-image>`.

Exporting can be done by clicking *File > Export*. When triggering the menu
action multiple times, Tiled will only ask for the file name the first time.
Exporting can also be automated using the ``--export-map`` and
``--export-tileset`` command-line parameters.

Several :ref:`export-options` are available, which are applied to maps
or tilesets before they are exported (without affecting the map
or tileset itself).

.. toctree::
   :maxdepth: 2
   :caption: Supported Formats

   export-generic
   export-defold
   export-gmx
   export-yy
   export-tscn
   export-tbin
   export-other
   export-custom
   python
   export-image

.. note::

   When exporting on the command-line on Linux, Tiled will still need an
   X server to run. To automate exports in a headless environment, you
   can use a headless X server such as `Xvfb`_. In this case you would
   run Tiled from the command-line as follows:

   ::

      xvfb-run tiled --export-map ...

.. _Xvfb: https://www.x.org/archive/X11R7.6/doc/man/man1/Xvfb.1.xhtml
