### Tiled 1.1.2 (...)

* Tile Collision Editor: Keep tile centered when resizing view
* Tile Collision Editor: Display tool info text in status bar

### Tiled 1.1.1 (4 January 2018)

* Fixed crash on load for template instances of non-tile objects
* Windows Installer: Include the Qt SVG image plugin

### Tiled 1.1.0 (3 January 2018)

* Added support for infinite maps (by Ketan Gupta, #260)
* Added support for Wang tiles and related tools (by Benjamin Trotter)
* Added support for reusable object templates (by Mohamed Thabet)
* Added working directory setting for custom commands (by Ketan Gupta, #1580)
* Added output of custom commands in Debug Console (by Ketan Gupta, #1552)
* Added autocrop action based on tile layers (by Ketan Gupta, #642)
* Added tool bar with tool-specific actions and settings (by Ketan Gupta, #1084)
* Added shape fill tool for filling rectangles or circles (by Benjamin Trotter, #1272)
* Added option to lock/unlock a layer (by Ketan Gupta, #734)
* Added .xml as possible file extension for TMX files
* Added keyboard shortcut for Save All (by Thomas ten Cate)
* Added actions to remove a segment from polygon or to split a polyline (by Ketan Gupta, #1685)
* Added icon for animation editor in the tileset editor (by Ketan Gupta, #1706)
* Added display of flip bits for hovered tile in status bar (#1707)
* Added ability to capture tiles while using fill tools (#790)
* Added option to have mouse wheel zoom by default (#1472)
* Added tab closing actions to context menu, and close by middle-click (by Justin Jacobs, #1720)
* Added ability to reorder terrain types (by Justin Jacobs, #1603)
* Added a point object for marking locations (by Antoine Gersant, #1325)
* Added 'New Tileset' button when no tileset is opened (by Rhenaud Dubois, #1789)
* Added 'Open File' button when no file opened (by Rhenaud Dubois, #1818)
* Added support for custom input formats and TMX output to the --export-map command-line option
* Added island RPG example based on Beach tileset by finalbossblues
* Added file-related context menu actions to tileset tabs
* Added action to reset to default window layout (by Keshav Sharma, #1794)
* Added support for exporting tilesets, including to Lua format (by Conrad Mercer, #1213)
* Keep object types sorted alphabetically (by Antoine Gersant, #1679)
* Improved polygon node handles and drag behavior
* Fixed %executablepath variable for executables found in PATH (#1648)
* Fixed Delete key to delete selected polygon nodes when appropriate (by Ketan Gupta, #1555)
* Fixed Terrain Brush going wild in some scenarios (#1632)
* Fixed the "Embed in Map" checkbox to be persistent (#1664)
* Fixed crash when saving two new maps using the same file name (#1734)
* Fixed issues caused by paths not being cleaned (#1713)
* Fixed suggested file name for tilesets to match the tileset name (by killerasus, #1783)
* Fixed selection rectangle's shadow offset when zooming (by Antoine Gersant, #1796)
* Fixed save dialog to reopen after heeding the file extension warning (by Antoine Gersant, #1782)
* Fixed potential crash when zooming out too much (#1824)
* Fixed potential crash after deleting object or group layers
* Fixed Object Selection tool clearing selection on double-click
* Enabled building with Qbs on macOS, including the Python plugin (by Jake Petroules)
* Automapping: Don't fail if an input/inputnot layer isn't found
* Automapping: Added a "StrictEmpty" flag to input layers
* GMX plugin: Added support for defining views with objects (by William Taylor, #1621)
* GMX plugin: Added support for setting scale and origin for instances (#1427)
* GMX plugin: Added support for setting the creation code for instances and the map
* GMX plugin: Start counting default tile layer depth from 1000000 (#1814)
* tBIN plugin: Added read/write support for the tBIN map format (by Chase Warrington, #1560)
* libtiled-java: Generate classes from XSD, some fixes and build with Maven (by Mike Thomas, #1637)
* libtiled-java: Added support for manipulating non-consecutive tile IDs in a tileset (by Stéphane Seng)
* Python plugin: Adjusted example scripts to API changes (by spiiin, #1769)
* Flare plugin: Various changes (by Justin Jacobs, #1781)
* TMW plugin: Removed since it is no longer needed
* Updated Dutch, Bulgarian, English, French, German, Korean, Norwegian Bokmål, Spanish and Turkish translations

### Tiled 1.0.3 (29 August 2017)

* Fixed crash on reload map (#1659, #1694)
* Fixed possible crash on undo/redo in collision editor (#1695)
* Fixed tile replacement to add tileset when needed (by Mohamed Thabet, #1641)
* Fixed the display of the image source property for tilesets
* Fixed shortcut for 'Copy tile coordinates' (Alt+C) in Portuguese translation (by olueiro)
* JSON plugin: Fixed reading of tileset column count
* JSON plugin: Fixed reading of custom properties on tile collision object group

### Tiled 1.0.2 (27 June 2017)

* Added read-only tile and terrain properties in map editor (#1615)
* Fixed Terrains view to display all tilesets with terrain
* Fixed hang when trying to fill with a pasted stamp (#1617, #1624)
* Fixed crash when editing collision when tile image wasn't loaded
* Fixed rendering of tile objects when the image couldn't be loaded
* Fixed rendering of tile object outlines for resized objects
* Fixed labels shown on objects hidden via a group layer
* Fixed updating of label positions when moving a group layer
* GMX plugin: Fixed tile type inheritance for tile objects
* Restored Ctrl+N shortcut on "New Map" action

### Tiled 1.0.1 (13 June 2017)

* Made the zoom level used in Tilesets view persistent
* Fixed mixed up polygon and polyline icons (by Ketan Gupta, #1588)
* Fixed reset of font size when using font dialog (#1596)
* Fixed several issues with the Properties dock (#1583, #1611)
* Fixed centering on object on layer with offset (#1600)
* Fixed handling of symbolic links in Recent Files menu and Maps view (#1589)
* Fixed labels for objects in grouped object layers
* Reverted the file format version back to "1.0" and added "tiledversion" attribute
* Lua plugin: Fixed group layers being exported with "imagelayer" type (#1595)
* Added Korean translation (by miru2533 and SshipSunBee, #1604)
* Updated Russian and Chinese translations

### Tiled 1.0.0 (25 May 2017)

* Added support for editing external tilesets (#242)
* Added a text object with configurable font and wrapping (#1429)
* Added layer grouping (#1038)
* Added Tile.type and inherit tile object properties from the tile (#436, #1248)
* Added a start page
* Added selection of underlying objects with Alt modifier (by Yuriy, #1491)
* Added an option to disable safe writing of files (#1402, #1404)
* Added invert selection action (by Leon Moctezuma, #1423)
* Added support for isometric terrain overlays and tile collision objects (#419, #757)
* Added 180-degree mirroring mode to terrain brush with Alt modifier
* Added short and consistent map format names to use with --export-map (by Marce Coll, #1382)
* Added Swap Tiles action (by Alexander Münch, #866)
* Added tileset background color property (#227)
* Added 60 degree tile rotation support for hexagonal maps (by Victor Nicolaichuk, #1447)
* Added a check for duplicates when adding tiles (by Simião, #1227)
* Added option to run commands from menu as well as edit them (by Ketan Gupta, #943)
* Added custom shortcuts for commands (by Ketan Gupta, #1456)
* Added optional ID and Position columns to objects view (by i-ka, #1462)
* Added an executable picker for custom commands (by Ketan Gupta, #942)
* Added marching ants effect on selected objects (by Mohamed Thabet, #1489)
* Added all open tilesets to the Tilesets view
* Added auto-show/hide all views (Clear View) action (by erem2k, #563)
* Added minimap in the resizing dialog (by Yuriy, #1516)
* Added drag-n-drop support in Layers view (#178)
* Added support for storing object type definitions in JSON format (#1313)
* Added cut/copy/paste actions for custom properties (#515)
* Allow changing the tile of tile objects (by Mohamed Thabet, #409)
* Allow selecting a folder to fix multiple broken links at once
* Added support for dragging external tilesets into the Tilesets dock
* Added support for dragging images into image collection tilesets
* Write out Tiled version in TMX/JSON "version" attribute (#1416)
* Remember last view on map also for closed files (#905)
* Remember tileset zoom level in the tileset editor (by Artem Sharganov, #408)
* Change current layer depending on selected objects (by Glavak, #1424)
* Improved support for using Tiled on HiDpi screens
* Improved the behavior of the tile selection tool
* Made Ctrl+D duplicate objects instead of deleting them
* Use an eye icon instead of a checkbox for layer visibility (by Ketan Gupta, #1127)
* JSON tileset: Save width/height of individual tile images
* Linux: Added MIME type for tileset files
* Fixed hexagonal rotation of tile stamps (by Bdtrotte, #1476)
* Fixed handling of broken tile references, which now render as a red marker
* Fixed manual reloading of images for image collection tilesets
* Fixed Offset Layers tool to wait until mouse is moved
* Fixed current stamp to always update when a tile is clicked
* Fixed handling of pinch gestures (#1305)
* Fixed flipping a group of objects to work like expected (by Vitek1425, #1475)
* Fixed stamp brush to work better on staggered maps (by Bdtrotte)
* Fixed objects offsetting while resizing (by Acuion, #1518)
* Fixed fill tool for hexagonal maps (#883)
* Fixed potential crash in Terrain Brush
* Windows: Fixed menus when using OpenGL in full screen mode (#1576)
* Windows: Added Sticker Knight and Python example scripts to installer (#819)
* Windows: Fixed bringing existing Tiled window to foreground (#1256)
* AutoMapping: Fixed object groups always getting added
* AutoMapping: Improved map boundary handling (by Stefan Beller, #1224)
* AutoMapping: Apply custom properties set on output layers
* terraingenerator: Made the amount of columns configurable
* terraingenerator: Copy tile properties from the source tilesets
* Added Ukrainian translation (by Olexandr Nesterenko)
* Added Hungarian translation (by Balázs Úr)
* Added Finnish translation (by ekeimaja)
* Updated Bulgarian, Dutch, French, German, Russian, Spanish and Turkish translations

### Tiled 0.18.2 (21 February 2017)

* Fixed crash when deleting multiple selected objects
* Fixed crash when moving multiple selected objects to another object layer
* Fixed updating of values displayed in Objects and Layers views
* GMX plugin: Added support for image collection tilesets
* Object Types Editor: Improved behavior when adding new types
* Linux: Fixed shipping of image format plugins in AppImage releases

### Tiled 0.18.1 (23 January 2017)

* Fixed terrain brush for isometric staggered maps (by Clyde)
* Fixed crash when resizing map causes objects to get removed
* Fixed crash when duplicating an object layer
* Fixed position of image layer after Resize or Offset Map
* Fixed the quality of the minimap on HiDpi displays
* Fixed Alt-drag behavior to not override resize handles
* When adding a new layer, insert it above the current one
* GMX plugin: Fixed positioning for non-tile objects and support scaling
* GMX plugin: Export tile objects without a type as tiles
* GMX plugin: Support horizontal and vertical flipping
* Windows: Fixed encoding problems with command-line output
* Windows: Fixed the architecture of shipped MSVC DLLs
* Updated Chinese translation (by Clyde)

### Tiled 0.18.0 (20 December 2016)

* Added Layer via Copy/Cut actions
* Added support for Paste in Place action for tile layers
* Added context menu to change custom property type (by Dmitry Hrabrov)
* Added support for higher precision for custom floating point properties
* Added %mappath variable to commands (by Jack Roper)
* Added snapping to pixels (by Mamed Ibrahimov)
* Added right-click to clear the tile selection
* Added a context menu action to reset the size of tile objects
* Added exporter for Game Maker Studio room files (by Jones Blunt)
* Added Move Up/Down buttons to Objects view (by iskolbin)
* Added pixel coordinates to status bar for object tools (by iskolbin)
* Added Sticker Knight platformer example (by Ponywolf)
* tmxrasterizer: Added --size argument and support local file URLs
* tmxrasterizer: Use smooth pixmap transform by default
* Linux: Register tmxrasterizer as thumbnail generator for TMX files
* Allow scrolling past map edges with mouse wheel
* Enabled HiDpi scaling and improved the quality of some icons
* Reversed the order of the objects in the Objects view
* JSON plugin: Added Node.js support to the JavaScript export
* Updated TMX schema definition (by assofohdz)
* Fixed unfinished objects getting saved
* Fixed OpenGL rendering mode when application is scaled (HiDpi screens)
* Fixed Remove and Rename actions for predefined properties
* Windows: Fixed console output
* libtiled-java: Use Maven, deploy to OSSRH and code updates (by Mike Thomas)
* libtiled-java: Added a basic isometric renderer (by Mike Thomas)
* Updated Brazilian Portuguese, Chinese, Czech, Dutch, Hebrew, Norwegian Bokmål and Spanish translations

### Tiled 0.17.2 (28 November 2016)

* Fixed bug with editing type and name for multiple objects
* Fixed ability to change the image of a tile in an image collection tileset
* Fixed wrong layer name getting edited when switching maps
* Fixed possible crash when missing tileset images and using tile animations
* Compiled against Qt 5.6.2 on macOS to avoid crashes with Qt 5.7

### Tiled 0.17.1 (4 November 2016)

* Fixed wrong alpha value when opening the color picker dialog
* Fixed saving of object group color alpha value
* Fixed tile id adjustment for newly added tilesets
* Fixed "Object Properties" entry in the context menu to be always enabled (by Erik Schilling)
* Fixed out-of-sync tile selection during layer offset change (by nykm)
* Fixed hidden objects becoming visible when offsetting the map (by ranjak)
* Fixed problems with using predefined file properties
* Lua plugin: Fixed type of animation frame properties
* OS X: Use standard shortcut for toggling full screen
* OS X: Fixed compile when pkg-config is present
* Windows: Include the Defold plugin
* Windows: Added support for DDS, TGA, WBMP and WEBP image formats
* Linux: Added 64-bit AppImage (with help from Simon Peter)
* Chinese translation updates (by endlesstravel and buckle2000)
* French translation updated (by Yohann Ferreira)

### Tiled 0.17.0 (15 August 2016)

* Added a platform-independent theme, which can be dark (#786)
* Added Paste in Place action for objects (#1257)
* Added custom property type 'color' (#1275)
* Added custom property type 'file' (#1278)
* Added option for removing invisible objects in resize dialog (#1032, by Mamed Ibrahimov)
* Added support for editing multi-line string properties (#205)
* Added %layername and %objectid to available command variables
* Added support for scrolling in tileset view with middle mouse button (#1050, with Will Luongo)
* Added a rectangle erase mode to the eraser (#1297)
* Added export to Defold .tilemap files (by Nikita Razdobreev)
* Added simple full screen mode
* Added "Copy File Path" and "Open Containing Folder" actions to tab context menu
* Added warning when saving with the wrong file extension
* Added color picker for setting transparent color of a tileset (#1173, by Ava Brumfield)
* Various object selection tool improvements
* Allow creating rectangle/ellipse objects in any direction (#1300)
* Enabled nested views and grouped dragging for stacked views (#1291)
* Fixed updating object drag cursor when exiting resize handles (#1277)
* Fixed tile animations to stay in sync when changing them (#1288)
* Fixed preservation of tile meta-data when tileset width is changed (#1315)
* Updated Bulgarian, Dutch, German, Norwegian Bokmål, Russian, Spanish and Turkish translations

### Tiled 0.16.2 (7 July 2016)

* JSON plugin: Fixed loading of custom properties on terrains
* Lua plugin: Fixed missing export of object layer drawing order
* Fixed tile index adjustment when tileset image changes width
* Fixed --export-map [format] option
* Fixed shortcuts for some tools when language is set to Dutch
* Fixed a painting related bug affecting the top edge after AutoMapping
* Fixed issues when compiling against Qt 5.6 on OS X and Windows
* Fixed crash on maximizing with Maps view open on Windows (Qt 5.6.1)
* Fixed focus issue while typing predefined object types (Qt 5.6)
* Fixed silent fail when saving to restricted location on Windows (Qt 5.6)

### Tiled 0.16.1 (6 May 2016)

* Fixed auto-updater not enabled for Windows release
* Fixed saving of object IDs assigned to tile collision shapes
* Fixed crash when pressing Backspace with Custom Properties section selected
* Fixed crash on exit when leaving the Tile Collision Editor open
* Added Norwegian Bokmål translation (by Peter André Johansen)
* Updated Turkish translation

### Tiled 0.16.0 (28 March 2016)

* Added checking for updates, based on Sparkle and WinSparkle
* Added default property definitions to object types (with Michael Bickel)
* Added types to custom properties: string, float, int, boolean (with CaptainFrog)
* Added Properties view to the Tile Collision Editor (by Seanba)
* Added a reset button for color properties
* Added eraser mode to Terrain Brush and fixed some small issues
* Reuse existing Tiled instance when opening maps from the file manager (with Will Luongo)
* Allow setting tile probability for multiple tiles (by Henrik Heino)
* New MSI based installer for Windows
* Optimized selection of many objects
* libtiled-java: Fixed loading of maps with CSV layer data that are not square (by Zachary Jia)
* Fixed potential crash when having Terrain Brush selected and switching maps
* Updated Dutch, French, German, Japanese, Russian and Spanish translations

### Tiled 0.15.2 (6 March 2016)

* Added Turkish translation (by Nuri Uzunoğlu)
* Fixed hiding of object labels when deleting an object layer
* Fixed updating of object label colors when changing object types
* TMX: Added image size attributes to image layer images
* Updated Brazilian Portuguese translation

### Tiled 0.15.1 (30 January 2016)

* Fixed adding/removing object name labels when set to always visible
* Fixed a problem with 'Execute in Terminal' on OS X
* Fixed mouse coordinate conversion for hexagonal renderer
* Fixed image layer offset handling
* Update Czech translation

### Tiled 0.15.0 (4 January 2016)

* Allow loading maps with broken external references
* Allow plugins to be enabled/disabled
* Allow changing tileset image parameters
* Allow changing the images of tiles in a collection tileset
* Allow changing external tileset references
* Allow panning over the edges of the map
* Added Terrain Generator tool
* Added column count property to image collection tilesets
* Added a combo box for changing the current layer to the status bar
* Moved the AutoMapping while drawing toggle into the menu
* Removing tiles from collection tilesets no longer changes tile IDs
* Unified layer offset handling
* Default tile layer data format changed to CSV
* Deprecated pure XML and Gzip-compressed tile layer data formats
* Fixed random tile picker for tiles with zero probability (by Henrik Heino)
* Fixed saving of alpha value of the map background color
* Fixed crash in tmxrasterizer and tmxviewer
* Fixed tmxrasterizer not reporting write errors
* Fixed isometric rendering bug with odd tile heights (by Ryan Schmitt)
* Updated Bulgarian, Dutch, French, German, Japanese, Russian and Spanish translations

### Tiled 0.14.2 (12 October 2015)

* Added Polish translation (by Tomasz Kubiak)
* Fixed layer offsets missing in the Lua export
* Fixed JSON tileset format missing in 'Add External Tileset' action
* Fixed language selection entries for Portuguese
* Fixed an issue with copy/pasting when using image collection tilesets
* Updated Brazilian Portuguese translation

### Tiled 0.14.1 (28 September 2015)

* Added missing 'renderorder' property to the Lua export
* Fixed editing of properties of tiles captured from the map

### Tiled 0.14.0 (21 September 2015)

* Added support for custom external tileset formats (JSON format added)
* Added support for shifting layers by some distance in pixels
* Added back object name labels in a much improved form
* Added tile stamp variation support to the fill tool
* Synchronize tileset selection when capturing tiles from the map
* Change tile in collision and animation editors based on selected tile object
* Keep the active brush when switching maps
* Python plugins can now add export-only map formats
* Fixed updating of current tile when changing map
* Fixed animated tile overlay to look less odd in some cases
* Fixed Save As dialog popping up when saving fails
* Fixed tilesets view collapsing when switching maps on OS X
* Updated Russian, Spanish, Czech, French, Japanese, German, Dutch and Bulgarian translations

### Tiled 0.13.1 (6 September 2015)

* Added Bulgarian translation (by Lyubomir Vasilev)
* Updated Spanish, French and Dutch translations

### Tiled 0.13.0 (10 August 2015)

* Added persistent Tile Stamps with support for variations (#969)
* Added Select Same Tile tool (by Mamed Ibrahimov)
* Added option to disable opening of last files on startup (by Mamed Ibrahimov)
* Added tilecount property to TMX, JSON and Lua map formats (#806)
* Added tileset properties to Properties view, as read-only (by Mamed Ibrahimov)
* Added Save All action (by Mamed Ibrahimov)
* Added translation of command line messages (by Mamed Ibrahimov)
* Added menu item linking to online documentation
* Object selection outlines are now drawn on top of everything
* Select new objects after they have been created
* Made the starting point for polylines and polygons visible
* Use the tile probability property also in random mode
* Ungrouped position and size properties (#892)
* CSV plugin: Extended to export all tile layers (by Alejandro Cámara)
* Lua and JSON plugins: Added support for layer data compression
* Fixed crash when changing flipping flag for multiple objects (by Mamed Ibrahimov)
* Fixed Ctrl+T causing a crash when no maps are open
* Fixed availability of 'Execute in Terminal' command on Linux with Qt 5
* Fixed drag object mouse cursor to appear only when it should
* Fixed selected file format when doing Save As with a non-TMX map
* Fixed problems with infinate scaling factors when resizing objects
* Require at least Qt 5.1.0
* Require compiler support for C++11
* Updated Russian, German, Czech and Italian translations

### Tiled 0.12.3 (1 June 2015)

* Fixed updating of map view when rotating objects with Z key
* Fixed updating of map view when joining, splitting or deleting polygon nodes
* Fixed a crash when reading an invalid TMX file
* Fixed live automapping updates when moving the mouse fast
* Made Backspace work for deleting collision objects and animation frames

### Tiled 0.12.2 (22 May 2015)

* Fixed updating of map view when moving objects with arrow keys
* Fixed compatibility issue with tile objects affecting the JSON format

### Tiled 0.12.1 (19 May 2015)

* Fixed updating of map view when changing objects from properties view
* Fixed updating of Properties view while objects are moved/resized
* Fixed terrain information getting lost when reading JSON maps

### Tiled 0.12.0 (14 May 2015)

* Added support for resizing any object as well as multiselection (with mauve)
* Added Control modifier for preserving aspect ratio while resizing
* Added Shift modifier for resizing with origin in the middle
* Added Alt modifier for suppressing selection changes when starting to drag
* Added a Magic Wand selection tool (by Henry Jia)
* Added tile probability attribute to tile properties view
* Added a Donate button to the About dialog
* Added a Patreon dialog to the Help menu
* Added an --export-formats command line option
* Remember the directory used for external tilesets (by Henry Jia)
* Don't set a window icon on Mac OS X
* Changed the way tile probability is applied (now it's relative)
* Fixed a crash in the terrain brush
* Fixed object selection behavior when Shift is held while clicking on nothing
* Fixed grid snapping being applied for staggered maps even when not enabled
* Fixed infinite memory allocation loop on invalid tile size in TMX file
* Fixed file icon associated with TMX files on Windows
* Fixed automapping of tile objects (by Seanba)
* Fixed 'Export as Image' to handle out of memory errors
* Fixed TMX files to be written in native line endings
* Fixed .desktop file missing %f argument for passing files (by Ying-Chun Liu)
* Fixed cursor position resetting when editing object type
* Added Arabic (Algeria) translation (by Damene Abdelkader)
* Updated, Czech, Dutch, French, German, Italian, Japanese, Portuguese, Russian and Spanish translations

### Tiled 0.11.0 (11 January 2015)

* Added support for hexagonal maps (offset coordinates)
* Added 'Export' action to repeat the last export
* Added a shortcut for the Reload action (Ctrl+R)
* Added ability to rename custom properties (by arn00d)
* Added unique IDs to objects (by Mark van Rij)
* Added a CSV export plugin
* Added visual feedback when properties differ between multiple selected objects (by Parker Miller)
* Added command-line export (by Brandon Dillon)
* Allow dynamically changing the map orientation and grid size
* Suppress the standard main window context menu in the collision editor
* Lua plugin: Write out tile terrain information
* Lua plugin: Include Tiled version in exported file
* Flare plugin: Fixed ability to open maps with absolute paths
* Fixed grid rendering for staggered maps
* Fully support building and running Tiled with Qbs
* Updated Czech, Dutch, French, German, Italian, Japanese, Portuguese and Spanish translations

### Tiled 0.10.2 (23 October 2014)

* Fixed hit area for polygon nodes when editing polygons while zoomed in or out
* Fixed another possible crash in the orthogonal renderer
* Fixed Select All action to work for object layers
* Fixed map pixel size preview for staggered maps
* Fixed repainting issues when tiles extend beyond their layer boundaries
* Fixed repainting issues when using tiles smaller than the grid size
* Display errors non-modal when applying automatic automapping rules
* Flare plugin: Fixed coordinate format for import and export (by Justin Jacobs)
* Lua plugin: Write out Image layer position
* Small updates to the Italian translation (by Omnomnobot)

### Tiled 0.10.1 (21 September 2014)

* Fixed a crash that could happen when using the terrain tool
* Fixed missing background color information from Lua export
* Allow using up to 3 or 4 GB RAM on 32 or 64 bit Windows systems respectively

### Tiled 0.10.0 (14 September 2014)

* Added object rotation (sponsored by Ben Wales)
* Added support for explicit object ordering (sponsored by Ben Wales)
* Added new Properties window with a rewritten properties editor
* Added support for writing plugins in Python (by Samuli Tuomola)
* Added image collection tilesets (sponsored by Jamie Rocks)
* Added map file watching and automatic reloading (sponsored by FlatRedBall.com)
* Added support for moving objects with arrow keys (sponsored by Ben Wales)
* Added a 'snap to fine grid' option (by Xenodora)
* Added support for JavaScript (JSONP) load/save (by Dobes Vandermeer)
* Added more zoom levels (by Joel Leclerc)
* Added shortcuts for finishing and canceling object creation
* Added a tile collision editor for defining collision shapes on tiles
* Added a tile animation editor and play defined animations
* Allow changing properties of multiple objects/tiles simultanously (by Parker Miller)
* Added tile rendering-order map property (by Lennert Raesch)
* Added support for changing the object line width
* Added support for CSV-encoded layers to libtiled-java (by Alexei Bratuhin)
* Added support for ellipse and polygon objects to libtiled-java (by Hendrik Brummermann)
* Added terrain properties to JSON export (by Dennis Hostetler)
* Added support for moving image layers in the Properties window (by Michael Aquilina)
* Added option to include background image when saving as image (by Sean Humeniuk)
* Added options to control layer visibility to tmxrasterizer (by Nathan Tolbert)
* Added display of tile ID in status bar (by Champi080)
* Added support for objects on staggered isometric maps (by Remco Kuijper)
* Added support for staggered maps to tmxviewer and tmxrasterizer
* Added a tool for moving the image of an image layer (by Mattia Basaglia)
* Added button to the tileset dock as shortcut to add a tileset (by Erik Schilling)
* Allow changing order of open document tabs (by Sean Humeniuk)
* Changed object position and size units from tiles to pixels (by mauve)
* Allow adding multiple tilesets at once (by mauve)
* Make highlighted grid cells outside map red (by Sean Humeniuk)
* Allow changing the drawing offset of a tileset
* Fixed hang on Mac OS X when drawing certain ellipse objects
* Fixed removal of polygon/polyline objects when resizing a map
* Fixed writing of tile offset in the Lua export
* Fixed updating of image layer when changing its image
* Fixed start drag distance check when editing polygons and moving objects
* Fixed console output of tmxrasterizer on Windows
* Raise the Layers dock for editing a new layer's name
* Avoid saving truncated files when compiled against Qt 5.1 or higher (by Erik Schilling)
* Made Tiled registering \*.tmx as MIME-type (by Erik Schilling)
* Added Traditional Chinese translation (by Yehnan Chiang)
* Updated Czech, Dutch, French, German, Russian and Spanish translations

### Tiled 0.9.1 (27 July 2013)

* Added saving of map background to JSON format (by Petr Viktorin)
* Added saving of terrain information to JSON format (by Petr Viktorin)
* Object Selection tool now always start selecting objects when holding Shift
* Increased maximum for tileset margin and spacing to 9999
* Some updates to libtiled-java (by Oskar Wiksten)
* Install the automappingconverter application (relevant on Linux)
* Avoid using Windows 95 style (was used on some Linux desktop environments)
* Removed layer name checks from the Flare export plugin (by Stefan Beller)
* Double-clicking an object now opens the Object Properties dialog
* Fixed Object Properties dialog not remembering its size
* Fixed object drawing order for image saving and mini-map
* Fixed some plurals in English translation
* Fixed line widths when zooming in Qt 5
* Fixed updating of image layer when its opacity or image is changed
* Fixed display of grid in tileset view on certain zoom levels
* Fixed save in wrong format after opening a map with plugin (by Mike Hendricks)
* Fixed closing Tiled being very slow with many maps
* Fixed saving of image layer properties in the Lua format
* Fixed escaping of special characters in the Lua format
* Fixed handling of relative paths for image layers in the JSON plugin

### Tiled 0.9.0 (27 January 2013)

* Added objects dock and per-object visibility toggle (by Tim Baker)
* Added maps dock (by Tim Baker)
* Added terrain tool for automatic terrain transitions (by Manu Evans)
* Added a minimap (by Christoph Schnackenberg)
* Added a staggered isometric map renderer, still without object layer support
* Added basic image layer support (by Gregory Nickonov and Alexander Kuhrt)
* Added display of current layer to the status bar (by Tim Baker)
* Added editable combo box for changing the zoom level (by Tim Baker)
* Added support for multiple input layers to automapping (by Stefan Beller)
* Added option to apply automapping rules while editing (by Stefan Beller)
* Added a converter to update old automapping rules (by Stefan Beller)
* Added support for objects layers to automapping (by Stefan Beller)
* Added support for random mode to the fill tool (by Stefan Beller)
* Added Replica Island plugin (by Eric Kidd)
* Added option to change the grid color (by Stefan Beller)
* Added support for ellipse objects (by devnewton and Christoph Schnackenberg)
* Added name labels for objects on isometric maps (by Andrew Motrenko)
* Added map property for changing the background color (by Emmanuel Barroga)
* Added shortcut to manually reload tilesets (Ctrl-T) (by Michael Williams)
* Added toggle for showing tile object outlines
* Added support for pinch zooming (by Pierre-David Bélanger)
* Added initial (non-GUI) support for individual and/or embedded tile images
  (by Petr Viktorin)
* Added reading support to Flare plugin (by Stefan Beller)
* Added a TMX rasterizer command line tool (by Vincent Petithory)
* Added man pages and desktop file (by Erik Schilling)
* Made the size and position of most dialogs persistent
* Respect the original layer data format of a loaded map (by Ben Longbons)
* Marked Tiled as high-resolution capable on Mac OS X
* Improved handling of external tilesets in Lua export
* Reverted tilesets view back to tabs, but with menu button (by Stefan Beller)
* Allowed plugins to support multiple file name filters (by Samuli Tuomola)
* Allow saving in any format that can also be read (by Stefan Beller)
* Fixed eraser skipping tiles when moving fast
* Fixed bug in Flare plugin (by Clint Bellanger)
* Fixed compile against Qt 5 (by Kenney Phillis)
* Fixed resolving of symbolic links while loading map
* Fixed a crash that could happen after trying to load a faulty map
* Updated Portuguese, Dutch, German, Spanish, Russian, French, Japanese,
  Chinese, Brazilian Portuguese, Hebrew and Czech translations

### Tiled 0.8.1 (7 May 2012)

* Added MacOS X Lion full screen support
* Fixed crash that could happen when painting with a pasted stamp
* Fixed zoom sensitivity for finer-resolution mouse wheels
* Fixed issues when using quickstamps in combination with the fill tool
* Fixed stamp tool not to miss tiles when drawing fast
* Fixed automapping to work with external tilesets
* Fixed crash in automapping when dealing with broken rule files
* Fixed object type getting erased on pressing Enter
* Changed the license of libtiled-java from LGPL to BSD
* Updated Italian and Hebrew translations

### Tiled 0.8.0 (11 December 2011)

* Added support for polygon and polyline objects
* Added support for tile rotation
* Added support for defining the color of custom object types
* Added a Delete action to delete selected tiles or objects
* Added random mode to the stamp brush
* Added Flare export plugin
* Added JSON plugin that supports both reading and writing
* Added ability to rename tilesets
* Added a mode in which the current layer is highlighted
* Added support for specifying a tile drawing offset
* Added a shortcut to copy the current tile position to clipboard (Alt+C)
* Added a command line option to disable OpenGL
* Allow custom properties on tilesets
* Many automapping improvements
* Improved tileset dock to handle a large amount of tilesets better
* Made the 'Show Grid' option in the tileset view persistent
* Raised the tile size limit in the New Tileset dialog from 999 to 9999
* Correctly handle changes in the width of a tileset image
* Worked around a long standing crash bug
* Added Russian translation
* Updated the German, Japanese, Spanish, Chinese, Czech, Dutch, French and
  Brazilian Portuguese translations

### Tiled 0.7.1 (27 September 2011)

* Select stamp tool when selecting tiles in tileset view
* Enable anti-aliasing for OpenGL mode
* Small improvement to the Lua export plugin (incompatible!)
* Fixed a bug in the Create Object tool
* Fixed reading of maps without tilesets but with a tile layer
* Fixed position of tile objects to center on the mouse on insertion
* Updated the Czech translation

### Tiled 0.7.0 (20 July 2011)

* Added support for horizontal and vertical flipping of tiles
* Added copy/paste support for objects
* Added merge layer down action
* Added Show or Hide all Other Layers action
* Added actions to select the previous/next layer
* Added Crop to Selection action
* Added a Lua export plugin
* Added Droidcraft plugin to read and export the map files
* Added option to turn off grid in the tileset view
* Added hand scrolling while holding the spacebar
* Made the object context menu available in all object tools
* Display tile coordinates also when using object tools
* Various improvements to running external commands
* Automapping stability and memory consumption improvements
* Objects that fall outside of the map on resize are now removed
* Fixed problems with watching tilesets multiple times
* Fixed several issues related to restoring previously opened files
* Updated Brazilian Portuguese, Chinese, German, Spanish, Japanese, Hebrew,
  Portuguese, Dutch and French translations

### Tiled 0.6.2 (2 May 2011)

* Fixed object layers losing their color when resizing the map
* Fixed the tabs in the Tilesets dock to use scroll buttons on MacOS X
* Fixed window title to update when saving a map with a different name

### Tiled 0.6.1 (3 April 2011)

* Added ability to open multiple files at once
* Added Ctrl+PageUp/PageDown shortcuts to switch documents
* Added an example to show how automatic mapping works
* Fixed bugs, crashes and leaks in the automatic mapping feature
* Fixed starting point for circles to be the click position
* Fixed a memory leak when using lines or circles
* Fixed layer opacity to be taken into account when saving as image
* Fixed endless loop when tile size is set to 0
* Fixed crash when passing an empty string as command line parameter
* Fixed problems with the tileset view after switching documents
* Fixed tile objects to be removed when their tileset is removed

### Tiled 0.6.0 (26 January 2011)

* Added support for opening multiple maps in one session
* Added support for placing tiles as objects
* Added automatic mapping feature, allowing placing of tiles based on rules
* Added ability to save/restore up to 9 stamps with Ctrl+<number>
* Added an object selection tool, allowing moving/deleting multiple objects
* Added ability to run external commands
* Added support for drawing lines and ellipses with the stamp brush
* Added icons to distinguish tile layers from object layers
* Added "Move To Layer" submenu to the context menu of objects
* Added option to use hardware rendering based on OpenGL
* Added a T-Engine4 map export plugin
* Added a simple TMX viewer application (BSD licensed)
* Added a New Layer dropdown menu to the layers dock
* Added a checkbox that enables snap to grid permanently
* Added an initial version of libtiled-java (LGPL licensed)
* Added Chinese and Hebrew translations
* Allowed dragging an image onto Tiled to add a tileset
* Center the map when it is smaller than the map view
* Remember the selected layer across restarts
* Changed the default layer data format to use zlib rather than gzip
* Store the tileset image width and height in the map file
* Compile fixes related to linking zlib
* Fixed the current stamp to get updated when switching tilesets
* Fixed the maximum sizes of the resize map dialog
* Fixed build issues when an older version of libtiled is installed
* Fixed saving of property when clicking OK while editing on MacOS X
* Allow Backspace to delete properties to make it easier on a MacBook
* Associate tmx files with Tiled on MacOS X
* Changed the license of libtiled from GPL to BSD
* Updated Czech, Spanish, German, Brazilian Portuguese, Dutch and French
  translations

### Tiled 0.5.1 (2 September 2010)

* Fixed saving of objects when tile width is different from tile height
* Updated Czech translation

### Tiled 0.5.0 (30 June 2010)

* Added support for import and export plugins
* Added support for external tilesets
* Added undo for adding tilesets and ability to remove tilesets
* Added error handling to the New Tileset dialog
* Added ability to change tileset order by dragging them around
* Added option to draw the tile grid when saving as image
* Added a context menu and tool buttons to the layer dock
* Added Latvian translation
* Added an install target to the Makefile
* Open local files when they are dropped onto Tiled
* Allow changing position and size of objects in the Object Properties dialog
* Fixed rendering issues with tiles wider than the tile width of the map
* Fixed eraser and fill tool working on invisible layers
* Fixed a crash when using some tools when no map is loaded
* Fixed compile errors related to detecting static builds
* Fixed the Save dialog not suggesting any particular file extension
* Updated Japanese, Dutch, German, Brazilian Portuguese, French, Portuguese
  and Spanish translations

### Tiled 0.4.1 (14 April 2010)

* Added support for saving tile layer data as CSV
* Added shift modifier to bucket fill tool for filling the selection
* Added Brazilian Portuguese, Japanese, French, Italian and Czech translations
* Made values used in the New Map and New Tileset dialogs persistent
* Fixed drawing selection highlight where brush is not painting
* Fixed an incompatibility with Tiled Java in 'trans' attribute

### Tiled 0.4.0 (30 January 2010)

* Added support for isometric maps
* Added automatic reloading of tileset images when they change
* Added Offset Map action that can shift a set of layers by a certain amount
* Added a fill tool
* Added ability to duplicate map objects
* Added support for choosing the tile layer data format used when saving
* Added mouse wheel zooming support to the tileset view
* Added an object display color attribute to object groups
* Added ability to edit tile properties through a context menu
* Made writing out a DTD reference optional and disabled it by default
* Made translations functional
* Updated Dutch, Portuguese, Spanish and German translations

### Tiled 0.3.1 (22 November 2009)

* Enabled undo command compression for stamp brush and eraser
* Fixed reading of maps with non-binary-encoded layer data
* Fixed a compile issue on Mac OS X related to QXmlStreamWriter
* Fixed a crash when loading a map while holding Ctrl
* Confirm overwrite on the right moment for 'Save as Image' dialog

### Tiled 0.3.0 (13 November 2009)

* Added a tile selection tool
* Added support for cut, copy and paste
* Added current cursor position to the status bar
* Added keyboard shortcuts to switch tools
* Added scrolling the map view with middle mouse button
* Snap objects to the grid when Ctrl is pressed

### Tiled 0.2.0 (1 October 2009)

* Added support for zooming the map view
* Added an eraser tool that allows you to erase tiles
* Added ability to save a map as an image
* Added support for masking tileset images based on a certain color
* Added a slider to change the opacity of the current layer
* Fixed the minimum row and column size in the tileset view
* Fixed stamp creation when not dragging topleft to bottomright

### Tiled 0.1.0 (1 September 2009)
