Libraries and Frameworks
========================

There are many libraries available for reading and/or writing Tiled maps (either stored in the
:doc:`tmx-map-format` or the :doc:`json-map-format`) as well as many
development frameworks that include support for Tiled maps. This list is
divided into two sections:

-  `Support by Language <#support-by-language>`__
-  `Support by Framework <#support-by-framework>`__

The first list is for developers who plan on implementing their own
renderer. The second list is for developers already using (or
considering) a particular game engine / graphics library who would
rather pass on having to write their own tile map renderer.

.. note::

   For updates to this page please open a pull request or issue
   `on GitHub <https://github.com/bjorn/tiled/issues>`__, thanks!

Support by Language
-------------------

These libraries typically include only a TMX parser, but no rendering
support. They can be used universally and should not require a specific
game engine or graphics library.

C
~

-  `TMX <https://github.com/baylej/tmx/>`__ - TMX map loader
   with Allegro5 and SDL2 examples (BSD).

C++
~~~

-  `C++/Boost <http://www.catnapgames.com/blog/2011/10/10/simple-tmx-tilemap-parser.html>`__
   by Tomas Andrle (limited functionality, single cpp file)
-  `C++/TinyXML based tmxparser <https://github.com/sainteos/tmxparser>`__ (BSD)
-  C++/Qt based libtiled, used by Tiled itself and included at
   `src/libtiled <https://github.com/bjorn/tiled/tree/master/src/libtiled>`__
   (BSD)
-  `C++11x/TinyXml2
   libtmx-parser <https://github.com/halsafar/libtmx-parser>`__ by
   halsafar. (zlib/tinyxml2)
-  `C++11/TinyXml2 libtmx <https://github.com/jube/libtmx>`__ by jube,
   for reading only (ISC licence). See
   `documentation <http://jube.github.io/libtmx/index.html>`__.
-  `TMXParser <https://github.com/solar-storm-studios/TMXParser>`__
   General \*.tmx tileset data loader. Intended to be used with
   TSXParser for external tileset loading. (No internal tileset support)
-  `TSXParser <https://github.com/solar-storm-studios/TSXParser>`__
   General \*.tsx tileset data loader. Intended to be used with
   TMXParser.
-  `TMXLoader <https://bitbucket.org/martingrant/tmxloader>`__ based on
   `RapidXml <http://rapidxml.sourceforge.net/>`__. Limited
   functionality (check the
   `website <http://www.midnightpacific.com/portfolio/tmxloader-for-tiled-map-editor/>`__
   for details).
-  `tmxlite <https://github.com/fallahn/tmxlite>`__ C++14 map parser
   with compressed map support but no external linking required.
   Includes examples for SFML and SDL2 rendering. Currently has full tmx
   support up to 0.16. (Zlib/libpng)

C#/.NET
~~~~~~~

-  `MonoGame.Extended <https://github.com/craftworkgames/MonoGame.Extended>`__
   has a Tiled map loader and renderer that works with MonoGame on all
   platforms that support portable class libraries.
-  `XNA map loader <https://github.com/zachmu/tiled-xna>`__ by Kevin
   Gadd, extended by Stephen Belanger and Zach Musgrave (has dependency
   on XNA but supposedly can be turned into a standalone parser easily)
-  `TiledSharp <https://github.com/marshallward/TiledSharp>`__: Yet
   another C# TMX importer library, with Tiled 0.11 support. TiledSharp
   is a generic parser which can be used in any framework, but it cannot
   be used to render the maps. Available via NuGet.
-  `NTiled <https://github.com/patriksvensson/ntiled>`__: Generic parser
   for 0.9.1 tiled maps. Available via NuGet.
-  `TmxCSharp <https://github.com/gwicksted/TmxCSharp>`__: Useful for
   multi-layer orthographic tile engines. No framework dependencies,
   used with a custom OpenTK tile engine soon to be open source, tested
   with Tiled 0.8.1 (multiple output formats). MIT license.
-  `tmx-mapper-pcl <https://github.com/aalmik/tmx-mapper-pcl>`__: PCL
   library for parsing Tiled map TMX files. This library could be used
   with MonoGame and Windows Runtime Universal apps.

D
~

-  `tiledMap.d <https://gist.github.com/gdm85/9896961>`__ simple
   single-layer and single-tileset example to load a map and its tileset
   in `D language <http://dlang.org/>`__. It also contains basic
   rendering logic using `DSFML <https://github.com/Jebbs/DSFML/>`__
-  `dtiled <https://github.com/rcorre/dtiled>`__ can load JSON-formatted Tiled
   maps. It also provides general tilemap-related functions and algorithms.

Go
~~

-  `github.com/salviati/go-tmx/tmx <https://github.com/salviati/go-tmx>`__

Haskell
~~~~~~~

-  `htiled <http://hackage.haskell.org/package/htiled>`__ (TMX) by `Christian
   Rødli Amble <https://github.com/chrra>`__.
-  `aeson-tiled <https://hackage.haskell.org/package/aeson-tiled>`__ (JSON) by `Schell Scivally <https://github.com/schell>`__.

Java
~~~~

-  A library for loading TMX files is included with Tiled at
   `util/java/libtiled-java <https://github.com/bjorn/tiled/tree/master/util/java/libtiled-java>`__.
-  Android-Specific:

   -  `AndroidTMXLoader <https://github.com/davidmi/Android-TMX-Loader>`__
      loads TMX data into an object and renders to an Android Bitmap
      (limited functionality)
   -  `libtiled-java
      port <http://chiselapp.com/user/devnewton/repository/libtiled-android/index>`__
      is a port of the libtiled-java to be used on Android phones.

PHP
~~~

-  `PHP TMX Viewer <https://github.com/sebbu2/php-tmx-viewer>`__ by
   sebbu : render the map as an image (allow some modifications as well)

Pike
~~~~

-  `TMX parser <https://gitlab.com/tmx-parser/tmx-parser>`__: a simple
   loader for TMX maps (CSV format only).

Processing
~~~~~~~~~~

-  `linux-man/ptmx <https://github.com/linux-man/ptmx>`__: Add Tiled
   maps to your Processing sketch.

Python
~~~~~~

-  `pytmxlib <http://pytmxlib.readthedocs.org/en/latest/>`__: library
   for programmatic manipulation of TMX maps
-  `python-tmx <http://python-tmx.nongnu.org>`__: a simple library for
   reading and writing TMX files.

Ruby
~~~~

-  `tmx gem <https://github.com/shawn42/tmx>`__ by erisdiscord

Vala
~~~~

-  `librpg <https://github.com/JumpLink/librpg>`__ A library to load and
   handle spritesets (own format) and orthogonal TMX maps.

Support by Framework
--------------------

Following entries are integrated solutions for specific game engines.
They are typically of little to no use if you're not using said game
engine.

AndEngine
~~~~~~~~~

-  `AndEngine <http://www.andengine.org/>`__ by Nicolas Gramlich
   supports `rendering TMX
   maps <http://www.andengine.org/blog/2010/07/andengine-tiledmaps-in-the-tmx-format/>`__

Allegro
~~~~~~~

-  `allegro\_tiled <https://github.com/dradtke/allegro_tiled>`__
   integrates Tiled support with `Allegro
   5 <http://alleg.sourceforge.net/>`__.

cocos2d
~~~~~~~

-  `cocos2d (Python) <http://python.cocos2d.org/>`__ supports loading
   `Tiled
   maps <http://python.cocos2d.org/doc/programming_guide/tiled_map.html>`__
   through its ``cocos.tiles`` module.
-  `cocos2d-x (C++) <http://www.cocos2d-x.org/>`__ supports loading TMX
   maps through the
   `CCTMXTiledMap <http://www.cocos2d-x.org/reference/native-cpp/V2.1.4/da/d68/classcocos2d_1_1_c_c_t_m_x_tiled_map.html>`__
   class.
-  `cocos2d-objc (Objective-C, Swift) <http://www.cocos2d-objc.org/>`__
   (previously known as: cocos2d-iphone, cocos2d-swift,
   cocos2d-spritebuilder) supports loading TMX maps through
   `CCTiledMap <http://cocos2d.spritebuilder.com/docs/api/Classes/CCTiledMap.html>`__
-  `TilemapKit <http://tilemapkit.com>`__ is a tilemapping framework for
   Cocos2D. It supports all TMX tilemap types, including staggered iso
   and all hex variations. No longer in development.

Construct 2 - Scirra
~~~~~~~~~~~~~~~~~~~~

-  `Construct 2 <http://www.scirra.com>`__, since the Beta Release 149,
   officially supports TMX maps, and importing it by simple dragging the
   file inside the editor. `Official
   Note <https://www.scirra.com/construct2/releases/r149>`__

Corona SDK
~~~~~~~~~~

-  `ponytiled <https://github.com/ponywolf/ponytiled>`__ is a simple
   Tiled Map Loader for Corona SDK (`forum
   announcement <http://discourse.mapeditor.org/t/new-lua-coronasdk-framework-ponytiled/1826>`__)
-  `Dusk Engine <https://github.com/GymbylCoding/Dusk-Engine>`__ is a
   fully featured Tiled map game engine for Corona SDK

Flixel
~~~~~~

-  Lithander demonstrated his `Flash TMX parser combined with Flixel
   rendering <http://blog.pixelpracht.net/?p=59>`__

Game Maker
~~~~~~~~~~

-  Tiled ships with a plug-in that can :ref:`export a map to a GameMaker: Studio 1.4 room file <gamemaker-export>`
-  `Tiled2GM Converter <http://gmc.yoyogames.com/index.php?showtopic=539494>`__ by Dmi7ry

Godot
~~~~~

-  `Tiled Map
   Importer <https://godotengine.org/asset-library/asset/25>`__ imports
   each map as Godot scene which can be instanced or inherited (`forum
   announcement <http://discourse.mapeditor.org/t/importer-plugin-for-godot-engine/1833/1>`__)

Haxe
~~~~

-  `HaxePunk <https://github.com/HaxePunk/tiled>`__ Tiled Loader for
   HaxePunk
-  `HaxeFlixel <https://github.com/HaxeFlixel/flixel-addons/tree/dev/flixel/addons/editors/tiled>`__
-  `OpenFL <https://github.com/Kasoki/openfl-tiled>`__ "openfl-tiled" is
   a library, which gives OpenFL developers the ability to use the Tiled
   Map Editor.
-  `OpenFL + Tiled +
   Flixel <https://github.com/kasoki/openfl-tiled-flixel>`__
   Experimental glue to use "openfl-tiled" with HaxeFlixel

HTML5 (multiple engines)
~~~~~~~~~~~~~~~~~~~~~~~~

-  `Canvas Engine <https://github.com/RSamaium/CanvasEngine>`__ A framework to create
   video games in HTML5 Canvas
-  `chem-tmx <https://github.com/andrewrk/chem-tmx>`__ Plugin for
   `chem <https://github.com/andrewrk/chem/>`__ game engine.
-  `chesterGL <https://github.com/funkaster/ChesterGL>`__ A simple
   WebGL/canvas game library
-  `Crafty <http://craftyjs.com>`__ JavaScript HTML5 Game Engine;
   supports loading Tiled maps through an external component
   `TiledMapBuilder <https://github.com/Kibo/TiledMapBuilder>`__.
-  `GameJs <http://gamejs.org>`__ JavaScript library for game
   programming; a thin wrapper to draw on HTML5 canvas and other useful
   modules for game development
-  `KineticJs-Ext <https://github.com/Wappworks/kineticjs-ext>`__ A
   multi-canvas based game rendering library
-  `melonJS <http://www.melonjs.org>`__ A lightweight HTML5 game engine
-  `Panda 2 <https://www.panda2.io/>`__, a HTML5 Game Development Platform for Mac, Windows and Linux. Has `a plugin for rendering Tiled <https://www.panda2.io/plugins>`__ maps, both orthogonal and isometric.
-  `Phaser <http://www.phaser.io>`__ A fast, free and fun open source
   framework supporting both JavaScript and TypeScript (`Tiled
   tutorial <http://www.gamedevacademy.org/html5-phaser-tutorial-top-down-games-with-tiled/>`__)
-  `linux-man/p5.tiledmap <https://github.com/linux-man/p5.tiledmap>`__
   adds Tiled maps to `p5.js <http://p5js.org/>`__.
-  `Platypus Engine <https://github.com/PBS-KIDS/Platypus/>`__ A robust
   orthogonal tile game engine with game entity library.
-  `sprite.js <https://github.com/batiste/sprite.js>`__ A game framework
   for image sprites.
-  `TMXjs <https://github.com/cdmckay/tmxjs>`__ A JavaScript, jQuery and
   RequireJS-based TMX (Tile Map XML) parser and renderer.

indielib-crossplatform
~~~~~~~~~~~~~~~~~~~~~~

-  `indielib cross-platform <http://www.indielib.com>`__ supports
   loading TMX maps through the `C++/TinyXML based
   tmx-parser <http://code.google.com/p/tmx-parser/>`__ by KonoM (BSD)

LibGDX
~~~~~~

-  `libgdx <http://libgdx.badlogicgames.com/>`__, a Java-based
   Android/desktop/HTML5 game library,
   `provides <https://github.com/libgdx/libgdx/wiki/Tile-maps>`__ a
   packer, loader and renderer for TMX maps

LITIengine
~~~~~~~~~~

-  `LITIengine <https://litiengine.com>`__ is a 2D Java Game Engine that
   supports loading, saving and rendering maps in the .tmx format.

LÖVE
~~~~

-  `Simple Tiled
   Implementation <https://github.com/Karai17/Simple-Tiled-Implementation>`__
   Lua loader for the LÖVE (Love2d) game framework.

MOAI SDK
~~~~~~~~

-  `Hanappe <https://github.com/makotok/Hanappe>`__ Framework for MOAI
   SDK.
-  `Rapanui <https://github.com/ymobe/rapanui>`__ Framework for MOAI
   SDK.

Monkey X
~~~~~~~~

-  `bit.tiled <https://github.com/bitJericho/bit.tiled>`__ Loads TMX
   file as objects. Aims to be fully compatible with native TMX files.
-  `Diddy <https://code.google.com/p/diddy/>`__ is an extensive
   framework for Monkey X that contains a module for loading and
   rendering TMX files. Supports orthogonal and isometric maps as both
   CSV and Base64 (uncompressed).

Node.js
~~~~~~~

-  `node-tmx-parser <https://github.com/andrewrk/node-tmx-parser>`__ -
   loads the TMX file into a JavaScript object

Oak Nut Engine (onut)
~~~~~~~~~~~~~~~~~~~~~

-  `Oak Nut Engine <http://daivuk.github.io/onut/>`__ supports Tiled maps
   through Javascript and C++. (see TiledMap `Javascript <https://github.com/Daivuk/onut/tree/master/samplesJS/TiledMap>`__ or `C++ <https://github.com/Daivuk/onut/tree/master/samples/TiledMap>`__ samples)

Orx Portable Game Engine
~~~~~~~~~~~~~~~~~~~~~~~~

-  `TMX to ORX
   Converter <http://orx-project.org/wiki/tutorials/community/sausage/tmx_to_orx>`__
   Tutorial and converter download for Orx.

Pygame
~~~~~~

-  `Pygame map loader <http://www.pygame.org/project/1158/>`__ by dr0id
-  `PyTMX <https://github.com/bitcraft/PyTMX>`__ by Leif Theden
   (bitcraft)
-  `tmx.py <https://bitbucket.org/r1chardj0n3s/pygame-tutorial/src/a383dd24790d/tmx.py>`__
   by Richard Jones, from his `2012 PyCon 'Introduction to Game
   Development'
   talk <http://pyvideo.org/video/615/introduction-to-game-development>`__.
-  `TMX <https://github.com/renfredxh/tmx>`__, a fork of tmx.py and a
   port to Python3. A demo called pylletTown can be found
   `here <https://github.com/renfredxh/pylletTown>`__.

Pyglet
~~~~~~

-  `JSON map loader/renderer for
   pyglet <https://github.com/reidrac/pyglet-tiled-json-map>`__ by Juan
   J. Martínez (reidrac)
-  `PyTMX <https://github.com/bitcraft/PyTMX>`__ by Leif Theden
   (bitcraft)

PySDL2
~~~~~~

-  `PyTMX <https://github.com/bitcraft/PyTMX>`__ by Leif Theden
   (bitcraft)

RPG Maker MV
~~~~~~~~~~~~

-  `Tiled
   Plugin <https://forums.rpgmakerweb.com/index.php?threads/tiled-plugin-version-1-3-0-released.50752/>`__
   by `Dr.Yami <http://yami.moe/>`__ & Archeia, from `RPG Maker
   Web <https://forums.rpgmakerweb.com>`__

SDL
~~~

-  `C++/TinyXML/SDL based
   loader <http://usefulgamedev.weebly.com/c-tiled-map-loader.html>`__
   example by Rohin Knight (limited functionality)

SFML
~~~~

-  `STP <https://github.com/edoren/STP>`__ (SFML TMX Parser) by edoren
-  `C++/SFML Tiled map
   loader <http://trederia.blogspot.co.uk/2013/05/tiled-map-loader-for-sfml.html>`__
   by fallahn. (Zlib/libpng)
-  `C++/SfTileEngine <https://github.com/Tresky/sf_tile_engine>`__ by
   Tresky (currently limited functionality)

Slick2D
~~~~~~~

-  `Slick2D <http://slick.ninjacave.com>`__ supports loading TMX maps
   through
   `TiledMap <http://slick.ninjacave.com/javadoc/org/newdawn/slick/tiled/TiledMap.html>`__.

Sprite Kit Framework
~~~~~~~~~~~~~~~~~~~~

-  `SKTilemap <https://github.com/TomLinthwaite/SKTilemap>`__ is built
   from the ground up in Swift. It's up to date, full of features and
   easy to integrate into any Sprite Kit project. Supports iOS and OSX.
-  `SKTiled <https://github.com/mfessenden/SKTiled>`__ - A Swift
   framework for working with Tiled assets in SpriteKit.
-  `TilemapKit <http://tilemapkit.com>`__ is a tilemapping framework for
   Sprite Kit. It supports all TMX tilemap types, including staggered
   iso and all hex variations. No longer in development.
-  `JSTileMap <https://github.com/slycrel/JSTileMap>`__ is a lightweight
   SpriteKit implementation of the TMX format supporting iOS 7 and OS X
   10.9 and above.

TERRA Engine (Delphi/Pascal)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  `TERRA Engine <http://pascalgameengine.com/>`__ supports loading and
   rendering of TMX maps.

Unity 3D
~~~~~~~~

-  `Orthello
   Pro <http://www.wyrmtale.com/products/unity3d-components/orthello-pro>`__
   (2D framework) offers `Tiled map
   support <http://www.wyrmtale.com/orthello-pro/tilemaps>`__.
-  `Tiled To
   Unity <https://www.assetstore.unity3d.com/#/content/17260/>`__ is a
   3D pipeline for Tiled maps. It uses prefabs as tiles, and can place
   decorations dynamically on tiles. Supports multiple layers (including
   object layers).
-  `Tiled2Unity <http://www.seanba.com/introtiled2unity.html>`__ exports
   TMX files to Unity with support for (non-simple) collisions.
-  `UniTMX <https://bitbucket.org/PolCPP/unitmx/overview>`__ imports TMX
   files into a mesh.
-  `X-UniTMX <https://bitbucket.org/Chaoseiro/x-unitmx>`__ supports
   almost all Tiled 0.11 features. Imports TMX/XML files into Sprite
   Objects or Meshes.
-  `Tiled TMX Importer <https://www.assetstore.unity3d.com/en/#!/content/102928>`__, imports into Unity 2017.2's new native Tilemap system.

Unreal Engine 4
~~~~~~~~~~~~~~~

-  `Paper2D <https://forums.unrealengine.com/showthread.php?3539-Project-Paper2D>`__
   provides built-in support for tile maps and tile sets, importing JSON
   exported from Tiled.

Urho3D
~~~~~~

-  `Urho3D <http://urho3d.github.io/>`__ natively supports loading Tiled
   maps as part of the
   `Urho2D <http://urho3d.github.io/documentation/1.4/_urho2_d.html>`__
   sublibrary
   (`Documentation <http://urho3d.github.io/documentation/1.4/class_urho3_d_1_1_tile_map2_d.html>`__,
   `HTML5
   example <http://urho3d.github.io/samples/36_Urho2DTileMap.html>`__).

XNA
~~~

-  `FlatRedBall Engine TMXGlue
   tool <http://www.flatredball.com/frb/docs/index.php?title=Kain%27s_Tavern#Tiled_Map_Editor.2C_TMX.2C_Glue_and_you.>`__
   by Domenic Datti loads TMX maps into the FlatRedBall engine, complete
   with node networks, pathfinding, and shapecollection support via
   object layers.
-  `TiledMax <http://tiledmax.xpod.be/>`__ by Aimee Bailey, a .NET
   library for parsing TMX maps without dependencies on Windows or XNA
-  `XTiled <https://bitbucket.org/vinull/xtiled>`__ by Michael C. Neel
   and Dylan Wolf, XNA library for loading and rendering TMX maps
-  `XNA map loader <https://github.com/zachmu/tiled-xna>`__ by Kevin
   Gadd, extended by Stephen Belanger and Zach Musgrave
