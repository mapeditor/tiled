<?xml version='1.0' encoding='utf-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="hu">
<context>
    <name>AboutDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/aboutdialog.ui"/>
        <source>About Tiled</source>
        <translation>A Tiled névjegye</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Donate</source>
        <translation>Támogatás</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location line="+36" filename="../src/tiled/aboutdialog.cpp"/>
        <source>&lt;p align="center">&lt;font size="+2">&lt;b>Tiled Map Editor&lt;/b>&lt;/font>&lt;br>&lt;i>Version %1&lt;/i>&lt;/p>
&lt;p align="center">Copyright 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br>(see the AUTHORS file for a full list of contributors)&lt;/p>
&lt;p align="center">You may modify and redistribute this program under the terms of the GPL (version 2 or later). A copy of the GPL is contained in the 'COPYING' file distributed with Tiled.&lt;/p>
&lt;p align="center">&lt;a href="http://www.mapeditor.org/">http://www.mapeditor.org/&lt;/a>&lt;/p>
</source>
        <translation>&lt;p align="center">&lt;font size="+2">&lt;b>Tiled térképszerkesztő&lt;/b>&lt;/font>&lt;br>&lt;i>Verzió: %1&lt;/i>&lt;/p>
&lt;p align="center">Copyright 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br>(a hozzájárulók teljes listájához nézze meg az AUTHORS fájlt)&lt;/p>
&lt;p align="center">Ez a program a GPL (2-es verzió vagy újabb) feltételei szerint módosítható, illetve terjeszthető. A GPL egy másolatát a Tiled programhoz melléklet „COPYING” fájl tartalmazza.&lt;/p>
&lt;p align="center">&lt;a href="http://www.mapeditor.org/">http://www.mapeditor.org/&lt;/a>&lt;/p>
</translation>
    </message>
</context>
<context>
    <name>AddPropertyDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/addpropertydialog.ui"/>
        <source>Add Property</source>
        <translation>Tulajdonság hozzáadása</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Property name</source>
        <translation>Tulajdonság neve</translation>
    </message>
</context>
<context>
    <name>Command line</name>
    <message>
        <location line="+229" filename="../src/tiled/main.cpp"/>
        <source>Export syntax is --export-map [format] &lt;tmx file> &lt;target file></source>
        <translation>Az exportálási szintaxis: --export-map [formátum] &lt;tmx-fájl> &lt;célfájl></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Format not recognized (see --export-formats)</source>
        <translation>A formátum nem ismerhető fel (lásd: --export-formats)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Non-unique file extension. Can't determine correct export format.</source>
        <translation>Nem egyedi fájlkiterjesztés. Nem lehet meghatározni a helyes exportálási formátumot.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>No exporter found for target file.</source>
        <translation>Nem található exportáló a célfájlhoz.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Failed to load source map.</source>
        <translation>Nem sikerült betölteni a forrástérképet.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Failed to export map to target file.</source>
        <translation>Nem sikerült a térkép exportálása a célfájlba.</translation>
    </message>
</context>
<context>
    <name>CommandDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/commanddialog.ui"/>
        <source>Properties</source>
        <translation>Tulajdonságok</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>&amp;Save map before executing</source>
        <translation>Térkép &amp;mentése végrehajtás előtt</translation>
    </message>
</context>
<context>
    <name>CommandLineHandler</name>
    <message>
        <location line="-188" filename="../src/tiled/main.cpp"/>
        <source>Display the version</source>
        <translation>A verzió megjelenítése</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Only check validity of arguments</source>
        <translation>Csak az argumentumok érvényességének ellenőrzése</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Disable hardware accelerated rendering</source>
        <translation>Hardveresen gyorsított megjelenítés letiltása</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Export the specified tmx file to target</source>
        <translation>A megadott tmx-fájl exportálása a célba</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Print a list of supported export formats</source>
        <translation>Támogatott exportálási formátumok listájának kiírása</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Start a new instance, even if an instance is already running</source>
        <translation>Új példány indítása akkor is, ha egy példány már fut</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Export formats:</source>
        <translation>Exportálási formátumok:</translation>
    </message>
</context>
<context>
    <name>CommandLineParser</name>
    <message>
        <location line="+75" filename="../src/tiled/commandlineparser.cpp"/>
        <source>Bad argument %1: lonely hyphen</source>
        <translation>Rossz argumentum: %1: magányos kötőjel</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Unknown long argument %1: %2</source>
        <translation>Ismeretlen hosszú argumentum: %1: %2</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Unknown short argument %1.%2: %3</source>
        <translation>Ismeretlen rövid argumentum: %1.%2: %3</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Usage:
  %1 [options] [files...]</source>
        <translation>Használat:
  %1 [kapcsolók] [fájlok…]</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Options:</source>
        <translation>Kapcsolók:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Display this help</source>
        <translation>Ezen súgó megjelenítése</translation>
    </message>
</context>
<context>
    <name>ConverterDataModel</name>
    <message>
        <location line="+75" filename="../src/automappingconverter/converterdatamodel.cpp"/>
        <source>File</source>
        <translation>Fájl</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Version</source>
        <translation>Verzió</translation>
    </message>
</context>
<context>
    <name>ConverterWindow</name>
    <message>
        <location line="+36" filename="../src/automappingconverter/converterwindow.cpp"/>
        <source>Save all as %1</source>
        <translation>Összes mentése mint %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>All Files (*)</source>
        <translation>Minden fájl (*)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled térképfájlok (*.tmx)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Open Map</source>
        <translation>Térkép megnyitása</translation>
    </message>
</context>
<context>
    <name>Csv::CsvPlugin</name>
    <message>
        <location line="+55" filename="../src/plugins/csv/csvplugin.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-fájlok (*.csv)</translation>
    </message>
</context>
<context>
    <name>Defold::DefoldPlugin</name>
    <message>
        <location line="+58" filename="../src/plugins/defold/defoldplugin.cpp"/>
        <source>Defold files (*.tilemap)</source>
        <translation>Defold-fájlok (*.tilemap)</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
</context>
<context>
    <name>Droidcraft::DroidcraftPlugin</name>
    <message>
        <location line="+56" filename="../src/plugins/droidcraft/droidcraftplugin.cpp"/>
        <source>This is not a valid Droidcraft map file!</source>
        <translation>Ez nem érvényes Droidcraft térképfájl!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>The map needs to have exactly one tile layer!</source>
        <translation>A térképnek pontosan egy csemperéteggel kell rendelkeznie!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The layer must have a size of 48 x 48 tiles!</source>
        <translation>A rétegnek 48 x 48 csempe méretűnek kell lennie!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Droidcraft map files (*.dat)</source>
        <translation>Droidcraft térképfájlok (*.dat)</translation>
    </message>
</context>
<context>
    <name>EditTerrainDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/editterraindialog.ui"/>
        <source>Edit Terrain Information</source>
        <translation>Terepinformációk szerkesztése</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+3"/>
        <source>Undo</source>
        <translation>Visszavonás</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+3"/>
        <source>Redo</source>
        <translation>Újra</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Erase</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Add Terrain Type</source>
        <translation>Tereptípus hozzáadása</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Hozzáadás</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Terrain Type</source>
        <translation>Tereptípus eltávolítása</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Eltávolítás</translation>
    </message>
</context>
<context>
    <name>ExportAsImageDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/exportasimagedialog.ui"/>
        <source>Export As Image</source>
        <translation>Exportálás képként</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Location</source>
        <translation>Hely</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Tallózás…</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Settings</source>
        <translation>Beállítások</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only include &amp;visible layers</source>
        <translation>Csak a &amp;látható rétegeket tartalmazza</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Use current &amp;zoom level</source>
        <translation>Jelenlegi &amp;nagyítási szint használata</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Draw tile grid</source>
        <translation>&amp;Csemperács rajzolása</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Include background color</source>
        <translation>&amp;Háttérszínt tartalmazza</translation>
    </message>
</context>
<context>
    <name>Flare::FlarePlugin</name>
    <message>
        <location line="+52" filename="../src/plugins/flare/flareplugin.cpp"/>
        <source>Could not open file for reading.</source>
        <translation>Nem sikerült megnyitni a fájlt olvasásra.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Error loading tileset %1, which expands to %2. Path not found!</source>
        <translation>Hiba a(z) %1 csempekészlet betöltésekor, amely ezt terjeszti ki: %2. Az útvonal nem található!</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>No tilesets section found before layer section.</source>
        <translation>Nem található csempekészletek szakasz a rétegszakasz előtt.</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Error mapping tile id %1.</source>
        <translation>Hiba a(z) %1 csempeazonosító leképezésekor.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>This seems to be no valid flare map. A Flare map consists of at least a header section, a tileset section and one tile layer.</source>
        <translation>Ez nem tűnik érvényes Flare térképnek. Egy Flare térkép legalább egy fejléc szakaszból, egy csempekészlet szakaszból és egy csemperétegből áll.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Flare map files (*.txt)</source>
        <translation>Flare térképfájlok (*.txt)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
</context>
<context>
    <name>Gmx::GmxPlugin</name>
    <message>
        <location line="+82" filename="../src/plugins/gmx/gmxplugin.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+143"/>
        <source>GameMaker room files (*.room.gmx)</source>
        <translation>GameMaker szobafájlok (*.room.gmx)</translation>
    </message>
</context>
<context>
    <name>Json::JsonMapFormat</name>
    <message>
        <location line="+53" filename="../src/plugins/json/jsonplugin.cpp"/>
        <source>Could not open file for reading.</source>
        <translation>Nem sikerült megnyitni a fájlt olvasásra.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error parsing file.</source>
        <translation>Hiba a fájl feldolgozásakor.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Error while writing file:
%1</source>
        <translation>Hiba a fájl írása közben:
%1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Json map files (*.json)</source>
        <translation>Json térképfájlok (*.json)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>JavaScript map files (*.js)</source>
        <translation>JavaScript térképfájlok (*.js)</translation>
    </message>
</context>
<context>
    <name>Json::JsonTilesetFormat</name>
    <message>
        <location line="+27"/>
        <source>Could not open file for reading.</source>
        <translation>Nem sikerült megnyitni a fájlt olvasásra.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Error parsing file.</source>
        <translation>Hiba a fájl feldolgozásakor.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error while writing file:
%1</source>
        <translation>Hiba a fájl írása közben:
%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Json tileset files (*.json)</source>
        <translation>Json csempekészletfájlok (*.json)</translation>
    </message>
</context>
<context>
    <name>Lua::LuaPlugin</name>
    <message>
        <location line="+58" filename="../src/plugins/lua/luaplugin.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Lua files (*.lua)</source>
        <translation>Lua fájlok (*.lua)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location line="+49" filename="../src/tiled/mainwindow.ui"/>
        <source>&amp;File</source>
        <translation>&amp;Fájl</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Recent Files</source>
        <translation>&amp;Legutóbbi fájlok</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>&amp;Edit</source>
        <translation>S&amp;zerkesztés</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Help</source>
        <translation>&amp;Súgó</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Map</source>
        <translation>&amp;Térkép</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;View</source>
        <translation>&amp;Nézet</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Show Object &amp;Names</source>
        <translation>Objektum&amp;nevek megjelenítése</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Main Toolbar</source>
        <translation>Fő eszköztár</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Tools</source>
        <translation>Eszközök</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Open...</source>
        <translation>&amp;Megnyitás…</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Save</source>
        <translation>M&amp;entés</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Quit</source>
        <translation>&amp;Kilépés</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Copy</source>
        <translation>&amp;Másolás</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Paste</source>
        <translation>&amp;Beillesztés</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;About Tiled</source>
        <translation>A Tiled &amp;névjegye</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>About Qt</source>
        <translation>A Qt névjegye</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Resize Map...</source>
        <translation>Térkép át&amp;méretezése…</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map &amp;Properties</source>
        <translation>&amp;Térkép tulajdonságai</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>AutoMap</source>
        <translation>Automatikus térkép</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A</source>
        <translation>A</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show &amp;Grid</source>
        <translation>&amp;Rács megjelenítése</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+G</source>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;As...</source>
        <translation>Me&amp;ntés másként…</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;New...</source>
        <translation>Ú&amp;j…</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>New &amp;Tileset...</source>
        <translation>Új &amp;csempekészlet…</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Close</source>
        <translation>&amp;Bezárás</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom In</source>
        <translation>Nagyítás</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom Out</source>
        <translation>Kicsinyítés</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normal Size</source>
        <translation>Normál méret</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+0</source>
        <translation>Ctrl+0</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Become a Patron</source>
        <translation>Legyen pártfogó</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save All</source>
        <translation>Összes mentése</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Documentation</source>
        <translation>Dokumentáció</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Never</source>
        <translation>&amp;Soha</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>For &amp;Selected Objects</source>
        <translation>&amp;Kijelölt objektumoknál</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>For &amp;All Objects</source>
        <translation>Öss&amp;zes objektumnál</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>AutoMap While Drawing</source>
        <translation>Automatikus térkép rajzolás közben</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Paste &amp;in Place</source>
        <translation>Beillesztés &amp;helyben</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+124" filename="../src/tiled/tilecollisioneditor.cpp"/>
        <source>Ctrl+Shift+V</source>
        <translation>Ctrl+Shift+V</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Full Screen</source>
        <translation>Teljes képernyő</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>F11</source>
        <translation>F11</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap To &amp;Pixels</source>
        <translation>Illesztés a &amp;képpontokhoz</translation>
    </message>
    <message>
        <location line="-200"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Kivágás</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Offset Map...</source>
        <translation>Térkép &amp;eltolása…</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Offsets everything in a layer</source>
        <translation>Mindent eltol egy rétegen</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pre&amp;ferences...</source>
        <translation>B&amp;eállítások…</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Clear Recent Files</source>
        <translation>Legutóbbi fájlok törlése</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Export</source>
        <translation>E&amp;xportálás</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+E</source>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <location line="-82"/>
        <source>&amp;Add External Tileset...</source>
        <translation>&amp;Külső csempekészlet hozzáadása…</translation>
    </message>
    <message>
        <location line="-50"/>
        <source>Export As &amp;Image...</source>
        <translation>Expo&amp;rtálás képként…</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>E&amp;xport As...</source>
        <translation>Exp&amp;ortálás másként…</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+E</source>
        <translation>Ctrl+Shift+E</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Snap to Grid</source>
        <translation>Illesztés a rá&amp;cshoz</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>C&amp;lose All</source>
        <translation>Összes be&amp;zárása</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+W</source>
        <translation>Ctrl+Shift+W</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Delete</source>
        <translation>&amp;Törlés</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Highlight Current Layer</source>
        <translation>Jelenlegi réteg &amp;kiemelése</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H</source>
        <translation>H</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Object &amp;Outlines</source>
        <translation>Csempeobjektum kör&amp;vonalainak megjelenítése</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap to &amp;Fine Grid</source>
        <translation>Illesztés a &amp;segédrácshoz</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Animations</source>
        <translation>Csempeanimációk megjelenítése</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Reload</source>
        <translation>Újratöltés</translation>
    </message>
    <message>
        <location line="+14" filename="../src/automappingconverter/converterwindow.ui"/>
        <source>Tiled Automapping Rule Files Converter</source>
        <translation>Tiled automatikus leképező szabály fájlok átalakítója</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Add new Automapping rules</source>
        <translation>Új automatikus leképező szabályok hozzáadása</translation>
    </message>
    <message>
        <location line="+581" filename="../src/tiled/propertybrowser.cpp"/>
        <source>All Files (*)</source>
        <translation>Minden fájl (*)</translation>
    </message>
</context>
<context>
    <name>MapDocument</name>
    <message>
        <location line="+178" filename="../src/tiled/adjusttileindexes.cpp"/>
        <source>Tile</source>
        <translation>Csempe</translation>
    </message>
</context>
<context>
    <name>MapReader</name>
    <message>
        <location line="+140" filename="../src/libtiled/mapreader.cpp"/>
        <source>Not a map file.</source>
        <translation>Nem térképfájl.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Not a tileset file.</source>
        <translation>Nem csempekészletfájl.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

%1. sor, %2. oszlop</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File not found: %1</source>
        <translation>A fájl nem található: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unable to read file: %1</source>
        <translation>Nem lehet olvasni a fájlt: %1</translation>
    </message>
    <message>
        <location line="+32"/>
        <location line="+59" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Unsupported map orientation: "%1"</source>
        <translation>Nem támogatott térképtájolás: „%1”</translation>
    </message>
    <message>
        <location line="+102"/>
        <location line="+21"/>
        <location line="+138" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Invalid tileset parameters for tileset '%1'</source>
        <translation>Érvénytelen csempekészlet-paraméterek a(z) „%1” csempekészletnél</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Invalid tile ID: %1</source>
        <translation>Érvénytelen csempeazonosító: %1</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>Too many &lt;tile> elements</source>
        <translation>Túl sok &lt;tile> elem</translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+43"/>
        <location line="+219" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Invalid tile: %1</source>
        <translation>Érvénytelen csempe: %1</translation>
    </message>
    <message>
        <location line="+29"/>
        <location line="+34" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Invalid draw order: %1</source>
        <translation>Érvénytelen rajzolási sorrend: %1</translation>
    </message>
    <message>
        <location line="+154"/>
        <source>Invalid points data for polygon</source>
        <translation>Érvénytelen pontadatok a sokszögnél</translation>
    </message>
    <message>
        <location line="-285"/>
        <location line="-90" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Unknown encoding: %1</source>
        <translation>Ismeretlen kódolás: %1</translation>
    </message>
    <message>
        <location line="-181"/>
        <source>Error reading embedded image for tile %1</source>
        <translation>Hiba a beágyazott kép olvasásakor a(z) %1 csempénél</translation>
    </message>
    <message>
        <location line="+176"/>
        <location line="-4" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Compression method '%1' not supported</source>
        <translation>A(z) „%1” tömörítési módszer nem támogatott</translation>
    </message>
    <message>
        <location line="+58"/>
        <location line="+19"/>
        <location line="+15" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <location line="+39"/>
        <source>Corrupt layer data for layer '%1'</source>
        <translation>Sérült rétegadatok a(z) „%1” rétegnél</translation>
    </message>
    <message>
        <location line="+12"/>
        <location line="-28" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Unable to parse tile at (%1,%2) on layer '%3'</source>
        <translation>Nem lehet feldolgozni a csempét (%1,%2) ezen a rétegen: „%3”</translation>
    </message>
    <message>
        <location line="-28"/>
        <location line="+44"/>
        <location line="+31" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Tile used but no tilesets specified</source>
        <translation>A csempe használatban van, de nincs csempekészlet megadva</translation>
    </message>
    <message>
        <location line="+113" filename="../src/libtiled/mapwriter.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="-170" filename="../src/libtiled/varianttomapconverter.cpp"/>
        <source>Invalid (negative) tile id: %1</source>
        <translation>Érvénytelen (negatív) csempeazonosító: %1</translation>
    </message>
</context>
<context>
    <name>NewMapDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/newmapdialog.ui"/>
        <source>New Map</source>
        <translation>Új térkép</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map size</source>
        <translation>Térkép mérete</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+68"/>
        <source>Width:</source>
        <translation>Szélesség:</translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+26"/>
        <source> tiles</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> csempe</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+68"/>
        <source>Height:</source>
        <translation>Magasság:</translation>
    </message>
    <message>
        <location line="-32"/>
        <source>Tile size</source>
        <translation>Csempe mérete</translation>
    </message>
    <message>
        <location line="+16"/>
        <location line="+26"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> képpont</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Map</source>
        <translation>Térkép</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Orientation:</source>
        <translation>Tájolás:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Tile layer format:</source>
        <translation>Csemperéteg formátuma:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile render order:</source>
        <translation>Csempemegjelenítési sorrend:</translation>
    </message>
</context>
<context>
    <name>NewTilesetDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/newtilesetdialog.ui"/>
        <location line="+235" filename="../src/tiled/newtilesetdialog.cpp"/>
        <source>New Tileset</source>
        <translation>Új csempekészlet</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tileset</source>
        <translation>Csempekészlet</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Based on Tileset Image</source>
        <translation>Csempekészlet képe alapján</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Collection of Images</source>
        <translation>Képek gyűjteménye</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Type:</source>
        <translation>Típus:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Name:</source>
        <translation>&amp;Név:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Tallózás…</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Use transparent color:</source>
        <translation>Átlátszó szín használata:</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>Tile width:</source>
        <translation>Csempe szélessége:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Pick color from image</source>
        <translation>Szín kiválasztása a képről</translation>
    </message>
    <message>
        <location line="-138"/>
        <location line="+42"/>
        <location line="+26"/>
        <location line="+16"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> képpont</translation>
    </message>
    <message>
        <location line="-142"/>
        <source>Image</source>
        <translation>Kép</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Source:</source>
        <translation>Forrás:</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>The space at the edges of the tileset.</source>
        <translation>A térköz a csempekészlet széleinél.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin:</source>
        <translation>Margó:</translation>
    </message>
    <message>
        <location line="-45"/>
        <source>Tile height:</source>
        <translation>Csempe magassága:</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>The space between the tiles.</source>
        <translation>A térköz a csempék között.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spacing:</source>
        <translation>Térköz:</translation>
    </message>
    <message>
        <location line="-2" filename="../src/tiled/newtilesetdialog.cpp"/>
        <source>Edit Tileset</source>
        <translation>Csempekészlet szerkesztése</translation>
    </message>
</context>
<context>
    <name>ObjectTypes</name>
    <message>
        <location line="+43" filename="../src/tiled/objecttypes.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Could not open file.</source>
        <translation>Nem sikerült megnyitni a fájlt.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File doesn't contain object types.</source>
        <translation>A fájl nem tartalmaz objektumtípusokat.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

%1. sor, %2. oszlop</translation>
    </message>
</context>
<context>
    <name>ObjectTypesEditor</name>
    <message>
        <location line="+14" filename="../src/tiled/objecttypeseditor.ui"/>
        <source>Object Types Editor</source>
        <translation>Objektumtípus szerkesztő</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>File</source>
        <translation>Fájl</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Export Object Types...</source>
        <translation>Objektumtípusok exportálása…</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Import Object Types...</source>
        <translation>Objektumtípusok importálása…</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Choose Object Types File...</source>
        <translation>Objektumtípus fájl kiválasztása…</translation>
    </message>
</context>
<context>
    <name>OffsetMapDialog</name>
    <message>
        <location line="+17" filename="../src/tiled/offsetmapdialog.ui"/>
        <source>Offset Map</source>
        <translation>Térkép eltolása</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Offset Contents of Map</source>
        <translation>Térkép tartalmának eltolása</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+46"/>
        <source> tiles</source>
        <translation> csempe</translation>
    </message>
    <message>
        <location line="-30"/>
        <location line="+46"/>
        <source>Wrap</source>
        <translation>Tördelés</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Layers:</source>
        <translation>Rétegek:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>All Visible Layers</source>
        <translation>Összes látható réteg</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>All Layers</source>
        <translation>Összes réteg</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Selected Layer</source>
        <translation>Kijelölt réteg</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bounds:</source>
        <translation>Határok:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Whole Map</source>
        <translation>Teljes térkép</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Current Selection</source>
        <translation>Jelenlegi kijelölés</translation>
    </message>
</context>
<context>
    <name>PatreonDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/patreondialog.ui"/>
        <source>Become a Patron</source>
        <translation>Legyen pártfogó</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Visit https://www.patreon.com/bjorn</source>
        <translation>Látogassa meg a https://www.patreon.com/bjorn oldalt</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I'm already a patron!</source>
        <translation>Már pártfogó vagyok!</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maybe later</source>
        <translation>Talán később</translation>
    </message>
</context>
<context>
    <name>PreferencesDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/preferencesdialog.ui"/>
        <source>Preferences</source>
        <translation>Beállítások</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Saving and Loading</source>
        <translation>Mentés és betöltés</translation>
    </message>
    <message>
        <location line="-489" filename="../src/tiled/propertybrowser.cpp"/>
        <source>XML</source>
        <translation>XML</translation>
    </message>
    <message>
        <location line="+79" filename="../src/tiled/newmapdialog.cpp"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Base64 (uncompressed)</source>
        <translation>Base64 (tömörítetlen)</translation>
    </message>
    <message>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Base64 (gzip compressed)</source>
        <translation>Base64 (gzip - tömörített)</translation>
    </message>
    <message>
        <location line="+1" filename="../src/tiled/newmapdialog.cpp"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Base64 (zlib compressed)</source>
        <translation>Base64 (zlib - tömörített)</translation>
    </message>
    <message>
        <location line="-2"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>CSV</source>
        <translation>CSV</translation>
    </message>
    <message>
        <location line="+4"/>
        <location line="+2" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Right Down</source>
        <translation>Jobbról lefelé</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Right Up</source>
        <translation>Jobbról felfelé</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Left Down</source>
        <translation>Balról lefelé</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Left Up</source>
        <translation>Balról felfelé</translation>
    </message>
    <message>
        <location line="+6" filename="../src/tiled/preferencesdialog.ui"/>
        <source>&amp;Reload tileset images when they change</source>
        <translation>&amp;Csempekészlet képeinek újratöltése, amikor megváltoznak</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Not enabled by default since a reference to an external DTD is known to cause problems with some XML parsers.</source>
        <translation>Alapértelmezetten nincs engedélyezve, mivel egy külső DTD-re mutató hivatkozás ismert problémát okozhat néhány XML-feldolgozónál.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Include &amp;DTD reference in saved maps</source>
        <translation>&amp;DTD-hivatkozás felvétele a mentett térképekbe</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Interface</source>
        <translation>Felület</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Language:</source>
        <translation>&amp;Nyelv:</translation>
    </message>
    <message>
        <location line="-7"/>
        <source>Hardware &amp;accelerated drawing (OpenGL)</source>
        <translation>&amp;Hardveresen gyorsított rajzolás (OpenGL)</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Open last files on startup</source>
        <translation>Utolsó fájlok megnyitása indításkor</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Grid color:</source>
        <translation>Rács színe:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Fine grid divisions:</source>
        <translation>Segédrács felosztások:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source> pixels</source>
        <translation> képpont</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Object line width:</source>
        <translation>Objektum vonalvastagsága:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Theme</source>
        <translation>Téma</translation>
    </message>
    <message>
        <location line="+67" filename="../src/tiled/preferencesdialog.cpp"/>
        <location line="+122"/>
        <source>Native</source>
        <translation>Eredeti</translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Fusion</source>
        <translation>Fúzió</translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Tiled Fusion</source>
        <translation>Tiled fúzió</translation>
    </message>
    <message>
        <location line="+22" filename="../src/tiled/preferencesdialog.ui"/>
        <source>Selection color:</source>
        <translation>Kijelölés színe:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Style:</source>
        <translation>Stílus:</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Base color:</source>
        <translation>Alapszín:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Updates</source>
        <translation>Frissítések</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Check Now</source>
        <translation>Ellenőrzés most</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Automatically check for updates</source>
        <translation>Frissítések automatikus ellenőrzése</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Plugins</source>
        <translation>Bővítmények</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enabled Plugins</source>
        <translation>Engedélyezett bővítmények</translation>
    </message>
</context>
<context>
    <name>Python::PythonMapFormat</name>
    <message>
        <location line="+268" filename="../src/plugins/python/pythonplugin.cpp"/>
        <source>-- Using script %1 to read %2</source>
        <translation>-- A(z) %1 parancsfájl használata %2 olvasásához</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>-- Using script %1 to write %2</source>
        <translation>-- A(z) %1 parancsfájl használata %2 írásához</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Uncaught exception in script. Please check console.</source>
        <translation>El nem fogott kivétel a parancsfájlban. Nézze meg a konzolt.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Script returned false. Please check console.</source>
        <translation>A parancsfájl hamis értékkel tért vissza. Nézze meg a konzolt.</translation>
    </message>
</context>
<context>
    <name>Python::PythonPlugin</name>
    <message>
        <location line="-164"/>
        <source>Reloading Python scripts</source>
        <translation>Python parancsfájlok újratöltése</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location line="+33" filename="../src/automappingconverter/convertercontrol.h"/>
        <source>v0.8 and before</source>
        <translation>0.8-as verzió és korábbi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>v0.9 and later</source>
        <translation>0.9-es verzió és későbbi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>unknown</source>
        <translation>ismeretlen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>not a map</source>
        <translation>nem térkép</translation>
    </message>
</context>
<context>
    <name>QtBoolEdit</name>
    <message>
        <location line="+237" filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp"/>
        <location line="+10"/>
        <location line="+25"/>
        <source>True</source>
        <translation>Igaz</translation>
    </message>
    <message>
        <location line="-25"/>
        <location line="+25"/>
        <source>False</source>
        <translation>Hamis</translation>
    </message>
</context>
<context>
    <name>QtBoolPropertyManager</name>
    <message>
        <location line="+1703" filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp"/>
        <source>True</source>
        <translation>Igaz</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>False</source>
        <translation>Hamis</translation>
    </message>
</context>
<context>
    <name>QtCharEdit</name>
    <message>
        <location line="+1712" filename="../src/qtpropertybrowser/src/qteditorfactory.cpp"/>
        <source>Clear Char</source>
        <translation>Karakter törlése</translation>
    </message>
</context>
<context>
    <name>QtColorEditWidget</name>
    <message>
        <location line="+614"/>
        <source>...</source>
        <translation>…</translation>
    </message>
</context>
<context>
    <name>QtColorPropertyManager</name>
    <message>
        <location line="+4736" filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp"/>
        <source>Red</source>
        <translation>Vörös</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Green</source>
        <translation>Zöld</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Blue</source>
        <translation>Kék</translation>
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
        <location line="-214" filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp"/>
        <source>Arrow</source>
        <translation>Nyíl</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Up Arrow</source>
        <translation>Felfelé nyíl</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Cross</source>
        <translation>Kereszt</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Wait</source>
        <translation>Várakozás</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>IBeam</source>
        <translation>Hiányjel</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Vertical</source>
        <translation>Méretezés függőlegesen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Horizontal</source>
        <translation>Méretezés vízszintesen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Backslash</source>
        <translation>Méretezés ÉNY-DK irányba</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Slash</source>
        <translation>Méretezés ÉK-DNY irányba</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size All</source>
        <translation>Méretezés minden irányba</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Blank</source>
        <translation>Üres</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Vertical</source>
        <translation>Felosztás függőlegesen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Horizontal</source>
        <translation>Felosztás vízszintesen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pointing Hand</source>
        <translation>Mutató kéz</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forbidden</source>
        <translation>Tiltott</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Open Hand</source>
        <translation>Nyitott kéz</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Closed Hand</source>
        <translation>Zárt kéz</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>What's This</source>
        <translation>Mi ez?</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Busy</source>
        <translation>Elfoglalt</translation>
    </message>
</context>
<context>
    <name>QtFontEditWidget</name>
    <message>
        <location line="+209" filename="../src/qtpropertybrowser/src/qteditorfactory.cpp"/>
        <source>...</source>
        <translation>…</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Select Font</source>
        <translation>Betűkészlet kiválasztása</translation>
    </message>
</context>
<context>
    <name>QtFontPropertyManager</name>
    <message>
        <location line="-362" filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp"/>
        <source>Family</source>
        <translation>Család</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Point Size</source>
        <translation>Pontméret</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bold</source>
        <translation>Félkövér</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Italic</source>
        <translation>Dőlt</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Underline</source>
        <translation>Alázúzott</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Strikeout</source>
        <translation>Áthúzott</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Kerning</source>
        <translation>Alávágás</translation>
    </message>
</context>
<context>
    <name>QtKeySequenceEdit</name>
    <message>
        <location line="+238" filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp"/>
        <source>Clear Shortcut</source>
        <translation>Gyorsbillentyű törlése</translation>
    </message>
</context>
<context>
    <name>QtLocalePropertyManager</name>
    <message>
        <location line="-3533" filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp"/>
        <source>%1, %2</source>
        <translation>%1, %2</translation>
    </message>
    <message>
        <location line="+53"/>
        <source>Language</source>
        <translation>Nyelv</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Country</source>
        <translation>Ország</translation>
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
        <location line="-144" filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp"/>
        <source>[%1, %2, %3] (%4)</source>
        <translation>[%1, %2, %3] (%4)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Not set</source>
        <translation>Nincs beállítva</translation>
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
        <location line="+1701" filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp"/>
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
        <translation>Szélesség</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Magasság</translation>
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
        <translation>Szélesség</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Magasság</translation>
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
        <translation>Szélesség</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
</context>
<context>
    <name>QtSizePolicyPropertyManager</name>
    <message>
        <location line="+1704"/>
        <location line="+1"/>
        <source>&lt;Invalid></source>
        <translation>&lt;Érvénytelen></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>[%1, %2, %3, %4]</source>
        <translation>[%1, %2, %3, %4]</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Horizontal Policy</source>
        <translation>Vízszintes irányelv</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vertical Policy</source>
        <translation>Függőleges irányelv</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Horizontal Stretch</source>
        <translation>Vízszintes nyújtás</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Vertical Stretch</source>
        <translation>Függőleges nyújtás</translation>
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
        <translation>Szélesség</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
</context>
<context>
    <name>QtTreePropertyBrowser</name>
    <message>
        <location line="+478" filename="../src/qtpropertybrowser/src/qttreepropertybrowser.cpp"/>
        <source>Property</source>
        <translation>Tulajdonság</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
</context>
<context>
    <name>ReplicaIsland::ReplicaIslandPlugin</name>
    <message>
        <location line="+58" filename="../src/plugins/replicaisland/replicaislandplugin.cpp"/>
        <source>Cannot open Replica Island map file!</source>
        <translation>Nem lehet megnyitni a Replica Island térképfájlt!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Can't parse file header!</source>
        <translation>Nem lehet feldolgozni a fájl fejlécét!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Can't parse layer header!</source>
        <translation>Nem lehet feldolgozni a réteg fejlécét!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Inconsistent layer sizes!</source>
        <translation>Következetlen rétegméretek!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File ended in middle of layer!</source>
        <translation>A fájl véget ért a réteg közepén!</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Unexpected data at end of file!</source>
        <translation>Váratlan adatok a fájl végén!</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Replica Island map files (*.bin)</source>
        <translation>Replica Island térképfájlok (*.bin)</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>You must define a background_index property on the map!</source>
        <translation>Meg kell határoznia egy background_index tulajdonságot a térképen!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Can't save non-tile layer!</source>
        <translation>Nem lehet menteni nem csempe réteget!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>You must define a type property on each layer!</source>
        <translation>Meg kell határoznia egy type tulajdonságot minden rétegen!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a tile_index property on each layer!</source>
        <translation>Meg kell határoznia egy tile_index tulajdonságot minden rétegen!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a scroll_speed property on each layer!</source>
        <translation>Meg kell határoznia egy scroll_speed tulajdonságot minden rétegen!</translation>
    </message>
</context>
<context>
    <name>ResizeDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/resizedialog.ui"/>
        <source>Resize</source>
        <translation>Átméretezés</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Size</source>
        <translation>Méret</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+33"/>
        <location line="+32"/>
        <location line="+23"/>
        <source> tiles</source>
        <translation> csempe</translation>
    </message>
    <message>
        <location line="-75"/>
        <source>Width:</source>
        <translation>Szélesség:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Height:</source>
        <translation>Magasság:</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Offset</source>
        <translation>Eltolás</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Remove objects outside of the map</source>
        <translation>Objektumok eltávolítása a térképen kívül</translation>
    </message>
</context>
<context>
    <name>Tengine::TenginePlugin</name>
    <message>
        <location line="+49" filename="../src/plugins/tengine/tengineplugin.cpp"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+246"/>
        <source>T-Engine4 map files (*.lua)</source>
        <translation>T-Engine4 térképfájlok (*.lua)</translation>
    </message>
</context>
<context>
    <name>TextEditorDialog</name>
    <message>
        <location line="+14" filename="../src/tiled/texteditordialog.ui"/>
        <source>Edit Text</source>
        <translation>Szöveg szerkesztése</translation>
    </message>
</context>
<context>
    <name>TileAnimationEditor</name>
    <message>
        <location line="+14" filename="../src/tiled/tileanimationeditor.ui"/>
        <source>Tile Animation Editor</source>
        <translation>Csempeanimáció szerkesztő</translation>
    </message>
    <message>
        <location line="+99"/>
        <location line="+523" filename="../src/tiled/tileanimationeditor.cpp"/>
        <source>Preview</source>
        <translation>Előnézet</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractObjectTool</name>
    <message>
        <location line="+167" filename="../src/tiled/abstractobjecttool.cpp"/>
        <location line="+70"/>
        <source>Reset Tile Size</source>
        <translation>Csempeméret visszaállítása</translation>
    </message>
    <message numerus="yes">
        <location line="-13"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Objektum kettőzése</numerusform>
            <numerusform>%n objektum kettőzése</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+2"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Objektum eltávolítása</numerusform>
            <numerusform>%n objektum eltávolítása</numerusform>
        </translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Flip Horizontally</source>
        <translation>Tükrözés vízszintesen</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Flip Vertically</source>
        <translation>Tükrözés függőlegesen</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Raise Object</source>
        <translation>Objektum előre hozása</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgUp</source>
        <translation>PgUp</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object</source>
        <translation>Objektum hátra küldése</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgDown</source>
        <translation>PgDown</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Object to Top</source>
        <translation>Objektum előre hozása legfelülre</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Home</source>
        <translation>Home</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object to Bottom</source>
        <translation>Objektum hátra küldése legalulra</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>End</source>
        <translation>End</translation>
    </message>
    <message numerus="yes">
        <location line="+5"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Objektum áthelyezése rétegre</numerusform>
            <numerusform>%n objektum áthelyezése rétegre</numerusform>
        </translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Object &amp;Properties...</source>
        <translation>Objektum &amp;tulajdonságai…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractTileTool</name>
    <message>
        <location line="+124" filename="../src/tiled/abstracttiletool.cpp"/>
        <source>empty</source>
        <translation>üres</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutoMapper</name>
    <message>
        <location line="+115" filename="../src/tiled/automapper.cpp"/>
        <source>'%1': Property '%2' = '%3' does not make sense. Ignoring this property.</source>
        <translation>„%1”: „%2” tulajdonság = „%3” megadásának nincs értelme. A tulajdonság mellőzése.</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Did you forget an underscore in layer '%1'?</source>
        <translation>Elfelejtett egy aláhúzást a(z) „%1” rétegen?</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Layer '%1' is not recognized as a valid layer for Automapping.</source>
        <translation>A(z) „%1” réteg nem ismerhető fel érvényes rétegként az automatikus leképezéshez.</translation>
    </message>
    <message>
        <location line="-105"/>
        <source>'regions_input' layer must not occur more than once.</source>
        <translation>A „regions_input” réteg nem fordulhat elő egynél többször.</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+13"/>
        <source>'regions_*' layers must be tile layers.</source>
        <translation>A „regions_*” rétegeknek csemperétegeknek kell lenniük.</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>'regions_output' layer must not occur more than once.</source>
        <translation>A „regions_output” réteg nem fordulhat elő egynél többször.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>'input_*' and 'inputnot_*' layers must be tile layers.</source>
        <translation>Az „input_*” és az „inputnot_*” rétegeknek csemperétegeknek kell lenniük.</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>No 'regions' or 'regions_input' layer found.</source>
        <translation>Nem található „regions” vagy „regions_input” réteg.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No 'regions' or 'regions_output' layer found.</source>
        <translation>Nem található „regions” vagy „regions_output” réteg.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No input_&lt;name> layer found!</source>
        <translation>Nem található input_&lt;név> réteg!</translation>
    </message>
    <message>
        <location line="+165"/>
        <source>Tile</source>
        <translation>Csempe</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutomappingManager</name>
    <message>
        <location line="+103" filename="../src/tiled/automappingmanager.cpp"/>
        <source>Apply AutoMap rules</source>
        <translation>Automatikus térkép szabályok alkalmazása</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>No rules file found at:
%1</source>
        <translation>Nem található szabályfájl itt:
%1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Error opening rules file:
%1</source>
        <translation>Hiba a szabályfájl megnyitásakor:
%1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>File not found:
%1</source>
        <translation>A fájl nem található:
%1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Opening rules map failed:
%1</source>
        <translation>A szabálytérkép megnyitása sikertelen:
%1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksModel</name>
    <message>
        <location line="+144" filename="../src/tiled/brokenlinks.cpp"/>
        <source>Tileset image</source>
        <translation>Csempekészlet kép</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tileset</source>
        <translation>Csempekészlet</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tile image</source>
        <translation>Csempekép</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File name</source>
        <translation>Fájlnév</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Location</source>
        <translation>Hely</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksWidget</name>
    <message>
        <location line="+66"/>
        <source>Some files could not be found</source>
        <translation>Néhány fájl nem található</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>One or more files referenced by the map could not be found. You can help locate them below.</source>
        <translation>A térkép által hivatkozott egy vagy több fájl nem található. Segíthet megkeresni azokat lent.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Locate File...</source>
        <translation>Fájl keresése…</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Locate File</source>
        <translation>Fájl keresése</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Loading Image</source>
        <translation>Hiba a kép betöltésekor</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>All Files (*)</source>
        <translation>Minden fájl (*)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Locate External Tileset</source>
        <translation>Külső csempekészlet keresése</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Reading Tileset</source>
        <translation>Hiba a csempekészlet olvasásakor</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BucketFillTool</name>
    <message>
        <location line="+40" filename="../src/tiled/bucketfilltool.cpp"/>
        <location line="+194"/>
        <source>Bucket Fill Tool</source>
        <translation>Kitöltés eszköz</translation>
    </message>
    <message>
        <location line="-191"/>
        <location line="+192"/>
        <source>F</source>
        <translation>F</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ClipboardManager</name>
    <message>
        <location line="+171" filename="../src/tiled/clipboardmanager.cpp"/>
        <source>Paste Objects</source>
        <translation>Objektumok beillesztése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandButton</name>
    <message>
        <location line="+130" filename="../src/tiled/commandbutton.cpp"/>
        <source>Execute Command</source>
        <translation>Parancs végrehajtása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location line="-67"/>
        <source>Error Executing Command</source>
        <translation>Hiba a parancs végrehajtásakor</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>You do not have any commands setup.</source>
        <translation>Nincs egyetlen parancs sem beállítva.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Edit commands...</source>
        <translation>Parancsok szerkesztése…</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Edit Commands...</source>
        <translation>Parancsok szerkesztése…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDataModel</name>
    <message>
        <location line="+60" filename="../src/tiled/commanddatamodel.cpp"/>
        <source>Open in text editor</source>
        <translation>Megnyitás szövegszerkesztőben</translation>
    </message>
    <message>
        <location line="+91"/>
        <location line="+69"/>
        <source>&lt;new command></source>
        <translation>&lt;új parancs></translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Set a name for this command</source>
        <translation>Név beállítása ehhez a parancshoz</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set the shell command to execute</source>
        <translation>A végrehajtandó héjparancs beállítása</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Show or hide this command in the command list</source>
        <translation>A parancs megjelenítése vagy elrejtése a parancslistában</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add a new command</source>
        <translation>Új parancs hozzáadása</translation>
    </message>
    <message>
        <location line="+107"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Command</source>
        <translation>Parancs</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable</source>
        <translation>Engedélyezés</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Move Up</source>
        <translation>Mozgatás fel</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Move Down</source>
        <translation>Mozgatás le</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Execute</source>
        <translation>Végrehajtás</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Execute in Terminal</source>
        <translation>Végrehajtás terminálban</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location line="+84"/>
        <source>%1 (copy)</source>
        <translation>%1 (másolás)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>New command</source>
        <translation>Új parancs</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDialog</name>
    <message>
        <location line="+44" filename="../src/tiled/commanddialog.cpp"/>
        <source>Edit Commands</source>
        <translation>Parancsok szerkesztése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandProcess</name>
    <message>
        <location line="+144" filename="../src/tiled/command.cpp"/>
        <source>Unable to create/open %1</source>
        <translation>Nem lehet létrehozni vagy megnyitni: %1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unable to add executable permissions to %1</source>
        <translation>Nem lehet hozzáadni a végrehajtható jogosultságot ehhez: %1</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>The command failed to start.</source>
        <translation>A parancs indítása sikertelen.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command crashed.</source>
        <translation>A parancs összeomlott.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command timed out.</source>
        <translation>A parancs túllépte az időkorlátot.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>An unknown error occurred.</source>
        <translation>Ismeretlen hiba történt.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Error Executing %1</source>
        <translation>Hiba a(z) %1 végrehajtásakor</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ConsoleDock</name>
    <message>
        <location line="+36" filename="../src/tiled/consoledock.cpp"/>
        <source>Debug Console</source>
        <translation>Hibakereső konzol</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateEllipseObjectTool</name>
    <message>
        <location line="+39" filename="../src/tiled/createellipseobjecttool.cpp"/>
        <source>Insert Ellipse</source>
        <translation>Ellipszis beszúrása</translation>
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
        <location line="+46" filename="../src/tiled/createobjecttool.cpp"/>
        <source>O</source>
        <translation>O</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreatePolygonObjectTool</name>
    <message>
        <location line="+39" filename="../src/tiled/createpolygonobjecttool.cpp"/>
        <source>Insert Polygon</source>
        <translation>Sokszög beszúrása</translation>
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
        <location line="+39" filename="../src/tiled/createpolylineobjecttool.cpp"/>
        <source>Insert Polyline</source>
        <translation>Töröttvonal beszúrása</translation>
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
        <location line="+39" filename="../src/tiled/createrectangleobjecttool.cpp"/>
        <source>Insert Rectangle</source>
        <translation>Téglalap beszúrása</translation>
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
        <location line="+79" filename="../src/tiled/createtileobjecttool.cpp"/>
        <source>Insert Tile</source>
        <translation>Csempe beszúrása</translation>
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
        <location line="+412" filename="../src/tiled/documentmanager.cpp"/>
        <source>%1:

%2</source>
        <translation>%1:

%2</translation>
    </message>
    <message>
        <location line="+146"/>
        <source>Copy File Path</source>
        <translation>Fájl útvonalának másolása</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Open Containing Folder...</source>
        <translation>Tartalmazó mappa megnyitása…</translation>
    </message>
    <message>
        <location line="+141"/>
        <source>Tileset Columns Changed</source>
        <translation>A csempekészlet oszlopai megváltoztak</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The number of tile columns in the tileset '%1' appears to have changed from %2 to %3. Do you want to adjust tile references?</source>
        <translation>Úgy tűnik, hogy a(z) „%1” csempekészletben lévő csempeoszlopok száma megváltozott: %2 → %3. Szeretné hozzáigazítani a csempehivatkozásokat?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditPolygonTool</name>
    <message>
        <location line="+129" filename="../src/tiled/editpolygontool.cpp"/>
        <location line="+209"/>
        <source>Edit Polygons</source>
        <translation>Sokszögek szerkesztése</translation>
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
            <numerusform>Pont áthelyezése</numerusform>
            <numerusform>%n pont áthelyezése</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+26"/>
        <location line="+45"/>
        <source>Delete %n Node(s)</source>
        <translation>
            <numerusform>Csomópont törlése</numerusform>
            <numerusform>%n csomópont törlése</numerusform>
        </translation>
    </message>
    <message>
        <location line="-40"/>
        <location line="+215"/>
        <source>Join Nodes</source>
        <translation>Csomópontok összekapcsolása</translation>
    </message>
    <message>
        <location line="-214"/>
        <location line="+250"/>
        <source>Split Segments</source>
        <translation>Szakaszok felosztása</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditTerrainDialog</name>
    <message>
        <location line="+149" filename="../src/tiled/editterraindialog.cpp"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>New Terrain</source>
        <translation>Új terep</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::Eraser</name>
    <message>
        <location line="+35" filename="../src/tiled/eraser.cpp"/>
        <location line="+56"/>
        <source>Eraser</source>
        <translation>Radír</translation>
    </message>
    <message>
        <location line="-53"/>
        <location line="+54"/>
        <source>E</source>
        <translation>E</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ExportAsImageDialog</name>
    <message>
        <location line="+63" filename="../src/tiled/exportasimagedialog.cpp"/>
        <source>Export</source>
        <translation>Exportálás</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Export as Image</source>
        <translation>Exportálás képként</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 already exists.
Do you want to replace it?</source>
        <translation>A(z) %1 már létezik.
Le szeretné cserélni?</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Out of Memory</source>
        <translation>Elfogyott a memória</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not allocate sufficient memory for the image. Try reducing the zoom level or using a 64-bit version of Tiled.</source>
        <translation>Nem sikerült elegendő memóriát lefoglalni a képhez. Próbálja meg csökkenteni a nagyítási szintet, vagy használja a Tiled 64-bites verzióját.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image too Big</source>
        <translation>A kép túl nagy</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The resulting image would be %1 x %2 pixels and take %3 GB of memory. Tiled is unable to create such an image. Try reducing the zoom level.</source>
        <translation>Az eredményül kapott kép %1 x %2 képpont méretű lenne és %3 GB memóriát foglalna. A Tiled nem képes ilyen képet létrehozni. Próbálja meg csökkenteni a nagyítási szintet.</translation>
    </message>
    <message>
        <location line="+100"/>
        <source>Image</source>
        <translation>Kép</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileChangedWarning</name>
    <message>
        <location line="-600" filename="../src/tiled/documentmanager.cpp"/>
        <source>File change detected. Discard changes and reload the map?</source>
        <translation>Fájlváltozás észlelhető. Eldobja a módosításokat és újratölti a térképet?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileEdit</name>
    <message>
        <location line="+113" filename="../src/tiled/fileedit.cpp"/>
        <source>Choose a File</source>
        <translation>Fájl kiválasztása</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerDock</name>
    <message>
        <location line="+217" filename="../src/tiled/layerdock.cpp"/>
        <source>Layers</source>
        <translation>Rétegek</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Opacity:</source>
        <translation>Átlátszatlanság:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerModel</name>
    <message>
        <location line="+151" filename="../src/tiled/layermodel.cpp"/>
        <source>Layer</source>
        <translation>Réteg</translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Show Other Layers</source>
        <translation>Egyéb rétegek megjelenítése</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Hide Other Layers</source>
        <translation>Egyéb rétegek elrejtése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerOffsetTool</name>
    <message>
        <location line="+38" filename="../src/tiled/layeroffsettool.cpp"/>
        <location line="+94"/>
        <source>Offset Layers</source>
        <translation>Rétegek eltolása</translation>
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
        <location line="+39" filename="../src/tiled/magicwandtool.cpp"/>
        <location line="+58"/>
        <source>Magic Wand</source>
        <translation>Varázspálca</translation>
    </message>
    <message>
        <location line="-55"/>
        <location line="+56"/>
        <source>W</source>
        <translation>W</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MainWindow</name>
    <message>
        <location line="+236" filename="../src/tiled/mainwindow.cpp"/>
        <location line="+11"/>
        <source>Undo</source>
        <translation>Visszavonás</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+9"/>
        <source>Redo</source>
        <translation>Újra</translation>
    </message>
    <message>
        <location line="+94"/>
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
        <location line="+1402"/>
        <source>Random Mode</source>
        <translation>Véletlen mód</translation>
    </message>
    <message>
        <location line="-1399"/>
        <source>D</source>
        <translation>D</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+1397"/>
        <source>&amp;Layer</source>
        <translation>&amp;Réteg</translation>
    </message>
    <message>
        <location line="-1396"/>
        <location line="+1397"/>
        <source>&amp;New</source>
        <translation>Ú&amp;j</translation>
    </message>
    <message>
        <location line="-1210"/>
        <source>Object Types Editor</source>
        <translation>Objektumtípus szerkesztő</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ctrl+Shift+O</source>
        <translation>Ctrl+Shift+O</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Ctrl+Shift+Tab</source>
        <translation>Ctrl+Shift+Tab</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Ctrl+Tab</source>
        <translation>Ctrl+Tab</translation>
    </message>
    <message>
        <location line="+6"/>
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
        <location line="+133"/>
        <source>Error Opening Map</source>
        <translation>Hiba a térkép megnyitásakor</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+196"/>
        <location line="+296"/>
        <source>All Files (*)</source>
        <translation>Minden fájl (*)</translation>
    </message>
    <message>
        <location line="-481"/>
        <source>Open Map</source>
        <translation>Térkép megnyitása</translation>
    </message>
    <message>
        <location line="+25"/>
        <location line="+88"/>
        <source>Error Saving Map</source>
        <translation>Hiba a térkép mentésekor</translation>
    </message>
    <message>
        <location line="-46"/>
        <source>untitled.tmx</source>
        <translation>nevtelen.tmx</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Extension Mismatch</source>
        <translation>Kiterjesztés nem megfelelő</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The file extension does not match the chosen file type.</source>
        <translation>A fájl kiterjesztése nem illeszkedik a választott fájltípushoz.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Tiled may not automatically recognize your file when loading. Are you sure you want to save with this extension?</source>
        <translation>A Tiled esetleg nem ismeri fel automatikusan a fájlt a betöltéskor. Biztosan el szeretné menteni ezzel a kiterjesztéssel?</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Unsaved Changes</source>
        <translation>Mentetlen változtatások</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>There are unsaved changes. Do you want to save now?</source>
        <translation>Mentetlen változtatások vannak. Szeretné most elmenteni?</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Exported to %1</source>
        <translation>Exportálva ide: %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+117"/>
        <source>Error Exporting Map</source>
        <translation>Hiba a térkép exportálásakor</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Export As...</source>
        <translation>Exportálás másként…</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Non-unique file extension</source>
        <translation>Nem egyedi fájlkiterjesztés</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Non-unique file extension.
Please select specific format.</source>
        <translation>Nem egyedi fájlkiterjesztés.
Válasszon egy adott formátumot.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Unknown File Format</source>
        <translation>Ismeretlen fájlformátum</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The given filename does not have any known file extension.</source>
        <translation>A megadott fájlnév nem rendelkezik semmilyen ismert fájlkiterjesztéssel.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Some export files already exist:</source>
        <translation>Néhány exportálási fájl már létezik:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Do you want to replace them?</source>
        <translation>Le szeretné cserélni azokat?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Overwrite Files</source>
        <translation>Fájlok felülírása</translation>
    </message>
    <message>
        <location line="+621"/>
        <source>[*]%1</source>
        <translation>[*]%1</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Error Reloading Map</source>
        <translation>Hiba a térkép újratöltésekor</translation>
    </message>
    <message>
        <location line="-516"/>
        <location line="+5"/>
        <source>Error Reading Tileset</source>
        <translation>Hiba a csempekészlet olvasásakor</translation>
    </message>
    <message>
        <location line="+77"/>
        <source>Automatic Mapping Warning</source>
        <translation>Automatikus leképezés figyelmeztetés</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Automatic Mapping Error</source>
        <translation>Automatikus leképezés hiba</translation>
    </message>
    <message>
        <location line="-874"/>
        <location line="+1212"/>
        <source>Views and Toolbars</source>
        <translation>Nézetek és eszköztárak</translation>
    </message>
    <message>
        <location line="-1209"/>
        <location line="+1210"/>
        <source>Tile Animation Editor</source>
        <translation>Csempeanimáció szerkesztő</translation>
    </message>
    <message>
        <location line="-1208"/>
        <location line="+1209"/>
        <source>Tile Collision Editor</source>
        <translation>Csempeütközés szerkesztő</translation>
    </message>
    <message>
        <location line="-1175"/>
        <source>Alt+Left</source>
        <translation>Alt+bal</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Alt+Right</source>
        <translation>Alt+jobb</translation>
    </message>
    <message>
        <location line="+737"/>
        <source>Add External Tileset(s)</source>
        <translation>Külső csempekészletek hozzáadása</translation>
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
            <numerusform>Csempekészlet hozzáadása</numerusform>
            <numerusform>%n csempekészlet hozzáadása</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocument</name>
    <message>
        <location line="+246" filename="../src/tiled/mapdocument.cpp"/>
        <source>untitled.tmx</source>
        <translation>nevtelen.tmx</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>Resize Map</source>
        <translation>Térkép átméretezése</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Offset Map</source>
        <translation>Térkép eltolása</translation>
    </message>
    <message numerus="yes">
        <location line="+28"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Objektum forgatása</numerusform>
            <numerusform>%n objektum forgatása</numerusform>
        </translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer %1</source>
        <translation>%1. csemperéteg</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Object Layer %1</source>
        <translation>%1. objektumréteg</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Image Layer %1</source>
        <translation>%1. képréteg</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Copy of %1</source>
        <translation>%1 másolata</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Duplicate Layer</source>
        <translation>Réteg kettőzése</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Merge Layer Down</source>
        <translation>Rétegek összefésülése lefelé</translation>
    </message>
    <message>
        <location line="+238"/>
        <source>Tile</source>
        <translation>Csempe</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tileset Changes</source>
        <translation>Csempekészlet változásai</translation>
    </message>
    <message numerus="yes">
        <location line="+190"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Objektum kettőzése</numerusform>
            <numerusform>%n objektum kettőzése</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+21"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Objektum eltávolítása</numerusform>
            <numerusform>%n objektum eltávolítása</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+12"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Objektum áthelyezése rétegre</numerusform>
            <numerusform>%n objektum áthelyezése rétegre</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+37"/>
        <source>Move %n Object(s) Up</source>
        <translation>
            <numerusform>Objektum mozgatása fel</numerusform>
            <numerusform>%n objektum mozgatása fel</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+36"/>
        <source>Move %n Object(s) Down</source>
        <translation>
            <numerusform>Objektum mozgatása le</numerusform>
            <numerusform>%n objektum mozgatása le</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocumentActionHandler</name>
    <message>
        <location line="+60" filename="../src/tiled/mapdocumentactionhandler.cpp"/>
        <source>Ctrl+Shift+A</source>
        <translation>Ctrl+Shift+A</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Ctrl+Shift+D</source>
        <translation>Ctrl+Shift+D</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Ctrl+J</source>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+J</source>
        <translation>Ctrl+Shift+J</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Ctrl+Shift+Up</source>
        <translation>Ctrl+Shift+fel</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+Down</source>
        <translation>Ctrl+Shift+le</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+H</source>
        <translation>Ctrl+Shift+H</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Select &amp;All</source>
        <translation>Öss&amp;zes kijelölése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;None</source>
        <translation>Kijelölés &amp;megszüntetése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Crop to Selection</source>
        <translation>&amp;Vágás a kijelölésre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Tile Layer</source>
        <translation>&amp;Csemperéteg</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Object Layer</source>
        <translation>&amp;Objektumréteg</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Image Layer</source>
        <translation>&amp;Képréteg</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+197"/>
        <source>Layer via Copy</source>
        <translation>Réteg másoláson keresztül</translation>
    </message>
    <message>
        <location line="-196"/>
        <location line="+196"/>
        <source>Layer via Cut</source>
        <translation>Réteg kivágáson keresztül</translation>
    </message>
    <message>
        <location line="-192"/>
        <source>Select Pre&amp;vious Layer</source>
        <translation>Elő&amp;ző réteg kijelölése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;Next Layer</source>
        <translation>&amp;Következő réteg kijelölése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R&amp;aise Layer</source>
        <translation>Réteg &amp;előre hozása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Lower Layer</source>
        <translation>Réteg &amp;hátra küldése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show/&amp;Hide all Other Layers</source>
        <translation>Összes egyéb réteg &amp;megjelenítése vagy elrejtése</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Cut</source>
        <translation>Kivágás</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
    <message numerus="yes">
        <location line="+327"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Objektum kettőzése</numerusform>
            <numerusform>%n objektum kettőzése</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+1"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Objektum eltávolítása</numerusform>
            <numerusform>%n objektum eltávolítása</numerusform>
        </translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Duplicate Objects</source>
        <translation>Objektumok kettőzése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Objects</source>
        <translation>Objektumok eltávolítása</translation>
    </message>
    <message>
        <location line="-417"/>
        <source>&amp;Duplicate Layer</source>
        <translation>Réteg &amp;kettőzése</translation>
    </message>
    <message>
        <location line="-84"/>
        <source>Ctrl+PgUp</source>
        <translation>Ctrl+PgUp</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgDown</source>
        <translation>Ctrl+PgDown</translation>
    </message>
    <message>
        <location line="+82"/>
        <source>&amp;Merge Layer Down</source>
        <translation>Rétegek öss&amp;zefésülése lefelé</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Layer</source>
        <translation>Réteg &amp;eltávolítása</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Layer &amp;Properties...</source>
        <translation>Réteg &amp;tulajdonságai…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapObjectModel</name>
    <message>
        <location line="+150" filename="../src/tiled/mapobjectmodel.cpp"/>
        <source>Change Object Name</source>
        <translation>Objektumnév megváltoztatása</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Change Object Type</source>
        <translation>Objektumtípus megváltoztatása</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapsDock</name>
    <message>
        <location line="+83" filename="../src/tiled/mapsdock.cpp"/>
        <source>Browse...</source>
        <translation>Tallózás…</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Choose the Maps Folder</source>
        <translation>A térképek mappa kiválasztása</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Maps</source>
        <translation>Térképek</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MiniMapDock</name>
    <message>
        <location line="+60" filename="../src/tiled/minimapdock.cpp"/>
        <source>Mini-map</source>
        <translation>Minitérkép</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewMapDialog</name>
    <message>
        <location line="+2" filename="../src/tiled/newmapdialog.cpp"/>
        <location line="-14" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Orthogonal</source>
        <translation>Ortogonális</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Isometric</source>
        <translation>Izometrikus</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Isometric (Staggered)</source>
        <translation>Izometrikus (lépcsőzetes)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+1" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Hexagonal (Staggered)</source>
        <translation>Hexagonális (lépcsőzetes)</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Tile Layer 1</source>
        <translation>1. csemperéteg</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Memory Usage Warning</source>
        <translation>Memóriahasználati figyelmeztetés</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile layers for this map will consume %L1 GB of memory each. Not creating one by default.</source>
        <translation>A térkép csemperétegei %L1 GB memóriát fognak használni egyenként. Alapértelmezetten egy sem jön létre.</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>%1 x %2 pixels</source>
        <translation>%1 x %2 képpont</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewTilesetDialog</name>
    <message>
        <location line="-40" filename="../src/tiled/newtilesetdialog.cpp"/>
        <location line="+7"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Failed to load tileset image '%1'.</source>
        <translation>Nem sikerült betölteni a(z) „%1” csempekészlet képet.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No tiles found in the tileset image when using the given tile size, margin and spacing!</source>
        <translation>Nem találhatók csempék a csempekészlet képben a megadott csempeméret, margó és térköz használatakor.</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Tileset Image</source>
        <translation>Csempekészlet kép</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectSelectionTool</name>
    <message>
        <location line="+309" filename="../src/tiled/objectselectiontool.cpp"/>
        <location line="+300"/>
        <source>Select Objects</source>
        <translation>Objektumok kijelölése</translation>
    </message>
    <message>
        <location line="-298"/>
        <location line="+299"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message numerus="yes">
        <location line="-186"/>
        <location line="+582"/>
        <source>Move %n Object(s)</source>
        <translation>
            <numerusform>Objektum áthelyezése</numerusform>
            <numerusform>%n objektum áthelyezése</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+87"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Objektum forgatása</numerusform>
            <numerusform>%n objektum forgatása</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+266"/>
        <source>Resize %n Object(s)</source>
        <translation>
            <numerusform>Objektum átméretezése</numerusform>
            <numerusform>%n objektum átméretezése</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesEditor</name>
    <message>
        <location line="+224" filename="../src/tiled/objecttypeseditor.cpp"/>
        <source>Add Object Type</source>
        <translation>Objektumtípus hozzáadása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Object Type</source>
        <translation>Objektumtípus eltávolítása</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Tulajdonság hozzáadása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Property</source>
        <translation>Tulajdonság eltávolítása</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+323"/>
        <source>Rename Property</source>
        <translation>Tulajdonság átnevezése</translation>
    </message>
    <message>
        <location line="-265"/>
        <location line="+129"/>
        <source>Error Writing Object Types</source>
        <translation>Hiba az objektumtípusok írásakor</translation>
    </message>
    <message>
        <location line="-128"/>
        <source>Error writing to %1:
%2</source>
        <translation>Hiba az írás közben: %1:
%2</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Choose Object Types File</source>
        <translation>Objektumtípus fájl kiválasztása</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+34"/>
        <location line="+44"/>
        <source>Object Types files (*.xml)</source>
        <translation>Objektumtípus fájlok (*.xml)</translation>
    </message>
    <message>
        <location line="-62"/>
        <location line="+44"/>
        <source>Error Reading Object Types</source>
        <translation>Hiba az objektumtípusok olvasásakor</translation>
    </message>
    <message>
        <location line="-28"/>
        <source>Import Object Types</source>
        <translation>Objektumtípusok importálása</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Export Object Types</source>
        <translation>Objektumtípusok exportálása</translation>
    </message>
    <message>
        <location line="+144"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesModel</name>
    <message>
        <location line="+59" filename="../src/tiled/objecttypesmodel.cpp"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Color</source>
        <translation>Szín</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectsDock</name>
    <message>
        <location line="+170" filename="../src/tiled/objectsdock.cpp"/>
        <source>Object Properties</source>
        <translation>Objektum tulajdonságai</translation>
    </message>
    <message>
        <location line="-1"/>
        <source>Add Object Layer</source>
        <translation>Objektumréteg hozzáadása</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Objects</source>
        <translation>Objektumok</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Move Objects Up</source>
        <translation>Objektumok mozgatása fel</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Move Objects Down</source>
        <translation>Objektumok mozgatása le</translation>
    </message>
    <message numerus="yes">
        <location line="+17"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Objektum áthelyezése rétegre</numerusform>
            <numerusform>%n objektum áthelyezése rétegre</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PatreonDialog</name>
    <message>
        <location line="+68" filename="../src/tiled/patreondialog.cpp"/>
        <source>&lt;html>&lt;head/>&lt;body>
&lt;h3>Thank you for support!&lt;/h3>
&lt;p>Your support as a patron makes a big difference to me as the main developer and maintainer of Tiled. It allows me to spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p>
&lt;p>Keep an eye out for exclusive updates in the Activity feed on my Patreon page to find out what I've been up to in the time I could spend on Tiled thanks to your support!&lt;/p>
&lt;p>&lt;i>Thorbj&amp;oslash;rn Lindeijer&lt;/i>&lt;/p>&lt;/body>&lt;/html></source>
        <translation>&lt;html>&lt;head/>&lt;body>
&lt;h3>Köszönöm a támogatását!&lt;/h3>
&lt;p>A pártfogóként való támogatása nagy változást hozhat nekem, a Tiled fő fejlesztőjének és karbantartójának. Lehetővé teszi számomra, hogy kevesebb időt kelljen fizetett munkával foglalkoznom máshol, és helyette több időt tölthessek a Tiled programon dolgozva.&lt;/p>
&lt;p>Kövesse figyelemmel az exkluzív frissítéseket az Activity hírforrásban a Patreon oldalamon, hogy többet tudjon meg arról, amiket elvégeztem abban az időben, amelyet a Tiled programra tudtam szánni a támogatásának köszönhetően!&lt;/p>
&lt;p>&lt;i>Thorbj&amp;oslash;rn Lindeijer&lt;/i>&lt;/p>&lt;/body>&lt;/html></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I'm no longer a patron</source>
        <translation>Többé nem vagyok pártfogó</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&lt;html>&lt;head/>&lt;body>
&lt;h3>With your help I can continue to improve Tiled!&lt;/h3>
&lt;p>Please consider supporting me as a patron. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p>
&lt;p>Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p>
&lt;p>&lt;i>Thorbj&amp;oslash;rn Lindeijer&lt;/i>&lt;/p>&lt;/body>&lt;/html></source>
        <translation>&lt;html>&lt;head/>&lt;body>
&lt;h3>A segítségével folytathatom a Tiled továbbfejlesztését!&lt;/h3>
&lt;p>Kérem, hogy fontolja meg azt, hogy pártfogóként támogasson engem. A támogatása nagy változást jelentene nekem, a Tiled fő fejlesztőjének és karbantartójának. Kevesebb időt kellene fizetett munkával foglalkoznom máshol, és helyette több időt tölthetnék a Tiled programon dolgozva.&lt;/p>
&lt;p>Minden kis támogatás segít. A Tiled programnak nagyon sok felhasználója van, és ha mindegyikük egy kis támogatással hozzájárulna havonta, akkor lenne időm annak biztosításához, hogy a Tiled egyre jobb legyen.&lt;/p>
&lt;p>&lt;i>Thorbj&amp;oslash;rn Lindeijer&lt;/i>&lt;/p>&lt;/body>&lt;/html></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I'm already a patron!</source>
        <translation>Már pártfogó vagyok!</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PreferencesDialog</name>
    <message>
        <location line="-127" filename="../src/tiled/preferencesdialog.cpp"/>
        <location line="+123"/>
        <source>System default</source>
        <translation>Rendszer alapértelmezettje</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Last checked: %1</source>
        <translation>Utoljára ellenőrizve: %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertiesDock</name>
    <message>
        <location line="+278" filename="../src/tiled/propertiesdock.cpp"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
    <message>
        <location line="+104"/>
        <source>Add Property</source>
        <translation>Tulajdonság hozzáadása</translation>
    </message>
    <message>
        <location line="-102"/>
        <location line="+104"/>
        <source>Rename Property</source>
        <translation>Tulajdonság átnevezése</translation>
    </message>
    <message>
        <location line="-71"/>
        <source>Convert To</source>
        <translation>Átalakítás erre</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rename...</source>
        <translation>Átnevezés…</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove</source>
        <translation>Eltávolítás</translation>
    </message>
    <message>
        <location line="+65"/>
        <source>Properties</source>
        <translation>Tulajdonságok</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove Property</source>
        <translation>Tulajdonság eltávolítása</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertyBrowser</name>
    <message>
        <location line="+13" filename="../src/tiled/propertybrowser.cpp"/>
        <source>Horizontal</source>
        <translation>Vízszintesen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertical</source>
        <translation>Függőlegesen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Top Down</source>
        <translation>Fentről le</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>Kézi</translation>
    </message>
    <message>
        <location line="+485"/>
        <source>Columns</source>
        <translation>Oszlopok</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Source</source>
        <translation>Forrás</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Relative chance this tile will be picked</source>
        <translation>Viszonylagos esély, hogy ez a csempe lesz kiválasztva</translation>
    </message>
    <message>
        <location line="+286"/>
        <source>Error Reading Tileset</source>
        <translation>Hiba a csempekészlet olvasásakor</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Custom Properties</source>
        <translation>Egyéni tulajdonságok</translation>
    </message>
    <message>
        <location line="-637"/>
        <source>Map</source>
        <translation>Térkép</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer Format</source>
        <translation>Csemperéteg formátuma</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Render Order</source>
        <translation>Csempemegjelenítési sorrend</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Background Color</source>
        <translation>Háttérszín</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Object</source>
        <translation>Objektum</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+26"/>
        <location line="+74"/>
        <location line="+60"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location line="-157"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+21"/>
        <source>Visible</source>
        <translation>Látható</translation>
    </message>
    <message>
        <location line="-435"/>
        <location line="+415"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="-414"/>
        <location line="+415"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="-413"/>
        <source>Odd</source>
        <translation>Páratlan</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Even</source>
        <translation>Páros</translation>
    </message>
    <message>
        <location line="+342"/>
        <source>Orientation</source>
        <translation>Tájolás</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+167"/>
        <source>Tile Width</source>
        <translation>Csempe szélessége</translation>
    </message>
    <message>
        <location line="-166"/>
        <location line="+167"/>
        <source>Tile Height</source>
        <translation>Csempe magassága</translation>
    </message>
    <message>
        <location line="-165"/>
        <source>Tile Side Length (Hex)</source>
        <translation>Csempe oldalhossza (hatszög)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Stagger Axis</source>
        <translation>Lépcsőzetesség tengelye</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Stagger Index</source>
        <translation>Lépcsőzetesség indexe</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Rotation</source>
        <translation>Forgatás</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Flipping</source>
        <translation>Tükrözés</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Opacity</source>
        <translation>Átlátszatlanság</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Horizontal Offset</source>
        <translation>Vízszintes eltolás</translation>
    </message>
    <message>
        <location line="-37"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Vertical Offset</source>
        <translation>Függőleges eltolás</translation>
    </message>
    <message>
        <location line="-41"/>
        <source>Tile Layer</source>
        <translation>Csemperéteg</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Object Layer</source>
        <translation>Objektumréteg</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Color</source>
        <translation>Szín</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Drawing Order</source>
        <translation>Rajzolási sorrend</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Image Layer</source>
        <translation>Képréteg</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+37"/>
        <location line="+39"/>
        <source>Image</source>
        <translation>Kép</translation>
    </message>
    <message>
        <location line="-71"/>
        <location line="+39"/>
        <source>Transparent Color</source>
        <translation>Átlátszó szín</translation>
    </message>
    <message>
        <location line="-29"/>
        <source>Tileset</source>
        <translation>Csempekészlet</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Filename</source>
        <translation>Fájlnév</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Drawing Offset</source>
        <translation>Rajzolási eltolás</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Margin</source>
        <translation>Margó</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spacing</source>
        <translation>Térköz</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Tile</source>
        <translation>Csempe</translation>
    </message>
    <message>
        <location line="-133"/>
        <location line="+134"/>
        <source>ID</source>
        <translation>Azonosító</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Probability</source>
        <translation>Valószínűség</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Terrain</source>
        <translation>Terep</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::SelectSameTileTool</name>
    <message>
        <location line="+33" filename="../src/tiled/selectsametiletool.cpp"/>
        <location line="+62"/>
        <source>Select Same Tile</source>
        <translation>Azonos csempe kijelölése</translation>
    </message>
    <message>
        <location line="-59"/>
        <location line="+60"/>
        <source>S</source>
        <translation>S</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::StampBrush</name>
    <message>
        <location line="+41" filename="../src/tiled/stampbrush.cpp"/>
        <location line="+127"/>
        <source>Stamp Brush</source>
        <translation>Bélyegecset</translation>
    </message>
    <message>
        <location line="-124"/>
        <location line="+125"/>
        <source>B</source>
        <translation>B</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainBrush</name>
    <message>
        <location line="+44" filename="../src/tiled/terrainbrush.cpp"/>
        <location line="+114"/>
        <source>Terrain Brush</source>
        <translation>Terepecset</translation>
    </message>
    <message>
        <location line="-111"/>
        <location line="+112"/>
        <source>T</source>
        <translation>T</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainDock</name>
    <message>
        <location line="+222" filename="../src/tiled/terraindock.cpp"/>
        <source>Terrains</source>
        <translation>Terepek</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Erase Terrain</source>
        <translation>Terep törlése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainView</name>
    <message>
        <location line="+97" filename="../src/tiled/terrainview.cpp"/>
        <source>Terrain &amp;Properties...</source>
        <translation>Terep t&amp;ulajdonságai…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TextPropertyEdit</name>
    <message>
        <location line="+121" filename="../src/tiled/textpropertyedit.cpp"/>
        <source>...</source>
        <translation>…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileAnimationEditor</name>
    <message>
        <location line="-58" filename="../src/tiled/tileanimationeditor.cpp"/>
        <source>Delete Frames</source>
        <translation>Keretek törlése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileCollisionEditor</name>
    <message>
        <location line="+263" filename="../src/tiled/tilecollisioneditor.cpp"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cut</source>
        <translation>Kivágás</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Tile Collision Editor</source>
        <translation>Csempeütközés szerkesztő</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileSelectionTool</name>
    <message>
        <location line="+34" filename="../src/tiled/tileselectiontool.cpp"/>
        <location line="+96"/>
        <source>Rectangular Select</source>
        <translation>Téglalap kijelölés</translation>
    </message>
    <message>
        <location line="-93"/>
        <location line="+94"/>
        <source>R</source>
        <translation>R</translation>
    </message>
    <message>
        <location line="-71"/>
        <source>%1, %2 - Rectangle: (%3 x %4)</source>
        <translation>%1, %2 - téglalap: (%3 x %4)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampModel</name>
    <message>
        <location line="+78" filename="../src/tiled/tilestampmodel.cpp"/>
        <source>Stamp</source>
        <translation>Bélyeg</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Probability</source>
        <translation>Valószínűség</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampsDock</name>
    <message>
        <location line="+194" filename="../src/tiled/tilestampsdock.cpp"/>
        <source>Delete Stamp</source>
        <translation>Bélyeg törlése</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Variation</source>
        <translation>Variáció eltávolítása</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Choose the Stamps Folder</source>
        <translation>A bélyegek mappa kiválasztása</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tile Stamps</source>
        <translation>Csempebélyegek</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add New Stamp</source>
        <translation>Új bélyeg hozzáadása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add Variation</source>
        <translation>Variáció hozzáadása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Duplicate Stamp</source>
        <translation>Bélyeg kettőzése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete Selected</source>
        <translation>Kijelölt törlése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Stamps Folder</source>
        <translation>Bélyegek mappa beállítása</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Filter</source>
        <translation>Szűrő</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetDock</name>
    <message>
        <location line="+731" filename="../src/tiled/tilesetdock.cpp"/>
        <source>Remove Tileset</source>
        <translation>Csempekészlet eltávolítása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The tileset "%1" is still in use by the map!</source>
        <translation>A(z) „%1” csempekészletet még mindig használja a térkép!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove this tileset and all references to the tiles in this tileset?</source>
        <translation>Eltávolítja ezt a csempekészletet és a csempekészletben lévő összes hivatkozást a csempékre?</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Tilesets</source>
        <translation>Csempekészletek</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>New Tileset</source>
        <translation>Új csempekészlet</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Import Tileset</source>
        <translation>Csempekészlet &amp;importálása</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Export Tileset As...</source>
        <translation>Csempekészlet &amp;exportálása másként…</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile&amp;set Properties</source>
        <translation>Csempekészlet t&amp;ulajdonságai</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Tileset</source>
        <translation>Csempekészlet &amp;eltávolítása</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+128"/>
        <location line="+15"/>
        <source>Add Tiles</source>
        <translation>Csempék hozzáadása</translation>
    </message>
    <message>
        <location line="-142"/>
        <location line="+199"/>
        <location line="+13"/>
        <source>Remove Tiles</source>
        <translation>Csempék eltávolítása</translation>
    </message>
    <message>
        <location line="-121"/>
        <source>Error saving tileset: %1</source>
        <translation>Hiba a csempekészlet mentésekor: %1</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Could not load "%1"!</source>
        <translation>Nem sikerült betölteni: „%1”!</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>One or more of the tiles to be removed are still in use by the map!</source>
        <translation>Az eltávolítandó csempék közül egyet vagy többet még mindig használ a térkép!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove all references to these tiles?</source>
        <translation>Eltávolítja az ezekre a csempékre mutató összes hivatkozást?</translation>
    </message>
    <message>
        <location line="-207"/>
        <source>Edit &amp;Terrain Information</source>
        <translation>&amp;Terepinformációk szerkesztése</translation>
    </message>
    <message>
        <location line="+69"/>
        <location line="+23"/>
        <source>Export Tileset</source>
        <translation>Csempekészlet exportálása</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiled csempekészletfájlok (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetParametersEdit</name>
    <message>
        <location line="+48" filename="../src/tiled/tilesetparametersedit.cpp"/>
        <source>Edit...</source>
        <translation>Szerkesztés…</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetView</name>
    <message>
        <location line="+628" filename="../src/tiled/tilesetview.cpp"/>
        <source>Add Terrain Type</source>
        <translation>Tereptípus hozzáadása</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Set Terrain Image</source>
        <translation>Terep képének beállítása</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Tile &amp;Properties...</source>
        <translation>Csempe t&amp;ulajdonságai…</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Show &amp;Grid</source>
        <translation>&amp;Rács megjelenítése</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TmxMapFormat</name>
    <message>
        <location line="+62" filename="../src/tiled/tmxmapformat.h"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled térképfájlok (*.tmx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TsxTilesetFormat</name>
    <message>
        <location line="+24"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiled csempekészletfájlok (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::UndoDock</name>
    <message>
        <location line="+64" filename="../src/tiled/undodock.cpp"/>
        <source>History</source>
        <translation>Előzmények</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;empty></source>
        <translation>&lt;üres></translation>
    </message>
</context>
<context>
    <name>Tmw::TmwPlugin</name>
    <message>
        <location line="+47" filename="../src/plugins/tmw/tmwplugin.cpp"/>
        <source>Multiple collision layers found!</source>
        <translation>Több ütközésréteg található!</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>No collision layer found!</source>
        <translation>Nem található ütközésréteg!</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Could not open file for writing.</source>
        <translation>Nem sikerült megnyitni a fájlt írásra.</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>TMW-eAthena collision files (*.wlk)</source>
        <translation>TMW-eAthena ütközésfájlok (*.wlk)</translation>
    </message>
</context>
<context>
    <name>TmxViewer</name>
    <message>
        <location line="+182" filename="../src/tmxviewer/tmxviewer.cpp"/>
        <source>TMX Viewer</source>
        <translation>TMX megjelenítő</translation>
    </message>
</context>
<context>
    <name>Undo Commands</name>
    <message>
        <location line="+67" filename="../src/tiled/addremovelayer.h"/>
        <source>Add Layer</source>
        <translation>Réteg hozzáadása</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Remove Layer</source>
        <translation>Réteg eltávolítása</translation>
    </message>
    <message>
        <location line="+76" filename="../src/tiled/addremovemapobject.cpp"/>
        <source>Add Object</source>
        <translation>Objektum hozzáadása</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove Object</source>
        <translation>Objektum eltávolítása</translation>
    </message>
    <message>
        <location line="+66" filename="../src/tiled/addremovetileset.cpp"/>
        <source>Add Tileset</source>
        <translation>Csempekészlet hozzáadása</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tileset</source>
        <translation>Csempekészlet eltávolítása</translation>
    </message>
    <message>
        <location line="+36" filename="../src/tiled/changemapobject.cpp"/>
        <source>Change Object</source>
        <translation>Objektum megváltoztatása</translation>
    </message>
    <message>
        <location line="+39" filename="../src/tiled/changeobjectgroupproperties.cpp"/>
        <source>Change Object Layer Properties</source>
        <translation>Objektumréteg tulajdonságainak megváltoztatása</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/changeproperties.cpp"/>
        <source>Change %1 Properties</source>
        <translation>%1 tulajdonság megváltoztatása</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Set Property</source>
        <translation>Tulajdonság beállítása</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Tulajdonság hozzáadása</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Remove Property</source>
        <translation>Tulajdonság eltávolítása</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Rename Property</source>
        <translation>Tulajdonság átnevezése</translation>
    </message>
    <message>
        <location line="+31" filename="../src/tiled/changeselectedarea.cpp"/>
        <source>Change Selection</source>
        <translation>Kijelölés megváltoztatása</translation>
    </message>
    <message>
        <location line="+39" filename="../src/tiled/erasetiles.cpp"/>
        <source>Erase</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location line="-30" filename="../src/tiled/bucketfilltool.cpp"/>
        <source>Fill Area</source>
        <translation>Terület kitöltése</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/movemapobject.cpp"/>
        <location line="+12"/>
        <source>Move Object</source>
        <translation>Objektum áthelyezése</translation>
    </message>
    <message>
        <location line="+41" filename="../src/tiled/movemapobjecttogroup.cpp"/>
        <source>Move Object to Layer</source>
        <translation>Objektum áthelyezése rétegre</translation>
    </message>
    <message>
        <location line="+31" filename="../src/tiled/movetileset.cpp"/>
        <source>Move Tileset</source>
        <translation>Csempekészlet áthelyezése</translation>
    </message>
    <message>
        <location line="+42" filename="../src/tiled/offsetlayer.cpp"/>
        <source>Offset Layer</source>
        <translation>Réteg eltolása</translation>
    </message>
    <message>
        <location line="+51" filename="../src/tiled/painttilelayer.cpp"/>
        <location line="+22"/>
        <source>Paint</source>
        <translation>Kifestés</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/renamelayer.cpp"/>
        <source>Rename Layer</source>
        <translation>Réteg átnevezése</translation>
    </message>
    <message>
        <location line="+37" filename="../src/tiled/resizetilelayer.cpp"/>
        <source>Resize Layer</source>
        <translation>Réteg átméretezése</translation>
    </message>
    <message>
        <location line="+32" filename="../src/tiled/resizemap.cpp"/>
        <source>Resize Map</source>
        <translation>Térkép átméretezése</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/resizemapobject.cpp"/>
        <location line="+12"/>
        <source>Resize Object</source>
        <translation>Objektum átméretezése</translation>
    </message>
    <message>
        <location line="-788" filename="../src/tiled/tilesetdock.cpp"/>
        <source>Import Tileset</source>
        <translation>Csempekészlet importálása</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Export Tileset</source>
        <translation>Csempekészlet exportálása</translation>
    </message>
    <message>
        <location line="+36" filename="../src/tiled/tilesetchanges.cpp"/>
        <source>Change Tileset Name</source>
        <translation>Csempekészlet nevének megváltoztatása</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Change Drawing Offset</source>
        <translation>Rajzolási eltolás megváltoztatása</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Edit Tileset</source>
        <translation>Csempekészlet szerkesztése</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Change Columns</source>
        <translation>Oszlopok megváltoztatása</translation>
    </message>
    <message>
        <location line="+37" filename="../src/tiled/movelayer.cpp"/>
        <source>Lower Layer</source>
        <translation>Réteg hátra küldése</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Layer</source>
        <translation>Réteg előre hozása</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/changepolygon.cpp"/>
        <location line="+12"/>
        <source>Change Polygon</source>
        <translation>Sokszög megváltoztatása</translation>
    </message>
    <message>
        <location line="+69" filename="../src/tiled/addremoveterrain.cpp"/>
        <source>Add Terrain</source>
        <translation>Terep hozzáadása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Terrain</source>
        <translation>Terep eltávolítása</translation>
    </message>
    <message>
        <location line="+39" filename="../src/tiled/changeimagelayerproperties.cpp"/>
        <source>Change Image Layer Properties</source>
        <translation>Képréteg tulajdonságainak megváltoztatása</translation>
    </message>
    <message>
        <location line="+133" filename="../src/tiled/changetileterrain.cpp"/>
        <source>Change Tile Terrain</source>
        <translation>Csempeterep megváltoztatása</translation>
    </message>
    <message>
        <location line="-135" filename="../src/tiled/editterraindialog.cpp"/>
        <source>Change Terrain Image</source>
        <translation>Terep képének megváltoztatása</translation>
    </message>
    <message>
        <location line="+41" filename="../src/tiled/changelayer.cpp"/>
        <source>Show Layer</source>
        <translation>Réteg megjelenítése</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Layer</source>
        <translation>Réteg elrejtése</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Change Layer Opacity</source>
        <translation>Réteg átlátszatlanságának megváltoztatása</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Change Layer Offset</source>
        <translation>Réteg eltolásának megváltoztatása</translation>
    </message>
    <message>
        <location line="+31" filename="../src/tiled/changemapobject.cpp"/>
        <source>Show Object</source>
        <translation>Objektum megjelenítése</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Object</source>
        <translation>Objektum elrejtése</translation>
    </message>
    <message>
        <location line="+37" filename="../src/tiled/renameterrain.cpp"/>
        <source>Change Terrain Name</source>
        <translation>Terep nevének megváltoztatása</translation>
    </message>
    <message>
        <location line="+69" filename="../src/tiled/addremovetiles.cpp"/>
        <source>Add Tiles</source>
        <translation>Csempék hozzáadása</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tiles</source>
        <translation>Csempék eltávolítása</translation>
    </message>
    <message>
        <location line="+36" filename="../src/tiled/changeimagelayerposition.cpp"/>
        <source>Change Image Layer Position</source>
        <translation>Képréteg helyzetének megváltoztatása</translation>
    </message>
    <message>
        <location line="+46" filename="../src/tiled/changemapobjectsorder.cpp"/>
        <location line="+67" filename="../src/tiled/raiselowerhelper.cpp"/>
        <source>Raise Object</source>
        <translation>Objektum előre hozása</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+29" filename="../src/tiled/raiselowerhelper.cpp"/>
        <source>Lower Object</source>
        <translation>Objektum hátra küldése</translation>
    </message>
    <message>
        <location line="+35" filename="../src/tiled/changetileanimation.cpp"/>
        <source>Change Tile Animation</source>
        <translation>Csempeanimáció megváltoztatása</translation>
    </message>
    <message>
        <location line="+16" filename="../src/tiled/changetileobjectgroup.cpp"/>
        <source>Change Tile Collision</source>
        <translation>Csempeütközés megváltoztatása</translation>
    </message>
    <message>
        <location line="+43" filename="../src/tiled/raiselowerhelper.cpp"/>
        <source>Raise Object To Top</source>
        <translation>Objektum előre hozása legfelülre</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Lower Object To Bottom</source>
        <translation>Objektum hátra küldése legalulra</translation>
    </message>
    <message>
        <location line="+40" filename="../src/tiled/rotatemapobject.cpp"/>
        <location line="+12"/>
        <source>Rotate Object</source>
        <translation>Objektum forgatása</translation>
    </message>
    <message>
        <location line="+41" filename="../src/tiled/changemapproperty.cpp"/>
        <source>Change Tile Width</source>
        <translation>Csempeszélesség megváltoztatása</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Tile Height</source>
        <translation>Csempemagasság megváltoztatása</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Hex Side Length</source>
        <translation>Csempe hatszög oldalhosszának megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Background Color</source>
        <translation>Háttérszín megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Axis</source>
        <translation>Lépcsőzetesség tengelyének megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Index</source>
        <translation>Lépcsőzetesség indexének megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Orientation</source>
        <translation>Tájolás megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Render Order</source>
        <translation>Megjelenítési sorrend megváltoztatása</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Layer Data Format</source>
        <translation>Réteg adatformátumának megváltoztatása</translation>
    </message>
    <message>
        <location line="+41" filename="../src/tiled/changetileprobability.cpp"/>
        <location line="+14"/>
        <source>Change Tile Probability</source>
        <translation>Csempe valószínűségének megváltoztatása</translation>
    </message>
    <message>
        <location line="-134" filename="../src/tiled/adjusttileindexes.cpp"/>
        <location line="+89"/>
        <source>Adjust Tile Indexes</source>
        <translation>Csempeindexek beállítása</translation>
    </message>
    <message>
        <location line="+39" filename="../src/tiled/changetileimagesource.cpp"/>
        <source>Change Tile Image</source>
        <translation>Csempe képének megváltoztatása</translation>
    </message>
    <message>
        <location line="+33" filename="../src/tiled/replacetileset.cpp"/>
        <source>Replace Tileset</source>
        <translation>Csempekészlet cseréje</translation>
    </message>
    <message numerus="yes">
        <location line="+39" filename="../src/tiled/flipmapobjects.cpp"/>
        <source>Flip %n Object(s)</source>
        <translation>
            <numerusform>Objektum tükrözése</numerusform>
            <numerusform>%n objektum tükrözése</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Utils</name>
    <message>
        <location line="+37" filename="../src/tiled/utils.cpp"/>
        <source>Image files</source>
        <translation>Képfájlok</translation>
    </message>
</context>
</TS>
