<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../src/tiled/aboutdialog.ui" line="+14"/>
        <source>About Tiled</source>
        <translation>Acerca de Tiled</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Donate</source>
        <translation>Donar</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/tiled/aboutdialog.cpp" line="+36"/>
        <source>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Copyright 2008-2015 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(see the AUTHORS file for a full list of contributors)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;You may modify and redistribute this program under the terms of the GPL (version 2 or later). A copy of the GPL is contained in the &apos;COPYING&apos; file distributed with Tiled.&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</source>
        <translation>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Copyright 2008-2015 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(vea en el fichero AUTHORS una lista completa de los colaboradores)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Usted puede modificar y redistribuir este programa bajo los terminos de la GPL (versión 2 ó superior). Una copia de la GPL se encuentra en el fichero &apos;COPYING&apos; que acompaña a Tiled.&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</translation>
    </message>
</context>
<context>
    <name>Command line</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="+214"/>
        <source>Export syntax is --export-map [format] &lt;tmx file&gt; &lt;target file&gt;</source>
        <translation>La sintaxis de exportación es --export-map [formato] &lt;archivo tmx&gt; &lt;archivo destino&gt;</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Format not recognized (see --export-formats)</source>
        <translation>Formato no reconocido (ver --export-formats)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Non-unique file extension. Can&apos;t determine correct export format.</source>
        <translation>Extensión de archivo no única. No se puede determinar el formato correcto al que exportar.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>No exporter found for target file.</source>
        <translation>No se encontró un exportador para el archivo de destino.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Failed to load source map.</source>
        <translation>Falló al cargar el mapa del archivo de origen.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Failed to export map to target file.</source>
        <translation>Falló al exportar el mapa al archivo de destino.</translation>
    </message>
</context>
<context>
    <name>CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.ui" line="+14"/>
        <source>Properties</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>&amp;Save map before executing</source>
        <translation>&amp;Guardar mapa antes de ejecutarlo</translation>
    </message>
</context>
<context>
    <name>CommandLineHandler</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="-184"/>
        <source>Display the version</source>
        <translation>Mostrar la versión</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Only check validity of arguments</source>
        <translation>Solo comprobar la validez de los argumentos</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Disable hardware accelerated rendering</source>
        <translation>Desactivar el pintado acelerado por hardware</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Export the specified tmx file to target</source>
        <translation>Exportar el archivo tmx especificado al destino</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Print a list of supported export formats</source>
        <translation>Mostrar una lista con los formatos de exportación soportados</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Export formats:</source>
        <translation>Formatos de exportación:</translation>
    </message>
</context>
<context>
    <name>CommandLineParser</name>
    <message>
        <location filename="../src/tiled/commandlineparser.cpp" line="+75"/>
        <source>Bad argument %1: lonely hyphen</source>
        <translation>Argumento no válido %1: guión solitario</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Unknown long argument %1: %2</source>
        <translation>Argumento largo desconocido %1: %2 </translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Unknown short argument %1.%2: %3</source>
        <translation>Argumento corto desconocido %1.%2: %3</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Usage:
  %1 [options] [files...]</source>
        <translation>Uso::
  %1 [opciones] [archivos...]</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Options:</source>
        <translation>Opciones:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Display this help</source>
        <translation>Mostrar esta ayuda</translation>
    </message>
</context>
<context>
    <name>ConverterDataModel</name>
    <message>
        <location filename="../src/automappingconverter/converterdatamodel.cpp" line="+75"/>
        <source>File</source>
        <translation>Archivo</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Version</source>
        <translation>Versión</translation>
    </message>
</context>
<context>
    <name>ConverterWindow</name>
    <message>
        <location filename="../src/automappingconverter/converterwindow.cpp" line="+36"/>
        <source>Save all as %1</source>
        <translation>Guardar todo como %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>All Files (*)</source>
        <translation>Todos los ficheros (*)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Archivos de mapas de Tiled (*.tmx)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Open Map</source>
        <translation>Abrir Mapa</translation>
    </message>
</context>
<context>
    <name>Csv::CsvPlugin</name>
    <message>
        <location filename="../src/plugins/csv/csvplugin.cpp" line="+55"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>CSV files (*.csv)</source>
        <translation>Archivos CSV (*.csv)</translation>
    </message>
</context>
<context>
    <name>Droidcraft::DroidcraftPlugin</name>
    <message>
        <location filename="../src/plugins/droidcraft/droidcraftplugin.cpp" line="+57"/>
        <source>This is not a valid Droidcraft map file!</source>
        <translation>¡Este no es un tipo de fichero de mapa Droidcraft válido!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>The map needs to have exactly one tile layer!</source>
        <translation>¡El mapa necesita tener exactamente una sola capa de patrones!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The layer must have a size of 48 x 48 tiles!</source>
        <translation>¡La capa debe tener un tamaño de 48x48 patrones!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Droidcraft map files (*.dat)</source>
        <translation>Ficheros de mapa Droidcraft (*.dat)</translation>
    </message>
</context>
<context>
    <name>EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.ui" line="+14"/>
        <source>Edit Terrain Information</source>
        <translation>Editar la Información de Terreno</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+3"/>
        <source>Undo</source>
        <translation>Deshacer</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+3"/>
        <source>Redo</source>
        <translation>Rehacer</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Erase</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Add Terrain Type</source>
        <translation>Añadir Tipo de Terreno</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Terrain Type</source>
        <translation>Eliminar Tipo de Terreno</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
</context>
<context>
    <name>ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.ui" line="+14"/>
        <source>Export As Image</source>
        <translation>Exportar como Imagen</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Explorar...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Settings</source>
        <translation>Preferencias</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only include &amp;visible layers</source>
        <translation>Solo incluir las &amp;capas visibles</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Use current &amp;zoom level</source>
        <translation>Usar el nivel actual de &amp;aumento</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Draw tile grid</source>
        <translation>&amp;Dibujar guía para patrones</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Include background color</source>
        <translation>&amp;Incluir el color de fondo</translation>
    </message>
</context>
<context>
    <name>Flare::FlarePlugin</name>
    <message>
        <location filename="../src/plugins/flare/flareplugin.cpp" line="+52"/>
        <source>Could not open file for reading.</source>
        <translation>No se pudo abrir el archivo para lectura.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Error loading tileset %1, which expands to %2. Path not found!</source>
        <translation>Error cargando el conjunto de patrones %1, el cual se expande a %2. ¡Ruta no encontrada!</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>No tilesets section found before layer section.</source>
        <translation>No se encontró una sección con el conjunto de patrones antes de la sección de capas.</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Error mapping tile id %1.</source>
        <translation>Error asignando el patrón con id %1.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>This seems to be no valid flare map. A Flare map consists of at least a header section, a tileset section and one tile layer.</source>
        <translation>Éste no parece ser un mapa Flare válido. Un mapa Flare consta de al menos una cabecera, un conjunto de patrones y una capa de patrones.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Flare map files (*.txt)</source>
        <translation>Archivos de mapas de Flare (*.txt)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
</context>
<context>
    <name>Json::JsonMapFormat</name>
    <message>
        <location filename="../src/plugins/json/jsonplugin.cpp" line="+53"/>
        <source>Could not open file for reading.</source>
        <translation>No se pudo abrir el archivo para lectura.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error parsing file.</source>
        <translation>Error interpretando el archivo.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Error while writing file:
%1</source>
        <translation>Error mientras se escribía el archivo:
%1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Json map files (*.json)</source>
        <translation>Archivos de mapa Json (*json)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>JavaScript map files (*.js)</source>
        <translation>Archivos de mapa JavaScript (*.js)</translation>
    </message>
</context>
<context>
    <name>Json::JsonTilesetFormat</name>
    <message>
        <location line="+27"/>
        <source>Could not open file for reading.</source>
        <translation>No se pudo abrir el archivo para lectura.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Error parsing file.</source>
        <translation>Error interpretando el archivo.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error while writing file:
%1</source>
        <translation>Error mientras se escribía el archivo:
%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Json tileset files (*.json)</source>
        <translation>Archivos de conjuto de patrones Json (*.json)</translation>
    </message>
</context>
<context>
    <name>Lua::LuaPlugin</name>
    <message>
        <location filename="../src/plugins/lua/luaplugin.cpp" line="+58"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Lua files (*.lua)</source>
        <translation>Archivos Lua (*.lua)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/tiled/mainwindow.ui" line="+46"/>
        <source>&amp;File</source>
        <translation>&amp;Archivo</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Recent Files</source>
        <translation>Archivos &amp;recientes</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>&amp;Edit</source>
        <translation>&amp;Editar</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Help</source>
        <translation>&amp;Ayuda</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Map</source>
        <translation>&amp;Mapa</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;View</source>
        <translation>&amp;Ver</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Show Object &amp;Names</source>
        <translation>Mostrar &amp;Nombres del Objeto</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Main Toolbar</source>
        <translation>Barra de Herramientas Principal</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Tools</source>
        <translation>Herramientas</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Open...</source>
        <translation>&amp;Abrir...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Save</source>
        <translation>&amp;Guardar</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Quit</source>
        <translation>&amp;Salir</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Copy</source>
        <translation>&amp;Copiar</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Paste</source>
        <translation>&amp;Pegar</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;About Tiled</source>
        <translation>&amp;Acerca de Tiled</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>About Qt</source>
        <translation>Acerca de Qt</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Resize Map...</source>
        <translation>&amp;Redimensionar Mapa...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map &amp;Properties</source>
        <translation>&amp;Atributos del Mapa</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>AutoMap</source>
        <translation>AutoMapa</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A</source>
        <translation>A</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show &amp;Grid</source>
        <translation>Mostrar &amp;Rejilla</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+G</source>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;As...</source>
        <translation>Guardar &amp;como...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;New...</source>
        <translation>&amp;Nuevo...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>New &amp;Tileset...</source>
        <translation>Nuevo Conjunto de &amp;Patrones...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Close</source>
        <translation>&amp;Cerrar</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom In</source>
        <translation>Ampliar</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom Out</source>
        <translation>Reducir</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normal Size</source>
        <translation>Tamaño Normal</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+0</source>
        <translation>Ctrl+0</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Become a Patron</source>
        <translation>Ser un Patrocinador</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save All</source>
        <translation>Guardar Todo</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Documentation</source>
        <translation>Documentación</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Never</source>
        <translation>&amp;Nunca</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>For &amp;Selected Objects</source>
        <translation>Para Objetos &amp;Seleccionados</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>For &amp;All Objects</source>
        <translation>Para &amp;Todos los Objetos</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>AutoMap While Drawing</source>
        <translation>Automapa mientras se dibuja</translation>
    </message>
    <message>
        <location line="-170"/>
        <source>Cu&amp;t</source>
        <translation>Cor&amp;tar</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Offset Map...</source>
        <translation>&amp;Desplazar Mapa...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Offsets everything in a layer</source>
        <translation>Desplazar todo en la capa</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pre&amp;ferences...</source>
        <translation>Pre&amp;ferencias...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Clear Recent Files</source>
        <translation>Limpiar archivos recientes</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Export</source>
        <translation>&amp;Exportar</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+E</source>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <location line="-82"/>
        <source>&amp;Add External Tileset...</source>
        <translation>&amp;Añadir un Conjunto de Patrones Externo...</translation>
    </message>
    <message>
        <location line="-50"/>
        <source>Export As &amp;Image...</source>
        <translation>Exportar como &amp;Imagen...</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>E&amp;xport As...</source>
        <translation>E&amp;xportar como...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+E</source>
        <translation>Ctrl+Shift+E</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Snap to Grid</source>
        <translation>&amp;Ajustar a la Rejilla</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>C&amp;lose All</source>
        <translation>C&amp;errar todo</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+W</source>
        <translation>Ctrl+Shift+W</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Delete</source>
        <translation>&amp;Borrar</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Highlight Current Layer</source>
        <translation>&amp;Destacar la Capa Activa</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H</source>
        <translation>D</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Object &amp;Outlines</source>
        <translation>Mostrar el c&amp;ontorno del Objeto Patrón</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap to &amp;Fine Grid</source>
        <translation>Ajustar a la Rejilla &amp;Fina</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Animations</source>
        <translation>Mostrar Animaciones de Patrones</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Reload</source>
        <translation>Recargar</translation>
    </message>
    <message>
        <location filename="../src/automappingconverter/converterwindow.ui" line="+14"/>
        <source>Tiled Automapping Rule Files Converter</source>
        <translation>Conversor de Archivos de Reglas de Automapeado Tiled</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Add new Automapping rules</source>
        <translation>Añadir nuevas reglas de Automapeado</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+531"/>
        <source>All Files (*)</source>
        <translation>Todos los archivos (*)</translation>
    </message>
</context>
<context>
    <name>MapReader</name>
    <message>
        <location filename="../src/libtiled/mapreader.cpp" line="+140"/>
        <source>Not a map file.</source>
        <translation>No es un archivo de Mapas.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Not a tileset file.</source>
        <translation>No es un archivo con un conjunto de patrones.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Línea %1, columna %2</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File not found: %1</source>
        <translation>Archivo no encontrado: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unable to read file: %1</source>
        <translation>Incapaz de leer archivo: %1</translation>
    </message>
    <message>
        <location line="+32"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+58"/>
        <source>Unsupported map orientation: &quot;%1&quot;</source>
        <translation>Orientación del Mapa no soportada: &quot;%1&quot;</translation>
    </message>
    <message>
        <location line="+102"/>
        <location line="+21"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+124"/>
        <source>Invalid tileset parameters for tileset &apos;%1&apos;</source>
        <translation>Parámetros del conjunto de patrones no válidos para el conjunto de patrones &apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Invalid tile ID: %1</source>
        <translation>ID de Patrón no válido: %1</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>Too many &lt;tile&gt; elements</source>
        <translation>Demasiados elementos &lt;Patrón&gt;</translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+43"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+216"/>
        <source>Invalid tile: %1</source>
        <translation>Patrón no válido: %1</translation>
    </message>
    <message>
        <location line="+29"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+34"/>
        <source>Invalid draw order: %1</source>
        <translation>Orden de dibujado no válido: %1</translation>
    </message>
    <message>
        <location line="+154"/>
        <source>Invalid points data for polygon</source>
        <translation>Datos de los puntos no válidos para el polígono</translation>
    </message>
    <message>
        <location line="-285"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-90"/>
        <source>Unknown encoding: %1</source>
        <translation>Codificación desconocida: %1</translation>
    </message>
    <message>
        <location line="-181"/>
        <source>Error reading embedded image for tile %1</source>
        <translation>Error cargando imagen empotrada para el patrón %1</translation>
    </message>
    <message>
        <location line="+176"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-4"/>
        <source>Compression method &apos;%1&apos; not supported</source>
        <translation>Método de compresión &apos;%1&apos; no soportado</translation>
    </message>
    <message>
        <location line="+58"/>
        <location line="+19"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+15"/>
        <location line="+39"/>
        <source>Corrupt layer data for layer &apos;%1&apos;</source>
        <translation>Datos corruptos para la Capa &apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-28"/>
        <source>Unable to parse tile at (%1,%2) on layer &apos;%3&apos;</source>
        <translation>Incapaz de interpretar patrón en (%1,%2) en la capa &apos;%3&apos;</translation>
    </message>
    <message>
        <location line="-28"/>
        <location line="+44"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+31"/>
        <source>Tile used but no tilesets specified</source>
        <translation>Patrón usado, pero no especificado en el conjunto de patrones</translation>
    </message>
    <message>
        <location filename="../src/libtiled/mapwriter.cpp" line="+115"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-168"/>
        <source>Invalid (negative) tile id: %1</source>
        <translation>ID de patrón no válido (negativo): %1</translation>
    </message>
</context>
<context>
    <name>NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.ui" line="+14"/>
        <source>New Map</source>
        <translation>Nuevo Mapa</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map size</source>
        <translation>Tamaño del Mapa</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+68"/>
        <source>Width:</source>
        <translation>Ancho:</translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+26"/>
        <source> tiles</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> patrones</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+68"/>
        <source>Height:</source>
        <translation>Alto:</translation>
    </message>
    <message>
        <location line="-32"/>
        <source>Tile size</source>
        <translation>Tamaño del Patrón</translation>
    </message>
    <message>
        <location line="+16"/>
        <location line="+26"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> px</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Orientation:</source>
        <translation>Orientación:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Tile layer format:</source>
        <translation>Formato de la capa de Patrones:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile render order:</source>
        <translation>Orden de pintado de Patrones:</translation>
    </message>
</context>
<context>
    <name>NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.ui" line="+14"/>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="+231"/>
        <source>New Tileset</source>
        <translation>Nuevo Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tileset</source>
        <translation>Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Based on Tileset Image</source>
        <translation>Basado en la Imagen del Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Collection of Images</source>
        <translation>Conjunto de Imagenes</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Name:</source>
        <translation>&amp;Nombre:</translation>
    </message>
    <message>
        <location line="+51"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Explorar...</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Use transparent color:</source>
        <translation>Usar color transparente:</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>Tile width:</source>
        <translation>Ancho:</translation>
    </message>
    <message>
        <location line="-100"/>
        <location line="+42"/>
        <location line="+26"/>
        <location line="+16"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> px</translation>
    </message>
    <message>
        <location line="-142"/>
        <source>Image</source>
        <translation>Imagen</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Source:</source>
        <translation>Fuente:</translation>
    </message>
    <message>
        <location line="+101"/>
        <source>The space at the edges of the tileset.</source>
        <translation>El espacio en los bordes del conjunto de patrones.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin:</source>
        <translation>Margen:</translation>
    </message>
    <message>
        <location line="-45"/>
        <source>Tile height:</source>
        <translation>Alto:</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>The space between the tiles.</source>
        <translation>El espacio entre los patrones.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spacing:</source>
        <translation>Espaciado:</translation>
    </message>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-2"/>
        <source>Edit Tileset</source>
        <translation>Editar Conjunto de Patrones</translation>
    </message>
</context>
<context>
    <name>ObjectTypes</name>
    <message>
        <location filename="../src/tiled/objecttypes.cpp" line="+38"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Could not open file.</source>
        <translation>No se pudo abrir el archivo.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>File doesn&apos;t contain object types.</source>
        <translation>El archivo no contiene tipos.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Línea %1, columna %2</translation>
    </message>
</context>
<context>
    <name>OffsetMapDialog</name>
    <message>
        <location filename="../src/tiled/offsetmapdialog.ui" line="+17"/>
        <source>Offset Map</source>
        <translation>Desplazar Mapa</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Offset Contents of Map</source>
        <translation>Desplazar el contenido del Mapa</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <location line="+23"/>
        <location line="+43"/>
        <source>Wrap</source>
        <translation>Cubrir</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Layers:</source>
        <translation>Capas:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>All Visible Layers</source>
        <translation>Todas las capas visibles</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>All Layers</source>
        <translation>Todas las capas</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Selected Layer</source>
        <translation>La capa seleccionada</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bounds:</source>
        <translation>Limites:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Whole Map</source>
        <translation>El Mapa al completo</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Current Selection</source>
        <translation>La selección actual</translation>
    </message>
</context>
<context>
    <name>PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.ui" line="+14"/>
        <source>Become a Patron</source>
        <translation>Ser un Patrocinador</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Visit https://www.patreon.com/bjorn</source>
        <translation>Visita https://www.patreon.com/bjorn</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I&apos;m already a patron!</source>
        <translation>¡Ya soy un patrocinador!</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maybe later</source>
        <translation>Puede que más adelante</translation>
    </message>
</context>
<context>
    <name>PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+14"/>
        <source>Preferences</source>
        <translation>Preferencias</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Saving and Loading</source>
        <translation>Guardando y Cargando</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="-442"/>
        <source>XML</source>
        <translation>XML</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+85"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (uncompressed)</source>
        <translation>Base64 (sin comprimir)</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (gzip compressed)</source>
        <translation>Base64 (con compresión gzip)</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (zlib compressed)</source>
        <translation>Base64 (con compresión zlib)</translation>
    </message>
    <message>
        <location line="-2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>CSV</source>
        <translation>CSV</translation>
    </message>
    <message>
        <location line="+4"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+2"/>
        <source>Right Down</source>
        <translation>Derecha Abajo</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Right Up</source>
        <translation>Derecha Arriba</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Down</source>
        <translation>Izquierda Abajo</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Up</source>
        <translation>Izquierda Arriba</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+6"/>
        <source>&amp;Reload tileset images when they change</source>
        <translation>&amp;Recargar las imagenes con el conjunto de patrones cuando cambien</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Not enabled by default since a reference to an external DTD is known to cause problems with some XML parsers.</source>
        <translation>No activar por defecto, desde que una referencia a un DTD externo es una causa conocida de problemas con algunos parser de XML.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Include &amp;DTD reference in saved maps</source>
        <translation>Incluir la referencia &amp;DTD en los mapas guardados</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Interface</source>
        <translation>Interfaz</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Language:</source>
        <translation>&amp;Idioma:</translation>
    </message>
    <message>
        <location line="-7"/>
        <source>Hardware &amp;accelerated drawing (OpenGL)</source>
        <translation>Pintado &amp;acelerado por Hardware (OpenGL)</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Open last files on startup</source>
        <translation>Abrir los últimos archivos al iniciar</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Grid color:</source>
        <translation>Color de la Malla:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Fine grid divisions:</source>
        <translation>Divisiones de la rejilla fina:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source> pixels</source>
        <translation> pixeles</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Object line width:</source>
        <translation>Ancho de la línea del Objeto:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Object Types</source>
        <translation>Tipos</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Add Object Type</source>
        <translation>Añadir Tipo</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Añadir</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Selected Object Types</source>
        <translation>Eliminar Tipos seleccionados</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Eliminar</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Import...</source>
        <translation>Importar...</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Export...</source>
        <translation>Exportar...</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Plugins</source>
        <translation>Complementos</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enabled Plugins</source>
        <translation>Complementos Habilitados</translation>
    </message>
</context>
<context>
    <name>Python::PythonMapFormat</name>
    <message>
        <location filename="../src/plugins/python/pythonplugin.cpp" line="+268"/>
        <source>-- Using script %1 to read %2</source>
        <translation>-- Usando script %1 para leer %2</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>-- Using script %1 to write %2</source>
        <translation>-- Usando script %1 para escribir %2</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Uncaught exception in script. Please check console.</source>
        <translation>Excepción no atrapada por el script. Por favor compruebe la consola.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Script returned false. Please check console.</source>
        <translation>Script devolvió falso. Por favor compruebe la consola.</translation>
    </message>
</context>
<context>
    <name>Python::PythonPlugin</name>
    <message>
        <location line="-164"/>
        <source>Reloading Python scripts</source>
        <translation>Recargando scripts en Python</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/automappingconverter/convertercontrol.h" line="+33"/>
        <source>v0.8 and before</source>
        <translation>v0.8 y anteriores</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>v0.9 and later</source>
        <translation>v0.9 y posteriores</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>unknown</source>
        <translation>desconocido</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>not a map</source>
        <translation>no es un mapa</translation>
    </message>
</context>
<context>
    <name>QtBoolEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+233"/>
        <location line="+10"/>
        <location line="+25"/>
        <source>True</source>
        <translation>Verdadero</translation>
    </message>
    <message>
        <location line="-25"/>
        <location line="+25"/>
        <source>False</source>
        <translation>Falso</translation>
    </message>
</context>
<context>
    <name>QtBoolPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+1696"/>
        <source>True</source>
        <translation>Verdadero</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>False</source>
        <translation>Falso</translation>
    </message>
</context>
<context>
    <name>QtCharEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qteditorfactory.cpp" line="+1700"/>
        <source>Clear Char</source>
        <translation>Limpiar Caracter</translation>
    </message>
</context>
<context>
    <name>QtColorEditWidget</name>
    <message>
        <location line="+614"/>
        <source>...</source>
        <translation>...</translation>
    </message>
</context>
<context>
    <name>QtColorPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+4724"/>
        <source>Red</source>
        <translation>Rojo</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Green</source>
        <translation>Verde</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Blue</source>
        <translation>Azul</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Alpha</source>
        <translation>Alfa</translation>
    </message>
</context>
<context>
    <name>QtCursorDatabase</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-210"/>
        <source>Arrow</source>
        <translation>Flecha</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Up Arrow</source>
        <translation>Flecha Arriba</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Cross</source>
        <translation>Cruz</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Wait</source>
        <translation>Esperar</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>IBeam</source>
        <translation>IRayo</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Vertical</source>
        <translation>Tamaño Vertical</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Horizontal</source>
        <translation>Tamaño Horizontal</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Backslash</source>
        <translation>Tamaño Oblicuo Inverso</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Slash</source>
        <translation>Tamaño Oblicuo</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size All</source>
        <translation>Tamaño Total</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Blank</source>
        <translation>Blanco</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Vertical</source>
        <translation>Ruptura Vertical</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Horizontal</source>
        <translation>Ruptura Horizontal</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pointing Hand</source>
        <translation>Mano Apuntando</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forbidden</source>
        <translation>Prohibido</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Open Hand</source>
        <translation>Mano Abierta</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Closed Hand</source>
        <translation>Mano Cerrada</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>What&apos;s This</source>
        <translation>Qué es Esto</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Busy</source>
        <translation>Ocupado</translation>
    </message>
</context>
<context>
    <name>QtFontEditWidget</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qteditorfactory.cpp" line="+209"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Select Font</source>
        <translation>Selecciona Fuente de Letras</translation>
    </message>
</context>
<context>
    <name>QtFontPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="-350"/>
        <source>Family</source>
        <translation>Familia</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Point Size</source>
        <translation>Tamaño en Puntos</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bold</source>
        <translation>Negrita</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Italic</source>
        <translation>Itálica</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Underline</source>
        <translation>Subrayado</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Strikeout</source>
        <translation>Tachado</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Kerning</source>
        <translation>Espaciado</translation>
    </message>
</context>
<context>
    <name>QtKeySequenceEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+234"/>
        <source>Clear Shortcut</source>
        <translation>Limpiar Atajo</translation>
    </message>
</context>
<context>
    <name>QtLocalePropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="-3533"/>
        <source>%1, %2</source>
        <translation>%1, %2</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Language</source>
        <translation>Idioma</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Country</source>
        <translation>País</translation>
    </message>
</context>
<context>
    <name>QtPointFPropertyManager</name>
    <message>
        <location line="+409"/>
        <source>(%1, %2)</source>
        <translation>(%1, %2)</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
</context>
<context>
    <name>QtPointPropertyManager</name>
    <message>
        <location line="-319"/>
        <source>(%1, %2)</source>
        <translation>(%1, %2)</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
</context>
<context>
    <name>QtPropertyBrowserUtils</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-141"/>
        <source>[%1, %2, %3] (%4)</source>
        <translation>[%1, %2, %3] (%4)</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>[%1, %2]</source>
        <translation>[%1, %2]</translation>
    </message>
</context>
<context>
    <name>QtRectFPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+1701"/>
        <source>[(%1, %2), %3 x %4]</source>
        <translation>[(%1, %2), %3 x %4]</translation>
    </message>
    <message>
        <location line="+156"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Alto</translation>
    </message>
</context>
<context>
    <name>QtRectPropertyManager</name>
    <message>
        <location line="-611"/>
        <source>[(%1, %2), %3 x %4]</source>
        <translation>[(%1, %2), %3 x %4]</translation>
    </message>
    <message>
        <location line="+120"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Alto</translation>
    </message>
</context>
<context>
    <name>QtSizeFPropertyManager</name>
    <message>
        <location line="-534"/>
        <source>%1 x %2</source>
        <translation>%1 x %2</translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Alto</translation>
    </message>
</context>
<context>
    <name>QtSizePolicyPropertyManager</name>
    <message>
        <location line="+1704"/>
        <location line="+1"/>
        <source>&lt;Invalid&gt;</source>
        <translation>&lt;No Válido&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>[%1, %2, %3, %4]</source>
        <translation>[%1, %2, %3, %4]</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Horizontal Policy</source>
        <translation>Norma Horizontal</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vertical Policy</source>
        <translation>Norma Vertical</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Horizontal Stretch</source>
        <translation>Estirado Horizontal</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Vertical Stretch</source>
        <translation>Estirado Vertical</translation>
    </message>
</context>
<context>
    <name>QtSizePropertyManager</name>
    <message>
        <location line="-2280"/>
        <source>%1 x %2</source>
        <translation>%1 x %2</translation>
    </message>
    <message>
        <location line="+96"/>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Alto</translation>
    </message>
</context>
<context>
    <name>QtTreePropertyBrowser</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qttreepropertybrowser.cpp" line="+478"/>
        <source>Property</source>
        <translation>Atributo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Value</source>
        <translation>Valor</translation>
    </message>
</context>
<context>
    <name>ReplicaIsland::ReplicaIslandPlugin</name>
    <message>
        <location filename="../src/plugins/replicaisland/replicaislandplugin.cpp" line="+59"/>
        <source>Cannot open Replica Island map file!</source>
        <translation>¡No se pudo abrir como un archivo de mapas de Replica Island!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Can&apos;t parse file header!</source>
        <translation>¡No se pudo analizar la cabezera del archivo!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Can&apos;t parse layer header!</source>
        <translation>¡No se pudo analizar la cabezera de la capa!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Inconsistent layer sizes!</source>
        <translation>¡Tamaños de capa inconsistentes!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File ended in middle of layer!</source>
        <translation>¡El archivo finalizó en mitad de la capa!</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Unexpected data at end of file!</source>
        <translation>¡Datos no esperados al final del archivo!</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Replica Island map files (*.bin)</source>
        <translation>Archivos de mapas de Replica Island (*.bin)</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>You must define a background_index property on the map!</source>
        <translation>¡Usted debe definir un atributo background_index en el mapa!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Can&apos;t save non-tile layer!</source>
        <translation>¡No se puede grabar una capa sin patrones!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>You must define a type property on each layer!</source>
        <translation>¡Usted debe definir un tipo de atributo en cada capa!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a tile_index property on each layer!</source>
        <translation>¡Usted debe definir un atributo tile_index en cada capa!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a scroll_speed property on each layer!</source>
        <translation>¡Usted debe definir un atributo scroll_speed en cada capa!</translation>
    </message>
</context>
<context>
    <name>ResizeDialog</name>
    <message>
        <location filename="../src/tiled/resizedialog.ui" line="+14"/>
        <source>Resize</source>
        <translation>Redimensionar</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Size</source>
        <translation>Tamaño</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Width:</source>
        <translation>Ancho:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Height:</source>
        <translation>Alto:</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Offset</source>
        <translation>Desplazamiento</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
</context>
<context>
    <name>Tengine::TenginePlugin</name>
    <message>
        <location filename="../src/plugins/tengine/tengineplugin.cpp" line="+49"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+246"/>
        <source>T-Engine4 map files (*.lua)</source>
        <translation>Archivos de mapa T-Engine4 (*.lua)</translation>
    </message>
</context>
<context>
    <name>TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.ui" line="+14"/>
        <source>Tile Animation Editor</source>
        <translation>Editor de Animaciones de Patrón</translation>
    </message>
    <message>
        <location line="+99"/>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="+523"/>
        <source>Preview</source>
        <translation>Previsualizar</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractObjectTool</name>
    <message numerus="yes">
        <location filename="../src/tiled/abstractobjecttool.cpp" line="+182"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Duplicar Objeto</numerusform>
            <numerusform>Duplicar %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+2"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Eliminar Objeto</numerusform>
            <numerusform>Eliminar %n Objetos</numerusform>
        </translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Flip Horizontally</source>
        <translation>Voltear Horizontalmente</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Flip Vertically</source>
        <translation>Voltear Verticalmente</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Raise Object</source>
        <translation>Subir Objeto</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgUp</source>
        <translation>RePág</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object</source>
        <translation>Bajar Objeto</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgDown</source>
        <translation>AvPág</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Object to Top</source>
        <translation>Subir Objeto a la Cima</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Home</source>
        <translation>Inicio</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object to Bottom</source>
        <translation>Bajar Objeto al Fondo</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>End</source>
        <translation>Fin</translation>
    </message>
    <message numerus="yes">
        <location line="+5"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Mover Objeto a la Capa</numerusform>
            <numerusform>Mover %n Objetos a la Capa</numerusform>
        </translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Object &amp;Properties...</source>
        <translation>&amp;Atributos del Objeto...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractTileTool</name>
    <message>
        <location filename="../src/tiled/abstracttiletool.cpp" line="+124"/>
        <source>empty</source>
        <translation>vacía</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutoMapper</name>
    <message>
        <location filename="../src/tiled/automapper.cpp" line="+115"/>
        <source>&apos;%1&apos;: Property &apos;%2&apos; = &apos;%3&apos; does not make sense. Ignoring this property.</source>
        <translation>&apos;%1&apos;: Atributo &apos;%2&apos; = &apos;%3&apos; no tiene sentido. Ignorando este atributo.</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Did you forget an underscore in layer &apos;%1&apos;?</source>
        <translation>¿Usted olvidó un guión bajo en la capa &apos;%1&apos;?</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Layer &apos;%1&apos; is not recognized as a valid layer for Automapping.</source>
        <translation>La capa &apos;%1&apos; no se reconoció como una válida para Automapeado.</translation>
    </message>
    <message>
        <location line="-105"/>
        <source>&apos;regions_input&apos; layer must not occur more than once.</source>
        <translation>La capa &apos;regions_input&apos; no debe aparecer más de una vez.</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+13"/>
        <source>&apos;regions_*&apos; layers must be tile layers.</source>
        <translation>Las capas &apos;regions_*&apos; deben ser capas de patrones.</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>&apos;regions_output&apos; layer must not occur more than once.</source>
        <translation>La capa &apos;regions_output&apos; no debe aparecer más de una vez.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>&apos;input_*&apos; and &apos;inputnot_*&apos; layers must be tile layers.</source>
        <translation>Las capas &apos;input_*&apos; y &apos;inputnot_*&apos; deben ser capas de patrones.</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>No &apos;regions&apos; or &apos;regions_input&apos; layer found.</source>
        <translation>No se encontró la capa &apos;regions&apos; ó &apos;regions_input&apos;.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No &apos;regions&apos; or &apos;regions_output&apos; layer found.</source>
        <translation>No se encontró la capa &apos;regions&apos; ó &apos;regions_output&apos;.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No input_&lt;name&gt; layer found!</source>
        <translation>¡No se encontró la capa input_&lt;name&gt;!</translation>
    </message>
    <message>
        <location line="+165"/>
        <source>Tile</source>
        <translation>Patrón</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutomappingManager</name>
    <message>
        <location filename="../src/tiled/automappingmanager.cpp" line="+103"/>
        <source>Apply AutoMap rules</source>
        <translation>Aplicar reglas de AutoMapa</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>No rules file found at:
%1</source>
        <translation>No se encontró el archivo de reglas en:
%1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Error opening rules file:
%1</source>
        <translation>Error abriendo el archivo de reglas:
%1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>File not found:
%1</source>
        <translation>Archivo no encontrado:
%1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Opening rules map failed:
%1</source>
        <translation>Fallo la carga de las reglas del mapa:
%1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksModel</name>
    <message>
        <location filename="../src/tiled/brokenlinks.cpp" line="+144"/>
        <source>Tileset image</source>
        <translation>Imagen del conjunto de patrones</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tileset</source>
        <translation>Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tile image</source>
        <translation>Imagen de Patrón</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File name</source>
        <translation>Nombre del archivo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Location</source>
        <translation>Localización</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksWidget</name>
    <message>
        <location line="+66"/>
        <source>Some files could not be found</source>
        <translation>Algunos archivos no pudieron ser encontrados</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>One or more files referenced by the map could not be found. You can help locate them below.</source>
        <translation>Uno o más archivos referenciados por el mapa no pudieron ser encontrados. Puedes ayudar a localizarlos abajo.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Locate File...</source>
        <translation>Localizar archivo...</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Locate File</source>
        <translation>Localizar archivo</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Loading Image</source>
        <translation>Error al cargar la imagen</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>All Files (*)</source>
        <translation>Todos los archivos (*)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Locate External Tileset</source>
        <translation>Localizar Conjunto de Patrones Externo</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Reading Tileset</source>
        <translation>Error al leer Conjunto de Patrones</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BucketFillTool</name>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="+40"/>
        <location line="+194"/>
        <source>Bucket Fill Tool</source>
        <translation>Herramienta de Rellenado</translation>
    </message>
    <message>
        <location line="-191"/>
        <location line="+192"/>
        <source>F</source>
        <translation>R</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ClipboardManager</name>
    <message>
        <location filename="../src/tiled/clipboardmanager.cpp" line="+166"/>
        <source>Paste Objects</source>
        <translation>Pegar Objetos</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandButton</name>
    <message>
        <location filename="../src/tiled/commandbutton.cpp" line="+130"/>
        <source>Execute Command</source>
        <translation>Ejecutar orden</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location line="-67"/>
        <source>Error Executing Command</source>
        <translation>Error ejecutando orden</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>You do not have any commands setup.</source>
        <translation>Usted no tiene ninguna orden preparada.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Edit commands...</source>
        <translation>Editar ordenes...</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Edit Commands...</source>
        <translation>Editar Ordenes...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDataModel</name>
    <message>
        <location filename="../src/tiled/commanddatamodel.cpp" line="+60"/>
        <source>Open in text editor</source>
        <translation>Abrir en el editor de texto</translation>
    </message>
    <message>
        <location line="+91"/>
        <location line="+69"/>
        <source>&lt;new command&gt;</source>
        <translation>&lt;Nueva Orden&gt;</translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Set a name for this command</source>
        <translation>Establecer un nombre para esta orden</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set the shell command to execute</source>
        <translation>Establecer la orden de consola a ejecutar</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Show or hide this command in the command list</source>
        <translation>Mostrar u ocultar esta orden en la lista de ordenes</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add a new command</source>
        <translation>Añadir una nueva orden</translation>
    </message>
    <message>
        <location line="+107"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Command</source>
        <translation>Orden</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable</source>
        <translation>Activar</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Move Up</source>
        <translation>Subir</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Move Down</source>
        <translation>Bajar</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Execute</source>
        <translation>Ejecutar</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Execute in Terminal</source>
        <translation>Ejecutar en la Consola</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location line="+84"/>
        <source>%1 (copy)</source>
        <translation>%1 (copiar)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>New command</source>
        <translation>Nueva orden</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.cpp" line="+44"/>
        <source>Edit Commands</source>
        <translation>Editar Ordenes</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandProcess</name>
    <message>
        <location filename="../src/tiled/command.cpp" line="+132"/>
        <source>Unable to create/open %1</source>
        <translation>No es posible crear ó abrir %1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unable to add executable permissions to %1</source>
        <translation>No es posible dar permiso de ejecución a %1</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>The command failed to start.</source>
        <translation>La orden fue incapaz de iniciarse.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command crashed.</source>
        <translation>La orden se bloqueó.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command timed out.</source>
        <translation>La orden agotó el tiempo.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>An unknown error occurred.</source>
        <translation>Ocurrió un error desconocido.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Error Executing %1</source>
        <translation>Error ejecutando %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ConsoleDock</name>
    <message>
        <location filename="../src/tiled/consoledock.cpp" line="+36"/>
        <source>Debug Console</source>
        <translation>Consola de Depuración</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateEllipseObjectTool</name>
    <message>
        <location filename="../src/tiled/createellipseobjecttool.cpp" line="+39"/>
        <source>Insert Ellipse</source>
        <translation>Insertar Elipse</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>C</source>
        <translation>C</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateObjectTool</name>
    <message>
        <location filename="../src/tiled/createobjecttool.cpp" line="+46"/>
        <source>O</source>
        <translation>O</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreatePolygonObjectTool</name>
    <message>
        <location filename="../src/tiled/createpolygonobjecttool.cpp" line="+39"/>
        <source>Insert Polygon</source>
        <translation>Insertar Polígono</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>P</source>
        <translation>P</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreatePolylineObjectTool</name>
    <message>
        <location filename="../src/tiled/createpolylineobjecttool.cpp" line="+39"/>
        <source>Insert Polyline</source>
        <translation>Insertar Polilínea</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>L</source>
        <translation>L</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateRectangleObjectTool</name>
    <message>
        <location filename="../src/tiled/createrectangleobjecttool.cpp" line="+39"/>
        <source>Insert Rectangle</source>
        <translation>Insertar Rectángulo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R</source>
        <translation>R</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateTileObjectTool</name>
    <message>
        <location filename="../src/tiled/createtileobjecttool.cpp" line="+79"/>
        <source>Insert Tile</source>
        <translation>Insertar Patrón</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>T</source>
        <translation>T</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::DocumentManager</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="+369"/>
        <source>%1:

%2</source>
        <translation>%1:

%2</translation>
    </message>
    <message>
        <location line="+250"/>
        <source>Tileset Columns Changed</source>
        <translation>Columnas Cambiadas en Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The number of tile columns in the tileset &apos;%1&apos; appears to have changed from %2 to %3. Do you want to adjust tile references?</source>
        <translation>El número de columnas de patrones en el conjunto &apos;%1&apos; parece haber sido cambiado de %2 a %3. ¿Deseas ajustar las referencias de los patrones?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditPolygonTool</name>
    <message>
        <location filename="../src/tiled/editpolygontool.cpp" line="+129"/>
        <location line="+209"/>
        <source>Edit Polygons</source>
        <translation>Editar Polígonos</translation>
    </message>
    <message>
        <location line="-207"/>
        <location line="+208"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message numerus="yes">
        <location line="+227"/>
        <source>Move %n Point(s)</source>
        <translation>
            <numerusform>Mover %n Punto</numerusform>
            <numerusform>Mover %n Puntos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+26"/>
        <location line="+45"/>
        <source>Delete %n Node(s)</source>
        <translation>
            <numerusform>Borrar %n Nodo</numerusform>
            <numerusform>Borrar %n Nodos</numerusform>
        </translation>
    </message>
    <message>
        <location line="-40"/>
        <location line="+215"/>
        <source>Join Nodes</source>
        <translation>Unir Nodos</translation>
    </message>
    <message>
        <location line="-214"/>
        <location line="+250"/>
        <source>Split Segments</source>
        <translation>Separar Segmentos</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="+149"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>New Terrain</source>
        <translation>Nuevo Terreno</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::Eraser</name>
    <message>
        <location filename="../src/tiled/eraser.cpp" line="+35"/>
        <location line="+36"/>
        <source>Eraser</source>
        <translation>Goma</translation>
    </message>
    <message>
        <location line="-33"/>
        <location line="+34"/>
        <source>E</source>
        <translation>G</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.cpp" line="+63"/>
        <source>Export</source>
        <translation>Exportar</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Export as Image</source>
        <translation>Exportar como Imagen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 already exists.
Do you want to replace it?</source>
        <translation>%1 ya existe.
¿Quiere reemplazarla?</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Out of Memory</source>
        <translation>Memoria agotada</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not allocate sufficient memory for the image. Try reducing the zoom level or using a 64-bit version of Tiled.</source>
        <translation>No se pudo reservar suficiente memoria para la imagen. Intenta reducir el nivel de zoom ó usa la versión de 64 bits de Tiled.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image too Big</source>
        <translation>Imagen demasiado grande</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The resulting image would be %1 x %2 pixels and take %3 GB of memory. Tiled is unable to create such an image. Try reducing the zoom level.</source>
        <translation>La imagen resultante sería de %1 x %2 pixeles y ocuparía %3 GBs de memoria. Tiled es incapaz de crear una imagen de tales dimensiones. Intenta reducir el nivel de zoom.</translation>
    </message>
    <message>
        <location line="+100"/>
        <source>Image</source>
        <translation>Imagen</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileChangedWarning</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="-553"/>
        <source>File change detected. Discard changes and reload the map?</source>
        <translation>Cambio en archivo detectado. Descartar los cambios y recargar el mapa?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileEdit</name>
    <message>
        <location filename="../src/tiled/fileedit.cpp" line="+113"/>
        <source>Choose a File</source>
        <translation>Elegir un Archivo</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerDock</name>
    <message>
        <location filename="../src/tiled/layerdock.cpp" line="+218"/>
        <source>Layers</source>
        <translation>Capas</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Opacity:</source>
        <translation>Transparencia:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerModel</name>
    <message>
        <location filename="../src/tiled/layermodel.cpp" line="+151"/>
        <source>Layer</source>
        <translation>Capa</translation>
    </message>
    <message>
        <location line="+160"/>
        <source>Show Other Layers</source>
        <translation>Mostrar Otras Capas</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Hide Other Layers</source>
        <translation>Ocultar Otras Capas</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerOffsetTool</name>
    <message>
        <location filename="../src/tiled/layeroffsettool.cpp" line="+36"/>
        <location line="+94"/>
        <source>Offset Layers</source>
        <translation>Desplazar Capas</translation>
    </message>
    <message>
        <location line="-92"/>
        <location line="+93"/>
        <source>M</source>
        <translation>M</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MagicWandTool</name>
    <message>
        <location filename="../src/tiled/magicwandtool.cpp" line="+39"/>
        <location line="+52"/>
        <source>Magic Wand</source>
        <translation>Varita Mágica</translation>
    </message>
    <message>
        <location line="-49"/>
        <location line="+50"/>
        <source>W</source>
        <translation>W</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MainWindow</name>
    <message>
        <location filename="../src/tiled/mainwindow.cpp" line="+232"/>
        <location line="+11"/>
        <source>Undo</source>
        <translation>Deshacer</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+9"/>
        <source>Redo</source>
        <translation>Rehacer</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ctrl+=</source>
        <translation>Ctrl+=</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>+</source>
        <translation>+</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location line="+23"/>
        <location line="+1399"/>
        <source>Random Mode</source>
        <translation>Modo Aleatorio</translation>
    </message>
    <message>
        <location line="-1396"/>
        <source>D</source>
        <translation>D</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+1394"/>
        <source>&amp;Layer</source>
        <translation>&amp;Capa</translation>
    </message>
    <message>
        <location line="-1216"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Ctrl+Shift+Tab</source>
        <translation>Ctrl+Shift+Tab</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Ctrl+Tab</source>
        <translation>Ctrl+Tab</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Shift+Z</source>
        <translation>Shift+Z</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Error Opening Map</source>
        <translation>Error Abriendo Mapa</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+181"/>
        <location line="+334"/>
        <source>All Files (*)</source>
        <translation>Todos los ficheros (*)</translation>
    </message>
    <message>
        <location line="-504"/>
        <source>Open Map</source>
        <translation>Abrir Mapa</translation>
    </message>
    <message>
        <location line="+25"/>
        <location line="+73"/>
        <source>Error Saving Map</source>
        <translation>Error Guardando Mapa</translation>
    </message>
    <message>
        <location line="-31"/>
        <source>untitled.tmx</source>
        <translation>untitled.tmx</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Unsaved Changes</source>
        <translation>Cambios sin guardar</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>There are unsaved changes. Do you want to save now?</source>
        <translation>Hay cambios sin guardar. ¿Desea guardarlos ahora?</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Exported to %1</source>
        <translation>Exportado a %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+117"/>
        <source>Error Exporting Map</source>
        <translation>Error Exportando Mapa</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Export As...</source>
        <translation>Exportar como...</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Non-unique file extension</source>
        <translation>Extensión de archivo no única</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Non-unique file extension.
Please select specific format.</source>
        <translation>Extensión de archivo no única.
Por favor seleccione un formato específico.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Unknown File Format</source>
        <translation>Formato de archivo desconocido</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The given filename does not have any known file extension.</source>
        <translation>El nombre de fichero dado tiene una extensión desconocida.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Some export files already exist:</source>
        <translation>Algunos archivos a exportar ya existen:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Do you want to replace them?</source>
        <translation>¿Quiere reemplazarlos?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Overwrite Files</source>
        <translation>Sobrescribir Archivos</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Cut</source>
        <translation>Cortar</translation>
    </message>
    <message>
        <location line="+587"/>
        <source>[*]%1</source>
        <translation>[*]%1</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Error Reloading Map</source>
        <translation>Error Recargando el Mapa</translation>
    </message>
    <message>
        <location line="-648"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location line="+133"/>
        <location line="+5"/>
        <source>Error Reading Tileset</source>
        <translation>Error Leyendo Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+77"/>
        <source>Automatic Mapping Warning</source>
        <translation>Advertencia de Mapeado Automático</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Automatic Mapping Error</source>
        <translation>Error de Mapeado Automático</translation>
    </message>
    <message>
        <location line="-886"/>
        <location line="+1222"/>
        <source>Views and Toolbars</source>
        <translation>Vistas y Barras de Herramientas</translation>
    </message>
    <message>
        <location line="-1221"/>
        <location line="+1222"/>
        <source>Tile Animation Editor</source>
        <translation>Editor de Animaciones de Patrón</translation>
    </message>
    <message>
        <location line="-1220"/>
        <location line="+1221"/>
        <source>Tile Collision Editor</source>
        <translation>Editor de Colisiones de Patrón</translation>
    </message>
    <message>
        <location line="-1192"/>
        <source>Alt+Left</source>
        <translation>Alt+Izquierda</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Alt+Right</source>
        <translation>Alt+Derecha</translation>
    </message>
    <message>
        <location line="+756"/>
        <source>Add External Tileset(s)</source>
        <translation>Añadir Conjunto(s) de Patrones Externos</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>%1: %2</source>
        <translation>%1: %2</translation>
    </message>
    <message numerus="yes">
        <location line="+10"/>
        <source>Add %n Tileset(s)</source>
        <translation>
            <numerusform>Añadir Conjunto de Patrones</numerusform>
            <numerusform>Añadir %n Conjuntos de Patrones</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocument</name>
    <message>
        <location filename="../src/tiled/mapdocument.cpp" line="+242"/>
        <source>untitled.tmx</source>
        <translation>untitled.tmx</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>Resize Map</source>
        <translation>Redimensionar Mapa</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Offset Map</source>
        <translation>Desplazar Mapa</translation>
    </message>
    <message numerus="yes">
        <location line="+28"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Rotar Objeto</numerusform>
            <numerusform>Rotar %n Objetos</numerusform>
        </translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Tile Layer %1</source>
        <translation>Capa de patrones %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Object Layer %1</source>
        <translation>Capa de Objetos %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Image Layer %1</source>
        <translation>Capa de Imagen %1</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Copy of %1</source>
        <translation>Copiar desde %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Duplicate Layer</source>
        <translation>Duplicar Capa</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Merge Layer Down</source>
        <translation>Mezclar capa hacía abajo</translation>
    </message>
    <message>
        <location line="+238"/>
        <source>Tile</source>
        <translation>Patrón</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tileset Changes</source>
        <translation>Cambios en el Conjunto de Patrones</translation>
    </message>
    <message numerus="yes">
        <location line="+189"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Duplicar Objeto</numerusform>
            <numerusform>Duplicar %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+20"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Eliminar Objeto</numerusform>
            <numerusform>Eliminar %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+12"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Mover Objeto a la Capa</numerusform>
            <numerusform>Mover %n Objetos a la Capa</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocumentActionHandler</name>
    <message>
        <location filename="../src/tiled/mapdocumentactionhandler.cpp" line="+55"/>
        <source>Ctrl+Shift+A</source>
        <translation>Ctrl+Shift+A</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Ctrl+Shift+D</source>
        <translation>Ctrl+Shift+D</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Ctrl+Shift+Up</source>
        <translation>Ctrl+Shift+Up</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+Down</source>
        <translation>Ctrl+Shift+Down</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+H</source>
        <translation>Ctrl+Shift+H</translation>
    </message>
    <message>
        <location line="+58"/>
        <source>Select &amp;All</source>
        <translation>Seleccionar &amp;Todo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;None</source>
        <translation>Seleccionar &amp;Nada</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Crop to Selection</source>
        <translation>&amp;Recortar la Selección</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add &amp;Tile Layer</source>
        <translation>Añadir Capa de &amp;Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add &amp;Object Layer</source>
        <translation>Añadir Capa de &amp;Objetos</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Select Pre&amp;vious Layer</source>
        <translation>Seleccionar la Capa Pre&amp;via</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;Next Layer</source>
        <translation>Seleccionar la &amp;Próxima Capa</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R&amp;aise Layer</source>
        <translation>S&amp;ubir Capa</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Lower Layer</source>
        <translation>&amp;Bajar Capa</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show/&amp;Hide all Other Layers</source>
        <translation>Mostrar/&amp;Ocultar el Resto de Capas</translation>
    </message>
    <message numerus="yes">
        <location line="+248"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Duplicar Objeto</numerusform>
            <numerusform>Duplicar %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+1"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Eliminar Objeto</numerusform>
            <numerusform>Eliminar %n Objetos</numerusform>
        </translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Duplicate Objects</source>
        <translation>Duplicar Objetos</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Objects</source>
        <translation>Eliminar Objetos</translation>
    </message>
    <message>
        <location line="-259"/>
        <source>&amp;Duplicate Layer</source>
        <translation>&amp;Duplicar Capa</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Ctrl+PgUp</source>
        <translation>Ctrl+RePág</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgDown</source>
        <translation>Ctrl+AvPág</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Add &amp;Image Layer</source>
        <translation>Añadir Capa de &amp;Imagen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&amp;Merge Layer Down</source>
        <translation>&amp;Mezclar Capas hacía Abajo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Layer</source>
        <translation>&amp;Eliminar Capa</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Layer &amp;Properties...</source>
        <translation>&amp;Atributos de la Capa...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapObjectModel</name>
    <message>
        <location filename="../src/tiled/mapobjectmodel.cpp" line="+152"/>
        <source>Change Object Name</source>
        <translation>Cambiar el Nombre del Objeto</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Change Object Type</source>
        <translation>Cambiar el Tipo del Objeto</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapsDock</name>
    <message>
        <location filename="../src/tiled/mapsdock.cpp" line="+83"/>
        <source>Browse...</source>
        <translation>Explorar...</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Choose the Maps Folder</source>
        <translation>Elegir la Carpeta de Mapas</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Maps</source>
        <translation>Mapas</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MiniMapDock</name>
    <message>
        <location filename="../src/tiled/minimapdock.cpp" line="+60"/>
        <source>Mini-map</source>
        <translation>Mini-mapa</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="-14"/>
        <source>Orthogonal</source>
        <translation>Ortogonal</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric</source>
        <translation>Isométrico</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric (Staggered)</source>
        <translation>Isométrico (Escalonado)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Hexagonal (Staggered)</source>
        <translation>Hexagonal (Escalonado)</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Tile Layer 1</source>
        <translation>Capa de Patrones 1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Memory Usage Warning</source>
        <translation>Advertencia por el Uso de Memoria</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile layers for this map will consume %L1 GB of memory each. Not creating one by default.</source>
        <translation>Cada capa de patrones para este mapa consumirá %L1 GB de memoria. Por defecto no crear ninguna.</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>%1 x %2 pixels</source>
        <translation>%1 x %2 pixels</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-38"/>
        <location line="+7"/>
        <source>Error</source>
        <translation>Error</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Failed to load tileset image &apos;%1&apos;.</source>
        <translation>Falló al cargar la imagen de conjunto de patrones &apos;%1&apos;.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No tiles found in the tileset image when using the given tile size, margin and spacing!</source>
        <translation>¡No se encontraron patrones en la imagen del conjunto de patrones usando las dimensiones, margen y espaciado de patrones suministradas!</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>Tileset Image</source>
        <translation>Imagen con el Conjunto de Patrones</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectSelectionTool</name>
    <message>
        <location filename="../src/tiled/objectselectiontool.cpp" line="+316"/>
        <location line="+302"/>
        <source>Select Objects</source>
        <translation>Seleccionar Objetos</translation>
    </message>
    <message>
        <location line="-300"/>
        <location line="+301"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message numerus="yes">
        <location line="-190"/>
        <location line="+548"/>
        <source>Move %n Object(s)</source>
        <translation>
            <numerusform>Mover Objeto</numerusform>
            <numerusform>Mover %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+67"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Rotar Objeto</numerusform>
            <numerusform>Rotar %n Objetos</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+266"/>
        <source>Resize %n Object(s)</source>
        <translation>
            <numerusform>Redimensionar Objeto</numerusform>
            <numerusform>Redimensionar %n Objetos</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesModel</name>
    <message>
        <location filename="../src/tiled/objecttypesmodel.cpp" line="+51"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Color</source>
        <translation>Color</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectsDock</name>
    <message>
        <location filename="../src/tiled/objectsdock.cpp" line="+145"/>
        <source>Object Properties</source>
        <translation>Atributos de Objeto</translation>
    </message>
    <message>
        <location line="-1"/>
        <source>Add Object Layer</source>
        <translation>Añadir capa de Objetos</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Objects</source>
        <translation>Objetos</translation>
    </message>
    <message numerus="yes">
        <location line="+17"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Mover Objeto a la Capa</numerusform>
            <numerusform>Mover %n Objetos a la Capa</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.cpp" line="+66"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;Thank you for support!&lt;/h3&gt;
&lt;p&gt;Your support as a patron makes a big difference to me as the main developer and maintainer of Tiled. It allows me to spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Keep an eye out for exclusive updates in the Activity feed on my Patreon page to find out what I&apos;ve been up to in the time I could spend on Tiled thanks to your support!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;¡Gracias por tu apoyo!&lt;/h3&gt;
&lt;p&gt;Tu apoyo como patrocinador hace una gran diferencia para mí, el principal desarrollador e mantenedor de Tiled. Me permite gastar menos tiempo trabajando por dinero en otras cosas y usar más de tiempo para trabajar en Tiled.&lt;/p&gt;
&lt;p&gt;Está atento a las actualizaciones exclusivas en el Activity feed de mi página de Patreon ¡y así verás en lo que he estado usando el tiempo que he podido dedicar a Tiled gracias a tu apoyo!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m no longer a patron</source>
        <translation>Ya no soy más un patrocinador</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;With your help I can continue to improve Tiled!&lt;/h3&gt;
&lt;p&gt;Please consider supporting me as a patron. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;¡Con tu ayuda puedo continuar mejorando Tiled!&lt;/h3&gt;
&lt;p&gt;Considera mostrar tu apoyo como mi patrocinador. Tu apoyo haría una gran diferencia para mí, el desarrollador princiapl e mantenedor de Tiled. Podría gastar menos tiempo trabajando por dinero en otras cosas y usar más de tiempo para trabajar en Tiled.&lt;/p&gt;
&lt;p&gt;Toda aportación ayuda. Tiled tiene muchos usuarios y si cada uno de ellos hiciera una pequeña donación al mes, yo tendría más tiempo para seguir mejorando Tiled.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m already a patron!</source>
        <translation>¡Ya soy un patrocinador!</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="+123"/>
        <location line="+74"/>
        <source>System default</source>
        <translation>Configuración por defecto</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Import Object Types</source>
        <translation>Importar Tipos</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+29"/>
        <source>Object Types files (*.xml)</source>
        <translation>Archivos de Tipos (*.xml)</translation>
    </message>
    <message>
        <location line="-16"/>
        <source>Error Reading Object Types</source>
        <translation>Error Leyendo Tipos</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Export Object Types</source>
        <translation>Exportar Tipos</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Error Writing Object Types</source>
        <translation>Error Escribiendo Tipos</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertiesDock</name>
    <message>
        <location filename="../src/tiled/propertiesdock.cpp" line="+198"/>
        <location line="+52"/>
        <source>Name:</source>
        <translation>Nombre:</translation>
    </message>
    <message>
        <location line="-51"/>
        <location line="+103"/>
        <source>Add Property</source>
        <translation>Añadir Atributo</translation>
    </message>
    <message>
        <location line="-50"/>
        <location line="+52"/>
        <source>Rename Property</source>
        <translation>Renombrar Atributo</translation>
    </message>
    <message>
        <location line="-4"/>
        <source>Properties</source>
        <translation>Atributos</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove Property</source>
        <translation>Eliminar Atributo</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertyBrowser</name>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+13"/>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertical</source>
        <translation>Vertical</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Top Down</source>
        <translation>De Arriba hacía Abajo</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>Manual</translation>
    </message>
    <message>
        <location line="+438"/>
        <source>Columns</source>
        <translation>Columnas</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Source</source>
        <translation>Fuente</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Relative chance this tile will be picked</source>
        <translation>Probabilidad relativa de que este patrón será escogido</translation>
    </message>
    <message>
        <location line="+302"/>
        <source>Error Reading Tileset</source>
        <translation>Error al leer Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+111"/>
        <source>Custom Properties</source>
        <translation>Atributos personalizados</translation>
    </message>
    <message>
        <location line="-621"/>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer Format</source>
        <translation>Formato de la Capa de Patrones</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Render Order</source>
        <translation>Orden de Pintado del Patrón</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Background Color</source>
        <translation>Color de Fondo</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Object</source>
        <translation>Objeto</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+26"/>
        <location line="+74"/>
        <location line="+60"/>
        <source>Name</source>
        <translation>Nombre</translation>
    </message>
    <message>
        <location line="-157"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+21"/>
        <source>Visible</source>
        <translation>Visible</translation>
    </message>
    <message>
        <location line="-388"/>
        <location line="+368"/>
        <location line="+71"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="-438"/>
        <location line="+368"/>
        <location line="+71"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="-437"/>
        <source>Odd</source>
        <translation>Impar</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Even</source>
        <translation>Par</translation>
    </message>
    <message>
        <location line="+296"/>
        <source>Orientation</source>
        <translation>Orientación</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+65"/>
        <location line="+125"/>
        <source>Width</source>
        <translation>Ancho</translation>
    </message>
    <message>
        <location line="-189"/>
        <location line="+65"/>
        <location line="+125"/>
        <source>Height</source>
        <translation>Alto</translation>
    </message>
    <message>
        <location line="-189"/>
        <location line="+166"/>
        <source>Tile Width</source>
        <translation>Ancho Patrón</translation>
    </message>
    <message>
        <location line="-165"/>
        <location line="+166"/>
        <source>Tile Height</source>
        <translation>Alto Patrón</translation>
    </message>
    <message>
        <location line="-164"/>
        <source>Tile Side Length (Hex)</source>
        <translation>Longitud Lateral del Patrón (Hex)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Stagger Axis</source>
        <translation>Eje de Escalonado</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Stagger Index</source>
        <translation>Índice de Escalonado</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Rotation</source>
        <translation>Rotación</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Flipping</source>
        <translation>Volteado</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Opacity</source>
        <translation>Opacidad</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+9"/>
        <source>Horizontal Offset</source>
        <translation>Desplazamiento Horizontal</translation>
    </message>
    <message>
        <location line="-8"/>
        <location line="+9"/>
        <source>Vertical Offset</source>
        <translation>Desplazamiento Vertical</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Tile Layer</source>
        <translation>Capa de Patrones</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Object Layer</source>
        <translation>Capa de Objetos</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Color</source>
        <translation>Color</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Drawing Order</source>
        <translation>Orden de Dibujado</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Image Layer</source>
        <translation>Capa de Imagen</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+37"/>
        <location line="+39"/>
        <source>Image</source>
        <translation>Imagen</translation>
    </message>
    <message>
        <location line="-71"/>
        <location line="+39"/>
        <source>Transparent Color</source>
        <translation>Color Transparente</translation>
    </message>
    <message>
        <location line="-29"/>
        <source>Tileset</source>
        <translation>Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Filename</source>
        <translation>Nombre de archivo</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Drawing Offset</source>
        <translation>Desplazamiento</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Margin</source>
        <translation>Margen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spacing</source>
        <translation>Espaciado</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Tile</source>
        <translation>Patrón</translation>
    </message>
    <message>
        <location line="-133"/>
        <location line="+134"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Probability</source>
        <translation>Probabilidad</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Terrain</source>
        <translation>Terreno</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::SelectSameTileTool</name>
    <message>
        <location filename="../src/tiled/selectsametiletool.cpp" line="+33"/>
        <location line="+56"/>
        <source>Select Same Tile</source>
        <translation>Seleccionar el Mismo Patrón</translation>
    </message>
    <message>
        <location line="-53"/>
        <location line="+54"/>
        <source>S</source>
        <translation>S</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::StampBrush</name>
    <message>
        <location filename="../src/tiled/stampbrush.cpp" line="+41"/>
        <location line="+128"/>
        <source>Stamp Brush</source>
        <translation>Brocha de Estampar</translation>
    </message>
    <message>
        <location line="-125"/>
        <location line="+126"/>
        <source>B</source>
        <translation>B</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainBrush</name>
    <message>
        <location filename="../src/tiled/terrainbrush.cpp" line="+45"/>
        <location line="+115"/>
        <source>Terrain Brush</source>
        <translation>Brocha de Terreno</translation>
    </message>
    <message>
        <location line="-112"/>
        <location line="+113"/>
        <source>T</source>
        <translation>T</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainDock</name>
    <message>
        <location filename="../src/tiled/terraindock.cpp" line="+174"/>
        <source>Terrains</source>
        <translation>Terrenos</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainView</name>
    <message>
        <location filename="../src/tiled/terrainview.cpp" line="+97"/>
        <source>Terrain &amp;Properties...</source>
        <translation>&amp;Atributos de Terreno...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="-58"/>
        <source>Delete Frames</source>
        <translation>Borrar Marcos</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileCollisionEditor</name>
    <message>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+351"/>
        <source>Delete</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cut</source>
        <translation>Cortar</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Tile Collision Editor</source>
        <translation>Editor de Colisiones de Patrón</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileSelectionTool</name>
    <message>
        <location filename="../src/tiled/tileselectiontool.cpp" line="+34"/>
        <location line="+81"/>
        <source>Rectangular Select</source>
        <translation>Selección Rectangular</translation>
    </message>
    <message>
        <location line="-78"/>
        <location line="+79"/>
        <source>R</source>
        <translation>R</translation>
    </message>
    <message>
        <location line="-56"/>
        <source>%1, %2 - Rectangle: (%3 x %4)</source>
        <translation>%1, %2 - Rectángulo: (%3 x %4)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampModel</name>
    <message>
        <location filename="../src/tiled/tilestampmodel.cpp" line="+78"/>
        <source>Stamp</source>
        <translation>Sello</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Probability</source>
        <translation>Probabilidad</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampsDock</name>
    <message>
        <location filename="../src/tiled/tilestampsdock.cpp" line="+196"/>
        <source>Delete Stamp</source>
        <translation>Borrar Sello</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Variation</source>
        <translation>Eliminar Variación</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Choose the Stamps Folder</source>
        <translation>Elegir la Carpeta de los Sellos</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tile Stamps</source>
        <translation>Patrón de Sellos</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add New Stamp</source>
        <translation>Añadir Nuevo Sello</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add Variation</source>
        <translation>Añadir Variación</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Duplicate Stamp</source>
        <translation>Duplicar Sello</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete Selected</source>
        <translation>Borrar Seleccionado</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Stamps Folder</source>
        <translation>Establecer Carpeta de Sellos</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Filter</source>
        <translation>Filtrar</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetDock</name>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="+709"/>
        <source>Remove Tileset</source>
        <translation>Eliminar Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The tileset &quot;%1&quot; is still in use by the map!</source>
        <translation>¡El conjunto de patrones &quot;%1&quot; está siendo todavía utilizado por el mapa!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove this tileset and all references to the tiles in this tileset?</source>
        <translation>¿Eliminar este conjunto de patrones y todas las referencias a patrones de dicho conjunto?</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Tilesets</source>
        <translation>Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>New Tileset</source>
        <translation>Nuevo Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Import Tileset</source>
        <translation>&amp;Importar conjuntos de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Export Tileset As...</source>
        <translation>&amp;Exportar conjuntos de Patrones como...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile&amp;set Properties</source>
        <translation>Propiedades del &amp;Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Tileset</source>
        <translation>&amp;Eliminar Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+128"/>
        <location line="+15"/>
        <source>Add Tiles</source>
        <translation>Añadir Patrones</translation>
    </message>
    <message>
        <location line="-142"/>
        <location line="+199"/>
        <location line="+13"/>
        <source>Remove Tiles</source>
        <translation>Eliminar Patrones</translation>
    </message>
    <message>
        <location line="-121"/>
        <source>Error saving tileset: %1</source>
        <translation>Error guardando el conjunto de patrones: %1</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Could not load &quot;%1&quot;!</source>
        <translation>No se pudo cargar &quot;%1&quot;!</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>One or more of the tiles to be removed are still in use by the map!</source>
        <translation>Uno ó más de los patrones para ser eliminados está siendo usado en el mapa!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove all references to these tiles?</source>
        <translation>Eliminar toda referencia a estos patrones?</translation>
    </message>
    <message>
        <location line="-207"/>
        <source>Edit &amp;Terrain Information</source>
        <translation>Editar Información del &amp;Terreno</translation>
    </message>
    <message>
        <location line="+69"/>
        <location line="+23"/>
        <source>Export Tileset</source>
        <translation>Exportar Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Archivos de conjunto de patrones de Tiled (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetParametersEdit</name>
    <message>
        <location filename="../src/tiled/tilesetparametersedit.cpp" line="+48"/>
        <source>Edit...</source>
        <translation>Editar...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetView</name>
    <message>
        <location filename="../src/tiled/tilesetview.cpp" line="+604"/>
        <source>Add Terrain Type</source>
        <translation>Añadir Tipo de Terreno</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Set Terrain Image</source>
        <translation>Establecer Imagen del Terreno</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Tile &amp;Properties...</source>
        <translation>&amp;Atributos del Patrón...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Show &amp;Grid</source>
        <translation>Mostrar &amp;Rejilla</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TmxMapFormat</name>
    <message>
        <location filename="../src/tiled/tmxmapformat.h" line="+62"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Archivos de mapas de Tiled (*.tmx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TsxTilesetFormat</name>
    <message>
        <location line="+24"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Archivos de conjunto de patrones de Tiled (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::UndoDock</name>
    <message>
        <location filename="../src/tiled/undodock.cpp" line="+64"/>
        <source>History</source>
        <translation>Historia</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;empty&gt;</source>
        <translation>&lt;Vacía&gt;</translation>
    </message>
</context>
<context>
    <name>Tmw::TmwPlugin</name>
    <message>
        <location filename="../src/plugins/tmw/tmwplugin.cpp" line="+47"/>
        <source>Multiple collision layers found!</source>
        <translation>¡Encontradas múltiples capas de colisiones!</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>No collision layer found!</source>
        <translation>¡No se encontró capa de colisiones!</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Could not open file for writing.</source>
        <translation>No se pudo abrir el archivo para escritura.</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>TMW-eAthena collision files (*.wlk)</source>
        <translation>Ficheros de colisiones TMW-eAthena (*.wlk)</translation>
    </message>
</context>
<context>
    <name>TmxViewer</name>
    <message>
        <location filename="../src/tmxviewer/tmxviewer.cpp" line="+182"/>
        <source>TMX Viewer</source>
        <translation>TMX Viewer</translation>
    </message>
</context>
<context>
    <name>Undo Commands</name>
    <message>
        <location filename="../src/tiled/addremovelayer.h" line="+67"/>
        <source>Add Layer</source>
        <translation>Añadir Capa</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Remove Layer</source>
        <translation>Eliminar Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovemapobject.cpp" line="+76"/>
        <source>Add Object</source>
        <translation>Añadir Objeto</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove Object</source>
        <translation>Eliminar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetileset.cpp" line="+66"/>
        <source>Add Tileset</source>
        <translation>Añadir Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tileset</source>
        <translation>Eliminar Conjunto de Patrones</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+36"/>
        <source>Change Object</source>
        <translation>Modificar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeobjectgroupproperties.cpp" line="+39"/>
        <source>Change Object Layer Properties</source>
        <translation>Cambiar las propiedades de la Capa de Objetos</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeproperties.cpp" line="+38"/>
        <source>Change %1 Properties</source>
        <translation>Cambiar %1 Atributos</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Set Property</source>
        <translation>Establecer Atributo</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Añadir Atributo</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Remove Property</source>
        <translation>Eliminar Atributo</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Rename Property</source>
        <translation>Renombrar Atributo</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeselectedarea.cpp" line="+31"/>
        <source>Change Selection</source>
        <translation>Cambiar Selección</translation>
    </message>
    <message>
        <location filename="../src/tiled/erasetiles.cpp" line="+39"/>
        <source>Erase</source>
        <translation>Borrar</translation>
    </message>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="-30"/>
        <source>Fill Area</source>
        <translation>Rellenar Área</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Move Object</source>
        <translation>Mover Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobjecttogroup.cpp" line="+41"/>
        <source>Move Object to Layer</source>
        <translation>Mover Objeto a la Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/movetileset.cpp" line="+31"/>
        <source>Move Tileset</source>
        <translation>Mover Conjunto de Patrones</translation>
    </message>
    <message>
        <location filename="../src/tiled/offsetlayer.cpp" line="+42"/>
        <source>Offset Layer</source>
        <translation>Desplazar Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/painttilelayer.cpp" line="+51"/>
        <source>Paint</source>
        <translation>Pintar</translation>
    </message>
    <message>
        <location filename="../src/tiled/renamelayer.cpp" line="+40"/>
        <source>Rename Layer</source>
        <translation>Renombrar Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizetilelayer.cpp" line="+37"/>
        <source>Resize Layer</source>
        <translation>Redimensionar Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemap.cpp" line="+32"/>
        <source>Resize Map</source>
        <translation>Redimensionar Mapa</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Resize Object</source>
        <translation>Redimensionar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="-765"/>
        <source>Import Tileset</source>
        <translation>Importar Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Export Tileset</source>
        <translation>Exportar Conjunto de Patrones</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetchanges.cpp" line="+36"/>
        <source>Change Tileset Name</source>
        <translation>Cambiar el Nombre del Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Change Drawing Offset</source>
        <translation>Cambiar el Desplazamiento</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Edit Tileset</source>
        <translation>Editar Conjunto de Patrones</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Change Columns</source>
        <translation>Cambiar Columnas</translation>
    </message>
    <message>
        <location filename="../src/tiled/movelayer.cpp" line="+37"/>
        <source>Lower Layer</source>
        <translation>Bajar Capa</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Layer</source>
        <translation>Subir Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/changepolygon.cpp" line="+40"/>
        <location line="+12"/>
        <source>Change Polygon</source>
        <translation>Cambiar Polígono</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremoveterrain.cpp" line="+69"/>
        <source>Add Terrain</source>
        <translation>Añadir Terreno</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Terrain</source>
        <translation>Eliminar Terreno</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerproperties.cpp" line="+39"/>
        <source>Change Image Layer Properties</source>
        <translation>Cambiar los atributos de la Capa de Imagen</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileterrain.cpp" line="+131"/>
        <source>Change Tile Terrain</source>
        <translation>Cambiar Patrón de Terreno</translation>
    </message>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="-135"/>
        <source>Change Terrain Image</source>
        <translation>Cambiar Imagen del Terreno</translation>
    </message>
    <message>
        <location filename="../src/tiled/changelayer.cpp" line="+41"/>
        <source>Show Layer</source>
        <translation>Mostrar Capa</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Layer</source>
        <translation>Ocultar Capa</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Change Layer Opacity</source>
        <translation>Cambiar Opacidad de la Capa</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Change Layer Offset</source>
        <translation>Cambiar Desplazamiento de la Capa</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+31"/>
        <source>Show Object</source>
        <translation>Mostrar Objeto</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Object</source>
        <translation>Ocultar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/renameterrain.cpp" line="+37"/>
        <source>Change Terrain Name</source>
        <translation>Cambiar el Nombre del Terreno</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetiles.cpp" line="+69"/>
        <source>Add Tiles</source>
        <translation>Añadir Patrones</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tiles</source>
        <translation>Eliminar Patrones</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerposition.cpp" line="+36"/>
        <source>Change Image Layer Position</source>
        <translation>Cambiar la Posición de la Capa de la Imagen</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobjectsorder.cpp" line="+44"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+67"/>
        <source>Raise Object</source>
        <translation>Subir Objeto</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+29"/>
        <source>Lower Object</source>
        <translation>Bajar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileanimation.cpp" line="+33"/>
        <source>Change Tile Animation</source>
        <translation>Cambiar Animación de Patrón</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileobjectgroup.cpp" line="+15"/>
        <source>Change Tile Collision</source>
        <translation>Cambiar Colisión de Patrón</translation>
    </message>
    <message>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+43"/>
        <source>Raise Object To Top</source>
        <translation>Subir Objeto a la Cima</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Lower Object To Bottom</source>
        <translation>Bajar Objeto al Fondo</translation>
    </message>
    <message>
        <location filename="../src/tiled/rotatemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Rotate Object</source>
        <translation>Rotar Objeto</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapproperty.cpp" line="+41"/>
        <source>Change Tile Width</source>
        <translation>Cambiar la Anchura del Patrón</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Tile Height</source>
        <translation>Cambiar la Altura del Patrón</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Hex Side Length</source>
        <translation>Cambiar la Longitud Lateral para Hex</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Background Color</source>
        <translation>Cambiar el Color de Fondo</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Axis</source>
        <translation>Cambiar el Eje de Escalonado</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Index</source>
        <translation>Cambiar el Índice de Escalonado</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Orientation</source>
        <translation>Cambiar la Orientación</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Render Order</source>
        <translation>Cambiar el Orden de Pintado</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Layer Data Format</source>
        <translation>Cambiar el Formato de la Capa de Datos</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileprobability.cpp" line="+38"/>
        <source>Change Tile Probability</source>
        <translation>Cambiar la Probabilidad de un Patrón</translation>
    </message>
    <message>
        <location filename="../src/tiled/adjusttileindexes.cpp" line="+39"/>
        <source>Adjust Tile Indexes</source>
        <translation>Ajustar Índices de Patrón</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileimagesource.cpp" line="+39"/>
        <source>Change Tile Image</source>
        <translation>Cambiar Imagen del Patrón</translation>
    </message>
    <message>
        <location filename="../src/tiled/replacetileset.cpp" line="+33"/>
        <source>Replace Tileset</source>
        <translation>Reemplazar Conjunto de Patrones</translation>
    </message>
</context>
<context>
    <name>Utils</name>
    <message>
        <location filename="../src/tiled/utils.cpp" line="+34"/>
        <source>Image files</source>
        <translation>Ficheros de imagen</translation>
    </message>
</context>
</TS>
