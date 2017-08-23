Using Wang Tiles
================

Wang Tiles is similar in concept to Terrains. This is, however, more focused
on filling larger areas without repetition. One defines the edge and
corner colors of tiles in a tileset. This information can then be used when
filling, or brushing to allow for smooth, non-repetitive transitions between
tiles. In most cases this tiling is random, and based on color probability.
More info on Wang Tiles can be found `Here <http://www.cr31.co.uk/stagecast/wang/intro.html>`_.

To demonstrate how to use wang tiles, I will describe the steps necessary
to recreate ``walkways.txs`` example tileset.

Defining Wang Tile Info
-----------------------
After making the tileset, from the tileset editor, click the Wang Sets button.

.. figure:: images/wangtiles/01-wangbutton.jpg
   :alt: Wang Set Button

   Wang Set Button

A single tileset can have many wangsets. Create a new wangset using the plus
button at the bottom of the wangset view.

.. figure:: images/wangtiles/02-wangsetview.jpg
   :alt: Wang Set View
   
   Wang Set View

You can now edit the properties of the wangset. Important for us is edge
and corner count. This will determine how the set is defined, and how it
behaves. This tileset is a 3 edge wangset.

.. figure:: images/wangtiles/03-wangsetproperties.jpg
   :alt: Wang Set Properties
   
   Wang Set Properties

Now in the complete pattern set will generate in the *Patterns* tab below
the wangset view. For the set to be complete (though this is unnecessary),
each pattern must be used at least once.

.. figure:: images/wangtiles/04-patternview.jpg
   :alt: Pattern View
   
   Pattern View

Once a pattern is selected, you can paint it directly onto the tileset.
Similar to when using the Stamp Brush, ``Z`` and ``Shift + Z`` can be used
to rotate the pattern 90 degrees clockwise and counterclockwise respectively.
``X`` and ``Y`` flip the pattern horizontally, and vertically respectively.

.. figure:: images/wangtiles/05-assigningpattern.jpg
   :alt: Painting on a pattern
   
   Painting on a pattern

In the other tab, there is the *Colors* view. This gives you access to 
edit properties and assign with each individual color of a set.

.. figure:: images/wangtiles/06-assigningedge.jpg
   :alt: Painting individual edge
   
   Painting individual edge

Using these methods, assign each tile matching all the edges. After this
is done, the set is ready to be used with all the wang methods.

.. figure:: images/wangtiles/07-fullassignment.jpg
   :alt: Completely assigned Wang Set
   
   Completely assigned Wang Set

Editing With Wang Methods
-------------------------
There are many places where wang tiles can be used. Similar to the random
mode, the Stamp Brush, and Bucket Fill tools can use wang methods to fill.

.. figure:: images/wangtiles/08-stampbrush.jpg
   :alt: Stamp Brush with Wang Fill Mode on
   
   Stamp Brush with Wang Fill Mode on

.. figure:: images/wangtiles/09-bucketfill.jpg
   :alt: Bucket Fill with Wang Fill Mode on
   
   Bucket Fill with Wang Fill Mode on

There is also the Wang Brush, which works very much like the terrain tool.
Further details about this can be found at `Wang Brush <editing-tile-layers.html#wang-brush>`__.

.. figure:: images/wangtiles/10-wangbrush.jpg
   :alt: Wang Brush
   
   Wang Brush
