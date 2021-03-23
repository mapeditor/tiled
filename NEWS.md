### Tiled 1.5.0 (23 March 2021)

* Unified Wang and Terrain tools (backwards incompatible change!)
* Added support for a per-layer parallax scrolling factor ([#2951](https://github.com/mapeditor/tiled/pull/2951))
* Added export to GameMaker Studio 2.3 ([#1642](https://github.com/mapeditor/tiled/issues/1642))
* Added option to change object selection behavior ([#2865](https://github.com/mapeditor/tiled/pull/2865))
* Added Monospace option to the multi-line text editor
* Added option to auto-scroll on middle click
* Added smooth scrolling option for arrow keys
* Added a 'Convert to Polygon' action for rectangle objects
* Added support for drawing with a blob tileset
* Added 'Duplicate Terrain Set' action
* Added Terrain Set type (Corner, Edge or Mixed)
* Added support for rotating and flipping Terrain tiles (by Christof Petig, [#2912](https://github.com/mapeditor/tiled/pull/2912))
* Added support for exporting to [RPTools MapTool](https://www.rptools.net/toolbox/maptool/) RpMap files (by Christof Petig, [#2926](https://github.com/mapeditor/tiled/pull/2926))
* Added Ctrl+Shift to toggle Snap to Fine Grid (by sverx, [#2895](https://github.com/bjorn/tiled/pull/2895))
* Eraser: Added Shift to erase on all layers (by Michael Aganier, [#2897](https://github.com/bjorn/tiled/pull/2897))
* Automatically add .world extension to new World files
* Shape Fill Tool now displays the size of the current shape ([#2808](https://github.com/mapeditor/tiled/issues/2808))
* Tile Collision Editor: Added action to add an auto-detected bounding box collision rectangle (by Robin Macharg, [#1960](https://github.com/bjorn/tiled/pull/1960))
* Tile Collision Editor: Added context menu action to copy selected collision objects to all other selected tiles (by Robin Macharg, [#1960](https://github.com/bjorn/tiled/pull/1960))
* Tilesets view: Added "Edit Tileset" action to tab context menu
* Tilesets view: Added "Add External Tileset" action to tilesets menu
* Scripting: Added initial API for creating and modifying Terrain Sets
* Scripting: Added API for working with images ([#2787](https://github.com/mapeditor/tiled/pull/2787))
* Scripting: Added API for launching other processes ([#2783](https://github.com/mapeditor/tiled/issues/2783))
* Scripting: Added MapView.center property
* Scripting: Added missing Layer.id and Layer.parentLayer properties
* Scripting: Enable extending most context menus
* Scripting: Fixed reset of file formats on script reload ([#2911](https://github.com/mapeditor/tiled/issues/2911))
* Scripting: Fixed missing GroupLayer and ImageLayer constructors
* Scripting: Added default icon for scripted actions
* Enabled high-DPI scaling on Linux and changed rounding policy
* Remember last file dialog locations in the session instead of globally
* Fixed loading extension path from project config (by Peter Ruibal, [#2956](https://github.com/mapeditor/tiled/pull/2956))
* Fixed performance issues when using a lot of custom properties
* Fixed storing template instance size when overriding the tile ([#2889](https://github.com/mapeditor/tiled/issues/2889))
* Fixed removal of object reference arrow when deleting target object ([#2944](https://github.com/mapeditor/tiled/issues/2944))
* Fixed updating of object references when layer visibility changes
* Fixed map positioning issues in the World Tool ([#2970](https://github.com/mapeditor/tiled/issues/2970))
* Fixed handling of Shift modifiers in Bucket and Shape Fill tools ([#2883](https://github.com/mapeditor/tiled/issues/2883))
* Fixed scrolling speed in Tileset view when holding Ctrl
* Fixed issue causing export.target to get written out as "."
* Fixed "Repeat last export on save" when using Save All ([#2969](https://github.com/mapeditor/tiled/issues/2969))
* Fixed interaction shape for rectangle objects to be more precise ([#2999](https://github.com/mapeditor/tiled/issues/2999))
* Fixed "AutoMap While Drawing" not applying when using Cut/Delete
* Fixed path in AutoMap error message when rules file doesn't exist
* Lua plugin: Don't embed external tilesets, unless enabled as export option ([#2120](https://github.com/mapeditor/tiled/issues/2120))
* Python plugin: Added missing values to MapObject.Shape enum ([#2898](https://github.com/bjorn/tiled/issues/2898))
* Python plugin: Fixed linking issue when compiling against Python 3.8
* CSV plugin: Include flipping flags in exported tile IDs
* GMX plugin: Take tile object alignment into account
* Linux: "Open Containing Folder" action now also selects the file
* libtiled-java: Many updates (by Henri Viitanen, [#2207](https://github.com/bjorn/tiled/pull/2207))
* Ported Tiled to Qt 6 (releases still use 5.15 for now)
* Updated Bulgarian, Chinese (Simplified), Czech, Finnish, French, Portuguese, Portuguese (Portugal), Russian, Swedish and Turkish translations

### Tiled 1.4.3 (17 November 2020)

* Fixed running Tiled on macOS Big Sur (#2845)
* Improved error message when adding external tileset
* Fixed opening of files in already open instance of Tiled
* Fixed crash in Edit Commands dialog (#2914)
* Fixed Object Alignment not getting set when reloading a tileset
* Tile Collision Editor: Fixed invisible tile for isometric oriented tileset (#2892)
* Ignore attempts to replace a tileset with itself
* qmake: Support linking to system Zstd on all UNIX-like systems

### Tiled 1.4.2 (5 August 2020)

* Reverted the default layer data format back to CSV (was changed to Zstd by accident in 1.4.0)
* Added ability to draw lines using click+drag (in addition to click and click) when holding Shift
* Improved positioning when adding maps to world via context menu
* Disable instead of hide the "Save As Template" action when using embedded tilesets
* Made Ctrl turn off snapping if Snap to Fine Grid is enabled (#2061)
* Set minimum value of tile width and height to 1
* Fixed Select Same Tile tool behavior for empty tiles
* Fixed clickability of the dot in point objects
* Fixed adjusting of terrain images when tileset width changes
* Worlds: Fixed potential data loss when opening .world file
* tmxrasterizer: Added --show-layer option (by Matthias Varnholt, #2858)
* tmxrasterizer: Added parameter to advance animations (by Sean Ballew, #2868)
* Scripting: Initialize tile layer size to map size upon add (#2879)
* Windows installer: Made creation of the desktop shortcut optional
* Windows installer: Made the launching of Tiled optional
* Updated Qt to 5.12.9 on all platforms except Windows XP and snap releases
* snap: Fixed issues with storing the default session (#2852)
* snap: Enabled support for Zstandard (#2850)

### Tiled 1.4.1 (25 June 2020)

* When opening a .world file, load the world and open its first map
* When opening an object template, show it in the Template Editor
* Fixed crash on trying to export using the command-line (#2842)
* Fixed crash when deleting multiple objects with manual drawing order (#2844)
* Fixed potential crash when removing a tileset
* Fixed potential scaling happening for maps used as tilesets (#2843)
* Fixed positioning of map view when switching between maps in a world
* Fixed file dialog start location
* Scripting: Fixed issues with absolute file paths on Windows (#2841)
* Lua plugin: Fixed syntax used for object properties (#2839)

### Tiled 1.4.0 (17 June 2020)

* Added support for projects (#1665)
* Added object reference property type (with Steve Le Roy Harris and Phlosioneer, #707)
* Added world editing tool for adding/removing and moving around maps in a world (with Nils Kübler, #2208)
* Added a quick "Open file in Project" (Ctrl+P) action
* Added new Object Alignment property to Tileset (with Phlosioneer, #91)
* Added layer tint color (by Gnumaru, #2687)
* Added support for using maps as images (with Phlosioneer, #2708)
* Added 'Open with System Editor' action for custom file properties (#2172)
* Added option to render object names when exporting as image (#2216)
* Added 'Replace Tileset' action to Tilesets view
* Added shortcut to tooltips for all registered actions
* Added automatic reloading of object templates (by Phlosioneer, #2699)
* Added 'Clear Console' button and context menu action (#2220)
* Added 'Reopen Closed File' (Ctrl+Shift+T) action
* Added status bar button to toggle the Console view
* Added a border around the tile selection highlight
* Switch current tileset tab if all selected tiles are from the same tileset (by Mitch Curtis, #2792)
* Made tileset dynamic wrapping toggle persistent
* Properties view: Added action for adding a property to context menu (#2796)
* Optimized loading of CSV tile layer data (by Phlosioneer, #2701)
* Improved map positioning when toggling 'Clear View'
* Remember the preferred format used for saving
* Normalize rotation values when rotating objects (#2775)
* Removed the Maps view (replaced by Project view)
* Removed file system hierarchy from Templates view (replaced by Project view)
* Fixed potential crash when triggering AutoMap (#2766)
* Fixed the status bar placement to be always at the bottom of the window
* Fixed potential issue with automatic reloading of files (#1904)
* Fixed issue where image layer images cannot be loaded from Qt resource files (by obeezzy, #2711)
* GmxPlugin: Added support for layer tint color
* Scripting: Assign global variables to console script evaluations (by Phlosioneer, #2724)
* Scripting: Added coordinate conversion to TileMap
* Scripting: Added support for custom "file" properties
* Scripting: Added checks for nullptr arguments (by Phlosioneer, #2736)
* Scripting: Added some missing tileset related properties
* Scripting: Added FileInfo API with various file path operations (with David Konsumer, #2822)
* Scripting: Provide access to registered file formats (by Phlosioneer, #2716)
* Scripting: Enabled scripted formats to be used on the command-line
* Scripting: Added functions to access inherited properties (by Bill Clark, #2813)
* Scripting: Introduced \__filename global value (with konsumer)
* Scripting: Fixed ObjectGroup.insertObjectAt to use the index
* docs: Clarify "can contain" documentation and error handling (by Phlosioneer, #2702)
* docs: Document all optional attributes, update some docs (by Phlosioneer, #2705)
* docs: Alphabetize scripting API reference (by Phlosioneer, #2720)
* docs: Added missing BinaryFile constructor docs (by Phlosioneer, #2732)
* docs: Enabled Algolia powered search
* libtiled-java: Big update to support newer TMX attributes (by Mike Thomas, #1925)
* libtiled-java: Fixed writing of the tile type (by Phlosioneer, #2704)
* libtiled-java: Enable loading of maps from jar files (by Adam Hornáček, #2829)
* Updated Bulgarian, Chinese (Simplified), Czech, Finnish, French, Norwegian Bokmål, Portuguese (Portugal) and Turkish translations


### Tiled 1.3.5 (27 May 2020)

* Fixed initialization and restoring of map view (#2779)
* Fixed skewed tile terrain/Wang overlays for non-square tiles (#1943)
* Fixed link color on dark theme
* Fixed small issue when right-clicking embedded tileset tab
* Fixed Wang Sets toggle to also appear in the Tileset menu
* Scripting: Fixed issue when closing/comitting BinaryFile (#2801)
* Scripting: Fixed "Safe writing of files" when writing with TextFile
* Updated Qt to 5.12.8 on all platforms except Windows XP and snap releases
* Small translation updates to Bulgarian, French and Portuguese

### Tiled 1.3.4 (14 April 2020)

* Fixed automatic reload issues when editing object types (regression in 1.3.1, #2768)
* Scripting: Added methods to get tileset's image size (backported from 1.4, #2733)
* Scripting: Fixed map.tilesets when 'Embed tilesets' is enabled
* Fixed the "Fix Tileset" button in the Template Editor
* macOS: Disabled unified tool bar to avoid repainting issues (#2667)
* macOS and Linux: Updated Qt from 5.12.6 to 5.12.7

### Tiled 1.3.3 (3 March 2020)

* Fixed loading of compression level
* Fixed default value for Hex Side Length property
* Fixed hiding of status bar text for some tools
* Fixed removing of object labels when removing a group layer
* GmxPlugin: Fixed compatibility with GameMaker 1.4.9999
* Scripting: Made TextFile.commit and BinaryFile.commit close as well
* Scripting: Fixed crashes when modifying certain new objects
* Scripting: Fixed potential crash in Asset.macro/undo/redo/isModified
* Scripting: Fixed potential crash when accessing Tool.preview
* Scripting: Fixed loading of images from extensions folder
* Scripting: Reload extensions also when files are added/removed
* Updated Bulgarian translation (by Любомир Василев)

### Tiled 1.3.2 (22 January 2020)

* Fixed initialization of selected layers (#2719)
* Fixed stamp action shortcuts not being configurable (#2684)
* Fixed the tileset view to respect the 'wheel zooms by default' preference
* Fixed insertion position when using drag-n-drop to rearrange layers
* Fixed displayed layer data format in Properties
* Fixed repeating of export when map is saved by a custom command (#2709)
* Fixed issue when multiple worlds are loaded that use pattern matching
* Issues view can now be hidden by clicking the status bar counters
* macOS: Fixed black toolbar when enabling OpenGL rendering (#1839)
* Windows: Fixed context menus activating first item on release (#2693)
* Windows installer: Include the 'defoldcollection' plugin (#2677)
* libtiled: Avoid inheriting Properties from QVariantMap (#2679)
* docs: Added some notes to Python and JavaScript pages (#2725)
* Updated Qt from 5.12.5 to 5.12.6
* Updated Finnish translation (by Tuomas Lähteenmäki and odamite)
* Updated part of Italian translation (by Katia Piazza)

### Tiled 1.3.1 (20 November 2019)

* Added reloading of object types when changed externally (by Jacob Coughenour, #2674)
* Added a status bar to the startup screen
* Made the shortcuts for the tools configurable (#2666)
* Made Undo/Redo shortcuts configurable (#2669)
* Fixed importing of keyboard settings (.kms files) (#2671)
* Fixed small window showing up on startup for a split second
* Windows: Fixed the shipped version of OpenSSL (fixes new version notification)
* Tiled Quick: Don't compile/install by default (#2673)

### Tiled 1.3.0 (13 November 2019)

* Added support for extending Tiled with JavaScript (#949)
* Added error and warning counts to the status bar
* Added Issues view where you can see warnings and errors and interact with them
* Added configuration of keyboard shortcuts (#215)
* Added status bar notification on new releases (replacing Sparkle and WinSparkle)
* Added option to show tile collision shapes on the map (#799)
* Added switching current layer with Ctrl + Right Click in map view
* Added search filter to the Objects view (#1467)
* Added icons to objects in the Objects view
* Added dynamic wrapping mode to the tileset view (#1241)
* Added a \*.world file filter when opening a world file
* Added support for .world files in tmxrasterizer (by Samuel Magnan, #2067)
* Added synchronization of selected layers and tileset when switching between maps in a world (by JustinZhengBC, #2087)
* Added actions to show/hide and lock/unlock the selected layers
* Added toggle button for "Highlight Current Layer" action
* Added custom output chunk size option to map properties (by Markus, #2130)
* Added support for Zstandard compression and configurable compression level (with BRULE Herman and Michael de Lang, #1888)
* Added option to minimize output on export (#944)
* Added export to Defold .collection files (by CodeSpartan, #2084)
* Added a warning when custom file properties point to non-existing files (#2080)
* Added shortcuts for next/previous tileset (#1238)
* Added saving of the last export target and format in the map/tileset file (#1610)
* Added option to repeat the last export on save (#1610)
* Added Fit Map in View action (by Mateo de Mayo, #2206)
* Tile Collision Editor: Added objects list view
* Changed the Type property from a text box to an editable combo box (#823)
* Changed animation preview to follow zoom factor for tiles (by Ruslan Gainutdinov, #2050)
* Changed the shortcut for AutoMap from A to Ctrl+M
* AutoMapping: Added "OverflowBorder" and "WrapBorder" options (by João Baptista de Paula e Silva, #2141)
* AutoMapping: Allow any supported map format to be used for rule maps
* Python plugin: Added support for loading external tileset files (by Ruin0x11, #2085)
* Python plugin: Added Tile.type() and MapObject.effectiveType() (by Ruin0x11, #2124)
* Python plugin: Added Object.propertyType() (by Ruin0x11, #2125)
* Python plugin: Added Tileset.sharedPointer() function (#2191)
* tmxrasterizer: Load plugins to support additional map formats (by Nathan Tolbert, #2152)
* tmxrasterizer: Added rendering of object layers (by oncer, #2187)
* Fixed missing native styles when compiled against Qt 5.10 or later (#1977)
* Fixed file change notifications no longer triggering when file was replaced (by Nathan Tolbert, #2158)
* Fixed layer IDs getting re-assigned when resizing the map (#2160)
* Fixed performance issues when switching to a new map in a world with many maps (by Simon Parzer, #2159)
* Fixed restoring of expanded group layers in Objects view
* Fixed tileset view to keep position at mouse stable when zooming (#2039)
* libtiled-java: Added support for image layers and flipped tiles (by Sergey Savchuk, #2006)
* libtiled-java: Optimized map reader and fixed path separator issues (by Pavel Bondoronok, #2006)
* Updated builds on all platforms to Qt 5.12 (except snap release)
* Raised minimum supported Qt version from 5.5 to 5.6
* Raised minimum supported macOS version from 10.7 to 10.12
* Removed option to include a DTD in the saved files
* Removed the automappingconverter tool
* snap: Updated from Ubuntu 16.04 to 18.04 (core18, Qt 5.9)
* Updated Chinese, Portuguese (Portugal), Turkish and Ukrainian translations

### Tiled 1.2.5 (9 October 2019)

* Fixed exporting to a file name containing multiple dots (#2149)
* Fixed possible crash in AutoMapper (#2157)
* Fixed crash when unloading certain plugins
* Fixed duplicated entries in Objects view after grouping layers
* Fixed adjacent maps within a world not being properly clickable
* Fixed empty maps within a world not being clickable
* Fixed handling of negative multiplierX/Y in a world file

### Tiled 1.2.4 (15 May 2019)

* Fixed view boundaries to take into account layer offsets (#2090)
* Fixed map size when switching infinite off (#2051)
* Fixed the image cache to check file modification time (#2081)
* Fixed updating a few things when changing tileset drawing offset
* Fixed position of tile object outline on isometric maps
* Fixed saving of tile stamps when using the Shape Fill Tool
* tBIN plugin: Fixed loading of some tilesets on Linux
* tBIN plugin: Fixed possible crash when images can't be found (#2106)
* Python plugin: Disable this plugin by default, to avoid crashes on startup (#2091)
* JSON plugin: Fixed writing of position for objects without ID
* Added Swedish translation (by Anton R)

### Tiled 1.2.3 (12 March 2019)

* Fixed cut/copy in Tile Collision Editor (#2075)
* Fixed crash when trying to add Wang colors without a selected Wang set (#2083)
* tBIN plugin: Fixed hang when locating missing tileset image (#2068)
* CSV plugin: Fixed exporting of grouped tile layers

### Tiled 1.2.2 (29 January 2019)

* Added 'json1' plugin that exports to the old JSON format (#2058)
* Enable the adding of point objects in Tile Collision Editor (#2043)
* Reload AutoMapping rules when they have changed on disk (by Justin Zheng, #1997)
* Fixed remembering of last used export filter
* Fixed label color to update when object layer color is changed (by Justin Zheng, #1976)
* Fixed stamp and fill tools to adjust when tile probability is changed (by Justin Zheng, #1996)
* Fixed misbehavior when trying to open non-existing files
* Fixed mini-map bounds when layer offsets are used in combination with group layers
* Fixed Templates view missing from the Views menu (#2054)
* Fixed Copy Path / Open Folder actions for embedded tilesets (#2059)
* Python plugin: Made the API more complete (#1867)
* Updated Chinese, German, Korean, Norwegian Bokmål, Portuguese (Portugal) and Ukrainian translations

### Tiled 1.2.1 (14 November 2018)

* Fixed JSON templates not being visible in Templates view (#2009)
* Fixed Maps view to show all readable map formats
* Fixed crash when deleting a command using the context menu (by Robert Lewicki, #2014)
* Fixed crash after a world file failed to load
* Fixed Select None action to be enabled when there is any selection
* Fixed disappearing of tile types on export/import of a tileset (#2023)
* Fixed tool shortcuts when using Spanish translation
* Fixed saving of the "Justify" alignment option for text objects (#2026)
* Changed Cut, Copy and Delete actions to apply based on selected layer types
* Windows: Updated builds to Qt 5.9.7
* Updated Russian translation (by Rafael Osipov, #2017)

### Tiled 1.2.0 (19 September 2018)

* Added multi-layer selection, including multi-layer tile layer editing
* Added support for multi-map worlds (#1669)
* Added ability to extend existing polylines (with Ketan Gupta, #1683)
* Added option to highlight the hovered object (#1190)
* Added news from website to the status bar (#1898)
* Added option to show object labels for hovered objects
* Added option to embed tilesets on export (#1850)
* Added option to detach templates on export (#1850)
* Added option to resolve object types and properties on export (#1850)
* Added Escape for switching to the Select Objects tool and for clearing the selection
* Added Escape to cancel the current action in all object layer tools
* Added double-click on polygon objects to switch to Edit Polygons tool
* Added interaction with segments for polygons, for selection and dragging
* Added double-clicking a polygon segment for inserting a new point at that location
* Added action to lock/unlock all other layers (by kralle333, #1883)
* Added --export-tileset command line argument (by Josh Bramlett, #1872)
* Added unique persistent layer IDs (#1892)
* Added 'version' and 'tiledversion' to external tileset files
* Added full paths to Recent Files menu as tool tips (by Gauthier Billot, #1992)
* Create Object Tools: Show preview already on hover (#537)
* Objects view: Only center view on object on press or activation
* Objects view: When clicking a layer, make it the current one (by kralle333, #1931)
* Unified the Create Polygon and Create Polyline tools
* JSON plugin: Made the JSON format easier to parse (by saeedakhter, #1868)
* Tile Collision Editor: Allowed using object templates
* Templates view: Don't allow hiding the template object
* Python plugin: Updated to Python 3 (by Samuli Tuomola)
* Python plugin: Fixed startup messages not appearing in debug console
* Python plugin: Fixed file change watching for main script files
* Lua plugin: Include properties from templates (#1901)
* Lua plugin: Include tileset column count in export (by Matt Drollette, #1969)
* tBIN plugin: Don't ignore objects that aren't perfectly aligned (#1985)
* tBIN plugin: Fixed "Unsupported property type" error for newly added float properties
* Automapping: Report error when no output layers are found
* AutoMapping: Changed matching outside of map boundaries and added 'MatchOutsideMap' option
* Linux: Modernized the appstream file (by Patrick Griffis)
* libtiled: Allow qrc-based tileset images (#1947)
* libtiled-java: Fixed loading maps with multiple external tilesets
* Optimized deletion of many objects (#1972)
* Make Ctrl+Q work for quitting also on Windows (#1998)
* Fixed randomizing of terrain, Wang tiles and stamp variations (#1949)
* Fixed tilesets getting added to maps when they shouldn't be (#2002)
* Fixed issue with default font size in combination with custom family (#1994)
* Fixed the tile grid to render below labels, handles and selection indicators
* Fixed confirming overwrite when exporting a tileset
* Fixed reading of infinite maps that don't use chunked layer data
* Updated Bulgarian, Dutch, French, German, Norwegian Bokmål, Portuguese (Portugal) and Turkish translations

### Tiled 1.1.6 (17 July 2018)

* Fixed Terrain Brush issue on staggered isometric maps (#1951)
* Fixed objects to stay selected when moving them between layers
* Fixed small tab bar rendering issue on high DPI displays
* Fixed rendering of arrows on scroll bar buttons
* Fixed object labels to adjust properly to the font DPI
* Fixed resize handle locations for multiple zero-sized objects
* Fixed handling of arrow keys on focused layer combo box (#1973)
* Tile Collision Editor: Fixed handling of tile offset (#1955)
* Tile Collision Editor: Fixed potential crash on Undo (#1965)
* Python plugin: Added some missing API to the Cell class
* Windows and Linux: Downgraded builds to Qt 5.9 (fixes #1928)
* macOS: Fixed library loading issues for tmxrasterizer and terraingenerator
* macOS: Downgraded to Qt 5.6 (fixes resizing of undocked views and reduces minimum macOS version to 10.7)
* Updates to German, Hungarian, Norwegian Bokmål, Polish, Portuguese (Portugal), Russian and Ukrainian translations

### Tiled 1.1.5 (25 April 2018)

* Fixed erasing mode of the Terrain Brush
* Fixed crash after editing a template
* Fixed rendering of eye/lock icons in Layers view
* Fixed object index when undoing Move Object to Layer action (#1932)
* Fixed shortcuts for flipping and rotating objects (#1926)
* Fixed dynamic retranslation of tools and tool actions
* Fixed possible crash when undoing/redoing Wang color changes
* Fixed handling of sub-properties in Object Type Editor (#1936)
* Fixed crash when deleting an object right before dragging it (#1933)
* Adjust Wang tile data when tileset column count changes (#1851)
* Improved fill behavior in case of selection on infinite map (#1921)
* Removed ability to hide tile collision objects (#1929)
* Remove tile collision layer along with the last object (#1230)
* JSON plugin: Made the reader more strict about object types (#1922)
* JSON plugin: Added support for Wang sets

### Tiled 1.1.4 (28 March 2018)

* Fixed exporting of external tilesets to JSON or TSX formats
* Fixed problem with embedding or exporting tilesets with Wang sets
* Fixed tiles placed by the terrain tool being considered different (#1913)
* Fixed text alignment values appearing at random in Properties view (#1767)
* macOS: Fixed eye/lock icon display in Layers view
* Re-enabled Space for toggling layer visibility
* Migrate properties set on tile collision layer to the tile (#1912)
* Don't reset stamp brush state when pressing Alt
* Automapping: Apply rules to selected area when there is one
* Windows and Linux: Updated builds to Qt 5.10.1
* Linux: Indicate Tiled can open multiple files at once in desktop file
* Lowered the minimum supported version of Qt to 5.5

### Tiled 1.1.3 (6 March 2018)

* Fixed crash when removing a tileset referenced by multiple objects
* Fixed crash on paste when it introduced more than one new tileset
* Fixed Invert Selection for non-infinite maps
* Fixed Select All to not select objects on locked layers
* Fixed logic determining the tilesets used by a tile layer
* Fixed copy/paste changing object order (#1896)
* Fixed tileset getting loaded twice when used by the map and a template
* Fixed repainting issues on undo/redo for new maps (#1887)
* JSON plugin: Fixed loading of infinite maps using CSV tile layer format (#1878)
* Linux: Updated AppImage to Qt 5.9.4
* Updated Hungarian, Japanese, Norwegian Bokmål, Portuguese and Ukrainian translations

### Tiled 1.1.2 (31 January 2018)

* Fixed possible crash while editing polygons
* Fixed hang when loading map file with empty compressed layer data
* Fixed selection of tile stamp to work on mouse click
* Fixed tools not being up to date on modifier keys after activation
* Fixed "Offset Map" action for infinite maps (#1866)
* Templates view: Keep template centered when resizing view
* Tile Collision Editor: Keep tile centered when resizing view
* Tile Collision Editor: Display tool info text in status bar
* JSON plugin: Fixed reading of infinite maps (#1858)
* libtiled-java: Fixed some bugs (by Henry Wang, #1840)
* libtiled-java: Fixed tile offset value not being considered (by digitalhoax, #1863)

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
