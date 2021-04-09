.. _export-as-image:

Export as Image
---------------

Maps can be exported as image. Tiled supports most common image formats.
Choose *File -> Export as Image...* to open the relevant dialog.

Since exporting a map can in some cases result in a huge image, a *Use current
zoom level* option is provided to allow exporting the map at the size it's
currently displayed at.

For repeatedly converting a map to an image, manually triggering this export
isn't very convenient. For this purpose, a tool called ``tmxrasterizer`` ships
with Tiled, which contrary to its name is able to render any supported map
format to an image. It is also able to render :doc:`entire worlds <worlds>` to
an image. On Linux this tool can be set up for generating thumbnail previews
of maps in the file manager.
