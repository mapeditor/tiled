.. raw:: html

   <div class="new new-prev">Since Tiled 1.1</div>

Using Infinite Maps
===================

Infinite maps in Tiled free you from the constraints of the fixed-size map. 
With an "auto-growing" canvas, you can paint on an infinite grid without being 
limited by width and height. This document guides you through creating, editing, 
and converting infinite maps in Tiled.

.. figure:: images/infinite/infinite-map-overview.png
   :alt: A zoomed-out Tiled window shows a very large infinite map being edited.

Creating an Infinite Map
------------------------

1. Open the New Map dialog (*File -> New -> New Map*).
2. Ensure the 'Infinite' option is selected.

.. figure:: images/infinite/infinite-new.png
   :alt: Tiled's New Map dialog window is shown. For the option "Map Size," the Infinite radio button is selected rather than Fixed.
   :scale: 66

The map you create will have an infinite canvas.

Editing an Infinite Map
------------------------

Most tools in Tiled work the same way for infinite maps as they do for fixed-size maps. 
However, the :ref:`bucket-fill-tool` fills only the current bounds of a tile layer.
As you paint, these bounds expand.

.. figure:: images/infinite/infinite-demo.gif
   :alt: An animation shows an infinite Tiled map being edited.

Converting Between Infinite And Fixed-Size Maps
-----------------------------------------------------

You can toggle between infinite and fixed-size maps in the Map Properties window. 
When converting an infinite map to a fixed-size map, Tiled determines the final map's 
width and height based on the bounds of all tile layers.

.. figure:: images/infinite/infinite-map-initial.png
   :alt: An initial view of an infinite map being edited in Tiled. The grid extends beyond the bounds of the editing window.

   The Initial Infinite Map

.. figure:: images/infinite/infinite-map-conversion.png
   :alt: Close-up of Map Properties panel showing the Infinite option's checkbox checked.

   Unchecking the Infinite property in Map Properties

.. figure:: images/infinite/infinite-map-converted.png
   :alt: An infinite map in Tiled has been converted to finite. The grid is fully contained and does not extend beyond the bounds of the editing window.

   The Converted Map
