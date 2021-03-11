Automatic Terrain Tile Placement
================================

When editing a tile map, sometimes we don't think in terms of *tiles* but
rather in terms of *terrains*. Say we want to draw a patch of grass, a road or
a certain platform. In this case, manually choosing the right tiles for the
various transitions or connections quickly gets tedious.

The :ref:`terrain-tool` was added to make editing tile maps easier in such
cases. The tool relies on the tileset providing one or more **Terrain Sets**.
Tiled supports the following terrain sets:

Corner Set
    Tiles that needs to match neighboring tiles at their corners, with a
    transition from one type of terrain to another in between. A complete set
    with 2 terrains has 16 tiles.

Edge Set
    Tiles that need to match neighboring tiles at their sides. This is common
    for roads, fences or platforms. A complete set with 2 terrains has 16
    tiles.

Mixed Set
    Tiles that rely on matching neighboring tiles using both their corners and
    sides. This allows a tileset to provide more variation, at the cost of
    needing significantly more tiles. A complete set with 2 terrains has 256
    tiles, but reduced sets like the 47-tile `Blob tileset`_ can be used with
    this type as well.

Based on the information in a terrain set, the :ref:`terrain-tool` can
understand the map and automatically choose the right tiles when making edits.
When necessary, it also adjusts neighboring tiles to make sure they correctly
connect to the modified area.

The :ref:`terrain-tool`, as well as the :ref:`bucket-fill-tool` and the
:ref:`shape-fill-tool`, also have a mode where they can fill an area with
random terrain.

.. _define-terrain-information:

Define the Terrain Information
------------------------------

Creating the Terrain Set
^^^^^^^^^^^^^^^^^^^^^^^^

First of all, switch to the tileset file. If you're looking at the map
and have the tileset selected, you can do this by clicking the small
*Edit Tileset* button below the Tilesets view.

.. figure:: images/terraintool/edit-tileset-button.png
   :alt: Edit Tileset button

   Edit Tileset button

Then, activate the terrain editing mode by clicking on the *Terrain Sets*
|terrain| button on the tool bar. With this mode activated, the *Terrain Sets*
view will become visible, with a button to add a new set. In this example,
we'll define a *Corner Set*.

.. figure:: images/terraintool/add-terrain-set.png
   :alt: Adding a Terrain Set

   Adding a Terrain Set

When adding a terrain set, the name of the new set will automatically get
focus. Give the set a recognizable name, in the example we'll type "Desert
Ground". We can also set one of the tiles as the icon of the set by
right-clicking a tile and choosing "Use as Terrain Set Image".

Adding Terrains
^^^^^^^^^^^^^^^

The new set will have one terrain added by default. If we already know we need
additional ones, click the *Add Terrain* button to add more. Each terrain has
a name, color and can have one of the tiles associated with it. Double-click
the terrain to edit its name. To change the color, right-click the terrain and
choose "Pick Custom Color". To assign a tile, select the terrain and then
right-click a tile, choosing "Use as Terrain Image".


.. figure:: images/terraintool/terrains-added.png
   :alt: Our Terrains

   Our Terrains

.. note::

    We generally don't need to define an explicit terrain for "empty tiles".
    If you have tiles transitioning to nothing, it should be enough to not
    mark those areas.

With our terrains set up we're ready to mark each of our tiles.

Marking the Tiles
^^^^^^^^^^^^^^^^^

Note that for a *Corner Set*, we can only mark the corners of the tiles. For a
*Edge Set*, we're limited to marking the edges of our tiles. If we need both
we need to use a *Mixed Set*. If it turns out that we chose the wrong type of
terrain set, we can still change the type in the Properties view (right-click
the terrain set and choose *Terrain Set Properties...*).

With the terrain we want to mark selected, click and drag to mark the regions
of the tiles that match this terrain.

.. figure:: images/terraintool/sand-marked.png
   :alt: Sand marked

   Here we have marked all the sandy corners in our example tileset.

If you make a mistake, just use Undo (or press ``Ctrl+Z``). Or if you
notice a mistake later, either use *Erase Terrain* to clear a terrain type
from a corner or select the correct terrain type and paint over it. Each
corner can only have one type of terrain associated with it.

Now do the same for each of the other terrain types. Eventually you'll have
marked all tiles apart from the special objects.

.. figure:: images/terraintool/done-marking-tiles.png
   :alt: Done marking tiles

   We're done marking the terrain of our tiles.

Now you can disable the *Terrain Sets* |terrain| mode by clicking the tool bar
button again.

Editing with the Terrain Brush
------------------------------

Switch back to the map and then activate the *Terrain Sets* window. Select the
terrain set we have just set up, so we can use its terrains.

Click on the Sand terrain and try to paint. You may immediately notice that
nothing is happening. This is because there are no other tiles on the map yet,
so the terrain tool doesn't really know how to help (because we also have no
transitions to "nothing" in our tileset). There are two ways out of this:

* We can hold ``Ctrl`` to paint a slightly larger area. This way we will paint
  at least a single tile filled with the selected terrain, though this is not
  convenient for painting larger areas.

* Assuming we're out to create a desert map, it's better to start by filling
  the entire map with sand. Just switch back to the *Tilesets* window for a
  moment, select the sand tile and then use the :ref:`bucket-fill-tool`.

Once we've painted some sand, let's select the Cobblestone terrain. Now you
can see the tool in action!

.. figure:: images/terraintool/drawing-cobblestone.png
   :alt: Drawing cobblestone

   Drawing cobblestone

Try holding ``Control`` (``Command`` on a Mac) while drawing. This increases
the modified area to cover all corners and/or edges of the hovered tile,
allowing for faster painting of larger areas.

Finally, see what happens when you try drawing some dirt on the
cobblestone. Because there are no transitions from dirt directly to
cobblestone, the Terrain tool first inserts transitions to sand and from
there to cobblestone. Neat!

.. figure:: images/terraintool/drawing-dirt.png
   :alt: Drawing dirt

   Drawing dirt

.. note::

    An *Erase Terrain* button is provided for the case where your terrain
    tiles transition to nothing. This allows for erasing parts of your terrain
    while choosing the right tiles as well. This mode does nothing useful when
    there are no transitions to nothing in the selected Terrain Set.

Final Words
-----------

Now you should have a pretty good idea about how to use this tool in
your own project. A few things to keep in mind:

- For one terrain to interact with another, they need to be part of the same
  *Terrain Set*. This also means all tiles need to be part of the same tileset.
  This usually means you may have to merge some tilesets into one image.

- Since defining the terrain information can be somewhat laboursome,
  you'll want to avoid using embedded tilesets so that terrain
  information can be shared among several maps.

- The Terrain tool works fine with isometric maps as well. To make sure
  the terrain overlay is displayed correctly, set up the *Orientation*,
  *Grid Width* and *Grid Height* in the tileset properties.

- The tool will handle any number of terrains (up to 255) and each corner of a
  tile can have a different type of terrain. Still, there are other ways of
  dealing with transitions that this tool can't handle. Also, it is not able
  to edit multiple layers at the same time. For a more flexible, but also more
  complicated way of automatic tile placement, check out :doc:`automapping`.

- There's a `collection of tilesets
  <http://opengameart.org/content/terrain-transitions>`__ that contain
  transitions that are compatible with this tool on `OpenGameArt.org
  <http://opengameart.org/>`__.

.. _blob tileset: http://www.cr31.co.uk/stagecast/wang/blob.html

.. |terrain| image:: ../../src/tiled/images/24/terrain.png

..
    TODO:
    * Images showing example of each terrain type
    * Section about randomization
    * Summarizing the Edge and Mixed sets
    * Mention the Patterns tab (marking tiles, checking set compleness)
    * Mention flipping and rotation
    * Mention the Terrain Fill Mode (along with probability?)
