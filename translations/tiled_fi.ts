<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fi">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../src/tiled/aboutdialog.ui" line="+14"/>
        <source>About Tiled</source>
        <translation>Tietoja Tiled:istä</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Donate</source>
        <translation>Lahjoita</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/tiled/aboutdialog.cpp" line="+36"/>
        <source>&lt;p align="center"&gt;&lt;font size="+2"&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align="center"&gt;Copyright 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(see the AUTHORS file for a full list of contributors)&lt;/p&gt;
&lt;p align="center"&gt;You may modify and redistribute this program under the terms of the GPL (version 2 or later). A copy of the GPL is contained in the 'COPYING' file distributed with Tiled.&lt;/p&gt;
&lt;p align="center"&gt;&lt;a href="http://www.mapeditor.org/"&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</source>
        <translation>&lt;p align="center"&gt;&lt;font size="+2"&gt;&lt;b&gt;Tiled Kenttäeditori&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Versio %1&lt;/i&gt;&lt;/p&gt;
&lt;p align="center"&gt;Copyright 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(katso AUTHORS tiedosto nähdäksesi kaikki osallistujat)&lt;/p&gt;
&lt;p align="center"&gt;Voit muokata ja levittää tätä ohjelmaa GPL -lisenssin alaisesti (versio 2 tai uudempi). Kopio lisenssistä on 'COPYING' tiedostossa.&lt;/p&gt;
&lt;p align="center"&gt;&lt;a href="http://www.mapeditor.org/"&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</translation>
    </message>
</context>
<context>
    <name>AddPropertyDialog</name>
    <message>
        <location filename="../src/tiled/addpropertydialog.ui" line="+14"/>
        <source>Add Property</source>
        <translation>Lisää ominaisuus</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Property name</source>
        <translation>Ominaisuuden nimi</translation>
    </message>
</context>
<context>
    <name>Command line</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="+229"/>
        <source>Export syntax is --export-map [format] &lt;tmx file&gt; &lt;target file&gt;</source>
        <translation>Viennin syntaksi on --export-map [tiedostomuoto] &lt;tmx-tiedosto&gt; &lt;kohdetiedosto&gt;</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Format not recognized (see --export-formats)</source>
        <translation>Tiedostomuotoa ei tunnistettu (katso --export-formats)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Non-unique file extension. Can't determine correct export format.</source>
        <translation>Epäselvä tiedostomuoto. Oikeaa vientimuotoa ei havaittu.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>No exporter found for target file.</source>
        <translation>Ei löydetty kääntäjää halutulle tiedostomuodolle.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Failed to load source map.</source>
        <translation>Kentän lähdetiedoston lataus epäonnistui.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Failed to export map to target file.</source>
        <translation>Kentän muuntaminen kohdetiedostoksi epäonnistui.</translation>
    </message>
</context>
<context>
    <name>CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.ui" line="+14"/>
        <source>Properties</source>
        <translation>Ominaisuudet</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>&amp;Save map before executing</source>
        <translation>&amp;Tallenna kenttä ennen suoritusta</translation>
    </message>
</context>
<context>
    <name>CommandLineHandler</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="-188"/>
        <source>Display the version</source>
        <translation>Näytä versio</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Only check validity of arguments</source>
        <translation>Tarkista ainoastaan argumenttien kelvollisuus</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Disable hardware accelerated rendering</source>
        <translation>Poista laitteistopohjainen kiihdytys käytöstä</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Export the specified tmx file to target</source>
        <translation>Vie määritetty tmx -tiedosto kohteeseen</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Print a list of supported export formats</source>
        <translation>Näytä lista tuetuista tiedostomuodoista</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Start a new instance, even if an instance is already running</source>
        <translation>Käynnistä uusi instanssi vaikka sellainen olisi jo käynnissä</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Export formats:</source>
        <translation type="unfinished">Tiedostomuodot</translation>
    </message>
</context>
<context>
    <name>CommandLineParser</name>
    <message>
        <location filename="../src/tiled/commandlineparser.cpp" line="+75"/>
        <source>Bad argument %1: lonely hyphen</source>
        <translation>Virheellinen argumentti %1: yksinäinen tavuviiva</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Unknown long argument %1: %2</source>
        <translation>Tuntematon pitkä argumentti %1: %2</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Unknown short argument %1.%2: %3</source>
        <translation>Tuntematon lyhyt argumentti %1.%2: %3</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Usage:
  %1 [options] [files...]</source>
        <translation>Käyttö:
  %1 [argumentit] [tiedostot...]</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Options:</source>
        <translation>Asetukset:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Display this help</source>
        <translation>Näytä tämän opas</translation>
    </message>
</context>
<context>
    <name>ConverterDataModel</name>
    <message>
        <location filename="../src/automappingconverter/converterdatamodel.cpp" line="+75"/>
        <source>File</source>
        <translation>Tiedosto</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Version</source>
        <translation>Versio</translation>
    </message>
</context>
<context>
    <name>ConverterWindow</name>
    <message>
        <location filename="../src/automappingconverter/converterwindow.cpp" line="+36"/>
        <source>Save all as %1</source>
        <translation>Tallenna kaikki nimellä %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>All Files (*)</source>
        <translation>Kaikki tiedostot (*)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled kenttätiedostot (*.tmx)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Open Map</source>
        <translation>Avaa kenttä</translation>
    </message>
</context>
<context>
    <name>Csv::CsvPlugin</name>
    <message>
        <location filename="../src/plugins/csv/csvplugin.cpp" line="+55"/>
        <source>Could not open file for writing.</source>
        <translation>TIedostoa ei voitu avata kirjoitettavaksi.</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV tiedostot (*.csv)</translation>
    </message>
</context>
<context>
    <name>Defold::DefoldPlugin</name>
    <message>
        <location filename="../src/plugins/defold/defoldplugin.cpp" line="+58"/>
        <source>Defold files (*.tilemap)</source>
        <translation type="unfinished">Korostetut tiedostot (*.tilemap)</translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
</context>
<context>
    <name>Droidcraft::DroidcraftPlugin</name>
    <message>
        <location filename="../src/plugins/droidcraft/droidcraftplugin.cpp" line="+56"/>
        <source>This is not a valid Droidcraft map file!</source>
        <translation>Tämä ei ole kelvollinen Droidcraft kenttätiedosto!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>The map needs to have exactly one tile layer!</source>
        <translation>Kentällä saa olla vain yksi tilelayer!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The layer must have a size of 48 x 48 tiles!</source>
        <translation>Layerin täytyy olla kooltaan 48x48 tileä!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Droidcraft map files (*.dat)</source>
        <translation>Droidcraft kenttätiedostot (*.dat)</translation>
    </message>
</context>
<context>
    <name>EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.ui" line="+14"/>
        <source>Edit Terrain Information</source>
        <translation>Muokkaa terrainin tietoja</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+3"/>
        <source>Undo</source>
        <translation>Kumoa</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+3"/>
        <source>Redo</source>
        <translation>Tee uudelleen</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Erase</source>
        <translation>Pyyhi</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Add Terrain Type</source>
        <translation>Lisää terrainin tyyppi</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Lisää</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Terrain Type</source>
        <translation>Poista terrainin tyyppi</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Poista</translation>
    </message>
</context>
<context>
    <name>ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.ui" line="+14"/>
        <source>Export As Image</source>
        <translation>Vie kuvatiedostona</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Location</source>
        <translation>Sijainti</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Name:</source>
        <translation>Nimi:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Selaa...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Settings</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only include &amp;visible layers</source>
        <translation>Sisällytä vain &amp;näkyvät layerit</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Use current &amp;zoom level</source>
        <translation>Käytä nykyistä &amp;zoomaustasoa</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Draw tile grid</source>
        <translation>&amp;Piirrä tileruudukko</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Include background color</source>
        <translation>&amp;Sisällytä taustaväri</translation>
    </message>
</context>
<context>
    <name>Flare::FlarePlugin</name>
    <message>
        <location filename="../src/plugins/flare/flareplugin.cpp" line="+52"/>
        <source>Could not open file for reading.</source>
        <translation>Tiedostoa ei voitu avata lukemista varten.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Error loading tileset %1, which expands to %2. Path not found!</source>
        <translation>Virhe ladatessa tilesetiä %1, joka laajentuu %2. Polkua ei löytynyt!</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>No tilesets section found before layer section.</source>
        <translation type="unfinished">Layer-aluetta ei voi lisätä ilman tileset-aluetta.</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Error mapping tile id %1.</source>
        <translation>Virhe piirrettäessä tileä %1.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>This seems to be no valid flare map. A Flare map consists of at least a header section, a tileset section and one tile layer.</source>
        <translation>Tämä ei näytä olevan kelvollinen flare-kenttä. Flare-kenttä koostuu ainakin otsikosta, tilesetistä sekä yhdestä tilelayerista.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Flare map files (*.txt)</source>
        <translation>Flare kenttätiedostot (*.txt)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
</context>
<context>
    <name>Gmx::GmxPlugin</name>
    <message>
        <location filename="../src/plugins/gmx/gmxplugin.cpp" line="+82"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+143"/>
        <source>GameMaker room files (*.room.gmx)</source>
        <translation>GameMaker huonetiedostot (*.room.gmx)</translation>
    </message>
</context>
<context>
    <name>Json::JsonMapFormat</name>
    <message>
        <location filename="../src/plugins/json/jsonplugin.cpp" line="+53"/>
        <source>Could not open file for reading.</source>
        <translation>Tiedostoa ei voitu avata lukemista varten.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error parsing file.</source>
        <translation type="unfinished">Virhe tiedoston parseroinnissa.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Error while writing file:
%1</source>
        <translation>Virhe kirjoitettaessa tiedostoa:
%1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Json map files (*.json)</source>
        <translation>Json kenttätiedostot (*.json)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>JavaScript map files (*.js)</source>
        <translation>JavaScript kenttätiedostot (*.js)</translation>
    </message>
</context>
<context>
    <name>Json::JsonTilesetFormat</name>
    <message>
        <location line="+27"/>
        <source>Could not open file for reading.</source>
        <translation>Tiedostoa ei voitu avata lukemista varten.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Error parsing file.</source>
        <translation type="unfinished">Virhe tiedoston parseroinnissa.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error while writing file:
%1</source>
        <translation>Virhe kirjoitettaessa tiedostoa:
%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Json tileset files (*.json)</source>
        <translation>Json tileset-tiedostot (*.json)</translation>
    </message>
</context>
<context>
    <name>Lua::LuaPlugin</name>
    <message>
        <location filename="../src/plugins/lua/luaplugin.cpp" line="+58"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Lua files (*.lua)</source>
        <translation>Lua tiedostot (*.lua)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/tiled/mainwindow.ui" line="+49"/>
        <source>&amp;File</source>
        <translation>&amp;Tiedosto</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Recent Files</source>
        <translation>&amp;Viimeisimmät tiedostot</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>&amp;Edit</source>
        <translation>&amp;Muokkaa</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Help</source>
        <translation>&amp;Ohje</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Map</source>
        <translation>&amp;Kenttä</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;View</source>
        <translation>&amp;Näytä</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Show Object &amp;Names</source>
        <translation>Näytä objektin &amp;Nimet</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Main Toolbar</source>
        <translation>Päätyökalupalkki</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Tools</source>
        <translation>Työkalut</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Open...</source>
        <translation>&amp;Avaa...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Save</source>
        <translation>&amp;Tallenna</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Quit</source>
        <translation>&amp;Lopeta</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Copy</source>
        <translation>&amp;Kopioi</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Paste</source>
        <translation>&amp;Liitä</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;About Tiled</source>
        <translation>&amp;Tietoja TIled:istä</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>About Qt</source>
        <translation>Tietoja Qt:stä</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Resize Map...</source>
        <translation>&amp;Muuta kentän kokoa...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map &amp;Properties</source>
        <translation>Kentän &amp;ominaisuudet</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>AutoMap</source>
        <translation>Automaattilisäys</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A</source>
        <translation>A</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show &amp;Grid</source>
        <translation>Näytä &amp;ruudukko</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+G</source>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;As...</source>
        <translation>Tallenna &amp;nimellä...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;New...</source>
        <translation>&amp;Uusi...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>New &amp;Tileset...</source>
        <translation>Uusi &amp;tileset...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Close</source>
        <translation>&amp;Sulje</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom In</source>
        <translation>Lähennä</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom Out</source>
        <translation>Loitonna</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normal Size</source>
        <translation>Tavallinen koko</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+0</source>
        <translation>Ctrl+0</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Become a Patron</source>
        <translation>Tule projektin suojelijaksi</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save All</source>
        <translation>Tallenna Kaikki</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Documentation</source>
        <translation>Dokumentaatio</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Never</source>
        <translation>&amp;Ei koskaan</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>For &amp;Selected Objects</source>
        <translation>Vain &amp;valituille objekteille</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>For &amp;All Objects</source>
        <translation>&amp;Kaikille objekteille</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>AutoMap While Drawing</source>
        <translation>Automaattilisäys piirrettäessä</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Paste &amp;in Place</source>
        <translation>Liitä &amp;sijaintiin</translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+124"/>
        <source>Ctrl+Shift+V</source>
        <translation>Ctrl+Shift+V</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Full Screen</source>
        <translation>Koko näyttö</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>F11</source>
        <translation>F11</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap To &amp;Pixels</source>
        <translation>Tartu pikseleihin</translation>
    </message>
    <message>
        <location line="-200"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Leikkaa</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Offset Map...</source>
        <translation>Siirrä kentän aloituspistettä...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Offsets everything in a layer</source>
        <translation type="unfinished">Siirtää kaikkia layerilla</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pre&amp;ferences...</source>
        <translation>&amp;Asetukset...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Clear Recent Files</source>
        <translation>Tyhjennä viimeisimmät tiedostot</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Export</source>
        <translation>&amp;Vie</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+E</source>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <location line="-82"/>
        <source>&amp;Add External Tileset...</source>
        <translation>&amp;Lisää ulkoinen tileset...</translation>
    </message>
    <message>
        <location line="-50"/>
        <source>Export As &amp;Image...</source>
        <translation>Vie &amp;kuvana...</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>E&amp;xport As...</source>
        <translation>&amp;Vie nimellä...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+E</source>
        <translation>Ctrl+Shift+E</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Snap to Grid</source>
        <translation>&amp;Tartu ruudukkoon</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>C&amp;lose All</source>
        <translation>&amp;Sulje kaikki</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+W</source>
        <translation>Ctrl+Shift+W</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Delete</source>
        <translation>&amp;Poista</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Delete</source>
        <translation>Poista</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Highlight Current Layer</source>
        <translation>&amp;Korosta nykyinen layeri</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H</source>
        <translation>H</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Object &amp;Outlines</source>
        <translation>Näytä tileobjektin &amp;ääriviivat</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap to &amp;Fine Grid</source>
        <translation>Tartu tarkkaan ruudukkoon</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Animations</source>
        <translation>Näytä tileanimaatiot</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Reload</source>
        <translation>Lataa uudelleen</translation>
    </message>
    <message>
        <location filename="../src/automappingconverter/converterwindow.ui" line="+14"/>
        <source>Tiled Automapping Rule Files Converter</source>
        <translation>Tiledin automaattipiirron sääntötiedostojen muunnin</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Add new Automapping rules</source>
        <translation>Lisää uusi automaattipiirron sääntö</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+581"/>
        <source>All Files (*)</source>
        <translation>Kaikki tiedostot (*)</translation>
    </message>
</context>
<context>
    <name>MapDocument</name>
    <message>
        <location filename="../src/tiled/adjusttileindexes.cpp" line="+178"/>
        <source>Tile</source>
        <translation>Tile</translation>
    </message>
</context>
<context>
    <name>MapReader</name>
    <message>
        <location filename="../src/libtiled/mapreader.cpp" line="+140"/>
        <source>Not a map file.</source>
        <translation>Ei ole kenttätiedosto.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Not a tileset file.</source>
        <translation>Ei ole tileset -tiedosto.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Rivi %1, kolumni %2</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File not found: %1</source>
        <translation>Tiedostoa ei löydy: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unable to read file: %1</source>
        <translation>Tiedostoa ei voida lukea: %1</translation>
    </message>
    <message>
        <location line="+32"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+59"/>
        <source>Unsupported map orientation: "%1"</source>
        <translation>Ei-tuettu kentän suunta: "%1"</translation>
    </message>
    <message>
        <location line="+102"/>
        <location line="+21"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+138"/>
        <source>Invalid tileset parameters for tileset '%1'</source>
        <translation>Virheelliset parametrit tilesetille '%1'</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Invalid tile ID: %1</source>
        <translation>Virheellinen tilen tunniste: %1</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>Too many &lt;tile&gt; elements</source>
        <translation>Liian monta &lt;tile&gt; elementtiä</translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+43"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+219"/>
        <source>Invalid tile: %1</source>
        <translation>Virheellinen tile: %1</translation>
    </message>
    <message>
        <location line="+29"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+34"/>
        <source>Invalid draw order: %1</source>
        <translation>Virheellinen piirtojärjestys: %1</translation>
    </message>
    <message>
        <location line="+154"/>
        <source>Invalid points data for polygon</source>
        <translation>Virhe polygonin pisteiden datassa</translation>
    </message>
    <message>
        <location line="-285"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-90"/>
        <source>Unknown encoding: %1</source>
        <translation>Tuntematon enkoodaus: %1</translation>
    </message>
    <message>
        <location line="-181"/>
        <source>Error reading embedded image for tile %1</source>
        <translation>Virhe luettaessa upotettua kuvaa tilelle %1</translation>
    </message>
    <message>
        <location line="+176"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-4"/>
        <source>Compression method '%1' not supported</source>
        <translation>Pakkausmetodia '%1' ei tueta</translation>
    </message>
    <message>
        <location line="+58"/>
        <location line="+19"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+15"/>
        <location line="+39"/>
        <source>Corrupt layer data for layer '%1'</source>
        <translation>Vahingoittunut data layerille '%1'</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-28"/>
        <source>Unable to parse tile at (%1,%2) on layer '%3'</source>
        <translation>Ei voitu lukea tilen dataa  (%1,%2) layerilta '%3'</translation>
    </message>
    <message>
        <location line="-28"/>
        <location line="+44"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+31"/>
        <source>Tile used but no tilesets specified</source>
        <translation>Tile käytössä mutta tilesetiä ei ole määritetty</translation>
    </message>
    <message>
        <location filename="../src/libtiled/mapwriter.cpp" line="+113"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoittamista varten.</translation>
    </message>
    <message>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-170"/>
        <source>Invalid (negative) tile id: %1</source>
        <translation>Virheellinen (negatiivinen) tilen tunniste: %1</translation>
    </message>
</context>
<context>
    <name>NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.ui" line="+14"/>
        <source>New Map</source>
        <translation>Uusi kenttä</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map size</source>
        <translation>Kentän koko</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+68"/>
        <source>Width:</source>
        <translation>Leveys:</translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+26"/>
        <source> tiles</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> tile(j)ä</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+68"/>
        <source>Height:</source>
        <translation>Korkeus:</translation>
    </message>
    <message>
        <location line="-32"/>
        <source>Tile size</source>
        <translation>Tilen koko</translation>
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
        <translation>Kenttä</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Orientation:</source>
        <translation>Suuntautuminen:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Tile layer format:</source>
        <translation>Tilelayerin tiedostomuoto:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile render order:</source>
        <translation>TIlen renderöintijärjestys:</translation>
    </message>
</context>
<context>
    <name>NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.ui" line="+14"/>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="+235"/>
        <source>New Tileset</source>
        <translation>Uusi tileset</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tileset</source>
        <translation>Tileset</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Based on Tileset Image</source>
        <translation>Perustuu tilesetin kuvaan</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Collection of Images</source>
        <translation>Kuvakokoelma</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Type:</source>
        <translation>Tyyppi:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Name:</source>
        <translation>&amp;Nimi:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Selaa...</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Use transparent color:</source>
        <translation>Käytä läpinäkyvää väriä:</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>Tile width:</source>
        <translation>Tilen leveys:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Pick color from image</source>
        <translation>Poimi väri kuvasta</translation>
    </message>
    <message>
        <location line="-138"/>
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
        <translation>Kuva</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Source:</source>
        <translation>Lähde:</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>The space at the edges of the tileset.</source>
        <translation>Tyhjä tila tilesetin reunoilla.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin:</source>
        <translation>Marginaali:</translation>
    </message>
    <message>
        <location line="-45"/>
        <source>Tile height:</source>
        <translation>Tilen korkeus:</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>The space between the tiles.</source>
        <translation>Tyhjä tila tilejen välissä.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spacing:</source>
        <translation>Välistys:</translation>
    </message>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-2"/>
        <source>Edit Tileset</source>
        <translation>Muokkaa tilesetiä</translation>
    </message>
</context>
<context>
    <name>ObjectTypes</name>
    <message>
        <location filename="../src/tiled/objecttypes.cpp" line="+43"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Could not open file.</source>
        <translation>Tiedostoa ei voitu avata.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File doesn't contain object types.</source>
        <translation>Tiedosto ei sisällä objektityyppejä.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Rivi %1, kolumni %2</translation>
    </message>
</context>
<context>
    <name>ObjectTypesEditor</name>
    <message>
        <location filename="../src/tiled/objecttypeseditor.ui" line="+14"/>
        <source>Object Types Editor</source>
        <translation>Objektityyppien muokkain</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>File</source>
        <translation>Tiedosto</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Export Object Types...</source>
        <translation>Vie objektityypit...</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Import Object Types...</source>
        <translation>Tuo objektityypit...</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Choose Object Types File...</source>
        <translation>Valitse objektityyppien tiedosto...</translation>
    </message>
</context>
<context>
    <name>OffsetMapDialog</name>
    <message>
        <location filename="../src/tiled/offsetmapdialog.ui" line="+17"/>
        <source>Offset Map</source>
        <translation>Siirrä kentän aloituspistettä</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Offset Contents of Map</source>
        <translation type="unfinished"></translation>
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
        <translation> tilet</translation>
    </message>
    <message>
        <location line="-30"/>
        <location line="+46"/>
        <source>Wrap</source>
        <translation>Sidonta</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Layers:</source>
        <translation>Layerit:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>All Visible Layers</source>
        <translation>Kaikki näkyvät layerit</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>All Layers</source>
        <translation>Kaikki layerit</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Selected Layer</source>
        <translation>Valitty layer</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bounds:</source>
        <translation>Rajat:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Whole Map</source>
        <translation>Koko kenttä</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Current Selection</source>
        <translation>Nykyinen valinta</translation>
    </message>
</context>
<context>
    <name>PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.ui" line="+14"/>
        <source>Become a Patron</source>
        <translation>Tule suojelijaksi</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Visit https://www.patreon.com/bjorn</source>
        <translation>Vieraile osoitteessa: https://www.patreon.com/bjorn</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I'm already a patron!</source>
        <translation>Olen jo suojelija!</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maybe later</source>
        <translation>Ehkä myöhemmin</translation>
    </message>
</context>
<context>
    <name>PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+14"/>
        <source>Preferences</source>
        <translation>Asetukset</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>General</source>
        <translation>Yleinen</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Saving and Loading</source>
        <translation type="unfinished">Tallennetaan ja ladataan</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="-489"/>
        <source>XML</source>
        <translation>XML</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+79"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (uncompressed)</source>
        <translation>Base64 (pakkaamaton)</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (gzip compressed)</source>
        <translation>Base64 (gzip pakattu)</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (zlib compressed)</source>
        <translation>Base64 (zlib pakattu)</translation>
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
        <translation>Alaoikealle</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Right Up</source>
        <translation>Yläoikealle</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Down</source>
        <translation>Alavasemmalle</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Up</source>
        <translation>Ylävasemmalle</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+6"/>
        <source>&amp;Reload tileset images when they change</source>
        <translation>&amp;Päivitä tilesetin kuvat niiden muuttuessa</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Not enabled by default since a reference to an external DTD is known to cause problems with some XML parsers.</source>
        <translation type="unfinished">Ei ole käytössä oletuksena koska reference ulkoiseen DTD:hen tiedetään aiheuttavan ongelmia joillakin XML parseroijilla.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Include &amp;DTD reference in saved maps</source>
        <translation>Sisällytä &amp;DTD reference tallennetuissa kentissä</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Interface</source>
        <translation>Käyttöliittymä</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Language:</source>
        <translation>&amp;Kieli:</translation>
    </message>
    <message>
        <location line="-7"/>
        <source>Hardware &amp;accelerated drawing (OpenGL)</source>
        <translation>Laitteistokiihdytetty piirto (OpenGL)</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Open last files on startup</source>
        <translation>Avaa viimeisimmät tiedostot ohjelman käynnistyessä</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Grid color:</source>
        <translation>Ruudukon väri:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Fine grid divisions:</source>
        <translation>Tarkka ruudukon jaottelu:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source> pixels</source>
        <translation> pikseliä</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Object line width:</source>
        <translation>Objektin ääriviivan paksuus:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Theme</source>
        <translation>Teema</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="+67"/>
        <location line="+122"/>
        <source>Native</source>
        <translation>Native</translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Fusion</source>
        <translation>Fusion</translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Tiled Fusion</source>
        <translation>Tiled Fusion</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+22"/>
        <source>Selection color:</source>
        <translation>Valinnan väri:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Style:</source>
        <translation>Tyyli:</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Base color:</source>
        <translation>Pohjaväri:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Updates</source>
        <translation>Päivitykset</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Check Now</source>
        <translation>Tarkista nyt</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Automatically check for updates</source>
        <translation>Tarkista päivitykset automaattisesti</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Plugins</source>
        <translation>Liitännäiset</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enabled Plugins</source>
        <translation>Käyttöön otetut liitännäiset</translation>
    </message>
</context>
<context>
    <name>Python::PythonMapFormat</name>
    <message>
        <location filename="../src/plugins/python/pythonplugin.cpp" line="+268"/>
        <source>-- Using script %1 to read %2</source>
        <translation>-- Käytetään skriptiä %1 lukemaan %2</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>-- Using script %1 to write %2</source>
        <translation>-- Käytetään skriptiä %1 kirjoittamaan %2</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Uncaught exception in script. Please check console.</source>
        <translation>Virhe skriptissä. Tarkista konsoli.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Script returned false. Please check console.</source>
        <translation>Skripti palautti epätoden. Ole hyvä ja tarkista konsoli.</translation>
    </message>
</context>
<context>
    <name>Python::PythonPlugin</name>
    <message>
        <location line="-164"/>
        <source>Reloading Python scripts</source>
        <translation>Ladataan uudelleen python-skriptejä</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/automappingconverter/convertercontrol.h" line="+33"/>
        <source>v0.8 and before</source>
        <translation>v0.8 ja ennen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>v0.9 and later</source>
        <translation>v0.9 ja jälkeen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>unknown</source>
        <translation>tuntematon</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>not a map</source>
        <translation>ei kenttä</translation>
    </message>
</context>
<context>
    <name>QtBoolEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+237"/>
        <location line="+10"/>
        <location line="+25"/>
        <source>True</source>
        <translation>Tosi</translation>
    </message>
    <message>
        <location line="-25"/>
        <location line="+25"/>
        <source>False</source>
        <translation>Epätosi</translation>
    </message>
</context>
<context>
    <name>QtBoolPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+1703"/>
        <source>True</source>
        <translation>Tosi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>False</source>
        <translation>Epätosi</translation>
    </message>
</context>
<context>
    <name>QtCharEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qteditorfactory.cpp" line="+1712"/>
        <source>Clear Char</source>
        <translation>Tyhjennä merkit</translation>
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
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+4736"/>
        <source>Red</source>
        <translation>Punainen</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Green</source>
        <translation>Vihreä</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Blue</source>
        <translation>Sininen</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Alpha</source>
        <translation>Läpinäkyvä</translation>
    </message>
</context>
<context>
    <name>QtCursorDatabase</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-214"/>
        <source>Arrow</source>
        <translation>Nuoli</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Up Arrow</source>
        <translation>Nuoli ylös</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Cross</source>
        <translation>Risti</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Wait</source>
        <translation>Odota</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>IBeam</source>
        <translation>IBeam</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Vertical</source>
        <translation>Koko pystysuunnassa</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Horizontal</source>
        <translation>Koko vaakasuunnassa</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Backslash</source>
        <translation type="unfinished">Vaaka- ja pystysuuntainen skaalaus</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Slash</source>
        <translation>Vaaka- ja pystysuuntainen skaalaus</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size All</source>
        <translation type="unfinished">Mittakaava joka suunnassa</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Blank</source>
        <translation>Tyhjä</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Vertical</source>
        <translation>Halkaise pystysuunnassa</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Horizontal</source>
        <translation>Halkaise vaakasuunnassa</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pointing Hand</source>
        <translation type="unfinished">Osoitin</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forbidden</source>
        <translation>Kielletty</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Open Hand</source>
        <translation>Avoin käsi</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Closed Hand</source>
        <translation>Suljettu käsi</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>What's This</source>
        <translation>Mikä tämä on</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Busy</source>
        <translation>Kiireinen</translation>
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
        <translation>Valitse fontti</translation>
    </message>
</context>
<context>
    <name>QtFontPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="-362"/>
        <source>Family</source>
        <translation>Suku</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Point Size</source>
        <translation>Pisteen koko</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bold</source>
        <translation>Lihavoi</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Italic</source>
        <translation>Kursivoi</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Underline</source>
        <translation>Alleviivattu</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Strikeout</source>
        <translation>Yliviivattu</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Kerning</source>
        <translation>Välistys</translation>
    </message>
</context>
<context>
    <name>QtKeySequenceEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+238"/>
        <source>Clear Shortcut</source>
        <translation type="unfinished">Tyhjennä pika</translation>
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
        <translation>Kieli</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Country</source>
        <translation>Maa</translation>
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
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-144"/>
        <source>[%1, %2, %3] (%4)</source>
        <translation>[%1, %2, %3] (%4)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Not set</source>
        <translation>Ei asetettu</translation>
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
        <translation>Leveys</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Korkeus</translation>
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
        <translation>Leveys</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Korkeus</translation>
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
        <translation>Leveys</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Korkeus</translation>
    </message>
</context>
<context>
    <name>QtSizePolicyPropertyManager</name>
    <message>
        <location line="+1704"/>
        <location line="+1"/>
        <source>&lt;Invalid&gt;</source>
        <translation>&lt;Virheellinen&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>[%1, %2, %3, %4]</source>
        <translation>[%1, %2, %3, %4]</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Horizontal Policy</source>
        <translation>Vaakasuunnan sääntö</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vertical Policy</source>
        <translation>Pystysuunnan sääntö</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Horizontal Stretch</source>
        <translation>Venytys pystysuunnassa</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Vertical Stretch</source>
        <translation>Venytys vaakasuunnassa</translation>
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
        <translation>Leveys</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Korkeus</translation>
    </message>
</context>
<context>
    <name>QtTreePropertyBrowser</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qttreepropertybrowser.cpp" line="+478"/>
        <source>Property</source>
        <translation>Ominaisuus</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Value</source>
        <translation>Arvo</translation>
    </message>
</context>
<context>
    <name>ReplicaIsland::ReplicaIslandPlugin</name>
    <message>
        <location filename="../src/plugins/replicaisland/replicaislandplugin.cpp" line="+58"/>
        <source>Cannot open Replica Island map file!</source>
        <translation>Ei voida avata Replica Island kenttätiedostoa!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Can't parse file header!</source>
        <translation>Tiedoston otsaketta ei voida lukea!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Can't parse layer header!</source>
        <translation>Ei voida lukea layerin otsaketietoja!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Inconsistent layer sizes!</source>
        <translation>Layerit ovat erikokoisia!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File ended in middle of layer!</source>
        <translation>Layertiedosto päättyi kesken!</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Unexpected data at end of file!</source>
        <translation>Tiedoston loppuosa on virheellinen!</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Replica Island map files (*.bin)</source>
        <translation>Replica Island kenttätiedostot (*.bin)</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voida avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>You must define a background_index property on the map!</source>
        <translation>Sinun täytyy määrittää background_index ominaisuus kentälle!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Can't save non-tile layer!</source>
        <translation type="unfinished">Vain tilelayerin voi tallentaa!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>You must define a type property on each layer!</source>
        <translation>Sinun täytyy määrittää tyyppi-ominaisuus joka layerilla!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a tile_index property on each layer!</source>
        <translation>Sinun täytyy määrittää tile_index ominaisuus joka layerilla!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a scroll_speed property on each layer!</source>
        <translation>Sinun täytyy määrittää scroll_speed ominaisuus joka layerilla!</translation>
    </message>
</context>
<context>
    <name>ResizeDialog</name>
    <message>
        <location filename="../src/tiled/resizedialog.ui" line="+14"/>
        <source>Resize</source>
        <translation>Muuta kokoa</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Size</source>
        <translation>Koko</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+33"/>
        <location line="+32"/>
        <location line="+23"/>
        <source> tiles</source>
        <translation type="unfinished"> tile(j)ä</translation>
    </message>
    <message>
        <location line="-75"/>
        <source>Width:</source>
        <translation>Leveys:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Height:</source>
        <translation>Korkeus:</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Offset</source>
        <translation>Siirto</translation>
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
        <translation>Poista objektit kentän ulkopuolelta</translation>
    </message>
</context>
<context>
    <name>Tengine::TenginePlugin</name>
    <message>
        <location filename="../src/plugins/tengine/tengineplugin.cpp" line="+49"/>
        <source>Could not open file for writing.</source>
        <translation>Ei voitu avata tiedostoa kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+246"/>
        <source>T-Engine4 map files (*.lua)</source>
        <translation>T-Engine4 kenttätiedostot (*.lua)</translation>
    </message>
</context>
<context>
    <name>TextEditorDialog</name>
    <message>
        <location filename="../src/tiled/texteditordialog.ui" line="+14"/>
        <source>Edit Text</source>
        <translation>Muokkaa tekstiä</translation>
    </message>
</context>
<context>
    <name>TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.ui" line="+14"/>
        <source>Tile Animation Editor</source>
        <translation>Muokkaa tilen animaatioita</translation>
    </message>
    <message>
        <location line="+99"/>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="+523"/>
        <source>Preview</source>
        <translation>Esikatselu</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractObjectTool</name>
    <message>
        <location filename="../src/tiled/abstractobjecttool.cpp" line="+167"/>
        <location line="+70"/>
        <source>Reset Tile Size</source>
        <translation>Palauta tilen koko</translation>
    </message>
    <message numerus="yes">
        <location line="-13"/>
        <source>Duplicate %n Object(s)</source>
        <translation><numerusform>Monista %n objekti(a)</numerusform>
        <numerusform>Monista %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+2"/>
        <source>Remove %n Object(s)</source>
        <translation><numerusform>Poista %n objekti(a)</numerusform>
        <numerusform>Poista %n objektia</numerusform>
        </translation></message>
    <message>
        <location line="+18"/>
        <source>Flip Horizontally</source>
        <translation>Käännä vaakasuuntaisesti</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Flip Vertically</source>
        <translation>Käännä pystysuuntaisesti</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Raise Object</source>
        <translation>Nosta objekti</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgUp</source>
        <translation>PageUp</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object</source>
        <translation>Laske objekti</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgDown</source>
        <translation>PageDown</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Object to Top</source>
        <translation>Nosta objekti ylös</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Home</source>
        <translation>Home</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object to Bottom</source>
        <translation>Pudota Objekti alimmaiseksi</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>End</source>
        <translation>End</translation>
    </message>
    <message numerus="yes">
        <location line="+5"/>
        <source>Move %n Object(s) to Layer</source>
        <translation><numerusform>Siirrä %n Objekti(a) layerille</numerusform>
        <numerusform>Siirrö %n objektia layerille</numerusform>
        </translation></message>
    <message>
        <location line="+11"/>
        <source>Object &amp;Properties...</source>
        <translation>Objektin &amp;ominaisuudet...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractTileTool</name>
    <message>
        <location filename="../src/tiled/abstracttiletool.cpp" line="+124"/>
        <source>empty</source>
        <translation>tyhjä</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutoMapper</name>
    <message>
        <location filename="../src/tiled/automapper.cpp" line="+115"/>
        <source>'%1': Property '%2' = '%3' does not make sense. Ignoring this property.</source>
        <translation>'%1': ominaisuus '%2' = '%3' ei ole järjellinen. Tämä ominaisuus jätetään huomiotta.</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Did you forget an underscore in layer '%1'?</source>
        <translation>Unohditko alaviivan layerilla '%1'?</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Layer '%1' is not recognized as a valid layer for Automapping.</source>
        <translation type="unfinished">Layeria '%1' ei tunnisteta kelvolliseksi automaattipiirtoa varten.</translation>
    </message>
    <message>
        <location line="-105"/>
        <source>'regions_input' layer must not occur more than once.</source>
        <translation>'regions_input' layer saa esiintyä vain kerran.</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+13"/>
        <source>'regions_*' layers must be tile layers.</source>
        <translation>'regions_*' -layerien täytyy olla tile layereita.</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>'regions_output' layer must not occur more than once.</source>
        <translation>'regions_output' layer saa esiintyä vain kerran.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>'input_*' and 'inputnot_*' layers must be tile layers.</source>
        <translation>'input_*' ja 'inputnot_*' layereiden täytyy olla tilelayereita.</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>No 'regions' or 'regions_input' layer found.</source>
        <translation>Ei löydetty 'regions' tai 'regions_input' layeria.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No 'regions' or 'regions_output' layer found.</source>
        <translation>Ei löydetty 'regions' tai 'regions_output' layeria.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No input_&lt;name&gt; layer found!</source>
        <translation>Ei löydetty input_&lt;name&gt; layeria!</translation>
    </message>
    <message>
        <location line="+165"/>
        <source>Tile</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutomappingManager</name>
    <message>
        <location filename="../src/tiled/automappingmanager.cpp" line="+103"/>
        <source>Apply AutoMap rules</source>
        <translation>Hyväksy automaattipiirron säännöt</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>No rules file found at:
%1</source>
        <translation>Sääntötiedostoa ei löytynyt:
%1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Error opening rules file:
%1</source>
        <translation>Virhe avattaessa sääntötiedostoa:
%1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>File not found:
%1</source>
        <translation>Tiedostoa ei löytynyt:
%1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Opening rules map failed:
%1</source>
        <translation>Sääntötiedoston avaus epäonnistui:
%1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksModel</name>
    <message>
        <location filename="../src/tiled/brokenlinks.cpp" line="+144"/>
        <source>Tileset image</source>
        <translation>Tileset kuva</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tileset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tile image</source>
        <translation>Tilekuva</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File name</source>
        <translation>Tiedostonimi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Location</source>
        <translation>Sijainti</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Tyyppi</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksWidget</name>
    <message>
        <location line="+66"/>
        <source>Some files could not be found</source>
        <translation>Joitain tiedostoja ei löydetty</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>One or more files referenced by the map could not be found. You can help locate them below.</source>
        <translation type="unfinished">Yhtä tai useampaa kentän tiedostoa ei löytynyt. Voit etsiä niitä tämän alta.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Locate File...</source>
        <translation>Paikanna tiedosto...</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Locate File</source>
        <translation>Paikanna tiedosto</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Loading Image</source>
        <translation>Virhe kuvan latauksessa</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>All Files (*)</source>
        <translation>Kaikki tiedostot (*)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Locate External Tileset</source>
        <translation>Etsi ulkoinen tileset</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Reading Tileset</source>
        <translation>Virhe luettaessa tilesetiä</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BucketFillTool</name>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="+40"/>
        <location line="+194"/>
        <source>Bucket Fill Tool</source>
        <translation>Täyttötyökalu</translation>
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
        <location filename="../src/tiled/clipboardmanager.cpp" line="+171"/>
        <source>Paste Objects</source>
        <translation type="unfinished">Liitä objekteja/objektit</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandButton</name>
    <message>
        <location filename="../src/tiled/commandbutton.cpp" line="+130"/>
        <source>Execute Command</source>
        <translation>Suorita komento</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location line="-67"/>
        <source>Error Executing Command</source>
        <translation>Virhe komennon suorittamisessa</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>You do not have any commands setup.</source>
        <translation>Sinulla ei ole yhtään asetettua komentoa.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Edit commands...</source>
        <translation>Muokkaa komentoja...</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Edit Commands...</source>
        <translation>Muokkaa komentoja...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDataModel</name>
    <message>
        <location filename="../src/tiled/commanddatamodel.cpp" line="+60"/>
        <source>Open in text editor</source>
        <translation>Avaa tekstieditorissa</translation>
    </message>
    <message>
        <location line="+91"/>
        <location line="+69"/>
        <source>&lt;new command&gt;</source>
        <translation>&lt;uusi komento&gt;</translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Set a name for this command</source>
        <translation>Anna nimi tälle komennolle</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set the shell command to execute</source>
        <translation>Anna suoritettava komentorivikomento</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Show or hide this command in the command list</source>
        <translation>Näytä tai piilota tämä komento listalla</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add a new command</source>
        <translation>Lisää uusi komento</translation>
    </message>
    <message>
        <location line="+107"/>
        <source>Name</source>
        <translation>Nimi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Command</source>
        <translation>Komento</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable</source>
        <translation>Ota käyttöön</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Move Up</source>
        <translation>Siirrä ylös</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Move Down</source>
        <translation>Siirrä alas</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Execute</source>
        <translation>Suorita</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Execute in Terminal</source>
        <translation>Suorita komentorivillä</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Delete</source>
        <translation>Poista</translation>
    </message>
    <message>
        <location line="+84"/>
        <source>%1 (copy)</source>
        <translation>%1 (kopio)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>New command</source>
        <translation>Uusi komento</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.cpp" line="+44"/>
        <source>Edit Commands</source>
        <translation>Muokkaa komentoja</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandProcess</name>
    <message>
        <location filename="../src/tiled/command.cpp" line="+144"/>
        <source>Unable to create/open %1</source>
        <translation>Ei voitu luoda/avata %1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unable to add executable permissions to %1</source>
        <translation>Ei voitu lisätä suoritusoikeuksia %1:lle</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>The command failed to start.</source>
        <translation>Komennon käynnistyminen epäonnistui.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command crashed.</source>
        <translation>Komento epäonnistui.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command timed out.</source>
        <translation>Komennon aikakatkaisu.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>An unknown error occurred.</source>
        <translation>Tapahtui tuntematon virhe.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Error Executing %1</source>
        <translation>Virhe suoritettaessa %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ConsoleDock</name>
    <message>
        <location filename="../src/tiled/consoledock.cpp" line="+36"/>
        <source>Debug Console</source>
        <translation>Virheenetsintäkonsoli</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateEllipseObjectTool</name>
    <message>
        <location filename="../src/tiled/createellipseobjecttool.cpp" line="+39"/>
        <source>Insert Ellipse</source>
        <translation>Luo ellipsi</translation>
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
        <translation>Lisää polygoni</translation>
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
        <translation>Lisää polygoniviiva</translation>
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
        <translation>Lisää nelikulmio</translation>
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
        <translation>Lisää tile</translation>
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
        <location filename="../src/tiled/documentmanager.cpp" line="+412"/>
        <source>%1:

%2</source>
        <translation>%1:

%2</translation>
    </message>
    <message>
        <location line="+146"/>
        <source>Copy File Path</source>
        <translation>Kopioi tiedostopolku</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Open Containing Folder...</source>
        <translation>Avaa sisältävä kansio...</translation>
    </message>
    <message>
        <location line="+141"/>
        <source>Tileset Columns Changed</source>
        <translation>Tilesetin kolumni muuttui</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The number of tile columns in the tileset '%1' appears to have changed from %2 to %3. Do you want to adjust tile references?</source>
        <translation>Kolumni tilesetissö '%1' näyttää muuttuneen %2:sta %3:een. Haluatko säätää tilen ominaisuuksia?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditPolygonTool</name>
    <message>
        <location filename="../src/tiled/editpolygontool.cpp" line="+129"/>
        <location line="+209"/>
        <source>Edit Polygons</source>
        <translation>Muokkaa polygoneja</translation>
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
        <translation><numerusform>Siirrä %n Piste(ttä)</numerusform>
        <numerusform>Siirrä %n pisteitä</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+26"/>
        <location line="+45"/>
        <source>Delete %n Node(s)</source>
        <translation type="unfinished"><numerusform>Poista %n Node(ja)</numerusform>
        <numerusform>Poista %n Nodea</numerusform>
        </translation></message>
    <message>
        <location line="-40"/>
        <location line="+215"/>
        <source>Join Nodes</source>
        <translation>Liitä Nodet</translation>
    </message>
    <message>
        <location line="-214"/>
        <location line="+250"/>
        <source>Split Segments</source>
        <translation>Halkaise segmentit</translation>
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
        <translation>Uusi Terrain</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::Eraser</name>
    <message>
        <location filename="../src/tiled/eraser.cpp" line="+35"/>
        <location line="+56"/>
        <source>Eraser</source>
        <translation>Pyyhekumi</translation>
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
        <location filename="../src/tiled/exportasimagedialog.cpp" line="+63"/>
        <source>Export</source>
        <translation>Vie</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Export as Image</source>
        <translation>Vie kuvatiedostona</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 already exists.
Do you want to replace it?</source>
        <translation>%1 on jo olemassa.
Haluatko korvata sen?</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Out of Memory</source>
        <translation>Muistialueen ylitys</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not allocate sufficient memory for the image. Try reducing the zoom level or using a 64-bit version of Tiled.</source>
        <translation>Keskusmuisti ei riitä kuvalle. Vähennä zoomausta tai käytä 64-bittistä versiota Tiledistä.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image too Big</source>
        <translation>Kuva on liian iso</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The resulting image would be %1 x %2 pixels and take %3 GB of memory. Tiled is unable to create such an image. Try reducing the zoom level.</source>
        <translation>Lopullinen kuva olisi %1 x %2 pikseliä ja veisi %3 gigaa muistia. Tiled ei pysty luomaan tällaista kuvaa. Kokeile vähentää kuvan zoomausta.</translation>
    </message>
    <message>
        <location line="+100"/>
        <source>Image</source>
        <translation>Kuva</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileChangedWarning</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="-600"/>
        <source>File change detected. Discard changes and reload the map?</source>
        <translation>Tiedostossa havaittiin muutos. Peru muutokset ja lataa kenttä uudelleen?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileEdit</name>
    <message>
        <location filename="../src/tiled/fileedit.cpp" line="+113"/>
        <source>Choose a File</source>
        <translation>Valitse tiedosto</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerDock</name>
    <message>
        <location filename="../src/tiled/layerdock.cpp" line="+217"/>
        <source>Layers</source>
        <translation>Layerit</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Opacity:</source>
        <translation>Läpinäkyvyys:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerModel</name>
    <message>
        <location filename="../src/tiled/layermodel.cpp" line="+151"/>
        <source>Layer</source>
        <translation>Layer</translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Show Other Layers</source>
        <translation>Näytä muut layerit</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Hide Other Layers</source>
        <translation>Piilota muut layerit</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerOffsetTool</name>
    <message>
        <location filename="../src/tiled/layeroffsettool.cpp" line="+38"/>
        <location line="+94"/>
        <source>Offset Layers</source>
        <translation type="unfinished"></translation>
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
        <location line="+58"/>
        <source>Magic Wand</source>
        <translation>Taikasauva</translation>
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
        <location filename="../src/tiled/mainwindow.cpp" line="+236"/>
        <location line="+11"/>
        <source>Undo</source>
        <translation>Kumoa</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+9"/>
        <source>Redo</source>
        <translation>Tee uudelleen</translation>
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
        <translation>Satunnaistila</translation>
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
        <translation>&amp;Layer</translation>
    </message>
    <message>
        <location line="-1396"/>
        <location line="+1397"/>
        <source>&amp;New</source>
        <translation>&amp;Uusi</translation>
    </message>
    <message>
        <location line="-1210"/>
        <source>Object Types Editor</source>
        <translation>Objektityyppien muokkain</translation>
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
        <translation>Virhe avatessa kenttää</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+196"/>
        <location line="+296"/>
        <source>All Files (*)</source>
        <translation>Kaikki tiedostot (*)</translation>
    </message>
    <message>
        <location line="-481"/>
        <source>Open Map</source>
        <translation>Avaa kenttä</translation>
    </message>
    <message>
        <location line="+25"/>
        <location line="+88"/>
        <source>Error Saving Map</source>
        <translation>Virhe tallennettaessa kenttää</translation>
    </message>
    <message>
        <location line="-46"/>
        <source>untitled.tmx</source>
        <translation>nimetön.tmx</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Extension Mismatch</source>
        <translation>Laajennusta ei löydy</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The file extension does not match the chosen file type.</source>
        <translation>Tiedostolaajennus ei sovi valittuun tiedostotyyppiin.</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Tiled may not automatically recognize your file when loading. Are you sure you want to save with this extension?</source>
        <translation>Tiled ei välttämättä tunnista automaattisesti tiedostoja latausvaiheessa. Haluatko varmasti tallentaa tällä tiedostopäätteellä?</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Unsaved Changes</source>
        <translation>Tallentamattomat muutokset</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>There are unsaved changes. Do you want to save now?</source>
        <translation>Kaikkia muutoksia ei ole tallennettu. Haluatko tallentaa ne nyt?</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Exported to %1</source>
        <translation>Viety nimellä %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+117"/>
        <source>Error Exporting Map</source>
        <translation>Virhe kentän viennissä</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Export As...</source>
        <translation>Vie nimellä...</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Non-unique file extension</source>
        <translation>Epäselvä tiedostolaajennus</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Non-unique file extension.
Please select specific format.</source>
        <translation>Tuntematon tiedostopääte.
Ole hyvä ja valitse tietty tiedostomuoto.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Unknown File Format</source>
        <translation>Tuntematon tiedostomuoto</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The given filename does not have any known file extension.</source>
        <translation>Annetulla tiedostonimellä ei ole yhtään tunnettua tiedostomuotoa.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Some export files already exist:</source>
        <translation>Jotkut vietävistä tiedostoista ovat jo olemassa:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Do you want to replace them?</source>
        <translation>Haluatko korvata nämä tiedostot?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Overwrite Files</source>
        <translation>Ylikirjoita tiedostot</translation>
    </message>
    <message>
        <location line="+621"/>
        <source>[*]%1</source>
        <translation>[*]%1</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Error Reloading Map</source>
        <translation>Virhe kentän lataamisessa</translation>
    </message>
    <message>
        <location line="-516"/>
        <location line="+5"/>
        <source>Error Reading Tileset</source>
        <translation>Virhe tilesetin lukemisessa</translation>
    </message>
    <message>
        <location line="+77"/>
        <source>Automatic Mapping Warning</source>
        <translation>Automaattipiirron varoitus</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Automatic Mapping Error</source>
        <translation>Automaattipiirron virhe</translation>
    </message>
    <message>
        <location line="-874"/>
        <location line="+1212"/>
        <source>Views and Toolbars</source>
        <translation>Näkymät ja työkaluvalikot</translation>
    </message>
    <message>
        <location line="-1209"/>
        <location line="+1210"/>
        <source>Tile Animation Editor</source>
        <translation>Tileanimaatioiden muokkain</translation>
    </message>
    <message>
        <location line="-1208"/>
        <location line="+1209"/>
        <source>Tile Collision Editor</source>
        <translation>Tiletörmäysten muokkain</translation>
    </message>
    <message>
        <location line="-1175"/>
        <source>Alt+Left</source>
        <translation>Alt+Vasen</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Alt+Right</source>
        <translation>Alt+Oikea</translation>
    </message>
    <message>
        <location line="+737"/>
        <source>Add External Tileset(s)</source>
        <translation>Lisää ulkoinen tileset</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>%1: %2</source>
        <translation>%1: %2</translation>
    </message>
    <message numerus="yes">
        <location line="+10"/>
        <source>Add %n Tileset(s)</source>
        <translation><numerusform>Lisää %n tileset(iä)</numerusform>
        <numerusform>Lisää %n tilesetiä</numerusform>
        </translation></message>
</context>
<context>
    <name>Tiled::Internal::MapDocument</name>
    <message>
        <location filename="../src/tiled/mapdocument.cpp" line="+246"/>
        <source>untitled.tmx</source>
        <translation>nimetön.tmx</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>Resize Map</source>
        <translation>Muuta kentän kokoa</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Offset Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location line="+28"/>
        <source>Rotate %n Object(s)</source>
        <translation><numerusform>Pyöritä %n Objekti(a)</numerusform>
        <numerusform>Pyöritä %n objektia</numerusform>
        </translation></message>
    <message>
        <location line="+36"/>
        <source>Tile Layer %1</source>
        <translation>Tile layer %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Object Layer %1</source>
        <translation>Objektilayer %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Image Layer %1</source>
        <translation>Kuvalayer %1</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Copy of %1</source>
        <translation type="unfinished">%1:s kopio</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Duplicate Layer</source>
        <translation>Monista layer</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Merge Layer Down</source>
        <translation type="unfinished">Yhdistä layer alempaan</translation>
    </message>
    <message>
        <location line="+238"/>
        <source>Tile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tileset Changes</source>
        <translation>Tilesetin muutokset</translation>
    </message>
    <message numerus="yes">
        <location line="+190"/>
        <source>Duplicate %n Object(s)</source>
        <translation><numerusform>Monista %n objekti(a)</numerusform>
        <numerusform>Monista %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+21"/>
        <source>Remove %n Object(s)</source>
        <translation><numerusform>Poista %n objekti(a)</numerusform>
        <numerusform>Poista %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+12"/>
        <source>Move %n Object(s) to Layer</source>
        <translation><numerusform>Siirrä %n objekti(a) layerille</numerusform>
        <numerusform>Siirrä %n objektia layerille</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+37"/>
        <source>Move %n Object(s) Up</source>
        <translation><numerusform>Siirrä %n objekti(a) ylös</numerusform>
        <numerusform>Siirrä %n objektia ylös</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+36"/>
        <source>Move %n Object(s) Down</source>
        <translation><numerusform>Siirrä %n objekti(a) alas</numerusform>
        <numerusform>Siirrä %n objektia ylös</numerusform>
        </translation></message>
</context>
<context>
    <name>Tiled::Internal::MapDocumentActionHandler</name>
    <message>
        <location filename="../src/tiled/mapdocumentactionhandler.cpp" line="+60"/>
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
        <translation>Ctrl+Shift+Nuoli ylös</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+Down</source>
        <translation>Ctrl+Shift+Nuoli alas</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+H</source>
        <translation>Ctrl+Shift+H</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Select &amp;All</source>
        <translation>Valitse &amp;kaikki</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;None</source>
        <translation>Älä valitse &amp;mitään</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Crop to Selection</source>
        <translation>&amp;Leikkaa valintaan</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Tile Layer</source>
        <translation>&amp;Tile layer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Object Layer</source>
        <translation>&amp;Objektilayer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Image Layer</source>
        <translation>&amp;Kuvalayer</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+197"/>
        <source>Layer via Copy</source>
        <translation type="unfinished">Layer kopioinnin kautta</translation>
    </message>
    <message>
        <location line="-196"/>
        <location line="+196"/>
        <source>Layer via Cut</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-192"/>
        <source>Select Pre&amp;vious Layer</source>
        <translation>Valitse edellinen layer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;Next Layer</source>
        <translation>Valitse seuraava layer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R&amp;aise Layer</source>
        <translation>Nosta layer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Lower Layer</source>
        <translation>Laske layer</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show/&amp;Hide all Other Layers</source>
        <translation>Näytä/&amp;Piilota muut layerit</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Cut</source>
        <translation>Leikkaa</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Delete</source>
        <translation>Poista</translation>
    </message>
    <message numerus="yes">
        <location line="+327"/>
        <source>Duplicate %n Object(s)</source>
        <translation><numerusform>Monista %n objekti(a)</numerusform>
        <numerusform>Monista %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+1"/>
        <source>Remove %n Object(s)</source>
        <translation><numerusform>Poista %n objekti(a)</numerusform>
        <numerusform>Poista %n objektia</numerusform>
        </translation></message>
    <message>
        <location line="+2"/>
        <source>Duplicate Objects</source>
        <translation>Monista objektit</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Objects</source>
        <translation>Poista objektit</translation>
    </message>
    <message>
        <location line="-417"/>
        <source>&amp;Duplicate Layer</source>
        <translation>&amp;Monista layer</translation>
    </message>
    <message>
        <location line="-84"/>
        <source>Ctrl+PgUp</source>
        <translation>Ctrl+PageUp</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgDown</source>
        <translation>Ctrl+PageDown</translation>
    </message>
    <message>
        <location line="+82"/>
        <source>&amp;Merge Layer Down</source>
        <translation>&amp;Yhdistä layer allaolevaan</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Layer</source>
        <translation>&amp;Poista layer</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Layer &amp;Properties...</source>
        <translation>Layerin &amp;ominaisuudet...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapObjectModel</name>
    <message>
        <location filename="../src/tiled/mapobjectmodel.cpp" line="+150"/>
        <source>Change Object Name</source>
        <translation>Vaihda objektin nimeä</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Change Object Type</source>
        <translation>Vaihda objektin tyyppiä</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Name</source>
        <translation>Nimi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Tyyppi</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapsDock</name>
    <message>
        <location filename="../src/tiled/mapsdock.cpp" line="+83"/>
        <source>Browse...</source>
        <translation>Selaa...</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Choose the Maps Folder</source>
        <translation>Valitse kenttien kansio</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Maps</source>
        <translation>Kentät</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MiniMapDock</name>
    <message>
        <location filename="../src/tiled/minimapdock.cpp" line="+60"/>
        <source>Mini-map</source>
        <translation>Pienoiskenttä</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="-14"/>
        <source>Orthogonal</source>
        <translation>Ortogonaalinen</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric</source>
        <translation>Isometrinen</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric (Staggered)</source>
        <translation>Isometrinen (pinottu)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Hexagonal (Staggered)</source>
        <translation>Heksagonaalinen (pinottu)</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Tile Layer 1</source>
        <translation>Tile layer 1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Memory Usage Warning</source>
        <translation type="unfinished">Muistinkäytön varoitus</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile layers for this map will consume %L1 GB of memory each. Not creating one by default.</source>
        <translation>Tile layerit tälle kentälle käyttävät %L1 gigaa muistia. Ei luoda yhtään layeria oletuksena.</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>%1 x %2 pixels</source>
        <translation>%1 x %2 pikseliä</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-40"/>
        <location line="+7"/>
        <source>Error</source>
        <translation>Virhe</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Failed to load tileset image '%1'.</source>
        <translation>Tilesetin kuvan '%1' lataaminen epäonnistui.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No tiles found in the tileset image when using the given tile size, margin and spacing!</source>
        <translation>Tileset-kuvasta ei löytynyt yhtään tileä annetulla koolla, marginaalilla ja välistyksellä!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Tileset Image</source>
        <translation>Tileset-kuva</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectSelectionTool</name>
    <message>
        <location filename="../src/tiled/objectselectiontool.cpp" line="+309"/>
        <location line="+300"/>
        <source>Select Objects</source>
        <translation>Valitse objektit</translation>
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
        <translation><numerusform>Siirrä %n objekti(a)</numerusform>
        <numerusform>Siirrä %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+87"/>
        <source>Rotate %n Object(s)</source>
        <translation><numerusform>Pyöritä %n objekti(a)</numerusform>
        <numerusform>Pyöritä %n objektia</numerusform>
        </translation></message>
    <message numerus="yes">
        <location line="+266"/>
        <source>Resize %n Object(s)</source>
        <translation type="unfinished"><numerusform>Muuta %n objekti(n) kokoa</numerusform>
        <numerusform>Muuta %n objektin kokoa</numerusform>
        </translation></message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesEditor</name>
    <message>
        <location filename="../src/tiled/objecttypeseditor.cpp" line="+224"/>
        <source>Add Object Type</source>
        <translation>Lisää objektin tyyppi</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Object Type</source>
        <translation>Poista objektin tyyppi</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Lisää ominaisuus</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Property</source>
        <translation>Poista ominaisuus</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+323"/>
        <source>Rename Property</source>
        <translation>Nimeä ominaisuus uudelleen</translation>
    </message>
    <message>
        <location line="-265"/>
        <location line="+129"/>
        <source>Error Writing Object Types</source>
        <translation>Virhe kirjoitettaessa objektin tyyppiä</translation>
    </message>
    <message>
        <location line="-128"/>
        <source>Error writing to %1:
%2</source>
        <translation>Virhe kirjoitettaessa %1:
%2</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Choose Object Types File</source>
        <translation>Valitse objektin tyyppitiedosto</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+34"/>
        <location line="+44"/>
        <source>Object Types files (*.xml)</source>
        <translation>Objektityyppitiedostot (*.xml)</translation>
    </message>
    <message>
        <location line="-62"/>
        <location line="+44"/>
        <source>Error Reading Object Types</source>
        <translation>Virhe luettaessa objektityyppejä</translation>
    </message>
    <message>
        <location line="-28"/>
        <source>Import Object Types</source>
        <translation>Tuo objektityyppejä</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Export Object Types</source>
        <translation>Vie objektityyppejä</translation>
    </message>
    <message>
        <location line="+144"/>
        <source>Name:</source>
        <translation>Nimi:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesModel</name>
    <message>
        <location filename="../src/tiled/objecttypesmodel.cpp" line="+59"/>
        <source>Type</source>
        <translation>Tyyppi</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Color</source>
        <translation>Väri</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectsDock</name>
    <message>
        <location filename="../src/tiled/objectsdock.cpp" line="+170"/>
        <source>Object Properties</source>
        <translation>Objektin ominaisuudet</translation>
    </message>
    <message>
        <location line="-1"/>
        <source>Add Object Layer</source>
        <translation>Lisää objektilayer</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Objects</source>
        <translation>Objektit</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Move Objects Up</source>
        <translation>Siirrä objektit ylös</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Move Objects Down</source>
        <translation>Siirrä objektit alas</translation>
    </message>
    <message numerus="yes">
        <location line="+17"/>
        <source>Move %n Object(s) to Layer</source>
        <translation type="unfinished"><numerusform></numerusform>
        <numerusform></numerusform>
        </translation></message>
</context>
<context>
    <name>Tiled::Internal::PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.cpp" line="+68"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;Thank you for support!&lt;/h3&gt;
&lt;p&gt;Your support as a patron makes a big difference to me as the main developer and maintainer of Tiled. It allows me to spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Keep an eye out for exclusive updates in the Activity feed on my Patreon page to find out what I've been up to in the time I could spend on Tiled thanks to your support!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;Kiitos tuestasi!&lt;/h3&gt;
&lt;p&gt;Sinun tukesi suojelijana merkitsee paljon minulle Tiledin pääkehittäjänä ja ylläpitäjänä. Se sallii minun työskennellä enemmän Tiledin parissa kun minun ei tarvitse kuluttaa aikaa saadakseni rahaa muilla keinoin.&lt;/p&gt;
&lt;p&gt;Keep an eye out for exclusive updates in the Activity feed on my Patreon page to find out what I've been up to in the time I could spend on Tiled thanks to your support!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I'm no longer a patron</source>
        <translation>En ole enää suojelija</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;With your help I can continue to improve Tiled!&lt;/h3&gt;
&lt;p&gt;Please consider supporting me as a patron. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;Sinun avullasi voin jatkaa Tiledin kehitystyötä!&lt;/h3&gt;
&lt;p&gt;Ole kiltti ja harkitse työni tukemista alkamalla suojelijaksi. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I'm already a patron!</source>
        <translation>Olen jo suojelija!</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="-127"/>
        <location line="+123"/>
        <source>System default</source>
        <translation>Järjestelmän oletus</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Last checked: %1</source>
        <translation>Viimeksi tarkistettu: %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertiesDock</name>
    <message>
        <location filename="../src/tiled/propertiesdock.cpp" line="+278"/>
        <source>Name:</source>
        <translation>Nimi:</translation>
    </message>
    <message>
        <location line="+104"/>
        <source>Add Property</source>
        <translation>Lisää ominaisuus</translation>
    </message>
    <message>
        <location line="-102"/>
        <location line="+104"/>
        <source>Rename Property</source>
        <translation>Nimeä ominaisuus uudelleen</translation>
    </message>
    <message>
        <location line="-71"/>
        <source>Convert To</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rename...</source>
        <translation>Nimeä uudelleen...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove</source>
        <translation>Poista</translation>
    </message>
    <message>
        <location line="+65"/>
        <source>Properties</source>
        <translation>Ominaisuudet</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove Property</source>
        <translation>Poista ominaisuus</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertyBrowser</name>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+13"/>
        <source>Horizontal</source>
        <translation>Vaakasuuntainen</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertical</source>
        <translation>Pystysuuntainen</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Top Down</source>
        <translation>Ylhäältä alas</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>Opas</translation>
    </message>
    <message>
        <location line="+485"/>
        <source>Columns</source>
        <translation type="unfinished">Kolumni(a/t)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Source</source>
        <translation>Lähde</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Relative chance this tile will be picked</source>
        <translation>Suhteellinen muutos tähän tileen valitaan</translation>
    </message>
    <message>
        <location line="+286"/>
        <source>Error Reading Tileset</source>
        <translation>Virhe luettaessa tilesetiä</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Custom Properties</source>
        <translation>Mukautetut ominaisuudet</translation>
    </message>
    <message>
        <location line="-637"/>
        <source>Map</source>
        <translation>Kenttä</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer Format</source>
        <translation>Tilelayerin tiedostomuoto</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Render Order</source>
        <translation>Tilen renderöintijärjestys</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Background Color</source>
        <translation>Taustaväri</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Object</source>
        <translation>Objekti</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+26"/>
        <location line="+74"/>
        <location line="+60"/>
        <source>Name</source>
        <translation>Nimi</translation>
    </message>
    <message>
        <location line="-157"/>
        <source>Type</source>
        <translation>Tyyppi</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+21"/>
        <source>Visible</source>
        <translation>Näkyvä</translation>
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
        <translation>Pariton</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Even</source>
        <translation>Parillinen</translation>
    </message>
    <message>
        <location line="+342"/>
        <source>Orientation</source>
        <translation>Suuntaus</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Width</source>
        <translation>Leveys</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Height</source>
        <translation>Korkeus</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+167"/>
        <source>Tile Width</source>
        <translation>Tilen leveys</translation>
    </message>
    <message>
        <location line="-166"/>
        <location line="+167"/>
        <source>Tile Height</source>
        <translation>Tilen korkeus</translation>
    </message>
    <message>
        <location line="-165"/>
        <source>Tile Side Length (Hex)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Stagger Axis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Stagger Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Rotation</source>
        <translation>Kierto</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Flipping</source>
        <translation>Kääntö</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Opacity</source>
        <translation>Läpinäkyvyys</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Horizontal Offset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-37"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Vertical Offset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-41"/>
        <source>Tile Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Object Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Color</source>
        <translation>Väri</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Drawing Order</source>
        <translation>Piirtojärjestys</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Image Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+37"/>
        <location line="+39"/>
        <source>Image</source>
        <translation>Kuva</translation>
    </message>
    <message>
        <location line="-71"/>
        <location line="+39"/>
        <source>Transparent Color</source>
        <translation>Läpinäkyvä väri</translation>
    </message>
    <message>
        <location line="-29"/>
        <source>Tileset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Filename</source>
        <translation>Tiedostonimi</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Drawing Offset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Margin</source>
        <translation>Marginaali</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spacing</source>
        <translation>Välistys</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Tile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-133"/>
        <location line="+134"/>
        <source>ID</source>
        <translation>Tunniste</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Probability</source>
        <translation>Todennäköisyys</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Terrain</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::SelectSameTileTool</name>
    <message>
        <location filename="../src/tiled/selectsametiletool.cpp" line="+33"/>
        <location line="+62"/>
        <source>Select Same Tile</source>
        <translation>Valitse sama tile</translation>
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
        <location filename="../src/tiled/stampbrush.cpp" line="+41"/>
        <location line="+127"/>
        <source>Stamp Brush</source>
        <translation>Leimasin</translation>
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
        <location filename="../src/tiled/terrainbrush.cpp" line="+44"/>
        <location line="+114"/>
        <source>Terrain Brush</source>
        <translation type="unfinished"></translation>
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
        <location filename="../src/tiled/terraindock.cpp" line="+222"/>
        <source>Terrains</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Erase Terrain</source>
        <translation>Poista Terrain</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainView</name>
    <message>
        <location filename="../src/tiled/terrainview.cpp" line="+97"/>
        <source>Terrain &amp;Properties...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TextPropertyEdit</name>
    <message>
        <location filename="../src/tiled/textpropertyedit.cpp" line="+121"/>
        <source>...</source>
        <translation>...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="-58"/>
        <source>Delete Frames</source>
        <translation>Poista framet</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileCollisionEditor</name>
    <message>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+263"/>
        <source>Delete</source>
        <translation>Poista</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cut</source>
        <translation>Leikkaa</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Tile Collision Editor</source>
        <translation>Törmäyseditori</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileSelectionTool</name>
    <message>
        <location filename="../src/tiled/tileselectiontool.cpp" line="+34"/>
        <location line="+96"/>
        <source>Rectangular Select</source>
        <translation>Suorakulmainen valinta</translation>
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
        <translation>%1, %2 - suorakaide: (%3 x %4)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampModel</name>
    <message>
        <location filename="../src/tiled/tilestampmodel.cpp" line="+78"/>
        <source>Stamp</source>
        <translation>Leima</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Probability</source>
        <translation>Mahdollisuus</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampsDock</name>
    <message>
        <location filename="../src/tiled/tilestampsdock.cpp" line="+194"/>
        <source>Delete Stamp</source>
        <translation>Poista leima</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Variation</source>
        <translation>Poista muunnos</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Choose the Stamps Folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tile Stamps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add New Stamp</source>
        <translation>Lisää uusi leima</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add Variation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Duplicate Stamp</source>
        <translation>Monista leima</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete Selected</source>
        <translation>Poista valitut</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Stamps Folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Filter</source>
        <translation>Suodatin</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetDock</name>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="+731"/>
        <source>Remove Tileset</source>
        <translation>Poista tileset</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The tileset "%1" is still in use by the map!</source>
        <translation>Tileset "%1" on yhä käytettynä kentässä!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove this tileset and all references to the tiles in this tileset?</source>
        <translation type="unfinished">Poista tämä tiletset ja kaikki siihen liittyvät tilet?</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Tilesets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>New Tileset</source>
        <translation>Uusi tileset</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Import Tileset</source>
        <translation>&amp;Tuo tileset</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Export Tileset As...</source>
        <translation>&amp;Vie tileset nimellä...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile&amp;set Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Tileset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+128"/>
        <location line="+15"/>
        <source>Add Tiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-142"/>
        <location line="+199"/>
        <location line="+13"/>
        <source>Remove Tiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-121"/>
        <source>Error saving tileset: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Could not load "%1"!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+57"/>
        <source>One or more of the tiles to be removed are still in use by the map!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove all references to these tiles?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-207"/>
        <source>Edit &amp;Terrain Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+69"/>
        <location line="+23"/>
        <source>Export Tileset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetParametersEdit</name>
    <message>
        <location filename="../src/tiled/tilesetparametersedit.cpp" line="+48"/>
        <source>Edit...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetView</name>
    <message>
        <location filename="../src/tiled/tilesetview.cpp" line="+628"/>
        <source>Add Terrain Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Set Terrain Image</source>
        <translation>Aseta terrainin kuva</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Tile &amp;Properties...</source>
        <translation>Tilen &amp;ominaisuudet...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Show &amp;Grid</source>
        <translation>Näytä &amp;ruudukko</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TmxMapFormat</name>
    <message>
        <location filename="../src/tiled/tmxmapformat.h" line="+62"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled kenttätiedostot (*.tmx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TsxTilesetFormat</name>
    <message>
        <location line="+24"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation type="unfinished"></translation>
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
        <translation>&lt;tyhjä&gt;</translation>
    </message>
</context>
<context>
    <name>Tmw::TmwPlugin</name>
    <message>
        <location filename="../src/plugins/tmw/tmwplugin.cpp" line="+47"/>
        <source>Multiple collision layers found!</source>
        <translation>Useita törmäyslayereita havaittu!</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>No collision layer found!</source>
        <translation>Törmäyslayeria ei löydy!</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Could not open file for writing.</source>
        <translation>Tiedostoa ei voitu avata kirjoitusta varten.</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>TMW-eAthena collision files (*.wlk)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TmxViewer</name>
    <message>
        <location filename="../src/tmxviewer/tmxviewer.cpp" line="+182"/>
        <source>TMX Viewer</source>
        <translation>TMX katselin</translation>
    </message>
</context>
<context>
    <name>Undo Commands</name>
    <message>
        <location filename="../src/tiled/addremovelayer.h" line="+67"/>
        <source>Add Layer</source>
        <translation>Lisää layer</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Remove Layer</source>
        <translation>Poista layer</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovemapobject.cpp" line="+76"/>
        <source>Add Object</source>
        <translation>Lisää objekti</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove Object</source>
        <translation>Poista objekti</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetileset.cpp" line="+66"/>
        <source>Add Tileset</source>
        <translation>Lisää tileset</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tileset</source>
        <translation>Poista tileset</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+36"/>
        <source>Change Object</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changeobjectgroupproperties.cpp" line="+39"/>
        <source>Change Object Layer Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changeproperties.cpp" line="+40"/>
        <source>Change %1 Properties</source>
        <translation>Vaihda %1 ominaisuutta</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Set Property</source>
        <translation>Aseta ominaisuus</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Lisää ominaisuus</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Remove Property</source>
        <translation>Poista ominaisuus</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Rename Property</source>
        <translation>Nimeä ominaisuus uudelleen</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeselectedarea.cpp" line="+31"/>
        <source>Change Selection</source>
        <translation>Muuta valintaa</translation>
    </message>
    <message>
        <location filename="../src/tiled/erasetiles.cpp" line="+39"/>
        <source>Erase</source>
        <translation>Pyyhi</translation>
    </message>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="-30"/>
        <source>Fill Area</source>
        <translation>Täytä alue</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Move Object</source>
        <translation>Siirrä objektia</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobjecttogroup.cpp" line="+41"/>
        <source>Move Object to Layer</source>
        <translation>Siirrä objekti layerille</translation>
    </message>
    <message>
        <location filename="../src/tiled/movetileset.cpp" line="+31"/>
        <source>Move Tileset</source>
        <translation>Siirrä tileset:iä</translation>
    </message>
    <message>
        <location filename="../src/tiled/offsetlayer.cpp" line="+42"/>
        <source>Offset Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/painttilelayer.cpp" line="+51"/>
        <location line="+22"/>
        <source>Paint</source>
        <translation type="unfinished">Sivellin</translation>
    </message>
    <message>
        <location filename="../src/tiled/renamelayer.cpp" line="+40"/>
        <source>Rename Layer</source>
        <translation>Nimeä layer uudelleen</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizetilelayer.cpp" line="+37"/>
        <source>Resize Layer</source>
        <translation>Muuta layerin kokoa</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemap.cpp" line="+32"/>
        <source>Resize Map</source>
        <translation>Muuta kentän kokoa</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Resize Object</source>
        <translation>Muuta objektin kokoa</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="-788"/>
        <source>Import Tileset</source>
        <translation>Tuo tileset</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Export Tileset</source>
        <translation>Vie tileset</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetchanges.cpp" line="+36"/>
        <source>Change Tileset Name</source>
        <translation>Muuta tilesetin nimeä</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Change Drawing Offset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Edit Tileset</source>
        <translation>Muokkaa tilesetiä</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Change Columns</source>
        <translation>Muuta kolumneja</translation>
    </message>
    <message>
        <location filename="../src/tiled/movelayer.cpp" line="+37"/>
        <source>Lower Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changepolygon.cpp" line="+40"/>
        <location line="+12"/>
        <source>Change Polygon</source>
        <translation>Vaihda polygoni</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremoveterrain.cpp" line="+69"/>
        <source>Add Terrain</source>
        <translation>Lisää terrain</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Terrain</source>
        <translation>Poista terrain</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerproperties.cpp" line="+39"/>
        <source>Change Image Layer Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileterrain.cpp" line="+133"/>
        <source>Change Tile Terrain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="-135"/>
        <source>Change Terrain Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changelayer.cpp" line="+41"/>
        <source>Show Layer</source>
        <translation>Näytä layer</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Layer</source>
        <translation>Piilota layer</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Change Layer Opacity</source>
        <translation>Muuta layerin läpinäkyvyyttä</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Change Layer Offset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+31"/>
        <source>Show Object</source>
        <translation>Näytä objekti</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Object</source>
        <translation>Piilota objekti</translation>
    </message>
    <message>
        <location filename="../src/tiled/renameterrain.cpp" line="+37"/>
        <source>Change Terrain Name</source>
        <translation>Vaihda terrainin nimi</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetiles.cpp" line="+69"/>
        <source>Add Tiles</source>
        <translation type="unfinished">Lisää tilet/tilejä</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tiles</source>
        <translation type="unfinished">Poista tilet/tilejä</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerposition.cpp" line="+36"/>
        <source>Change Image Layer Position</source>
        <translation>Vaihda kuvalayerin sijaintia</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobjectsorder.cpp" line="+46"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+67"/>
        <source>Raise Object</source>
        <translation>Nosta objektia</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+29"/>
        <source>Lower Object</source>
        <translation>Laske objektia</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileanimation.cpp" line="+35"/>
        <source>Change Tile Animation</source>
        <translation>Vaihda tilen animaatiota</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileobjectgroup.cpp" line="+16"/>
        <source>Change Tile Collision</source>
        <translation>Vaihda tilen törmäystä</translation>
    </message>
    <message>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+43"/>
        <source>Raise Object To Top</source>
        <translation>Nosta objekti ylimmäiseksi</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Lower Object To Bottom</source>
        <translation>Laske objekti alimmaiseksi</translation>
    </message>
    <message>
        <location filename="../src/tiled/rotatemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Rotate Object</source>
        <translation>Pyöritä objektia</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapproperty.cpp" line="+41"/>
        <source>Change Tile Width</source>
        <translation>Muuta tilen leveyttä</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Tile Height</source>
        <translation>Muuta tilen korkeutta</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Hex Side Length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Background Color</source>
        <translation>Vaihda taustaväri</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Axis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Orientation</source>
        <translation>Muuta suuntausta</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Render Order</source>
        <translation>Muuta renderöintijärjestys</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Layer Data Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileprobability.cpp" line="+41"/>
        <location line="+14"/>
        <source>Change Tile Probability</source>
        <translation>Vaihda tilen todennäköisyyttä</translation>
    </message>
    <message>
        <location filename="../src/tiled/adjusttileindexes.cpp" line="-134"/>
        <location line="+89"/>
        <source>Adjust Tile Indexes</source>
        <translation>Säädä tilen indeksejä</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileimagesource.cpp" line="+39"/>
        <source>Change Tile Image</source>
        <translation>Vaihda tilekuva</translation>
    </message>
    <message>
        <location filename="../src/tiled/replacetileset.cpp" line="+33"/>
        <source>Replace Tileset</source>
        <translation>Korvaa tileset</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/tiled/flipmapobjects.cpp" line="+39"/>
        <source>Flip %n Object(s)</source>
        <translation type="unfinished"><numerusform></numerusform>
        <numerusform></numerusform>
        </translation></message>
</context>
<context>
    <name>Utils</name>
    <message>
        <location filename="../src/tiled/utils.cpp" line="+37"/>
        <source>Image files</source>
        <translation>Kuvatiedostot</translation>
    </message>
</context>
</TS>
