User Preferences
================

There are only a few options located in the Preferences, accessible
though the menu via *Edit > Preferences*. Most other options, like
whether to draw the grid, what kind snapping to do or the last used
settings when creating a new map are simply remembered persistently.

The preferences are stored in a system-dependent format and location:

+-------------+-----------------------------------------------------------------+
| **Windows** | Registry key ``HKEY_CURRENT_USER\SOFTWARE\mapeditor.org\Tiled`` |
+-------------+-----------------------------------------------------------------+
| **macOS**   | ``~/Library/Preferences/org.mapeditor.Tiled.plist``             |
+-------------+-----------------------------------------------------------------+
| **Linux**   |  ``~/.config/mapeditor.org/tiled.conf``                         |
+-------------+-----------------------------------------------------------------+


General
-------

.. figure:: images/preferences-general.png
   :alt: General Preferences
   :scale: 66
   :align: right

Saving and Loading
~~~~~~~~~~~~~~~~~~

Include DTD reference in saved maps
    This option is not enabled by default, since it is of very little
    use whereas it can in some environments cause problems. Feel free to
    enable it if it helps with validation for example, but note that the
    referenced DTD is likely out of date (there is a somewhat more up-to-date
    XSD file available in the repository).

Reload tileset images when they change
    This is very useful while working on the tiles or when the tiles
    might change as a result of a source control system.

Open last files on startup
    Generally a useful thing to keep enabled.

Use safe writing of files
    This setting causes files to be written to a temporary file, and
    when all went well, to be swapped with the target file. This avoids
    data getting lost due to errors while saving or due to insufficient
    disk space. Unfortunately, it is known to cause issues when saving
    files to a Dropbox folder or a network drive, in which case it helps
    to disable this feature.

Interface
~~~~~~~~~

Language
    By default the language tries to match that of the system, but if it
    picks the wrong one you can change it here.

Grid colour
    Because black is not always the best color for the grid.

Fine grid divisions
    The tile grid can be divided further using this setting, which
    affects the "Snap to Fine Grid" setting in the *View > Snapping*
    menu.

Object line width
    Shapes are by default rendered with a 2 pixel wide line, but some
    people like it thinner or even thicker. On some systems the DPI-based
    scaling will affect this setting as well.

Hardware accelerated drawing (OpenGL)
    This enables a rather unoptimized way of rendering the map using
    OpenGL. It's usually not an improvement and may lead to crashes, but
    in some scenarios it can make editing more responsive.

.. raw:: html

   <div class="new">New in Tiled 1.1</div>

Mouse wheel zooms by default
    This option causes the mouse wheel to zoom without the need to hold
    Control (or Command on macOS). It can be a convenient way to
    navigate the map, but it can also interfere with panning on a
    touchpad.

Theme
-----

On Windows and Linux, the default style used by Tiled is "Tiled Fusion".
This is a customized version of the "Fusion" style that ships with Qt.
On macOS, this style can also be used, but because it looks so out of
place the default is "Native" there.

The "Tiled Fusion" style allows customizing the base color. When
choosing a dark base color, the text automatically switches to white and
some other adjustments are made to keep things readable. You can also
choose a custom selection color.

The "Native" style tries to fit in with the operating system, and is
available since it is in some cases preferable to the custom style. The
base color and selection color can't be changed when using this style,
as they depend on the system.

Updates
-------

The official macOS builds and the Windows installers of Tiled ship with
an automatic update check, done by `Sparkle`_ and `WinSparkle`_
respectively. These solutions also offer to download the new version
and to upgrade or run the installer afterwards.

You can turn off the update checks or trigger them manually using the
*Check Now* button.

Plugins
-------

Here you can choose which plugins are enabled. Currently plugins only
serve to add support for additional map and/or tileset file formats.
Some generic plugins are enabled by default, while more specific ones
need to be manually enabled.

There is no need to restart Tiled when enabling or disabling plugins.
When a plugin fails to load, try hovering its icon to see if the tool
tip displays a useful error message.

See :doc:`export` for more information about supported file formats.

.. _Sparkle: https://sparkle-project.org/
.. _WinSparkle: https://winsparkle.org/
