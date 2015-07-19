<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../src/tiled/aboutdialog.ui" line="+14"/>
        <source>About Tiled</source>
        <translation>О Tiled</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Donate</source>
        <translation>Спонсировать</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OK</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/tiled/aboutdialog.cpp" line="+36"/>
        <source>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Copyright 2008-2015 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(see the AUTHORS file for a full list of contributors)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;You may modify and redistribute this program under the terms of the GPL (version 2 or later). A copy of the GPL is contained in the &apos;COPYING&apos; file distributed with Tiled.&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</source>
        <translation>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Версия %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Copyright 2008-2015 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(смотрите файл AUTHORS с полным списком участников)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Вы можете модифицировать и распространять программу в рамках условий лицензии GPL (версии 2 и выше). Копия текста лицензии GPL находится в файле &apos;COPYING&apos; поставляемым с Tiled.&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</translation>
    </message>
</context>
<context>
    <name>Command line</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="+214"/>
        <source>Export syntax is --export-map [format] &lt;tmx file&gt; &lt;target file&gt;</source>
        <translation>Синтакс экспорта --export-map [format] &lt;tmx файл&gt; &lt;целевой файл&gt;</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Non-unique file extension. Can&apos;t determine correct export format.</source>
        <translation>Не уникальное расширение файла. Невозможно определить формат экспорта.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>No exporter found for target file.</source>
        <translation>Не найдер конвертер для целевого файла.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Failed to load source map.</source>
        <translation>Ошибка загрузки исходной карты.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Failed to export map to target file.</source>
        <translation>Ошибка экспорта карты в целевой файл.</translation>
    </message>
</context>
<context>
    <name>CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.ui" line="+14"/>
        <source>Properties</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>&amp;Save map before executing</source>
        <translation>&amp;Сохранить карту перед выполнением</translation>
    </message>
</context>
<context>
    <name>CommandLineHandler</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="-172"/>
        <source>Display the version</source>
        <translation>Отобразить версию</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Only check validity of arguments</source>
        <translation>Проверять только валидность аргументов</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Disable hardware accelerated rendering</source>
        <translation>Выключить аппаратное ускорение отрисовки</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Export the specified tmx file to target</source>
        <translation>Экспортировать указанный tmx-файл</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Print a list of supported export formats</source>
        <translation>Отобразить список поддерживаемых форматов экспорта</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Export formats:</source>
        <translation>Форматы экспорта:</translation>
    </message>
</context>
<context>
    <name>CommandLineParser</name>
    <message>
        <location filename="../src/tiled/commandlineparser.cpp" line="+75"/>
        <source>Bad argument %1: lonely hyphen</source>
        <translation>Неверный аргумент %1: одиночный дефис</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Unknown long argument %1: %2</source>
        <translation>Неверный long аргумент %1: %2</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Unknown short argument %1.%2: %3</source>
        <translation>Неверный short аргумент %1.%2: %3</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Usage:
  %1 [options] [files...]</source>
        <translation>Использование:
  %1 [параметры] [файлы...]</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Options:</source>
        <translation>Параметры:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Display this help</source>
        <translation>Отобразить эту справку</translation>
    </message>
</context>
<context>
    <name>ConsoleDock</name>
    <message>
        <location filename="../src/tiled/consoledock.cpp" line="+34"/>
        <source>Debug Console</source>
        <translation>Консоль отладки</translation>
    </message>
</context>
<context>
    <name>ConverterDataModel</name>
    <message>
        <location filename="../src/automappingconverter/converterdatamodel.cpp" line="+75"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
</context>
<context>
    <name>ConverterWindow</name>
    <message>
        <location filename="../src/automappingconverter/converterwindow.cpp" line="+36"/>
        <source>Save all as %1</source>
        <translation>Сохранить всё как %1</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>All Files (*)</source>
        <translation>Все файлы (*)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled map файлы (*.tmx)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Open Map</source>
        <translation>Открыть карту</translation>
    </message>
</context>
<context>
    <name>Csv::CsvPlugin</name>
    <message>
        <location filename="../src/plugins/csv/csvplugin.cpp" line="+55"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV файлы (*.csv)</translation>
    </message>
</context>
<context>
    <name>Droidcraft::DroidcraftPlugin</name>
    <message>
        <location filename="../src/plugins/droidcraft/droidcraftplugin.cpp" line="+57"/>
        <source>This is not a valid Droidcraft map file!</source>
        <translation>Это неверный Droidcraft map файл!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>The map needs to have exactly one tile layer!</source>
        <translation>Карта должна содержать минимум один слой тайлов!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The layer must have a size of 48 x 48 tiles!</source>
        <translation>Слой должен иметь размер 48 x 48 тайлов!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Droidcraft map files (*.dat)</source>
        <translation>Droidcraft map файлы (*.dat)</translation>
    </message>
</context>
<context>
    <name>EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.ui" line="+14"/>
        <source>Edit Terrain Information</source>
        <translation>Изменить информацию участка</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+3"/>
        <source>Undo</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+3"/>
        <source>Redo</source>
        <translation>Повторить</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Erase</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Add Terrain Type</source>
        <translation>Добавить тип участка</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Terrain Type</source>
        <translation>Удалить тип участка</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
</context>
<context>
    <name>ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.ui" line="+14"/>
        <source>Export As Image</source>
        <translation>Экспортировать как изображение</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Location</source>
        <translation>Путь</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Обзор...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Settings</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only include &amp;visible layers</source>
        <translation>Включить только &amp;видимые слои</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Use current &amp;zoom level</source>
        <translation>&amp;Использовать текущий уровень масштабирования</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Draw tile grid</source>
        <translation>&amp;Отображать сетку</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Include background color</source>
        <translation>&amp;Включить цвет фона</translation>
    </message>
</context>
<context>
    <name>Flare::FlarePlugin</name>
    <message>
        <location filename="../src/plugins/flare/flareplugin.cpp" line="+53"/>
        <source>Could not open file for reading.</source>
        <translation>Не удается открыть файл для чтения.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Error loading tileset %1, which expands to %2. Path not found!</source>
        <translation>Возникла ошибка при загрузке набора тайлов %1, расширяемого в %2. Путь не найден!</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>No tilesets section found before layer section.</source>
        <translation>Не найдено набора тайлов до слоя тайлов.</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Error mapping tile id %1.</source>
        <translation>Ошибка при создании карты для тайла с id %1.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>This seems to be no valid flare map. A Flare map consists of at least a header section, a tileset section and one tile layer.</source>
        <translation>Файл не соответствует спецификации Flare-карты. Flare карта состоит по крайней мере из одной секции заголовка, набора тайлов и слоя тайлов.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Flare map files (*.txt)</source>
        <translation>Flare map файлы (*.txt)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
</context>
<context>
    <name>Json::JsonPlugin</name>
    <message>
        <location filename="../src/plugins/json/jsonplugin.cpp" line="+44"/>
        <source>Could not open file for reading.</source>
        <translation>Не удается открыть файл для чтения.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error parsing file.</source>
        <translation>Ошибка при открытии файла.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Error while writing file:
%1</source>
        <translation>Ошибка во время записи файла:
%1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Json files (*.json)</source>
        <translation>Json файлы (*.json)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>JavaScript files (*.js)</source>
        <translation>JavaScript файлы (*.js)</translation>
    </message>
</context>
<context>
    <name>Lua::LuaPlugin</name>
    <message>
        <location filename="../src/plugins/lua/luaplugin.cpp" line="+58"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Lua files (*.lua)</source>
        <translation>Lua файлы (*.lua)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/tiled/mainwindow.ui" line="+46"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Recent Files</source>
        <translation>&amp;Последние открытые</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>&amp;Help</source>
        <translation>&amp;Помощь</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Map</source>
        <translation>&amp;Карта</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Main Toolbar</source>
        <translation>Главная панель инструментов</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Tools</source>
        <translation>Инструменты</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Open...</source>
        <translation>&amp;Открыть...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Quit</source>
        <translation>&amp;Выход</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Paste</source>
        <translation>&amp;Вставить</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;About Tiled</source>
        <translation>&amp;О Tiled</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>About Qt</source>
        <translation>О Qt</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Resize Map...</source>
        <translation>&amp;Изменить размер карты...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map &amp;Properties</source>
        <translation>Параметры &amp;карты</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>AutoMap</source>
        <translation>Авто карта</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show &amp;Grid</source>
        <translation>Отображать &amp;сетку</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+G</source>
        <translation></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;As...</source>
        <translation>Сохранить &amp;как...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;New...</source>
        <translation>&amp;Создать...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>New &amp;Tileset...</source>
        <translation>Новый набор &amp;тайлов...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom In</source>
        <translation>Приблизить</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom Out</source>
        <translation>Отдалить</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normal Size</source>
        <translation>Нормальный размер</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+0</source>
        <translation></translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Become a Patron</source>
        <translation>Стать спонсором (Patreon)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save All</source>
        <translation>Сохранить всё</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Documentation</source>
        <translation>Документация</translation>
    </message>
    <message>
        <location line="-135"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Offset Map...</source>
        <translation>&amp;Смещение карты...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Offsets everything in a layer</source>
        <translation>Смещается всё, что на слое</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pre&amp;ferences...</source>
        <translation>&amp;Настройки...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Clear Recent Files</source>
        <translation>Очистить список недавних файлов</translation>
    </message>
    <message>
        <location line="+87"/>
        <source>Ctrl+R</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Export</source>
        <translation>&amp;Экспорт</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+E</source>
        <translation></translation>
    </message>
    <message>
        <location line="-82"/>
        <source>&amp;Add External Tileset...</source>
        <translation>&amp;Добавить внешний набор тайлов...</translation>
    </message>
    <message>
        <location line="-50"/>
        <source>Export As &amp;Image...</source>
        <translation>Экспортировать как &amp;изображение...</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>E&amp;xport As...</source>
        <translation>Эк&amp;спортировать как...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+E</source>
        <translation></translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Snap to Grid</source>
        <translation>&amp;Пристыковывать к сетке</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>C&amp;lose All</source>
        <translation>&amp;Закрыть всё</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+W</source>
        <translation></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Delete</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Highlight Current Layer</source>
        <translation>&amp;Подсветить текущий слой</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H</source>
        <translation></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Object &amp;Outlines</source>
        <translation>&amp;Отображать границы тайлов-объектов</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap to &amp;Fine Grid</source>
        <translation>&amp;Прилипать к мелкой сетке</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Animations</source>
        <translation>Отображать анимацию тайла</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Reload</source>
        <translation>Перезагрузить</translation>
    </message>
    <message>
        <location filename="../src/automappingconverter/converterwindow.ui" line="+14"/>
        <source>Tiled Automapping Rule Files Converter</source>
        <translation>Конвертер файлов правил авто-карты</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Add new Automapping rules</source>
        <translation>Добавить правило Авто-карты</translation>
    </message>
</context>
<context>
    <name>MapReader</name>
    <message>
        <location filename="../src/libtiled/mapreader.cpp" line="+140"/>
        <source>Not a map file.</source>
        <translation>Файл не является файлом карты.</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Not a tileset file.</source>
        <translation>Файл не является файлом набора тайлов.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Линия %1, колонка %2</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File not found: %1</source>
        <translation>Файл не найден: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unable to read file: %1</source>
        <translation>Не удается прочитать файл: %1</translation>
    </message>
    <message>
        <location line="+37"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+57"/>
        <source>Unsupported map orientation: &quot;%1&quot;</source>
        <translation>Не поддерживаемая ориентация карты: &quot;%1&quot;</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+19"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+83"/>
        <source>Invalid tileset parameters for tileset &apos;%1&apos;</source>
        <translation>Неверный параметр набора тайлов: &apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Error while loading tileset &apos;%1&apos;: %2</source>
        <translation>Возникла ошибка при загрузке набора тайлов %1: %2</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Invalid tile ID: %1</source>
        <translation>Неверный ID тайла: %1</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Tile ID does not exist in tileset image: %1</source>
        <translation>Не найден ID тайла в наборе тайлов: %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Invalid (nonconsecutive) tile ID: %1</source>
        <translation>Неверный (непоследовательный) ID тайла: %1</translation>
    </message>
    <message>
        <location line="+89"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+19"/>
        <source>Error loading tileset image:
&apos;%1&apos;</source>
        <translation>Возникла ошибка при загрузке изображения набора тайлов: &apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+33"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+322"/>
        <source>Error loading image:
&apos;%1&apos;</source>
        <translation>Ошибка при загрузке изображения:
&apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+112"/>
        <source>Too many &lt;tile&gt; elements</source>
        <translation>Слишком много &lt;tile&gt; элементов</translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+43"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-120"/>
        <source>Invalid tile: %1</source>
        <translation>Неверный тайл: %1</translation>
    </message>
    <message>
        <location line="+29"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+34"/>
        <source>Invalid draw order: %1</source>
        <translation>Неверный порядок отображения: %1</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Error loading image layer image:
&apos;%1&apos;</source>
        <translation>Ошибка при загрузке изображения слоя изображений:
&apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+98"/>
        <source>Invalid points data for polygon</source>
        <translation>Неверные координаты полигона</translation>
    </message>
    <message>
        <location line="-290"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-90"/>
        <source>Unknown encoding: %1</source>
        <translation>Неизвестная кодировка: %1</translation>
    </message>
    <message>
        <location line="-5"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-4"/>
        <source>Compression method &apos;%1&apos; not supported</source>
        <translation>Метод компрессии &apos;%1&apos; не поддерживается</translation>
    </message>
    <message>
        <location line="+57"/>
        <location line="+19"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+15"/>
        <location line="+39"/>
        <source>Corrupt layer data for layer &apos;%1&apos;</source>
        <translation>Содержимое слоя &apos;%1&apos; повреждено</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-28"/>
        <source>Unable to parse tile at (%1,%2) on layer &apos;%3&apos;</source>
        <translation>Не удается обработать тайл по координатам (%1, %2) на слое &apos;%3&apos;</translation>
    </message>
    <message>
        <location line="-28"/>
        <location line="+44"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+31"/>
        <source>Tile used but no tilesets specified</source>
        <translation>Тайл используется, но не указан набор тайлов</translation>
    </message>
    <message>
        <location filename="../src/libtiled/mapwriter.cpp" line="+106"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-177"/>
        <source>Tileset tile index negative:
&apos;%1&apos;</source>
        <translation>Отрицательный индекс тайла в наборе тайлов:
&apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Tileset tile index too high:
&apos;%1&apos;</source>
        <translation>Индекс тайла слишком большой:
&apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.ui" line="+14"/>
        <source>New Map</source>
        <translation>Новая карта</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map size</source>
        <translation>Размер карты</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+68"/>
        <source>Width:</source>
        <translation>Ширина:</translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+26"/>
        <source> tiles</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> тайлов</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+68"/>
        <source>Height:</source>
        <translation>Высота:</translation>
    </message>
    <message>
        <location line="-32"/>
        <source>Tile size</source>
        <translation>Размер тайлов</translation>
    </message>
    <message>
        <location line="+16"/>
        <location line="+26"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation></translation>
    </message>
    <message>
        <location line="+55"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Orientation:</source>
        <translation>Ориентация:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Tile layer format:</source>
        <translation>Формат слоя тайлов:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile render order:</source>
        <translation>Порядок отображения тайлов:</translation>
    </message>
</context>
<context>
    <name>NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.ui" line="+14"/>
        <source>New Tileset</source>
        <translation>Новый набор тайлов</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tileset</source>
        <translation>Набор тайлов</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Based on Tileset Image</source>
        <translation>Основано на наборе тайлов</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Collection of Images</source>
        <translation>Коллекция изображений</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Name:</source>
        <translation>&amp;Имя:</translation>
    </message>
    <message>
        <location line="+51"/>
        <source>&amp;Browse...</source>
        <translation>&amp;Обзор...</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Use transparent color:</source>
        <translation>Использовать цвет прозрачности:</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>Tile width:</source>
        <translation>Ширина тайла:</translation>
    </message>
    <message>
        <location line="-100"/>
        <location line="+42"/>
        <location line="+26"/>
        <location line="+16"/>
        <source> px</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation></translation>
    </message>
    <message>
        <location line="-142"/>
        <source>Image</source>
        <translation>Изображение</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Source:</source>
        <translation>Источник:</translation>
    </message>
    <message>
        <location line="+101"/>
        <source>The space at the edges of the tileset.</source>
        <translation>Отступ по краям набора тайлов.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin:</source>
        <translation>Отступ:</translation>
    </message>
    <message>
        <location line="-45"/>
        <source>Tile height:</source>
        <translation>Высота тайла:</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>The space between the tiles.</source>
        <translation>Расстояние между тайлами.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spacing:</source>
        <translation>Промежуток:</translation>
    </message>
</context>
<context>
    <name>ObjectTypes</name>
    <message>
        <location filename="../src/tiled/objecttypes.cpp" line="+38"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Could not open file.</source>
        <translation>Не удается открыть файл.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>File doesn&apos;t contain object types.</source>
        <translation>Файл не содержит типов объектов.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

Линия %1, колонка %2</translation>
    </message>
</context>
<context>
    <name>OffsetMapDialog</name>
    <message>
        <location filename="../src/tiled/offsetmapdialog.ui" line="+17"/>
        <source>Offset Map</source>
        <translation>Смещение карты</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Offset Contents of Map</source>
        <translation>Смещение содержимого карты</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+23"/>
        <location line="+43"/>
        <source>Wrap</source>
        <translation>Перенос</translation>
    </message>
    <message>
        <location line="-36"/>
        <source>Y:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Layers:</source>
        <translation>Слои:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>All Visible Layers</source>
        <translation>Все видимые слои</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>All Layers</source>
        <translation>Все слои</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Selected Layer</source>
        <translation>Выделенный слой</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bounds:</source>
        <translation>Границы:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Whole Map</source>
        <translation>Вся карта</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Current Selection</source>
        <translation>Текущее выделение</translation>
    </message>
</context>
<context>
    <name>PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.ui" line="+14"/>
        <source>Become a Patron</source>
        <translation>Стать спонсором (Patreon)</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Visit https://www.patreon.com/bjorn</source>
        <translation>Перейти на https://www.patreon.com/bjorn</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I&apos;m already a patron!</source>
        <translation>Я уже спонсор!</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maybe later</source>
        <translation>Возможно позже</translation>
    </message>
</context>
<context>
    <name>PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+14"/>
        <source>Preferences</source>
        <translation>Настройки</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>General</source>
        <translation>Основные</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Saving and Loading</source>
        <translation>Сохранение и загрузка</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+62"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+84"/>
        <source>XML</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (uncompressed)</source>
        <translation>Base64 (без сжатия)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (gzip compressed)</source>
        <translation>Base64 (gzip сжатие)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (zlib compressed)</source>
        <translation>Base64 (zlib сжатие)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>CSV</source>
        <translation></translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+2"/>
        <source>Right Down</source>
        <translation>Справа снизу</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Right Up</source>
        <translation>Справа сверху</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Down</source>
        <translation>Слева снизу</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Up</source>
        <translation>Слева сверху</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+6"/>
        <source>&amp;Reload tileset images when they change</source>
        <translation>&amp;Перезагружать изображение тайлов при изменении</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Not enabled by default since a reference to an external DTD is known to cause problems with some XML parsers.</source>
        <translation>Отключено по умолчанию, поскольку внешние ссылки DTD, как известно, вызывают проблемы с некоторыми XML парсерами.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Include &amp;DTD reference in saved maps</source>
        <translation>Включать &amp;DTD заголовки в файл сохраняемой карты</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Interface</source>
        <translation>Интерфейс</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Language:</source>
        <translation>&amp;Язык:</translation>
    </message>
    <message>
        <location line="+220"/>
        <source>Automapping</source>
        <translation>Авто-карта</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Use Automapping, when drawing into layers</source>
        <translation>Использовать Авто-карту при рисовании на слое</translation>
    </message>
    <message>
        <location line="-239"/>
        <source>Hardware &amp;accelerated drawing (OpenGL)</source>
        <translation>Аппаратное &amp;ускорение отрисовки (OpenGL)</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Open last files on startup</source>
        <translation>Открывать последние файлы при старте</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Grid color:</source>
        <translation>Цвет сетки:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Fine grid divisions:</source>
        <translation>Деление сетки:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source> pixels</source>
        <translation> пиксели</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Object line width:</source>
        <translation>Толщина линии объектов:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Object Types</source>
        <translation>Типы объектов</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Add Object Type</source>
        <translation>Добавить тип объекта</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Selected Object Types</source>
        <translation>Удалить выбранные типы объектов</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Import...</source>
        <translation>Импортировать...</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Export...</source>
        <translation>Экспортировать...</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/automappingconverter/convertercontrol.h" line="+33"/>
        <source>v0.8 and before</source>
        <translation>v0.8 и ниже</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>v0.9 and later</source>
        <translation>v0.9 и выше</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>unknown</source>
        <translation>неизвестно</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>not a map</source>
        <translation>не карта</translation>
    </message>
</context>
<context>
    <name>QtBoolEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+233"/>
        <location line="+10"/>
        <location line="+25"/>
        <source>True</source>
        <translation>Да</translation>
    </message>
    <message>
        <location line="-25"/>
        <location line="+25"/>
        <source>False</source>
        <translation>Нет</translation>
    </message>
</context>
<context>
    <name>QtBoolPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+1696"/>
        <source>True</source>
        <translation>Да</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>False</source>
        <translation>Нет</translation>
    </message>
</context>
<context>
    <name>QtCharEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qteditorfactory.cpp" line="+1700"/>
        <source>Clear Char</source>
        <translation>Очистить символ</translation>
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
        <translation>Красный</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Green</source>
        <translation>Зелёный</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Blue</source>
        <translation>Синий</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Alpha</source>
        <translation>Прозрачность</translation>
    </message>
</context>
<context>
    <name>QtCursorDatabase</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-210"/>
        <source>Arrow</source>
        <translation>Стрелка</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Up Arrow</source>
        <translation>Стрелка вверх</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Cross</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Wait</source>
        <translation>Подождите</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>IBeam</source>
        <translation>I</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Vertical</source>
        <translation>Размер по вертикали</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Horizontal</source>
        <translation>Размер по горизонтали</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Backslash</source>
        <translation>Размер обратного слэша</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Slash</source>
        <translation>Размер слэша</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size All</source>
        <translation>Общий размер</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Blank</source>
        <translation>Чистый</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Vertical</source>
        <translation>Разделить по вертикали</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Horizontal</source>
        <translation>Разделить по горизонтали</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pointing Hand</source>
        <translation>Рука-указатель</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forbidden</source>
        <translation>Запрещено</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Open Hand</source>
        <translation>Открытая рука</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Closed Hand</source>
        <translation>Сжатая рука</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>What&apos;s This</source>
        <translation>Что это</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Busy</source>
        <translation>Занят</translation>
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
        <translation>Выберите шрифт</translation>
    </message>
</context>
<context>
    <name>QtFontPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="-350"/>
        <source>Family</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Point Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bold</source>
        <translation>Жирный</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Italic</source>
        <translation>Курсив</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Underline</source>
        <translation>Подчеркнутый</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Strikeout</source>
        <translation>Перечеркнутый</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Kerning</source>
        <translation>Кернинг</translation>
    </message>
</context>
<context>
    <name>QtKeySequenceEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+234"/>
        <source>Clear Shortcut</source>
        <translation>Очистить быстрый вызов</translation>
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
        <translation>Язык</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Country</source>
        <translation>Страна</translation>
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
        <translation>Длина</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Высота</translation>
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
        <translation>Длина</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Высота</translation>
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
        <translation>Длина</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
</context>
<context>
    <name>QtSizePolicyPropertyManager</name>
    <message>
        <location line="+1704"/>
        <location line="+1"/>
        <source>&lt;Invalid&gt;</source>
        <translation>&lt;Ошибка&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>[%1, %2, %3, %4]</source>
        <translation>[%1, %2, %3, %4]</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Horizontal Policy</source>
        <translation>Горизонтальная политика</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vertical Policy</source>
        <translation>Вертикальная политика</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Horizontal Stretch</source>
        <translation>Горизонтальное растяжение</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Vertical Stretch</source>
        <translation>Вертикальное растяжение</translation>
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
        <translation>Длина</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
</context>
<context>
    <name>QtTreePropertyBrowser</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qttreepropertybrowser.cpp" line="+478"/>
        <source>Property</source>
        <translation>Параметр</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
</context>
<context>
    <name>ReplicaIsland::ReplicaIslandPlugin</name>
    <message>
        <location filename="../src/plugins/replicaisland/replicaislandplugin.cpp" line="+59"/>
        <source>Cannot open Replica Island map file!</source>
        <translation>Не удается открыть Replica Insland файл!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Can&apos;t parse file header!</source>
        <translation>Не удается распарсить заголовок файла!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Can&apos;t parse layer header!</source>
        <translation>Не удается распарсить заголовок слоя!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Inconsistent layer sizes!</source>
        <translation>Несовместимый размер слоя!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File ended in middle of layer!</source>
        <translation>Файл закончился в середине слоя!</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Unexpected data at end of file!</source>
        <translation>Неожиданное содержимое в конце файла!</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Replica Island map files (*.bin)</source>
        <translation>Replica Island файлы (*.bin)</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>You must define a background_index property on the map!</source>
        <translation>Вы должны установить параметр background_index для карты!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Can&apos;t save non-tile layer!</source>
        <translation>Не удается сохранить не тайловый слой!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>You must define a type property on each layer!</source>
        <translation>Вы должны установить параметр типа для каждого слоя!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a tile_index property on each layer!</source>
        <translation>Вы должны установить параметр tile_index для каждого слоя!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a scroll_speed property on each layer!</source>
        <translation>Вы должны установить параметр scroll_speed для каждого слоя!</translation>
    </message>
</context>
<context>
    <name>ResizeDialog</name>
    <message>
        <location filename="../src/tiled/resizedialog.ui" line="+14"/>
        <source>Resize</source>
        <translation>Масштабирование</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Width:</source>
        <translation>Ширина:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Height:</source>
        <translation>Высота:</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Offset</source>
        <translation>Смещение</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Y:</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tengine::TenginePlugin</name>
    <message>
        <location filename="../src/plugins/tengine/tengineplugin.cpp" line="+49"/>
        <source>Could not open file for writing.</source>
        <translation>Не удается открыть файл для записи.</translation>
    </message>
    <message>
        <location line="+246"/>
        <source>T-Engine4 map files (*.lua)</source>
        <translation>T-Engine4 map файлы (*.lua)</translation>
    </message>
</context>
<context>
    <name>TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.ui" line="+14"/>
        <source>Tile Animation Editor</source>
        <translation>Редактор анимации</translation>
    </message>
    <message>
        <location line="+99"/>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="+507"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractObjectTool</name>
    <message numerus="yes">
        <location filename="../src/tiled/abstractobjecttool.cpp" line="+177"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Дублировать %n объект</numerusform>
            <numerusform>Дублировать %n объекта</numerusform>
            <numerusform>Дублировать %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+2"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Удалить %n объект</numerusform>
            <numerusform>Удалить %n объекта</numerusform>
            <numerusform>Удалить %n объектов</numerusform>
        </translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Flip Horizontally</source>
        <translation>Перевернуть по горизонтали</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Flip Vertically</source>
        <translation>Перевернуть по вертикали</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Raise Object</source>
        <translation>Поднять объект</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgUp</source>
        <translation>PgUp</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object</source>
        <translation>Опустить объект</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgDown</source>
        <translation>PgDown</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Object to Top</source>
        <translation>На передний план</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Home</source>
        <translation>Home</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object to Bottom</source>
        <translation>На задний план</translation>
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
            <numerusform>Переместить %n объект на слой</numerusform>
            <numerusform>Переместить %n объекта на слой</numerusform>
            <numerusform>Переместить %n объектов на слой</numerusform>
        </translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Object &amp;Properties...</source>
        <translation>Свойства &amp;объекта...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractTileTool</name>
    <message>
        <location filename="../src/tiled/abstracttiletool.cpp" line="+119"/>
        <source>empty</source>
        <translation>пусто</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutoMapper</name>
    <message>
        <location filename="../src/tiled/automapper.cpp" line="+115"/>
        <source>&apos;%1&apos;: Property &apos;%2&apos; = &apos;%3&apos; does not make sense. Ignoring this property.</source>
        <translation>&apos;%1&apos;: Параметр &apos;%2&apos; = &apos;%3&apos; не выполняет ничего важного. Параметр игнорируется.</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Did you forget an underscore in layer &apos;%1&apos;?</source>
        <translation>Вы забыли подчеркивание в слое &apos;%1&apos;?</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Layer &apos;%1&apos; is not recognized as a valid layer for Automapping.</source>
        <translation>Слой &apos;%1&apos; не является действительным слоем для Авто-карты.</translation>
    </message>
    <message>
        <location line="-105"/>
        <source>&apos;regions_input&apos; layer must not occur more than once.</source>
        <translation>&apos;regions_input&apos; слой может объявляться лишь однократно.</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+13"/>
        <source>&apos;regions_*&apos; layers must be tile layers.</source>
        <translation>&apos;regions_*&apos; слои должны быть слоями тайлов.</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>&apos;regions_output&apos; layer must not occur more than once.</source>
        <translation>&apos;regions_output&apos; слой может объявляться лишь однократно.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>&apos;input_*&apos; and &apos;inputnot_*&apos; layers must be tile layers.</source>
        <translation>&apos;input_*&apos; и &apos;inputnot_*&apos; слои должны быть слоями тайлов.</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>No &apos;regions&apos; or &apos;regions_input&apos; layer found.</source>
        <translation>Не найдено слоев &apos;regions&apos; и &apos;regions_input&apos;.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No &apos;regions&apos; or &apos;regions_output&apos; layer found.</source>
        <translation>Не найдено слоев &apos;regions&apos; и &apos;regions_outpu&apos;.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No input_&lt;name&gt; layer found!</source>
        <translation>Не найден слой input_&lt;name&gt;!</translation>
    </message>
    <message>
        <location line="+173"/>
        <source>Tile</source>
        <translation>Тайл</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutomappingManager</name>
    <message>
        <location filename="../src/tiled/automappingmanager.cpp" line="+103"/>
        <source>Apply AutoMap rules</source>
        <translation>Применить правила авто-карты</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>No rules file found at:
%1</source>
        <translation>Не найден файл правил:
%1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Error opening rules file:
%1</source>
        <translation>Ошибка при открытии файла правил:
%1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>File not found:
%1</source>
        <translation>Файл не найден:
%1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Opening rules map failed:
%1</source>
        <translation>Ошибка при открытии карты правил: %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BucketFillTool</name>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="+40"/>
        <location line="+175"/>
        <source>Bucket Fill Tool</source>
        <translation>Заливка</translation>
    </message>
    <message>
        <location line="-172"/>
        <location line="+173"/>
        <source>F</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ClipboardManager</name>
    <message>
        <location filename="../src/tiled/clipboardmanager.cpp" line="+167"/>
        <source>Paste Objects</source>
        <translation>Вставить объекты</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandButton</name>
    <message>
        <location filename="../src/tiled/commandbutton.cpp" line="+130"/>
        <source>Execute Command</source>
        <translation>Выполнить команду</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>F5</source>
        <translation></translation>
    </message>
    <message>
        <location line="-67"/>
        <source>Error Executing Command</source>
        <translation>Ошибка выполнения команды</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>You do not have any commands setup.</source>
        <translation>У Вас нет никаких настроек команд.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Edit commands...</source>
        <translation>Редактировать команды...</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Edit Commands...</source>
        <translation>Редактировать команды...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDataModel</name>
    <message>
        <location filename="../src/tiled/commanddatamodel.cpp" line="+60"/>
        <source>Open in text editor</source>
        <translation>Открыть в текстовом редакторе</translation>
    </message>
    <message>
        <location line="+92"/>
        <location line="+69"/>
        <source>&lt;new command&gt;</source>
        <translation>&lt;новая команда&gt;</translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Set a name for this command</source>
        <translation>Задать имя данной команде</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set the shell command to execute</source>
        <translation>Задать команду для выполнения</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Show or hide this command in the command list</source>
        <translation>Показать или спрятать данную команду в списке</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add a new command</source>
        <translation>Добавить новую команду</translation>
    </message>
    <message>
        <location line="+107"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Command</source>
        <translation>Команда</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable</source>
        <translation>Включено</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Move Up</source>
        <translation>Поднять вверх</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Move Down</source>
        <translation>Опустить вниз</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Execute</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Execute in Terminal</source>
        <translation>Выполнить в терминале</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+84"/>
        <source>%1 (copy)</source>
        <translation>%1 (копия)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>New command</source>
        <translation>Новая команда</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.cpp" line="+44"/>
        <source>Edit Commands</source>
        <translation>Редактировать команды</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandProcess</name>
    <message>
        <location filename="../src/tiled/command.cpp" line="+130"/>
        <source>Unable to create/open %1</source>
        <translation>Не удалось создать/открыть %1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unable to add executable permissions to %1</source>
        <translation>Не удалось назначить права для выполнения %1</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>The command failed to start.</source>
        <translation>Не удалось начать выполнение команды.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command crashed.</source>
        <translation>Ошибка при выполнении команды.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command timed out.</source>
        <translation>Тайм-аут выполнения команды.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>An unknown error occurred.</source>
        <translation>Неизвестная ошибка.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Error Executing %1</source>
        <translation>Ошибка выполнения %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateEllipseObjectTool</name>
    <message>
        <location filename="../src/tiled/createellipseobjecttool.cpp" line="+39"/>
        <source>Insert Ellipse</source>
        <translation>Вставить эллипс</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>C</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateObjectTool</name>
    <message>
        <location filename="../src/tiled/createobjecttool.cpp" line="+46"/>
        <source>O</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreatePolygonObjectTool</name>
    <message>
        <location filename="../src/tiled/createpolygonobjecttool.cpp" line="+39"/>
        <source>Insert Polygon</source>
        <translation>Вставить полигон</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>P</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreatePolylineObjectTool</name>
    <message>
        <location filename="../src/tiled/createpolylineobjecttool.cpp" line="+39"/>
        <source>Insert Polyline</source>
        <translation>Вставить линию</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>L</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateRectangleObjectTool</name>
    <message>
        <location filename="../src/tiled/createrectangleobjecttool.cpp" line="+39"/>
        <source>Insert Rectangle</source>
        <translation>Вставить прямоугольник</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateTileObjectTool</name>
    <message>
        <location filename="../src/tiled/createtileobjecttool.cpp" line="+78"/>
        <source>Insert Tile</source>
        <translation>Вставить тайл</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>T</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::DocumentManager</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="+338"/>
        <source>%1:

%2</source>
        <translation>%1:

%2</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditPolygonTool</name>
    <message>
        <location filename="../src/tiled/editpolygontool.cpp" line="+128"/>
        <location line="+209"/>
        <source>Edit Polygons</source>
        <translation>Редактировать полигоны</translation>
    </message>
    <message>
        <location line="-207"/>
        <location line="+208"/>
        <source>E</source>
        <translation></translation>
    </message>
    <message numerus="yes">
        <location line="+217"/>
        <source>Move %n Point(s)</source>
        <translation>
            <numerusform>Сдвинуть %n узел</numerusform>
            <numerusform>Сдвинуть %n узла</numerusform>
            <numerusform>Сдвинуть %n узлов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+26"/>
        <location line="+45"/>
        <source>Delete %n Node(s)</source>
        <translation>
            <numerusform>Удалить %n узел</numerusform>
            <numerusform>Удалить %n узла</numerusform>
            <numerusform>Удалить %n узлов</numerusform>
        </translation>
    </message>
    <message>
        <location line="-40"/>
        <location line="+215"/>
        <source>Join Nodes</source>
        <translation>Объединить узлы</translation>
    </message>
    <message>
        <location line="-214"/>
        <location line="+250"/>
        <source>Split Segments</source>
        <translation>Разделить сегменты</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="+149"/>
        <source>E</source>
        <translation></translation>
    </message>
    <message>
        <location line="+36"/>
        <source>New Terrain</source>
        <translation>Новый участок</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::Eraser</name>
    <message>
        <location filename="../src/tiled/eraser.cpp" line="+35"/>
        <location line="+36"/>
        <source>Eraser</source>
        <translation>Ластик</translation>
    </message>
    <message>
        <location line="-33"/>
        <location line="+34"/>
        <source>E</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.cpp" line="+63"/>
        <source>Export</source>
        <translation>Экспорт</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Export as Image</source>
        <translation>Экспортировать как изображение</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 already exists.
Do you want to replace it?</source>
        <translation>%1 уже существует.
Хотите перезаписать файл?</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Out of Memory</source>
        <translation>Недостаточно памяти</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not allocate sufficient memory for the image. Try reducing the zoom level or using a 64-bit version of Tiled.</source>
        <translation>Не удалось выделить память для изображения. Увеличьте масштаб или используйте 64-битную версию Tiled.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image too Big</source>
        <translation>Изображение слишком большое</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The resulting image would be %1 x %2 pixels and take %3 GB of memory. Tiled is unable to create such an image. Try reducing the zoom level.</source>
        <translation>Конечное изображение имеет размеры %1 x %2 пикселей и требует %3 GB памяти. Tiled не удалось создать изображение. Попробуйте увеличить масштаб.</translation>
    </message>
    <message>
        <location line="+88"/>
        <source>Image</source>
        <translation>Изображение</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileChangedWarning</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="-274"/>
        <source>File change detected. Discard changes and reload the map?</source>
        <translation>Обнаружены изменения файла. Отменить изменения и перезагрузить карту?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileEdit</name>
    <message>
        <location filename="../src/tiled/fileedit.cpp" line="+113"/>
        <source>Choose a File</source>
        <translation>Выберите файл</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ImageMovementTool</name>
    <message>
        <location filename="../src/tiled/imagemovementtool.cpp" line="+35"/>
        <location line="+65"/>
        <source>Move Images</source>
        <translation>Переместить изображение</translation>
    </message>
    <message>
        <location line="-63"/>
        <location line="+64"/>
        <source>M</source>
        <translation>M</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerDock</name>
    <message>
        <location filename="../src/tiled/layerdock.cpp" line="+218"/>
        <source>Layers</source>
        <translation>Слои</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Opacity:</source>
        <translation>Прозрачность:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerModel</name>
    <message>
        <location filename="../src/tiled/layermodel.cpp" line="+135"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location line="+97"/>
        <source>Show Other Layers</source>
        <translation>Показать другие слои</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Hide Other Layers</source>
        <translation>Спрятать другие слои</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MagicWandTool</name>
    <message>
        <location filename="../src/tiled/magicwandtool.cpp" line="+40"/>
        <location line="+52"/>
        <source>Magic Wand</source>
        <translation>Волшебная палочка</translation>
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
        <location filename="../src/tiled/mainwindow.cpp" line="+178"/>
        <location line="+8"/>
        <source>Undo</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location line="-7"/>
        <location line="+6"/>
        <source>Redo</source>
        <translation>Повторить</translation>
    </message>
    <message>
        <location line="+54"/>
        <source>Ctrl+T</source>
        <translation></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ctrl+=</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>+</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>-</source>
        <translation></translation>
    </message>
    <message>
        <location line="+23"/>
        <location line="+1406"/>
        <source>Random Mode</source>
        <translation>Случайный режим</translation>
    </message>
    <message>
        <location line="-1403"/>
        <source>D</source>
        <translation></translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+1401"/>
        <source>&amp;Layer</source>
        <translation>&amp;Слой</translation>
    </message>
    <message>
        <location line="-1223"/>
        <source>Ctrl+Shift+O</source>
        <translation></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Ctrl+Shift+Tab</source>
        <translation></translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Ctrl+Tab</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Y</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Z</source>
        <translation></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Shift+Z</source>
        <translation></translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Alt+C</source>
        <translation></translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Error Opening Map</source>
        <translation>Ошибка открытия карты</translation>
    </message>
    <message>
        <location line="+81"/>
        <location line="+218"/>
        <source>All Files (*)</source>
        <translation>Все файлы (*)</translation>
    </message>
    <message>
        <location line="-215"/>
        <location line="+68"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled map файлы (*.tmx)</translation>
    </message>
    <message>
        <location line="-51"/>
        <source>Open Map</source>
        <translation>Открыть карту</translation>
    </message>
    <message>
        <location line="+28"/>
        <location line="+88"/>
        <source>Error Saving Map</source>
        <translation>Ошибка сохранения карты</translation>
    </message>
    <message>
        <location line="-34"/>
        <source>untitled.tmx</source>
        <translation>безымянный.tmx</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Unsaved Changes</source>
        <translation>Не сохраненные изменения</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>There are unsaved changes. Do you want to save now?</source>
        <translation>Имеются не сохраненные изменения. Хотите сохранить сейчас?</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Exported to %1</source>
        <translation>Экспортировано в %1</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+134"/>
        <source>Error Exporting Map</source>
        <translation>Ошибка экспорта карты</translation>
    </message>
    <message>
        <location line="-86"/>
        <source>Export As...</source>
        <translation>Экспортировать как...</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Non-unique file extension</source>
        <translation>Не уникальное расширение файла</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Non-unique file extension.
Please select specific format.</source>
        <translation>Не уникальное расширение файла.
Пожалуйста, выберите соответствующий формат.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Unknown File Format</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The given filename does not have any known file extension.</source>
        <translation>Указанный файл имеет неизвестное расширение.</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Some export files already exist:</source>
        <translation>Некоторые экпортируемые файлы уже существуют:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Do you want to replace them?</source>
        <translation>Хотите заменить их?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Overwrite Files</source>
        <translation>Переписать файлы</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Cut</source>
        <translation></translation>
    </message>
    <message>
        <location line="+543"/>
        <source>[*]%1</source>
        <translation></translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Error Reloading Map</source>
        <translation>Ошибка загрузки карты</translation>
    </message>
    <message>
        <location line="-597"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiled tileset файлы (*.tsx)</translation>
    </message>
    <message>
        <location line="+15"/>
        <location line="+6"/>
        <source>Error Reading Tileset</source>
        <translation>Ошибка чтения набора тайлов</translation>
    </message>
    <message>
        <location line="+212"/>
        <source>Current layer: %1</source>
        <translation>Текущий слой: %1</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;none&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location line="-136"/>
        <source>Automatic Mapping Warning</source>
        <translation>Предупреждение авто-карты</translation>
    </message>
    <message>
        <location line="-923"/>
        <location line="+1229"/>
        <source>Views and Toolbars</source>
        <translation>Окна и панели</translation>
    </message>
    <message>
        <location line="-1228"/>
        <location line="+1229"/>
        <source>Tile Animation Editor</source>
        <translation>Редактор анимации</translation>
    </message>
    <message>
        <location line="-1227"/>
        <location line="+1228"/>
        <source>Tile Collision Editor</source>
        <translation>Редактор столкновений</translation>
    </message>
    <message>
        <location line="-1199"/>
        <source>Alt+Left</source>
        <translation>Alt+Left</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Alt+Right</source>
        <translation>Alt+Right</translation>
    </message>
    <message>
        <location line="+784"/>
        <source>Add External Tileset(s)</source>
        <translation>Добавить внешний набор тайлов</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>%1: %2</source>
        <translation>%1: %2</translation>
    </message>
    <message numerus="yes">
        <location line="+10"/>
        <source>Add %n Tileset(s)</source>
        <translation>
            <numerusform>Добавить %n набор тайлов</numerusform>
            <numerusform>Добавить %n набора тайлов</numerusform>
            <numerusform>Добавить %n наборов тайлов</numerusform>
        </translation>
    </message>
    <message>
        <location line="+54"/>
        <source>Automatic Mapping Error</source>
        <translation>Ошибка авто-карты</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocument</name>
    <message>
        <location filename="../src/tiled/mapdocument.cpp" line="+218"/>
        <source>untitled.tmx</source>
        <translation>безымянный.tmx</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>Resize Map</source>
        <translation>Изменить размер карты</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Offset Map</source>
        <translation>Смещение карты</translation>
    </message>
    <message numerus="yes">
        <location line="+28"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Повернуть %n объект</numerusform>
            <numerusform>Повернуть %n объекта</numerusform>
            <numerusform>Повернуть %n объектов</numerusform>
        </translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Tile Layer %1</source>
        <translation>Слой тайлов %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Object Layer %1</source>
        <translation>Слой объектов %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Image Layer %1</source>
        <translation>Слой изображений %1</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Copy of %1</source>
        <translation>Копия %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Duplicate Layer</source>
        <translation>Дублировать слой</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Merge Layer Down</source>
        <translation>Объединить слой с предыдущим</translation>
    </message>
    <message>
        <location line="+203"/>
        <source>Tile</source>
        <translation>Тайл</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Tileset Changes</source>
        <translation></translation>
    </message>
    <message numerus="yes">
        <location line="+189"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Дублировать %n объект</numerusform>
            <numerusform>Дублировать %n объекта</numerusform>
            <numerusform>Дублировать %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+20"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Удалить %n объект</numerusform>
            <numerusform>Удалить %n объекта</numerusform>
            <numerusform>Удалить %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+12"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Переместить %n объект на слой</numerusform>
            <numerusform>Переместить %n объекта на слой</numerusform>
            <numerusform>Переместить %n объектов на слой</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocumentActionHandler</name>
    <message>
        <location filename="../src/tiled/mapdocumentactionhandler.cpp" line="+55"/>
        <source>Ctrl+Shift+A</source>
        <translation></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Ctrl+Shift+D</source>
        <translation></translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Ctrl+Shift+Up</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+Down</source>
        <translation></translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Ctrl+Shift+H</source>
        <translation></translation>
    </message>
    <message>
        <location line="+58"/>
        <source>Select &amp;All</source>
        <translation>Выделить вс&amp;ё</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;None</source>
        <translation>&amp;Убрать выделение</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Crop to Selection</source>
        <translation>&amp;Обрезать по выделению</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add &amp;Tile Layer</source>
        <translation>Добавить &amp;слой тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add &amp;Object Layer</source>
        <translation>Добавить слой &amp;объектов</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Select Pre&amp;vious Layer</source>
        <translation>Выделить &amp;предыдущий слой</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;Next Layer</source>
        <translation>Выделить с&amp;ледующий слой</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R&amp;aise Layer</source>
        <translation>&amp;Поднять слой</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Lower Layer</source>
        <translation>&amp;Опустить слой</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show/&amp;Hide all Other Layers</source>
        <translation>Показать/&amp;спрятать другие слои</translation>
    </message>
    <message numerus="yes">
        <location line="+248"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>Дублировать %n объект</numerusform>
            <numerusform>Дублировать %n объекта</numerusform>
            <numerusform>Дублировать %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+1"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>Удалить %n объект</numerusform>
            <numerusform>Удалить %n объекта</numerusform>
            <numerusform>Удалить %n объектов</numerusform>
        </translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Duplicate Objects</source>
        <translation>Дублировать объекты</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Objects</source>
        <translation>Удалить объекты</translation>
    </message>
    <message>
        <location line="-259"/>
        <source>&amp;Duplicate Layer</source>
        <translation>&amp;Дублировать слой</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Ctrl+PgUp</source>
        <translation>Ctrl+PgUp</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgDown</source>
        <translation>Ctrl+PgDown</translation>
    </message>
    <message>
        <location line="+76"/>
        <source>Add &amp;Image Layer</source>
        <translation>Добавить слой &amp;изображений</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&amp;Merge Layer Down</source>
        <translation>Объединить слой с п&amp;редыдущим</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Layer</source>
        <translation>&amp;Удалить слой</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Layer &amp;Properties...</source>
        <translation>Настрой&amp;ки слоя...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapObjectModel</name>
    <message>
        <location filename="../src/tiled/mapobjectmodel.cpp" line="+152"/>
        <source>Change Object Name</source>
        <translation>Переименовать объект</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Change Object Type</source>
        <translation>Сменить тип объекта</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapsDock</name>
    <message>
        <location filename="../src/tiled/mapsdock.cpp" line="+83"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Choose the Maps Folder</source>
        <translation>Выберите папку с картами</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Maps</source>
        <translation>Карты</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MiniMapDock</name>
    <message>
        <location filename="../src/tiled/minimapdock.cpp" line="+60"/>
        <source>Mini-map</source>
        <translation>Мини-карта</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="-14"/>
        <source>Orthogonal</source>
        <translation>Ортогональная</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric</source>
        <translation>Изометрическая</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric (Staggered)</source>
        <translation>Изометрическая (смещенная)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Hexagonal (Staggered)</source>
        <translation>Гексагональная (смещенная)</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Tile Layer 1</source>
        <translation>Слой тайлов 1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Memory Usage Warning</source>
        <translation>Предупреждение о нехватке памяти</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile layers for this map will consume %L1 GB of memory each. Not creating one by default.</source>
        <translation>Слои тайлов для этой карты потребляют более чем 1GB памяти каждый. Не создано ни одного по умолчанию.</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>%1 x %2 pixels</source>
        <translation>%1 x %2 пикс</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="+151"/>
        <location line="+7"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Failed to load tileset image &apos;%1&apos;.</source>
        <translation>Не удалось загрузить изображение для набора тайлов: &apos;%1&apos;.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No tiles found in the tileset image when using the given tile size, margin and spacing!</source>
        <translation>Не удалось найти тайлы в изображении набора тайлов, соответствующие заданным параметрам!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Tileset Image</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectSelectionTool</name>
    <message>
        <location filename="../src/tiled/objectselectiontool.cpp" line="+316"/>
        <location line="+273"/>
        <source>Select Objects</source>
        <translation>Выбрать объекты</translation>
    </message>
    <message>
        <location line="-271"/>
        <location line="+272"/>
        <source>S</source>
        <translation></translation>
    </message>
    <message numerus="yes">
        <location line="-162"/>
        <location line="+513"/>
        <source>Move %n Object(s)</source>
        <translation>
            <numerusform>Переместить %n объект</numerusform>
            <numerusform>Переместить %n объекта</numerusform>
            <numerusform>Переместить %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+65"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>Повернуть %n объект</numerusform>
            <numerusform>Повернуть %n объекта</numerusform>
            <numerusform>Повернуть %n объектов</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+251"/>
        <source>Resize %n Object(s)</source>
        <translation>
            <numerusform>Масштабировать %n объект</numerusform>
            <numerusform>Масштабировать %n объекта</numerusform>
            <numerusform>Масштабировать %n объектов</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesModel</name>
    <message>
        <location filename="../src/tiled/objecttypesmodel.cpp" line="+51"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectsDock</name>
    <message>
        <location filename="../src/tiled/objectsdock.cpp" line="+145"/>
        <source>Object Properties</source>
        <translation>Параметры объекта</translation>
    </message>
    <message>
        <location line="-1"/>
        <source>Add Object Layer</source>
        <translation>Добавить слой объектов</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Objects</source>
        <translation>Объекты</translation>
    </message>
    <message numerus="yes">
        <location line="+17"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>Переместить %n объект на слой</numerusform>
            <numerusform>Переместить %n объекта на слой</numerusform>
            <numerusform>Переместить %n объектов на слой</numerusform>
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
&lt;h3&gt;Спасибо за поддержку!&lt;/h3&gt;
&lt;p&gt;Ваш вклад имеет большое значение для меня - главного разработчика и руководителя проекта Tiled. Это позволяет мне тратить меньше времени на работу за деньги и больше на развитие Tiled.&lt;/p&gt;
&lt;p&gt;Следите за обновлениями в ленте активности на моей Patreon странице, чтобы быть в курсе событий в развитии Tiled. Спасибо за Вашу поддержку!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m no longer a patron</source>
        <translation>Я больше не спонсор</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;With your help I can continue to improve Tiled!&lt;/h3&gt;
&lt;p&gt;Please consider supporting me as a patron. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;С вашей помощью я могу продолжать улучшение Tiled!&lt;/h3&gt;
&lt;p&gt;Пожалуйста, подумайте о спонсировании проекта. Ваша поддержка имеет большое значение для меня - главного разработчика и руководителя проекта Tiled. Это позволяет мне тратить меньше времени на работу за деньги и больше на развитие Tiled.&lt;/p&gt;
&lt;p&gt;Любая поддержка очень важна. Tiled пользуется много людей и если каждый внесёт свой небольшой вклад раз в месяц, у меня будет больше времени на развитие Tiled.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m already a patron!</source>
        <translation>Я уже спонсор!</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="+121"/>
        <location line="+60"/>
        <source>System default</source>
        <translation>Системные настройки</translation>
    </message>
    <message>
        <location line="+74"/>
        <source>Import Object Types</source>
        <translation>Импортировать типы объектов</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+29"/>
        <source>Object Types files (*.xml)</source>
        <translation>Файл типов объектов (*.xml)</translation>
    </message>
    <message>
        <location line="-16"/>
        <source>Error Reading Object Types</source>
        <translation>Ошибка чтения типов объектов</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Export Object Types</source>
        <translation>Экспортировать типы объектов</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Error Writing Object Types</source>
        <translation>Ошибка записи типов объектов</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertiesDock</name>
    <message>
        <location filename="../src/tiled/propertiesdock.cpp" line="+196"/>
        <location line="+52"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location line="-51"/>
        <location line="+103"/>
        <source>Add Property</source>
        <translation>Добавить параметр</translation>
    </message>
    <message>
        <location line="-50"/>
        <location line="+52"/>
        <source>Rename Property</source>
        <translation>Переименовать параметр</translation>
    </message>
    <message>
        <location line="-4"/>
        <source>Properties</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove Property</source>
        <translation>Удалить параметр</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertyBrowser</name>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+13"/>
        <source>Horizontal</source>
        <translation>Горизонталь</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertical</source>
        <translation>Вертикаль</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Top Down</source>
        <translation>Сверху снизу</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>Руководство</translation>
    </message>
    <message>
        <location line="+433"/>
        <source>Relative chance this tile will be picked</source>
        <translation>Относительный шанс выбора этого тайла</translation>
    </message>
    <message>
        <location line="+364"/>
        <source>Custom Properties</source>
        <translation>Свои параметры</translation>
    </message>
    <message>
        <location line="-544"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer Format</source>
        <translation>Формат слоя тайлов</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Render Order</source>
        <translation>Порядок отображения тайлов</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Background Color</source>
        <translation>Фоновый цвет</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Object</source>
        <translation>Объект</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+26"/>
        <location line="+56"/>
        <location line="+40"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location line="-119"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+21"/>
        <source>Visible</source>
        <translation>Видимость</translation>
    </message>
    <message>
        <location line="-372"/>
        <location line="+352"/>
        <location line="+67"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="-418"/>
        <location line="+352"/>
        <location line="+67"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="-417"/>
        <source>Odd</source>
        <translation>Нечетное</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Even</source>
        <translation>Четное</translation>
    </message>
    <message>
        <location line="+280"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+65"/>
        <source>Width</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location line="-64"/>
        <location line="+65"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location line="-64"/>
        <location line="+144"/>
        <source>Tile Width</source>
        <translation>Длина тайла</translation>
    </message>
    <message>
        <location line="-143"/>
        <location line="+144"/>
        <source>Tile Height</source>
        <translation>Высота тайла</translation>
    </message>
    <message>
        <location line="-142"/>
        <source>Tile Side Length (Hex)</source>
        <translation>Длина сторон тайла (Гекс)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Stagger Axis</source>
        <translation>Ориентация смещения</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Stagger Index</source>
        <translation>Индекс смещения</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Rotation</source>
        <translation>Вращение</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Flipping</source>
        <translation>Отражение</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Opacity</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Layer</source>
        <translation>Слой тайлов</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Object Layer</source>
        <translation>Слой объектов</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Drawing Order</source>
        <translation>Порядок отображения</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Image Layer</source>
        <translation>Слой изображений</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Image</source>
        <translation>Изображение</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Transparent Color</source>
        <translation>Цвет прозрачности</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tileset</source>
        <translation>Набор тайлов</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Drawing Offset</source>
        <translation>Смещение рисования</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Source Image</source>
        <translation>Исходное изображение</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin</source>
        <translation>Отступ</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spacing</source>
        <translation>Промежуток</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile</source>
        <translation>Тайл</translation>
    </message>
    <message>
        <location line="-107"/>
        <location line="+108"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Probability</source>
        <translation>Вероятность</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Terrain</source>
        <translation>Участок</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::SelectSameTileTool</name>
    <message>
        <location filename="../src/tiled/selectsametiletool.cpp" line="+50"/>
        <location line="+57"/>
        <source>Select Same Tile</source>
        <translation>Выбрать одинаковые тайлы</translation>
    </message>
    <message>
        <location line="-54"/>
        <location line="+55"/>
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
        <translation>Штамп</translation>
    </message>
    <message>
        <location line="-125"/>
        <location line="+126"/>
        <source>B</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainBrush</name>
    <message>
        <location filename="../src/tiled/terrainbrush.cpp" line="+45"/>
        <location line="+115"/>
        <source>Terrain Brush</source>
        <translation>Рисование участком</translation>
    </message>
    <message>
        <location line="-112"/>
        <location line="+113"/>
        <source>T</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainDock</name>
    <message>
        <location filename="../src/tiled/terraindock.cpp" line="+174"/>
        <source>Terrains</source>
        <translation>Участки</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainView</name>
    <message>
        <location filename="../src/tiled/terrainview.cpp" line="+97"/>
        <source>Terrain &amp;Properties...</source>
        <translation>&amp;Параметры участка...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="-56"/>
        <source>Delete Frames</source>
        <translation>Удалить рамки</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileCollisionEditor</name>
    <message>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+337"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cut</source>
        <translation>Вырезать</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Tile Collision Editor</source>
        <translation>Редактор столкновений</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileSelectionTool</name>
    <message>
        <location filename="../src/tiled/tileselectiontool.cpp" line="+34"/>
        <location line="+81"/>
        <source>Rectangular Select</source>
        <translation>Прямоугольное выделение</translation>
    </message>
    <message>
        <location line="-78"/>
        <location line="+79"/>
        <source>R</source>
        <translation></translation>
    </message>
    <message>
        <location line="-56"/>
        <source>%1, %2 - Rectangle: (%3 x %4)</source>
        <translation>%1, %2 - Прямоугольник: (%3 x %4)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampModel</name>
    <message>
        <location filename="../src/tiled/tilestampmodel.cpp" line="+78"/>
        <source>Stamp</source>
        <translation>Штамп</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Probability</source>
        <translation>Вероятность</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampsDock</name>
    <message>
        <location filename="../src/tiled/tilestampsdock.cpp" line="+196"/>
        <source>Delete Stamp</source>
        <translation>Удалить штамп</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Variation</source>
        <translation>Удалить вариант</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Choose the Stamps Folder</source>
        <translation>Выбрать папку штампов</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tile Stamps</source>
        <translation>Тайл-штампы</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add New Stamp</source>
        <translation>Создать новый штамп</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add Variation</source>
        <translation>Добавить вариант</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Duplicate Stamp</source>
        <translation>Дублировать штамп</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete Selected</source>
        <translation>Удалить выделенные</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Stamps Folder</source>
        <translation>Установить папку штампов</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Filter</source>
        <translation>Фильтр</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetDock</name>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="+652"/>
        <source>Remove Tileset</source>
        <translation>Удалить набор тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The tileset &quot;%1&quot; is still in use by the map!</source>
        <translation>Набор тайлов &quot;%1&quot; используется на карте!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove this tileset and all references to the tiles in this tileset?</source>
        <translation>Удалить набор тайлов и все ссылки на тайлы в данном наборе?</translation>
    </message>
    <message>
        <location line="+74"/>
        <source>Tilesets</source>
        <translation>Наборы тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>New Tileset</source>
        <translation>Новый набор тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Import Tileset</source>
        <translation>&amp;Импортировать набор тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Export Tileset As...</source>
        <translation>&amp;Экспортировать набор тайлов как...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile&amp;set Properties</source>
        <translation>&amp;Параметры набора тайлов</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Tileset</source>
        <translation>&amp;Удалить набор тайлов</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+103"/>
        <location line="+13"/>
        <source>Add Tiles</source>
        <translation>Добавить тайлы</translation>
    </message>
    <message>
        <location line="-115"/>
        <location line="+164"/>
        <location line="+13"/>
        <source>Remove Tiles</source>
        <translation>Удалить тайлы</translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Could not load &quot;%1&quot;!</source>
        <translation>Не загружено &quot;%1&quot;!</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>One or more of the tiles to be removed are still in use by the map!</source>
        <translation>Один или более тайлов для удаления используется на карте!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove all references to these tiles?</source>
        <translation>Удалить все ссылки на эти тайлы?</translation>
    </message>
    <message>
        <location line="-172"/>
        <source>Edit &amp;Terrain Information</source>
        <translation>Редактировать информа&amp;цию участка</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Export Tileset</source>
        <translation>Экспортировать набор тайлов</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiled tileset файлы (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetView</name>
    <message>
        <location filename="../src/tiled/tilesetview.cpp" line="+574"/>
        <source>Tile &amp;Properties...</source>
        <translation>Свойства &amp;тайла...</translation>
    </message>
    <message>
        <location line="-11"/>
        <source>Add Terrain Type</source>
        <translation>Добавить тип участка</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Set Terrain Image</source>
        <translation>Установить изображением участка</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Show &amp;Grid</source>
        <translation>Показать &amp;сетку</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::UndoDock</name>
    <message>
        <location filename="../src/tiled/undodock.cpp" line="+64"/>
        <source>History</source>
        <translation>История</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;empty&gt;</source>
        <translation>&lt;пусто&gt;</translation>
    </message>
</context>
<context>
    <name>Tmw::TmwPlugin</name>
    <message>
        <location filename="../src/plugins/tmw/tmwplugin.cpp" line="+47"/>
        <source>Multiple collision layers found!</source>
        <translation></translation>
    </message>
    <message>
        <location line="+9"/>
        <source>No collision layer found!</source>
        <translation></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Could not open file for writing.</source>
        <translation></translation>
    </message>
    <message>
        <location line="+30"/>
        <source>TMW-eAthena collision files (*.wlk)</source>
        <translation>TMW-eAthena collision файлы (*.wlk)</translation>
    </message>
</context>
<context>
    <name>TmxMapReader</name>
    <message>
        <location filename="../src/tiled/tmxmapreader.h" line="+56"/>
        <location filename="../src/tiled/tmxmapwriter.h" line="+56"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiled map файлы (*.tmx)</translation>
    </message>
</context>
<context>
    <name>TmxViewer</name>
    <message>
        <location filename="../src/tmxviewer/tmxviewer.cpp" line="+180"/>
        <source>TMX Viewer</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Undo Commands</name>
    <message>
        <location filename="../src/tiled/addremovelayer.h" line="+67"/>
        <source>Add Layer</source>
        <translation>Добавить слой</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Remove Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovemapobject.cpp" line="+76"/>
        <source>Add Object</source>
        <translation>Добавить объект</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove Object</source>
        <translation>Удалить объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetileset.cpp" line="+66"/>
        <source>Add Tileset</source>
        <translation>Добавить набор тайлов</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tileset</source>
        <translation>Удалить набор тайлов</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+36"/>
        <source>Change Object</source>
        <translation>Изменить объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeobjectgroupproperties.cpp" line="+39"/>
        <source>Change Object Layer Properties</source>
        <translation>Изменить свойства слоя объектов</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeproperties.cpp" line="+38"/>
        <source>Change %1 Properties</source>
        <translation>Изменить параметры %1</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Set Property</source>
        <translation>Указать параметр</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>Добавить параметр</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Remove Property</source>
        <translation>Удалить параметр</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Rename Property</source>
        <translation>Переименовать параметр</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeselectedarea.cpp" line="+31"/>
        <source>Change Selection</source>
        <translation>Изменить выделение</translation>
    </message>
    <message>
        <location filename="../src/tiled/erasetiles.cpp" line="+39"/>
        <source>Erase</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/tiled/filltiles.cpp" line="+37"/>
        <source>Fill Area</source>
        <translation>Залить область</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Move Object</source>
        <translation>Переместить объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobjecttogroup.cpp" line="+41"/>
        <source>Move Object to Layer</source>
        <translation>Переместить объект на слой</translation>
    </message>
    <message>
        <location filename="../src/tiled/movetileset.cpp" line="+31"/>
        <source>Move Tileset</source>
        <translation>Переместить набор тайлов</translation>
    </message>
    <message>
        <location filename="../src/tiled/offsetlayer.cpp" line="+42"/>
        <source>Offset Layer</source>
        <translation>Сместить слой</translation>
    </message>
    <message>
        <location filename="../src/tiled/painttilelayer.cpp" line="+49"/>
        <source>Paint</source>
        <translation>Рисование</translation>
    </message>
    <message>
        <location filename="../src/tiled/renamelayer.cpp" line="+40"/>
        <source>Rename Layer</source>
        <translation>Переименовать слой</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizetilelayer.cpp" line="+37"/>
        <source>Resize Layer</source>
        <translation>Изменить размер слоя</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemap.cpp" line="+32"/>
        <source>Resize Map</source>
        <translation>Изменить размер карты</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Resize Object</source>
        <translation>Изменить размер объекта</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="-704"/>
        <source>Import Tileset</source>
        <translation>Импортировать набор тайлов</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Export Tileset</source>
        <translation>Экспортировать набор тайлов</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetchanges.cpp" line="+35"/>
        <source>Change Tileset Name</source>
        <translation>Переименовать набор тайлов</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Change Drawing Offset</source>
        <translation>Изменить свещение отображения</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerproperties.cpp" line="+39"/>
        <source>Change Image Layer Properties</source>
        <translation>Изменить параметры слоя изображений</translation>
    </message>
    <message>
        <location filename="../src/tiled/movelayer.cpp" line="+37"/>
        <source>Lower Layer</source>
        <translation>Сместить слой ниже</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Layer</source>
        <translation>Сместить слой выше</translation>
    </message>
    <message>
        <location filename="../src/tiled/changepolygon.cpp" line="+40"/>
        <location line="+12"/>
        <source>Change Polygon</source>
        <translation>Изменить полигон</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremoveterrain.cpp" line="+69"/>
        <source>Add Terrain</source>
        <translation>Добавить участок</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Terrain</source>
        <translation>Удалить участок</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileterrain.cpp" line="+131"/>
        <source>Change Tile Terrain</source>
        <translation>Изменить участок тайла</translation>
    </message>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="-136"/>
        <source>Change Terrain Image</source>
        <translation>Изменить изображение участка</translation>
    </message>
    <message>
        <location filename="../src/tiled/changelayer.cpp" line="+41"/>
        <source>Show Layer</source>
        <translation>Показать слой</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Layer</source>
        <translation>Спрятать слой</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Change Layer Opacity</source>
        <translation>Изменить прозрачность слоя</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+31"/>
        <source>Show Object</source>
        <translation>Показать объект</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Object</source>
        <translation>Спрятать объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/renameterrain.cpp" line="+37"/>
        <source>Change Terrain Name</source>
        <translation>Переименовать участок</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetiles.cpp" line="+74"/>
        <source>Add Tiles</source>
        <translation>Добавить тайлы</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Tiles</source>
        <translation>Удалить тайлы</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerposition.cpp" line="+36"/>
        <source>Change Image Layer Position</source>
        <translation>Изменить позицию слоя изображений</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobjectsorder.cpp" line="+44"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+67"/>
        <source>Raise Object</source>
        <translation>Поднять объект</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+29"/>
        <source>Lower Object</source>
        <translation>Опустить объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileanimation.cpp" line="+33"/>
        <source>Change Tile Animation</source>
        <translation>Изменить анимацию тайла</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileobjectgroup.cpp" line="+15"/>
        <source>Change Tile Collision</source>
        <translation>Изменить столкновение тайла</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/tiled/flipmapobjects.cpp" line="+39"/>
        <source>Flip %n Object(s)</source>
        <translation>
            <numerusform>Перевернуть %n объект</numerusform>
            <numerusform>Перевернуть %n объекта</numerusform>
            <numerusform>Перевернуть %n объектов</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+43"/>
        <source>Raise Object To Top</source>
        <translation>На передний план</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Lower Object To Bottom</source>
        <translation>На задний план</translation>
    </message>
    <message>
        <location filename="../src/tiled/rotatemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Rotate Object</source>
        <translation>Вращать объект</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapproperty.cpp" line="+41"/>
        <source>Change Tile Width</source>
        <translation>Изменить длину тайла</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Tile Height</source>
        <translation>Изменить высоту тайла</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Hex Side Length</source>
        <translation>Изменить длину гекса</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Background Color</source>
        <translation>Изменить фоновый цвет</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Axis</source>
        <translation>Изменить ось смещения</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Index</source>
        <translation>Изменить индекс смещения</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Orientation</source>
        <translation>Изменить ориентацию</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Render Order</source>
        <translation>Изменить порядок отрисовки</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Layer Data Format</source>
        <translation>Изменить формат слоя</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileprobability.cpp" line="+38"/>
        <source>Change Tile Probability</source>
        <translation>Изменить вероятность тайла</translation>
    </message>
</context>
<context>
    <name>Utils</name>
    <message>
        <location filename="../src/tiled/utils.cpp" line="+34"/>
        <source>Image files</source>
        <translation>Графические файлы</translation>
    </message>
</context>
</TS>
