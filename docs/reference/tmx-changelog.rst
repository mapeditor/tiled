TMX Changelog
=============

Below are described the changes/additions that were made to the
:doc:`tmx-map-format` for recent versions of Tiled.

Tiled 1.10
----------

-  Renamed the ``class`` attribute on :ref:`tmx-tileset-tile` and
   :ref:`tmx-object` back to ``type``, to keep compatibility with Tiled 1.8
   and earlier. The attribute remains ``class`` for other elements since it
   could not be renamed to ``type`` everywhere.

Tiled 1.9
---------

-  Renamed the ``type`` attribute on :ref:`tmx-tileset-tile` and
   :ref:`tmx-object` to ``class``.

-  Added ``class`` attribute to :ref:`tmx-map`, :ref:`tmx-tileset`,
   :ref:`tmx-layer`, :ref:`tmx-imagelayer`, :ref:`tmx-objectgroup`,
   :ref:`tmx-group`, :ref:`tmx-wangset` and :ref:`tmx-wangcolor`.

-  Added ``x``, ``y``, ``width`` and ``height`` attributes to the
   :ref:`tmx-tileset-tile` element, which store the sub-rectangle of a tile's
   image used to represent this tile. By default the entire image is used.

-  Added ``tilerendersize`` and ``fillmode`` attributes to the
   :ref:`tmx-tileset` element, which affect the way tiles are rendered.

Tiled 1.8
---------

-  Added support for user-defined custom property types. A reference to the
   type is saved as the new ``propertytype`` attribute on the
   :ref:`tmx-property` element.

-  The :ref:`tmx-property` element can now contain a :ref:`tmx-properties`
   element, in case the property value is a class and at least one member value
   has been set. The ``type`` attribute will have the new value ``class``.

-  Added ``parallaxoriginx`` and ``parallaxoriginy`` attributes to the
   :ref:`tmx-map` element.

-  Added ``repeatx`` and ``repeaty`` attributes to the :ref:`tmx-imagelayer`
   element.

Tiled 1.7
---------

-  The :ref:`tmx-tileset-tile` elements in a tileset are no longer always saved
   with increasing IDs. They are now saved in the display order, which can be
   changed in Tiled.

Tiled 1.5
---------

-  The colors that are part of a :ref:`tmx-wangset` are no longer separated in
   corner colors and edge colors. Instead, there is now a single
   :ref:`tmx-wangcolor` element to define a Wang color. This new element also
   stores :ref:`tmx-properties`.

-  The ``wangid`` attribute on the :ref:`tmx-wangtile` element is now stored as
   a comma-separated list of values, instead of a 32-bit unsigned integer in
   hex format. This is because the number of colors supported in a Wang set was
   increased from 15 to 255.

-  Valid transformations of tiles in a set (flipping, rotation) are specified
   in a :ref:`tmx-tileset-transformations` element. The partial support for the
   ``vflip``, ``hflip`` and ``dflip`` attributes on the :ref:`tmx-wangtile`
   element has been removed.

-  The :ref:`tmx-wangset` element has replaced the now deprecated
   :ref:`tmx-terraintypes` element.

-  Added ``parallaxx`` and ``parallaxy`` attributes to the :ref:`tmx-layer`,
   :ref:`tmx-objectgroup`, :ref:`tmx-imagelayer` and :ref:`tmx-group` elements.

Tiled 1.4
---------

-  Added the ``objectalignment`` attribute to the :ref:`tmx-tileset` element,
   allowing the tileset to control the alignment used for tile objects.

-  Added the ``tintcolor`` attribute to the :ref:`tmx-layer`,
   :ref:`tmx-objectgroup`, :ref:`tmx-imagelayer` and :ref:`tmx-group` elements,
   allowing for a number of graphical effects like darkening or coloring a
   layer.

-  Added a new ``object`` property type, which refers to an
   :ref:`object <tmx-object>` by its ID.

Tiled 1.3
---------

-  Added an :ref:`tmx-editorsettings` element, which is used to store editor
   specific settings that are generally not relevant when loading a map.

-  Added support for Zstandard compression for tile layer data
   (``compression="zstd"`` on :ref:`tmx-data` elements).

-  Added the ``compressionlevel`` attribute to the :ref:`tmx-map` element,
   which stores the compression level to use for compressed tile layer data.

Tiled 1.2.1
-----------

-  Text objects can now get their horizontal alignment saved as ``justify``.
   This option existed in the UI before but wasn't saved properly.

Tiled 1.2
---------

-  Added an ``id`` attribute to the :ref:`tmx-layer`, :ref:`tmx-objectgroup`,
   :ref:`tmx-imagelayer` and :ref:`tmx-group` elements, which stores a
   map-unique ID of the layer.

-  Added a ``nextlayerid`` attribute to the :ref:`tmx-map` element, which
   stores the next available ID for new layers. This number is stored
   to prevent reuse of the same ID after layers have been removed.

Tiled 1.1
---------

-  Added a :ref:`map.infinite <tmx-map>` attribute, which indicates whether
   the map is considered unbounded. Tile layer data for infinite maps is
   stored in chunks.

-  A new :ref:`tmx-chunk` element was added for infinite maps which
   contains the similar content as :ref:`tmx-data`, except it stores
   the data of the area specified by its ``x``, ``y``, ``width`` and
   ``height`` attributes.

-  :doc:`Templates </manual/using-templates>` were added, a
   template is an :ref:`external file <tmx-template-files>` referenced
   by template instance objects:

   .. code:: xml

      <object id="3" template="diamond.tx" x="200" y="100"/>

-  Tilesets can now contain :doc:`Terrain Sets </manual/terrain>`.
   They are saved in the new :ref:`tmx-wangsets` element.

-  A new :ref:`tmx-point` child element was added to :ref:`tmx-object`, which
   marks point objects. Point objects do not have a size or rotation.

Tiled 1.0
---------

-  A new :ref:`tmx-group` element was added which is a group layer that can
   have other layers as child elements. This means layers now form a hierarchy.

-  Added Text objects, identified by a new :ref:`tmx-text` element which is
   used as a child of the :ref:`tmx-object` element.

-  Added a :ref:`tile.type <tmx-tileset-tile>` attribute for supporting
   :ref:`typed-tiles`.

Tiled 0.18
----------

*No file format changes.*

Tiled 0.17
----------

-  Added ``color`` and ``file`` as possible values for the
   :ref:`property.type <tmx-property>` attribute.

-  Added support for editing multi-line string properties, which are
   written out differently.

Tiled 0.16
----------

-  The :ref:`tmx-property` element gained a ``type`` attribute, storing the
   type of the value. Currently supported types are ``string`` (the default),
   ``int``, ``float`` and ``bool``.

Tiled 0.15
----------

-  The ``offsetx`` and ``offsety`` attributes are now also used for
   :ref:`tmx-imagelayer` elements, replacing the ``x`` and ``y`` attributes
   previously used. This change was made for consistency with the other layer
   types.

-  The tiles in an image collection tileset are no longer guaranteed to
   be consecutive, because removing tiles from the collection will no
   longer change the IDs of other tiles.

-  The pure XML and Gzip-compressed tile layer data formats were
   deprecated, since they didn't have any advantage over other formats.
   Remaining formats are CSV, base64 and Zlib-compressed layer data.

-  Added ``columns`` attribute to the
   :ref:`tmx-tileset` element, which specifies the number of tile columns in
   the tileset. For image collection tilesets it is editable and is used when
   displaying the tileset.

-  The ``backgroundcolor`` attribute of the
   :ref:`tmx-map` element will now take the format ``#AARRGGBB`` when its alpha
   value differs from 255. Previously the alpha value was silently discarded.

Tiled 0.14
----------

-  Added optional ``offsetx`` and ``offsety`` attributes to the
   ``layer`` and ``objectgroup`` elements. These specify an offset in
   pixels that is to be applied when rendering the layer. The default
   values are 0.

Tiled 0.13
----------

-  Added an optional ``tilecount`` attribute to the ``tileset`` element,
   which is written by Tiled to help parsers determine the amount of
   memory to allocate for tile data.

Tiled 0.12
----------

-  Previously tile objects never had ``width`` and ``height``
   properties, though the format technically allowed this. Now these
   properties are used to store the size the image should be rendered
   at. The default values for these attributes are the dimensions of the
   tile image.

Tiled 0.11
----------

-  Added ``hexagonal`` to the supported values for the ``orientation``
   attribute on the ``map`` element. This also adds ``staggerindex``
   (``even`` or ``odd``) and ``staggeraxis`` (``x`` or ``y``) and
   ``hexsidelength`` (integer value) attributes to the ``map`` element,
   in order to support the many variations of staggered hexagonal. The
   new ``staggerindex`` and ``staggeraxis`` attributes are also
   supported when using the ``staggered`` map orientation.

-  Added an ``id`` attribute to the ``object`` element, which stores a
   map-unique ID of the object.

-  Added a ``nextobjectid`` attribute to the ``map`` element, which
   stores the next available ID for new objects. This number is stored
   to prevent reuse of the same ID after objects have been removed.

Tiled 0.10
----------

-  Tile objects can now be horizontally or vertically flipped. This is
   stored in the ``gid`` attribute using the same mechanism as for
   regular tiles. The image is expected to be flipped without affecting
   its position, same way as flipped tiles.

-  Objects can be rotated freely. The rotation is stored in degrees as a
   ``rotation`` attribute, with positive rotation going clockwise.

-  The render order of the tiles on tile layers can be configured in a
   number of ways through a new ``renderorder`` property on the ``map``
   element. Valid values are ``right-down`` (the default), ``right-up``,
   ``left-down`` and ``left-up``. In all cases, the map is drawn
   row-by-row. This is only supported for orthogonal maps at the moment.

-  The render order of objects on object layers can be configured to be
   either sorted by their y-coordinate (previous behavior and still the
   default) or simply the order of appearance in the map file. The
   latter enables manual control over the drawing order with actions
   that "Raise" and "Lower" selected objects. It is controlled by the
   ``draworder`` property on the ``objectgroup`` element, which can be
   either ``topdown`` (default) or ``index``.

-  Tiles can have an ``objectgroup`` child element, which can contain
   objects that define the collision shape to use for that tile. This
   information can be edited in the new Tile Collision Editor.

-  Tiles can have a single looping animation associated with them using
   an ``animation`` child element. Each frame of the animation refers to
   a local tile ID from this tileset and defines the frame duration in
   milliseconds. Example:

   .. code:: xml

      <tileset name="Animations">
        ...
        <tile id="[n]">
          <animation>
            <frame tileid="0" duration="100"/>
            <frame tileid="1" duration="100"/>
            <frame tileid="2" duration="100"/>
          </animation>
        </tile>
      </tileset>

Tiled 0.9
---------

-  Per-object visibility flag is saved (defaults to 1):

   .. code:: xml

      <object visible="0|1">

-  Terrain information was added to tileset definitions (this is
   generally not very relevant for games):

   .. code:: xml

      <tileset name="Terrain">
        ...
        <terraintypes>
          <terrain name="Name" tile="local_id"/>
        </terraintypes>
        <tile id="local_id" terrain="[n],[n],[n],[n]" probability="percentage"/>
        ...
      </tileset>

-  There is preliminary support for a "staggered" (isometric) projection
   (new value for the ``orientation`` attribute of the ``map`` element).

-  A basic image layer type was added:

   .. code:: xml

      <imagelayer name="...">
        <image source="..."/>
      </imagelayer>

-  Added ellipse object shape. Same parameters as rectangular objects,
   but marked as ellipse with a child element:

   .. code:: xml

      <object name="..." x="..." y="...">
        <ellipse/>
      </object>

-  Added map property for specifying the background color:

   .. code:: xml

      <map backgroundcolor="#RRGGBB">

-  Added initial (non-GUI) support for individual and/or embedded tile
   images (since there is no way to set this up in Tiled Qt but only in
   Tiled Java or with
   `pytmxlib <https://github.com/encukou/pytmxlib>`__, this is not very
   important to support at the moment):

   .. code:: xml

      <tileset name="Embedded images">
        ...
        <tile id="[n]">
          <!-- an embedded image -->
          <image format="png">
            <data encoding="base64">
                ...
            </data>
          </image>
        </tile>
        <tile id="[n]">
          <!-- an individually referenced image for a single tile -->
          <image source="file.png"/>
        </tile>
        ...
      </tileset>

Tiled 0.8
---------

-  Tilesets can now have custom properties (using the ``properties``
   child element, just like everything else).

-  Tilesets now support defining a drawing offset in pixels, which is to
   be used when drawing any tiles from that tileset. Example:

   .. code:: xml

      <tileset name="perspective_walls" tilewidth="64" tileheight="64">
        <tileoffset x="-32" y="0"/>
        ...
      </tileset>

-  Support for tile rotation in 90-degree increments was added by using
   the third most significant bit in the global tile id. This new bit
   means "anti-diagonal flip", which swaps the x and y axis when
   rendering a tile.
