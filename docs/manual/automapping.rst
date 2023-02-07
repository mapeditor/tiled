***********
Automapping
***********

What is Automapping?
====================

Automapping can automatically place or replace tiles based on predefined
rules. This enables complex or repetitive tile placement to be entirely
automated, which can make level creation faster and more enjoyable. It can
also help to avoid certain types of errors.

.. note::

   The Automapping feature changed significantly in Tiled 1.9. It became
   10-30x faster, no longer needs input and output regions to be explicitly
   defined, works very well while drawing since it no longer produces separate
   undo steps and a lot of new options were added. Make sure you are up to
   date, or look for an older version of the documentation!

Setting Up The Rules File
=========================

Automapping rules are defined in regular map files, which we'll call **rule
maps**. These files are then referenced by a text file, usually called
``rules.txt``. The ``rules.txt`` can list any number of rule maps, in the
order in which their rules should be applied.

There are two ways to make the rule maps defined in the ``rules.txt`` apply to
a map:

.. raw:: html

   <div class="new new-prev">Since Tiled 1.4</div>

* Open *Project > Project Properties* and set the "Automapping rules" property
  to the ``rules.txt`` file that you created in your project. If you have only a
  single rule map, you can also refer to that map file directly.

* Alternatively, you can save your ``rules.txt`` in the same directory as the
  map files to which you want the rules to apply. This can also be used to
  override the project-wide rules for a certain set of maps.

Each line in the ``rules.txt`` file is either:

-  A path to a **rule map**.
-  A path to another ``.txt`` file which has the same syntax (e.g. in
   another directory).
-  A map filename filter, enclosed in ``[]`` and using ``*`` as a wildcard.
-  A comment, when the line starts  with ``#`` or ``//``.

.. raw:: html

   <div class="new">New in Tiled 1.9</div>

By default, all Automapping rules will run on any map you Automap. The map
filename filters let you restrict which maps rules apply to. For example, any
rule maps listed after ``[town*]`` will only apply to maps whose filenames
start with "town". To start applying rules to all maps again, you can use
``[*]``, which will match any map name.

Setting Up a Rule Map
=====================

A **rule map** is a standard map file, which can be read and written by Tiled
(usually in TMX or JSON format). A rule map can define any number of rules.
At a minimum, a rule map contains:

* One or more input layers, describing which kind of pattern the working
  map will be searched for.

* One or more output layers, describing how the working map is changed
  when an input pattern is found.

In addition, custom properties on the rule map, its layers and on objects can
be used to fine-tune the overall behavior or the behavior of specific rules.

Finally, you may need some :ref:`special tiles <automapping-SpecialCases>` to
set up certain rules. Tiled provides a built-in "Automapping Rules Tileset",
which can be added to your rule map through *Map > Add Automapping Rules
Tileset*.

Defining the Rules
==================

Multiple Rules in one Rule Map
------------------------------

If multiple rules are defined in one rule map, there must be at least one tile
of unused space between the rules. Adjacently placed tiles are interpreted as
a single rule. Diagonal connections are considered adjacent as well.

.. note::

   If the output tiles are not adjacent to the matching input tiles, special
   ignored tiles can be used to connect the two parts of the rule. See
   :ref:`automapping-SpecialCases`.

Since Tiled 1.9, the rules within one rule map are matched concurrently by
default, so they can't take into account changes made by other rules defined in
the same rule map.

If you have some rules that depend on the output of other rules, you have two
options. The easiest is to use the :ref:`MatchInOrder
<automapping-MatchInOrder>` option to disable concurrent matching of rules. In
this case, each match will apply its changes directly, affecting the further
application of rules (including the current one).

Alternatively, use multiple **rule maps** and define the desired sequence
within the ``rules.txt`` file. This can provide better performance since it
still allows the rules within each rule map to be matched concurrently.

Rules are applied (and matched, if using :ref:`MatchInOrder
<automapping-MatchInOrder>`) in the order in which they appear in the rule map.
Rules with smaller Y value come first, and if there are rules at the same Y
value, then the rules with smaller X come first. On orthogonal maps this
ordering scheme is the same as for reading in most western countries (left to
right, top to bottom).

Definition of Inputs
--------------------

Inputs are generally defined by tile layers which name follows this
scheme:

**input[not][index]\_name**

where the **[not]** and **[index]** are optional. After the first
underscore there will be the name of the input layer. The input layer
name can of course include more underscores.

The **name** determines which layer on the working map is examined. So
for example the layer *input\_Ground* will check the layer called
*Ground* in the working map for this rule. *input\_test\_case* will
check the layer *test\_case* in the working map for this rule.

Multiple layers having the same name and index is explicitly allowed and
is intended. Having multiple layers of the same name and index, will
allow you to define different possible tiles per coordinate as input.

The index is used to create complete different input conditions. All
layers having the same index are taken into account for forming one
condition. Each of these conditions are checked individually.

#. index must not contain an underscore.
#. index must not start with *not*.
#. index may be empty.

If there are tiles in the standard input layers one of these tiles must
be there to match the rule. The optional **[not]** inverts the meaning
of that layer. So if there are **inputnot** layers, the tiles placed on
them, must not occur in the working map at the examined region to make a
rule match. Within one rule you can combine the usage of both input and
inputnot layers to make rules input conditions as accurate as you need
or as fuzzy as you need.

.. raw:: html

   <div class="new">New in Tiled 1.9</div>

.. _automapping-SpecialCases:

Matching Special Cases
~~~~~~~~~~~~~~~~~~~~~~

In addition to placing any of your own tiles on an input or inputnot layer,
there are a few special cases that are covered by tiles in the "Automapping
Rules Tileset" mentioned previously:

Empty
   This tile matches any empty cell.

Ignore
   This tile does not affect the rule in any way. Its only function is to
   allow connecting otherwise disconnected parts into a single rule, but it
   can also be used for clarity.

NonEmpty
   This tile matches any non-empty cell.

Other
   This tile matches any non-empty cell, which contains a tile that is
   *different* from all the tiles used on the current input layer in the
   current rule.

Negate
   This tile negates the condition at a specific location. It is effectively
   the same as swapping all tiles from any input layer with all tiles from any
   inputnot layer, but might in some cases be more convenient or more
   readable.

Note that the meaning of these tiles is derived from their custom "MatchType"
property. This means that you can set up your own tiles for matching these
special cases as well!

Definition of Outputs
---------------------

Outputs are generally defined by layers whose name follows this scheme:

**output[index]\_name**

which is very similar to the input section. At first there must be the
word output. Then optionally an **[index]** may occur. After the first
underscore there will be the name of the target layer. The target layer
name can of course include more underscores.

All layers of the same index are treated as one possible output. So the
intention of indexes in the outputs of rules is only used for random
output.

The indexes in the output section have nothing to do with the indexes in the
input section, they are independent. In the output section they are used for
randomness. In the input section they are used to define multiple possible
layers as input. So when there are multiple indexes within one rule, the
output will be chosen fairly (uniformly distributed) across all indexes. Only
the output layers with the chosen index will be put out into the working map.

Note that the output is by default not being checked for overlapping on
itself. This can be achieved by setting the map property
:ref:`NoOverlappingOutput <automapping-NoOverlappingOutput>` to ``true``. This
option applies independently for each rule, so different rules can still
overlap each other.

Map Properties
--------------

The following map properties can be used to customize the behavior of
the rules in a **rule map**:

.. _automapping-DeleteTiles:

DeleteTiles
   This map property is a boolean property: it can be
   true or false. If rules of this rule map get applied at some location
   in your map, this map property determines if all other tiles are
   deleted before applying the rules. Consider a map where you have
   multiple layers. Not all layers are filled at all places. In that
   case all tiles of all layers should be cleared, so afterwards there
   are only the tiles which are defined by the rules. Since when not all
   tiles are cleared before, you will have still tiles from before at
   these places, which are not covered by any tile.

AutomappingRadius
   This map property is a number: 1, 2, 3 ... It
   determines how many tiles around your changes will be checked as well
   for redoing the Automapping at live Automapping.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.2</div>

MatchOutsideMap
   This map property determines whether rules can match even when their input
   region falls partially outside of a map. By default it is ``false`` for
   bounded maps and ``true`` for infinite maps. In some cases it can be useful
   to enable this also for bounded maps. Tiles outside of the map boundaries
   are simply considered empty unless one of either **OverflowBorder** or
   **WrapBorder** are also true.

   Tiled 1.0 and 1.1 behaved as if this property was ``true``, whereas older
   versions of Tiled have behaved as if this property was ``false``.

.. raw:: html

   <div class="new new-prev">Since Tiled 1.3</div>

OverflowBorder
   This map property customizes the behavior intended by the **MatchOutsideMap**
   property. When this property is ``true``, tiles outside of the map boundaries
   are considered as if they were copies of the nearest inbound tiles, effectively
   "overflowing" the map's borders to the outside region.

   When this property is ``true``, it implies **MatchOutsideMap**. Note that
   this property has no effect on infinite maps (since there is no notion of border).

.. raw:: html

   <div class="new new-prev">Since Tiled 1.3</div>

WrapBorder
   This map property customizes the behavior intended by the **MatchOutsideMap**
   property. When this property is ``true``, the map effectively "wraps" around itself,
   making tiles on one border of the map influence the regions on the other border and
   vice versa.

   When this property is ``true``, it implies **MatchOutsideMap**. Note that
   this property has no effect on infinite maps (since there is no notion of border).

   If both **WrapBorder** and **OverflowBorder** are ``true``, **WrapBorder** takes
   precedence over **OverflowBorder**.

.. raw:: html

   <div class="new">New in Tiled 1.9</div>

.. _automapping-MatchInOrder:

MatchInOrder
   When set to ``true``, each rule is applied immediately after a match is
   found. This disables concurrent matching of rules, but allows each rule to
   rely on the fact that the modifications resulting from any previous match
   have already been applied (as used to be the case before Tiled 1.9).

   Alternatively, split up your rules over multiple rule maps. They are always
   applied in-order so one rule map can rely on any modifications by previous
   rule maps having been applied.

These properties are map wide, meaning it applies to all rules which are
part of the rule map. If you need rules with different properties you
can use multiple rule maps.

A number of per-rule options are also supported and can be specified as
:ref:`object properties <automapping-ObjectProperties>`. These can also be
placed on the rule map, in which case they apply to all rules in the map.

Layer Properties
----------------

The following properties are supported on a per-layer basis:

.. _automapping-StrictEmpty:

AutoEmpty (alias: StrictEmpty)
   This layer property is a boolean property. It can be added to
   **input** and **inputnot** layers to customize the behavior for
   empty tiles within a rule.

   In "AutoEmpty" mode, empty tiles within the rule match empty tiles in the
   set layer. This can only happen when you have multiple input/inputnot
   layers and some of the tiles that are part of the same coherent rule are
   empty. Normally these tiles would be ignored, unless the special "Empty"
   tile was placed. With this option they behave as tiles matching "Empty".

.. raw:: html

   <div class="new">New in Tiled 1.9</div>

.. _automapping-ObjectProperties:

Object Properties
-----------------

A number of options can be set on individual rules, even within the same rule
map. To do this, add an Object Layer to your rule map called "rule_options".
On this layer, you can create plain rectangle objects and any options you set
on these objects will apply to all rules they contain.

The following options are supported per-rule:

ModX
   Only apply a rule every N tiles on the X axis (defaults to 1).

ModY
   Only apply a rule every N tiles on the Y axis (defaults to 1).

OffsetX
   An offset applied in combination with ModX (defaults to 0).

OffsetY
   An offset applied in combination with ModY (defaults to 0).

Probability
   The chance that a rule is skipped even if its input layers would have
   matched, from 0 to 1. A value of 0 effectively disables the rule, whereas
   a value of 1 (the default) means it is never skipped.

Disabled
   A convenient way to (temporarily) disable some rules (defaults to false).

.. _automapping-NoOverlappingOutput:

NoOverlappingOutput
   When set to true, the output of a rule is not allowed to overlap on itself.

.. raw:: html

   <div class="new">New in Tiled 1.10</div>

IgnoreLock
   Since Tiled 1.10, rules will no longer modify locked layers. Set this
   property to true to ignore the lock. This can be useful, when you want to
   keep layers locked which are only changed by rules.

All these options can also be set on the rule map itself, in which case they
apply as defaults for all rules, which can then be overridden for specific
rules by placing rectangle objects.

Examples
========

Abstract Input Layer Examples
-----------------------------

Having Multiple Input Layers with the Same Name
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assume the following 3 tile layers as input, which possible inputs are
there in the working map?

+----------------------------------------------------+-----------------+
| Tile layer                                         | Name            |
+====================================================+=================+
| .. image:: images/automapping/abstract/12.png      | input\_Ground   |
|    :alt: tiles 1 and 2                             |                 |
+----------------------------------------------------+-----------------+
| .. image:: images/automapping/abstract/34.png      | input\_Ground   |
|    :alt: tiles 3 and 4                             |                 |
+----------------------------------------------------+-----------------+
| .. image:: images/automapping/abstract/56.png      | input\_Ground   |
|    :alt: tiles 5 and 6                             |                 |
+----------------------------------------------------+-----------------+

The following parts would be detected as matches for this rule:

+----------------------------------------------------+--------------------------------------------------+------------------------------------------------+
| .. image:: images/automapping/abstract/12.png      | .. image:: images/automapping/abstract/32.png    | .. image:: images/automapping/abstract/52.png  |
|    :alt: tiles 1 and 2                             |    :alt: tiles 3 and 2                           |    :alt: tiles 5 and 2                         |
+----------------------------------------------------+--------------------------------------------------+------------------------------------------------+
| .. image:: images/automapping/abstract/14.png      | .. image:: images/automapping/abstract/34.png    | .. image:: images/automapping/abstract/54.png  |
|    :alt: tiles 1 and 4                             |    :alt: tiles 3 and 4                           |    :alt: tiles 5 and 4                         |
+----------------------------------------------------+--------------------------------------------------+------------------------------------------------+
| .. image:: images/automapping/abstract/16.png      | .. image:: images/automapping/abstract/36.png    | .. image:: images/automapping/abstract/56.png  |
|    :alt: tiles 1 and 6                             |    :alt: tiles 3 and 6                           |    :alt: tiles 5 and 6                         |
+----------------------------------------------------+--------------------------------------------------+------------------------------------------------+

Input Layers Using Different Indexes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Given the following 3 input tile layers:

+----------------------------------------------------+-----------------+
| Tile layer                                         | Name            |
+====================================================+=================+
| .. image:: images/automapping/abstract/12.png      | input\_Ground   |
|    :alt: tiles 1 and 2                             |                 |
+----------------------------------------------------+-----------------+
| .. image:: images/automapping/abstract/34.png      | input\_Ground   |
|    :alt: tiles 3 and 4                             |                 |
+----------------------------------------------------+-----------------+
| .. image:: images/automapping/abstract/56.png      | input2\_Ground  |
|    :alt: tiles 5 and 6                             |                 |
+----------------------------------------------------+-----------------+

The last layer has an index unequal to the other indexes (which are
empty). All following parts would be recognized as matches within the
working map:

+----------------------------------------------------+--------------------------------------------------+
| .. image:: images/automapping/abstract/12.png      | .. image:: images/automapping/abstract/32.png    |
|    :alt: tiles 1 and 2                             |    :alt: tiles 3 and 2                           |
+----------------------------------------------------+--------------------------------------------------+
| .. image:: images/automapping/abstract/14.png      | .. image:: images/automapping/abstract/34.png    |
|    :alt: tiles 1 and 4                             |    :alt: tiles 3 and 4                           |
+----------------------------------------------------+--------------------------------------------------+
| .. image:: images/automapping/abstract/56.png      |                                                  |
|    :alt: tiles 5 and 6                             |                                                  |
+----------------------------------------------------+--------------------------------------------------+

The Mana World Examples
-----------------------

The Mana World examples will demonstrate quite a lot of different
Automapping features. At first a shoreline will be constructed, by first
adding all the straight parts and afterwards another rule will correct
the corners to make them also fit the given tileset. After the shoreline
has been added, the waters will be marked as unwalkable for the game
engine. Last but not least the grass should be made
random by using 5 different grass tiles.

.. figure:: images/automapping/TheManaWorld/before.png

   This is what we want to draw.

.. figure:: images/automapping/TheManaWorld/flow1.png

   Here we have straight shorelines applied.

.. figure:: images/automapping/TheManaWorld/flow2.png

   Here we have some corners.

.. figure:: images/automapping/TheManaWorld/flow3.png

   And corners the other way round as well.

.. figure:: images/automapping/TheManaWorld/flow4.png

   Here all unwalkable tiles are marked.

.. figure:: images/automapping/TheManaWorld/flow5.png

   If you look closely at the grass, you'll see they are now randomized.

.. _automapping-BasicShoreline:

Basic Shoreline
~~~~~~~~~~~~~~~

.. warning::

    The below examples are not adjusted yet to Tiled 1.9! They still work,
    since compatibility has been largely maintained, but they use explicit
    region layers to achieve what you can now do with :ref:`special Automapping
    tiles <automapping-SpecialCases>`.

This example will demonstrate how a straight shoreline can easily be
setup between shallow water grass tiles. In this example we will only
implement the shoreline, which has grass in southern and water in
northern direction.

So basically the meaning we will define in the input region is: *All
tiles which are south of a water tile and are not water tiles themselves,
will be replaced by a shoreline tile.*

+-----------------------------------------------------------+------------------+
| Tile layer                                                | Name             |
+===========================================================+==================+
| .. image:: images/automapping/TheManaWorld/1/regions.png  | regions          |
+-----------------------------------------------------------+------------------+
| .. image:: images/automapping/TheManaWorld/1/input.png    | input\_Ground    |
+-----------------------------------------------------------+------------------+
| .. image:: images/automapping/TheManaWorld/1/output.png   | output\_Ground   |
+-----------------------------------------------------------+------------------+

The region in which this Automapping rule should be defined is of 2
tiles in height and 1 tile in width. Therefore we need a layer called
*regions* and it will have 2 tiles placed to indicate this region.

The input layer called *input\_Ground* is depicted in the middle. Only
the upper tile is filled by the water tile. The lower tile contains no
tile. It is not an invisible tile, just no tile at all.

And whenever there is no tile in a place within the rule regions in an
input layer, what kind of tiles will be allowed there? There will be
allowed any tiles except all used tiles within all input layer with the
same index and name.

Here we only have one tile layer as an input layer carrying only the
water tile. Hence at the position, where no tile is located, all tiles
except that water tile are allowed.

The output layer called *output\_Ground* shows the tile which gets
placed, if this rule matches.

Corners on a Shore Line
~~~~~~~~~~~~~~~~~~~~~~~

This example is a continuation of the previous example. Now the corners
of the given shoreline should be implemented automatically. Within this
article we will just examine the bent in corner shoreline in the top left
corner. The other shoreline corners are constructed the same way. So
after the example is applied, we would like to have the corners of the
shoreline get suitable tiles. Since we rely on the other example being
finished, we will put the rules needed for the corners into another new
rule map (which is listed afterwards in ``rules.txt``).

+-----------------------------------------------------------+-----------------------------------------------------------+-----------------------------------------------------------+
| .. image:: images/automapping/TheManaWorld/2/pattern1.png | .. image:: images/automapping/TheManaWorld/2/pattern2.png | .. image:: images/automapping/TheManaWorld/2/pattern3.png |
+-----------------------------------------------------------+-----------------------------------------------------------+-----------------------------------------------------------+
| .. image:: images/automapping/TheManaWorld/2/pattern4.png | .. image:: images/automapping/TheManaWorld/2/pattern5.png | .. image:: images/automapping/TheManaWorld/2/pattern6.png |
+-----------------------------------------------------------+-----------------------------------------------------------+-----------------------------------------------------------+
| .. image:: images/automapping/TheManaWorld/2/pattern7.png | .. image:: images/automapping/TheManaWorld/2/pattern8.png | .. image:: images/automapping/TheManaWorld/2/pattern9.png |
+-----------------------------------------------------------+-----------------------------------------------------------+-----------------------------------------------------------+

The shoreline may have some more corners nearby, which means there may
be more different tiles than the straight corner lines. In the figure we
see all inputs which should be covered.

Both the tiles in the top right corner and in the lower left corner are
directly adjacent to the desired (slightly transparent) tile in the top
left corner.

We can see 3 different tiles for the lower left corner, which is
straight shore line, bent inside and bent outside shore lines.

Also we see 3 different inputs for the top right corner, which also is
straight, bent in or out shore line.

Input and Output Regions
^^^^^^^^^^^^^^^^^^^^^^^^

So with this rule we want to put the bent in shore line tile in the top
left corner; we don't care which tile was there before. We also don't
care about the tile in the lower right corner (probably water, but can
be any decorative water tile, so just ignore it).

+-----------------------------------------------------------------+------------------------------------------------------------------+-------------------------------------------------------------------+
| .. image:: images/automapping/TheManaWorld/2/regions_input.png  | .. image:: images/automapping/TheManaWorld/2/regions_output.png  | .. image:: images/automapping/TheManaWorld/2/regions_united.png   |
+-----------------------------------------------------------------+------------------------------------------------------------------+-------------------------------------------------------------------+

Therefore we will need different input and output regions. In the figure
we can see the both tile layers regions input and regions output. The
input section covers just these two tiles as we discussed. The output
region covers just the single tile we want to output. Though the input
and output region do not overlap, the united region of both the input
and the output region is still one coherent region, so it's one rule and
works.

Output regions can be larger than absolutely required, since where there
are no tiles in an output region, the tiles in the working map are not
overwritten but just kept as is, hence each output region could also be
sized as the united region of both the output and input region.

Input Layers
^^^^^^^^^^^^

Now we want to put all the nine possible patterns we observed as
possible input for this rule. We could of course define nine different
layers *input1\_Ground* up to *input9\_Ground*.

Nine TileLayers?! What a mess; we'll do it a better way.

Also, consider having not just 3 possible tiles at the 2 locations but 4.
Then we would need 4\*4=16 tilelayers to get all conditions. Another
downside of this comes with more needed locations: Think of more than 2
locations needed to construct a rule input. So for 3 locations, each
location could have the 3 possibilities, hence you need 3\*3\*3 = 27
tilelayers. It's not getting better...

So let's try a smart way: All input layers have the same name, so at
each position any of the three different tiles is valid.

+------------------------------------------------------------------+-----------------+
| Tile layer                                                       | Name            |
+==================================================================+=================+
| .. image:: images/automapping/TheManaWorld/2/input_Ground1.png   | input\_Ground   |
+------------------------------------------------------------------+-----------------+
| .. image:: images/automapping/TheManaWorld/2/input_Ground2.png   | input\_Ground   |
+------------------------------------------------------------------+-----------------+
| .. image:: images/automapping/TheManaWorld/2/input_Ground3.png   | input\_Ground   |
+------------------------------------------------------------------+-----------------+

Output Layer
^^^^^^^^^^^^

The output is straightforward, since only one tile is needed. No
randomness is needed, hence the index is not needed to be varied, so
it's kept empty. The desired output layer is called Ground, so the over
all name of the single output layer will be output\_Ground. The correct
tile is placed at the correct location with this layer.

+------------------------------------------------------------------+
| .. image:: images/automapping/TheManaWorld/2/output_Ground.png   |
+------------------------------------------------------------------+

The Other Corners on a Shore Line
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is for corners bent the other way round. Basically it has the same
concepts, just other tiles.

+-------------------------------------------------------------------+-------------------+
| Tile layer                                                        | Name              |
+===================================================================+===================+
| .. image:: images/automapping/TheManaWorld/3/input_Ground1.png    | input\_Ground     |
+-------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/3/input_Ground2.png    | input\_Ground     |
+-------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/3/input_Ground3.png    | input\_Ground     |
+-------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/3/output_Ground.png    | output\_Ground    |
+-------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/3/regions_input.png    | regions\_input    |
+-------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/3/regions_output.png   | regions\_output   |
+-------------------------------------------------------------------+-------------------+

Adding Collision Tiles
~~~~~~~~~~~~~~~~~~~~~~

The Mana World uses an extra tile layer called *Collision* to have
information about whether a player is able to walk on certain tiles or
not. That layer is invisible to the player, but the game engine
parses it, whether there is a tile or there is no tile.

So we need to decide for each position if a player can walk there and
put a tile into the *Collision* layer if it is unwalkable.

As *input* layer we will parse the *Ground* layer and put collision
tiles where the player should not walk.

Actually this task is a bunch of rules, but each rule itself is very
easy:

+----------------------------------------------------------------------+---------------------+
| Tile layer                                                           | Name                |
+======================================================================+=====================+
| .. image:: images/automapping/TheManaWorld/4/regions.png             | regions             |
+----------------------------------------------------------------------+---------------------+
| .. image:: images/automapping/TheManaWorld/4/input_Ground.png        | input\_Ground       |
+----------------------------------------------------------------------+---------------------+
| .. image:: images/automapping/TheManaWorld/4/output_Collision.png    | output\_Collision   |
+----------------------------------------------------------------------+---------------------+

In the above *regions* layer we have 14 different rules, because there
are 14 incoherent regions in the *regions* layer. That's 9 different
water tiles, which should be unwalkable and 5 different grass tiles
which will be placed randomly in the next example.

As input we will have one of all the used tiles and as output there is
either a tile in the *Collision* layer or not.

**Do we need the rules with clean output?** No, it is not needed for one
run of Automapping. But if you are designing a map, you will likely add
areas with collision and then remove some parts of it again and so on.

So we need to also remove the collision tiles from positions, which are
not marked by a collision any more. This can be done by adding the map
property :ref:`DeleteTiles <automapping-DeleteTiles>` and setting it to
``true``. Then all the parts in the *Collision* layer will be erased before the
Automapping takes place, so the collision tiles are only placed at real
unwalkable tiles and the history of if there has been a collision tile placed
is neglected.

Random Grass Tiles
~~~~~~~~~~~~~~~~~~

In this example we will shuffle all grass tiles, so each grass tile will
be replaced with a randomly chosen tile.

As input we will choose all of our grass tiles. This is done by having
each tile in its own input layer, so each grass tile gets accepted for
this rule.

As output we will also put each grass tile into one output layer. To
make it random the *index* of the output layers needs to be different
for each layer.

The following rule might look the same, but there are different
grass tiles. Each grass tile is in both one of the input and one of the
output layers (the order of the layers doesn't matter).

+-------------------------------------------------------------------------------+-------------------+
| Tile layer                                                                    | Name              |
+===============================================================================+===================+
| .. image:: images/automapping/TheManaWorld/5/1.png                            | input\_Ground     |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/2.png                            | input\_Ground     |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/3.png                            | input\_Ground     |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/4.png                            | input\_Ground     |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/5.png                            | input\_Ground     |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/1.png                            | output1\_Ground   |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/2.png                            | output2\_Ground   |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/3.png                            | output3\_Ground   |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/4.png                            | output4\_Ground   |
+-------------------------------------------------------------------------------+-------------------+
| .. image:: images/automapping/TheManaWorld/5/5.png                            | output5\_Ground   |
+-------------------------------------------------------------------------------+-------------------+

An Alternating Wall
-------------------

This example will demonstrate how a wall as a transition between a
walkable area and the unwalkable black void can easily be setup. As
input a dedicated set layer will be used.

+-------------------------------------------------------+--------------------------------------------------------+
| .. image:: images/automapping/LoneCoder/desired.png   | .. image:: images/automapping/LoneCoder/setlayer.png   |
|    :alt: Vertically the tiles are alternating         |    :alt: A dedicated set layer                         |
+-------------------------------------------------------+--------------------------------------------------------+

In my opinion a dedicated set layer is much easier to use for the rough
draft, but for adding details such as collision information on
decorative tiles the input should use the decoration.

The structure of the input, output and region layer is very similar to
the example of the straight shoreline in The Mana World examples. The
main difference is the different size. Since the wall contains multiple
tiles in height, the height of the rule layers are different as well.
Vertically the tiles are also alternating. As you can see in the
following figure, every second tile displaying the base board of the
wall has a notch for example.

+-----------------------------------------------------------+-----------------+
| Tile layer                                                | Name            |
+===========================================================+=================+
| .. image:: images/automapping/LoneCoder/regions.png       | regions         |
+-----------------------------------------------------------+-----------------+
| .. image:: images/automapping/LoneCoder/input_Ground.png  | input\_Ground   |
+-----------------------------------------------------------+-----------------+
| .. image:: images/automapping/LoneCoder/output_Ground.png | output\_Walls   |
+-----------------------------------------------------------+-----------------+

Hence the region in which this Automapping rule should be defined is of
4 tiles in height and 2 tile in width. Therefore we need a layer called
*regions* and it will have 8 tiles placed to indicate this region. In
the figure the top graphics shows such a region layer.

The input layer has the following meaning:

*If there are 2 vertical adjacent brown tiles in the set layer and in
the 3x2 tiles above here are no brown tiles, this rule matches.*

Only the lowest 2 coordinates contain the brown tile. The upper
coordinates contain no tile. (It is not an invisible tile, just no tile
at all.) The input layer called *input\_set* is depicted in the middle
of the figure.

The output consists of only one layer as well called *output\_Walls*. It
contains the actual wall tiles.

.. figure:: images/automapping/LoneCoder/desired.png

   Vertically the tiles are alternating.


.. figure:: images/automapping/LoneCoder/firstattempt.png

   A broken version of the rule, because
   :ref:`NoOverlappingOutput <automapping-NoOverlappingOutput>` was not yet set.

When trying to match the input layer to the desired set layer (right
picture of the figure at the beginning of the example), you will see it
matches all the way along, with no regard of the vertical adjustment.

Hence when we use the rule as discussed now, we will get not the desired
result, because this rule overlaps itself. The overlapping problem is shown
in figure above.

Since the overlapping is not desired, we can turn it off by adding the map
property :ref:`NoOverlappingOutput <automapping-NoOverlappingOutput>` to the
rule map and setting it to ``true``.

Keep in mind that the map property applies for all rules on that rule map,
unless we set it only for specific rules using a "rule_options" layer.
