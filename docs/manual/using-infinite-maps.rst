.. raw:: html

   <div class="new">New in Tiled 1.1</div>

Using Infinite Maps
===================

Infinite maps give you independence from bounds of the map. The canvas is
"auto-growing", which basically means, that you have an infinite grid which
can be painted upon without worrying about the width and height of the map.
The bounds of a particular layer get expanded whenever tiles are painted
outside the current bounds.

.. figure:: images/infinite/infinite-map-overview.png
   :alt: Infinite Maps Overview

Creating an Infinite Map
------------------------

In the order to create an infinite map, make sure the 'Infinite' option is
checked in New Map dialog.

.. figure:: images/infinite/infinite-new.png
   :alt: New Infinite Map

The newly created map will then have an infinite canvas.

Editing the Infinite Map
------------------------

Except the Bucket Fill Tool, every other tool works exactly in the same way as
in the fixed size maps. The bucket fill tool fills the current bounds of that
particular tile layer. These bounds get increased upon further painting of
that tile layer.

.. figure:: images/infinite/infinite-demo.gif
   :alt: Infinite Maps Editing

Conversion from Infinite to Finite Map and vice versa
-----------------------------------------------------

In the map properties, you can check/uncheck whether the map should be
infinite or not. When converting from infinite to a finite map, the width and
height of the final map are chosen on the basis of bounds of all the tile
layers.

.. figure:: images/infinite/infinite-map-initial.png
   :alt: Initial Map

   The initial Infinite Map

.. figure:: images/infinite/infinite-map-conversion.png
   :alt: Properties

   Unchecking the Infinite property in Map Properties

.. figure:: images/infinite/infinite-map-converted.png
   :alt: Final Map

   The converted Map
