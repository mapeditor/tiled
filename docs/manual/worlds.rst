.. raw:: html

   <div class="new new-prev">Since Tiled 1.2</div>

Working with Worlds
===================

Sometimes a game has a large world which is split over multiple maps to make
the world more digestible by the game (less memory usage) or easier to edit by
multiple people (avoiding merge conflicts). It would be useful if the maps
from such a world could be seen within the same view, and to be able to
quickly switch between editing different maps. Defining a world allows you to
do exactly that.

.. figure:: images/world-view.png
   :alt: Many maps from The Mana World shown at once

   Many maps from `The Mana World <https://www.themanaworld.org/>`__ shown at
   once.

Defining a World
----------------

A world is defined in a ``.world`` file, which is a JSON file that
tells Tiled which maps are part of the world and at what location. Worlds
can be created by using the *Map > New World...* action.

You may also create `.world files` by hand. Here is a simple example of a
world definition, which defines the global position (in pixels) of three maps:

.. code:: json

    {
        "maps": [
            {
                "fileName": "001-1.tmx",
                "x": 0,
                "y": 0
            },
            {
                "fileName": "002-1.tmx",
                "x": 0,
                "y": 3200
            },
            {
                "fileName": "006-1.tmx",
                "x": 3840,
                "y": 4704
            }
        ],
        "type": "world"
    }

Once defined, a world needs to be loaded by choosing *Map > Load World...*
from the menu. Multiple worlds can be loaded at the same time, and worlds will
be automatically loaded again when Tiled is restarted.

When is map is opened, Tiled checks whether it is part of any of the loaded
worlds. If so, any other maps in the same world are loaded as well and
displayed alongside the opened map. You can click any of the other maps to
open them for editing, which will switch files while keeping the view in the
same position.

Worlds are reloaded automatically when their file is changed on disk.

.. raw:: html

   <div class="new">New in Tiled 1.4</div>

Editing Worlds
--------------

Once you have loaded a world, you can select the 'World Tool' from the toolbar
to add, remove and move maps within the world.

Adding Maps
    Click the 'Add the current map to a loaded world' button on the toolbar,
    from the dropdown menu select the world you want to add it to. To add a
    different map to the current world, you can use the 'Add another map to
    the current world' button from the toolbar. Alternatively, both actions
    can be accessed by rightclicking in the  map editor.

Removing Maps
    Hit the 'Remove the current map from the current world' button on the 
    toolbar. Alternatively, rightclick a map in the map editor and select the
    'Remove ... from World ...' action from the context menu.

Moving Maps
    Simply drag around maps within the map editor. You can abort moving a map
    by hitting 'Escape' or by right-clicking.

    Alternatively you can use the arrow keys to move the current selected map
    - holding Shift will perform bigger steps.

Saving World files
    You can save manipulated world files by using the *Map > Save World* 
    menu. Worlds will also automatically be saved if you launch any external
    tool that has the 'Save Map Before Executing' option enabled.

Using Pattern Matching
----------------------

For projects where the maps follow a certain naming style that allows the
location of each map in the world to be derived from the file name, a regular
expression can be used in combination with a multiplier and an offset.

.. note::

    Currently no interface exists in Tiled to define a world using pattern
    matching, nor can it be modified. World files with patterns have to be
    manually edited.

Here is an example:

.. code:: json

    {
        "patterns": [
            {
                "regexp": "ow-p0*(\\d+)-n0*(\\d+)-o0000\\.tmx",
                "multiplierX": 6400,
                "multiplierY": 6400,
                "offsetX": -6400,
                "offsetY": -6400
            }
        ],
        "type": "world"
    }

The regular expression is matched on all files that live in the same directory
as the world file. It captures two numbers, the first is taken as ``x`` and
the second as ``y``. These will then be multiplied by ``multiplierX`` and
``multiplierY`` respectively, and finally ``offsetX`` and ``offsetY`` are
added. The offset exists mainly to allow multiple sets of maps in the same
world to be positioned relative to each other. The final value becomes the
position (in pixels) of each map.

A world definition can use a combination of manually defined maps and
patterns.

Showing Only Direct Neighbors
-----------------------------

Tiled takes great care to only load each map, tileset and image once, but
sometimes the world is just too large for it to be loaded completely. Maybe
there is not enough memory, or rendering the entire map is too slow.

In this case, there is an option to only load the direct neighbors of the
current map. Add ``"onlyShowAdjacentMaps": true`` to the top-level JSON object.

To make this possible, not only the position but also the size of each map
needs to be defined. For individual maps, this is done using ``width`` and
``height`` properties. For patterns, the properties are ``mapWidth`` and
``mapHeight``, which default to the defined multipliers for convenience. All
values are in pixels.

.. note::

    In the future, I will probably change this option to allow specifying a
    distance around the current map in which other maps are loaded.
