<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ja_JP">
<context>
    <name>AboutDialog</name>
    <message>
        <location filename="../src/tiled/aboutdialog.ui" line="+14"/>
        <source>About Tiled</source>
        <translation>Tiledについて</translation>
    </message>
    <message>
        <location line="+83"/>
        <source>Donate</source>
        <translation>寄付</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/tiled/aboutdialog.cpp" line="+36"/>
        <source>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled Map Editor&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;Copyright 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(see the AUTHORS file for a full list of contributors)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;You may modify and redistribute this program under the terms of the GPL (version 2 or later). A copy of the GPL is contained in the &apos;COPYING&apos; file distributed with Tiled.&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</source>
        <translation>&lt;p align=&quot;center&quot;&gt;&lt;font size=&quot;+2&quot;&gt;&lt;b&gt;Tiled マップエディタ&lt;/b&gt;&lt;/font&gt;&lt;br&gt;&lt;i&gt;Version %1&lt;/i&gt;&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;著作権 2008-2017 Thorbj&amp;oslash;rn Lindeijer&lt;br&gt;(このプロジェクトへの全貢献者の一覧については、AUTHORS ファイルを参照してください。)&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;このプログラムは GPL (バージョン 2 またはそれ以降のバージョン) の定める条件の下で改変または再頒布することができます。GPL のコピーは Tiled と一緒に配布された COPYING ファイルを参照してください。&lt;/p&gt;
&lt;p align=&quot;center&quot;&gt;&lt;a href=&quot;http://www.mapeditor.org/&quot;&gt;http://www.mapeditor.org/&lt;/a&gt;&lt;/p&gt;
</translation>
    </message>
</context>
<context>
    <name>AddPropertyDialog</name>
    <message>
        <location filename="../src/tiled/addpropertydialog.ui" line="+14"/>
        <source>Add Property</source>
        <translation>プロパティを追加</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Property name</source>
        <translation>プロパティ名</translation>
    </message>
</context>
<context>
    <name>Command line</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="+229"/>
        <source>Export syntax is --export-map [format] &lt;tmx file&gt; &lt;target file&gt;</source>
        <translation>エクスポートの文法は --export-map [フォーマット] &lt;.tmxファイル&gt; &lt;出力先&gt;</translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Format not recognized (see --export-formats)</source>
        <translation>認識できないフォーマットです (--export-formatsを確認してください)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Non-unique file extension. Can&apos;t determine correct export format.</source>
        <translation>拡張子がありません。出力フォーマットを指定してください.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>No exporter found for target file.</source>
        <translation>その出力フォーマットはサポートしていません.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Failed to load source map.</source>
        <translation>ソース・マップの読み込みに失敗.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Failed to export map to target file.</source>
        <translation>マップのエクスポートに失敗.</translation>
    </message>
</context>
<context>
    <name>CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.ui" line="+14"/>
        <source>Properties</source>
        <translation>プロパティ</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>&amp;Save map before executing</source>
        <translation>コマンド実行前に保存(&amp;S)</translation>
    </message>
</context>
<context>
    <name>CommandLineHandler</name>
    <message>
        <location filename="../src/tiled/main.cpp" line="-188"/>
        <source>Display the version</source>
        <translation>バージョンを表示</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Only check validity of arguments</source>
        <translation>正しい引数かチェックだけします</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Disable hardware accelerated rendering</source>
        <translation>レンダリングにハードウェア支援を使用しない</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Export the specified tmx file to target</source>
        <translation>指定したtmxファイルへエクスポート</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Print a list of supported export formats</source>
        <translation>エクスポート可能なフォーマットを表示</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Start a new instance, even if an instance is already running</source>
        <translation>インスタンスがすでに起動されている場合でも、新しくインスタンスを起動します</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Export formats:</source>
        <translation>エクスポート可能なフォーマット:</translation>
    </message>
</context>
<context>
    <name>CommandLineParser</name>
    <message>
        <location filename="../src/tiled/commandlineparser.cpp" line="+75"/>
        <source>Bad argument %1: lonely hyphen</source>
        <translation>引数エラー %1: ハイフンしかありません</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Unknown long argument %1: %2</source>
        <translation>不明な引数 %1: %2</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Unknown short argument %1.%2: %3</source>
        <translation>不明な引数 %1.%2: %3</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Usage:
  %1 [options] [files...]</source>
        <translation>使い方:
  %1 [オプション] [ファイルパス...]</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Options:</source>
        <translation>オプション:</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Display this help</source>
        <translation>このヘルプを表示</translation>
    </message>
</context>
<context>
    <name>ConverterDataModel</name>
    <message>
        <location filename="../src/automappingconverter/converterdatamodel.cpp" line="+75"/>
        <source>File</source>
        <translation>ファイル</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Version</source>
        <translation>バージョン</translation>
    </message>
</context>
<context>
    <name>ConverterWindow</name>
    <message>
        <location filename="../src/automappingconverter/converterwindow.cpp" line="+36"/>
        <source>Save all as %1</source>
        <translation>%1にすべて保存</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>All Files (*)</source>
        <translation>すべてのファイル (*)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiledマップファイル (*.tmx)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Open Map</source>
        <translation>マップを開く</translation>
    </message>
</context>
<context>
    <name>Csv::CsvPlugin</name>
    <message>
        <location filename="../src/plugins/csv/csvplugin.cpp" line="+55"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+75"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV ファイル (*.csv)</translation>
    </message>
</context>
<context>
    <name>Defold::DefoldPlugin</name>
    <message>
        <location filename="../src/plugins/defold/defoldplugin.cpp" line="+58"/>
        <source>Defold files (*.tilemap)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+69"/>
        <source>Could not open file for writing.</source>
        <translation type="unfinished">書き込み用ファイルを開けませんでした.</translation>
    </message>
</context>
<context>
    <name>Droidcraft::DroidcraftPlugin</name>
    <message>
        <location filename="../src/plugins/droidcraft/droidcraftplugin.cpp" line="+56"/>
        <source>This is not a valid Droidcraft map file!</source>
        <translation>これは正常なDroidcraftマップではありません!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>The map needs to have exactly one tile layer!</source>
        <translation>そのマップはタイルを１枚だけ持つ必要があります!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>The layer must have a size of 48 x 48 tiles!</source>
        <translation>レイヤーは48x48のタイルを持つ必要があります!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Droidcraft map files (*.dat)</source>
        <translation>Droidcraft マップファイル (*.dat)</translation>
    </message>
</context>
<context>
    <name>EditTerrainDialog</name>
    <message>
        <location filename="../src/tiled/editterraindialog.ui" line="+14"/>
        <source>Edit Terrain Information</source>
        <translation>地形の情報を変更</translation>
    </message>
    <message>
        <location line="+11"/>
        <location line="+3"/>
        <source>Undo</source>
        <translation>元に戻す</translation>
    </message>
    <message>
        <location line="+20"/>
        <location line="+3"/>
        <source>Redo</source>
        <translation>やり直す</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Erase</source>
        <translation>消去</translation>
    </message>
    <message>
        <location line="+89"/>
        <source>Add Terrain Type</source>
        <translation>地形の種類を追加</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add</source>
        <translation>追加</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Remove Terrain Type</source>
        <translation>地形の種類を削除</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove</source>
        <translation>削除</translation>
    </message>
</context>
<context>
    <name>ExportAsImageDialog</name>
    <message>
        <location filename="../src/tiled/exportasimagedialog.ui" line="+14"/>
        <source>Export As Image</source>
        <translation>画像でエクスポート</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Location</source>
        <translation>場所</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Name:</source>
        <translation>名前:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Browse...</source>
        <translation>参照(&amp;B)...</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Settings</source>
        <translation>設定</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Only include &amp;visible layers</source>
        <translation>表示レイヤーのみ(&amp;v)</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Use current &amp;zoom level</source>
        <translation>現在表示している拡大率(&amp;z)</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>&amp;Draw tile grid</source>
        <translation>タイルのグリッドを描画(&amp;D)</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Include background color</source>
        <translation>背景色を含む (&amp;I)</translation>
    </message>
</context>
<context>
    <name>Flare::FlarePlugin</name>
    <message>
        <location filename="../src/plugins/flare/flareplugin.cpp" line="+52"/>
        <source>Could not open file for reading.</source>
        <translation>読み込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+79"/>
        <source>Error loading tileset %1, which expands to %2. Path not found!</source>
        <translation>エラー %2のタイルセット%1読み込み中、パスが見つかりませんでした!</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>No tilesets section found before layer section.</source>
        <translation>レイヤーセクションの前に１つもタイルセットセクションが見つかりませんでした。</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Error mapping tile id %1.</source>
        <translation>エラー マッピングタイルID %1.</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>This seems to be no valid flare map. A Flare map consists of at least a header section, a tileset section and one tile layer.</source>
        <translation>正しいflareマップファイルではありません。Flareマップファイルは最低１つのヘッダセクション、タイルセットセクション、タイルレイヤーを含んでいます。</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Flare map files (*.txt)</source>
        <translation>Flare マップファイル (*.txt)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
</context>
<context>
    <name>Gmx::GmxPlugin</name>
    <message>
        <location filename="../src/plugins/gmx/gmxplugin.cpp" line="+82"/>
        <source>Could not open file for writing.</source>
        <translation type="unfinished">書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+143"/>
        <source>GameMaker room files (*.room.gmx)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Json::JsonMapFormat</name>
    <message>
        <location filename="../src/plugins/json/jsonplugin.cpp" line="+53"/>
        <source>Could not open file for reading.</source>
        <translation>読み込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error parsing file.</source>
        <translation>ファイルをパース中にエラーが発生.</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+39"/>
        <source>Error while writing file:
%1</source>
        <translation>ファイルへ書き込み中にエラー:
%1</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Json map files (*.json)</source>
        <translation>Json マップファイル (*.json)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>JavaScript map files (*.js)</source>
        <translation>JavaScript マップファイル (*.js)</translation>
    </message>
</context>
<context>
    <name>Json::JsonTilesetFormat</name>
    <message>
        <location line="+27"/>
        <source>Could not open file for reading.</source>
        <translation>読み込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Error parsing file.</source>
        <translation>ファイルをパース中にエラーが発生.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Error while writing file:
%1</source>
        <translation>ファイルへ書き込み中にエラー:
%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Json tileset files (*.json)</source>
        <translation>Json タイルセットファイル (*.json)</translation>
    </message>
</context>
<context>
    <name>Lua::LuaPlugin</name>
    <message>
        <location filename="../src/plugins/lua/luaplugin.cpp" line="+58"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Lua files (*.lua)</source>
        <translation>Lua ファイル (*.lua)</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../src/tiled/mainwindow.ui" line="+49"/>
        <source>&amp;File</source>
        <translation>ファイル(&amp;F)</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>&amp;Recent Files</source>
        <translation>最近使ったファイル(&amp;R)</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>&amp;Edit</source>
        <translation>編集(&amp;E)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Help</source>
        <translation>ヘルプ(&amp;H)</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Map</source>
        <translation>マップ(&amp;M)</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;View</source>
        <translation>表示(&amp;V)</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Show Object &amp;Names</source>
        <translation>オブジェクト名を表示 (&amp;N)</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>Main Toolbar</source>
        <translation>メインツールバー</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Tools</source>
        <translation>ツール</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Open...</source>
        <translation>開く(&amp;O)...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Save</source>
        <translation>保存(&amp;S)</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;Quit</source>
        <translation>終了(&amp;Q)</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>&amp;Copy</source>
        <translation>コピー(&amp;C)</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Paste</source>
        <translation>貼り付け(&amp;P)</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;About Tiled</source>
        <translation>Tiledについて(&amp;A)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>About Qt</source>
        <translation>Qtについて</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Resize Map...</source>
        <translation>マップのサイズを変更(&amp;R)...</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>AutoMap</source>
        <translation>自動マップ</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>A</source>
        <translation>A</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show &amp;Grid</source>
        <translation>グリッドの表示(&amp;G)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+G</source>
        <translation>Ctrl+G</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Save &amp;As...</source>
        <translation>ファイル名をつけて保存(&amp;A)...</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>&amp;New...</source>
        <translation>新しいファイル(&amp;N)...</translation>
    </message>
    <message>
        <location line="+193"/>
        <source>Become a Patron</source>
        <translation>パトロンになる</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Save All</source>
        <translation>すべて保存</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Documentation</source>
        <translation>ドキュメント</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Never</source>
        <translation>表示しない (&amp;N)</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>For &amp;Selected Objects</source>
        <translation>選択されたオブジェクトのみ (&amp;S)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>For &amp;All Objects</source>
        <translation>すべて (&amp;A)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>AutoMap While Drawing</source>
        <translation>描画中の自動マップ</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Paste &amp;in Place</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+124"/>
        <source>Ctrl+Shift+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Full Screen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>F11</source>
        <translation type="unfinished">F11</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap To &amp;Pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-88"/>
        <source>Ctrl+R</source>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Export</source>
        <translation>エクスポート (&amp;E)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+E</source>
        <translation>Ctrl+E</translation>
    </message>
    <message>
        <location line="-82"/>
        <source>&amp;Add External Tileset...</source>
        <translation>タイルセットを読み込み(&amp;A)...</translation>
    </message>
    <message>
        <location line="-50"/>
        <source>Export As &amp;Image...</source>
        <translation>画像でエクスポート(&amp;I)...</translation>
    </message>
    <message>
        <location line="+42"/>
        <source>E&amp;xport As...</source>
        <translation>名前をつけてエクスポート (&amp;x)...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+E</source>
        <translation>Ctrl+Shift+E</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Snap to Grid</source>
        <translation>吸着グリッド(&amp;S)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>C&amp;lose All</source>
        <translation>すべて閉じる(&amp;L)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+W</source>
        <translation>Ctrl+Shift+W</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>&amp;Delete</source>
        <translation>削除(&amp;D)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Delete</source>
        <translation>削除</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>&amp;Highlight Current Layer</source>
        <translation>現在のレイヤーをハイライト表示(&amp;H)</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>H</source>
        <translation>H</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Object &amp;Outlines</source>
        <translation>オブジェクトの境界線を表示 (&amp;O)</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Snap to &amp;Fine Grid</source>
        <translation>&amp;Fine Gridに吸着</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Show Tile Animations</source>
        <translation>タイルのアニメーションを表示</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Reload</source>
        <translation>再読み込み</translation>
    </message>
    <message>
        <location line="-168"/>
        <source>New &amp;Tileset...</source>
        <translation>新しいタイルセット(&amp;T)...</translation>
    </message>
    <message>
        <location line="-46"/>
        <source>Map &amp;Properties</source>
        <translation>マップのプロパティ (&amp;P)</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>&amp;Close</source>
        <translation>閉じる(&amp;C)</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom In</source>
        <translation>拡大</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Zoom Out</source>
        <translation>縮小</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Normal Size</source>
        <translation>拡大リセット</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+0</source>
        <translation>Ctrl+0</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Cu&amp;t</source>
        <translation>切り取り(&amp;t)</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&amp;Offset Map...</source>
        <translation>マップをずらす(&amp;O)...</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Offsets everything in a layer</source>
        <translation>対象となるレイヤーのすべてをずらします</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Pre&amp;ferences...</source>
        <translation>設定(&amp;f)...</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Clear Recent Files</source>
        <translation>最近使ったファイルをクリア</translation>
    </message>
    <message>
        <location filename="../src/automappingconverter/converterwindow.ui" line="+14"/>
        <source>Tiled Automapping Rule Files Converter</source>
        <translation>Tiled自動マップ・ルールファイル変換プログラム</translation>
    </message>
    <message>
        <location line="+25"/>
        <source>Add new Automapping rules</source>
        <translation>新しい自動マップのルールを追加</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+581"/>
        <source>All Files (*)</source>
        <translation>すべてのファイル (*)</translation>
    </message>
</context>
<context>
    <name>MapDocument</name>
    <message>
        <location filename="../src/tiled/adjusttileindexes.cpp" line="+178"/>
        <source>Tile</source>
        <translation type="unfinished">タイル</translation>
    </message>
</context>
<context>
    <name>MapReader</name>
    <message>
        <location filename="../src/libtiled/mapreader.cpp" line="+140"/>
        <source>Not a map file.</source>
        <translation>マップファイルではありません。</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Not a tileset file.</source>
        <translation>タイルセットファイルではありません。</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

行 %1, 位置 %2</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File not found: %1</source>
        <translation>ファイルは見つかりませんでした: %1</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Unable to read file: %1</source>
        <translation>読み込めませんでした: %1</translation>
    </message>
    <message>
        <location line="+32"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+59"/>
        <source>Unsupported map orientation: &quot;%1&quot;</source>
        <translation>マップの回転がサポートされていないものです: &quot;%1&quot;</translation>
    </message>
    <message>
        <location line="+102"/>
        <location line="+21"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+138"/>
        <source>Invalid tileset parameters for tileset &apos;%1&apos;</source>
        <translation>タイルセットのパラメータが無効です &apos;%1&apos;</translation>
    </message>
    <message>
        <location line="+43"/>
        <source>Invalid tile ID: %1</source>
        <translation>タイルのIDが無効です: %1</translation>
    </message>
    <message>
        <location line="+228"/>
        <source>Too many &lt;tile&gt; elements</source>
        <translation>&lt;tile&gt; 要素が多すぎます</translation>
    </message>
    <message>
        <location line="+44"/>
        <location line="+43"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+219"/>
        <source>Invalid tile: %1</source>
        <translation>無効なタイル: %1</translation>
    </message>
    <message>
        <location line="+29"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+34"/>
        <source>Invalid draw order: %1</source>
        <translation>描画順がおかしいです: %1</translation>
    </message>
    <message>
        <location line="+154"/>
        <source>Invalid points data for polygon</source>
        <translation>無効なポリゴンのポイントデータ</translation>
    </message>
    <message>
        <location line="-285"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-90"/>
        <source>Unknown encoding: %1</source>
        <translation>エンコーディングがよく分かりません: %1</translation>
    </message>
    <message>
        <location line="-181"/>
        <source>Error reading embedded image for tile %1</source>
        <translation>タイル %1 の埋め込み画像を読込中にエラー</translation>
    </message>
    <message>
        <location line="+176"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-4"/>
        <source>Compression method &apos;%1&apos; not supported</source>
        <translation>&apos;%1&apos;という圧縮方法には対応していません</translation>
    </message>
    <message>
        <location line="+58"/>
        <location line="+19"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+15"/>
        <location line="+39"/>
        <source>Corrupt layer data for layer &apos;%1&apos;</source>
        <translation>レイヤー &apos;%1&apos; のデータが壊れています</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-28"/>
        <source>Unable to parse tile at (%1,%2) on layer &apos;%3&apos;</source>
        <translation>レイヤー &apos;%3&apos;の(%1,%2)のタイルをパースできません</translation>
    </message>
    <message>
        <location line="-28"/>
        <location line="+44"/>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="+31"/>
        <source>Tile used but no tilesets specified</source>
        <translation>タイルセットが指定されていません</translation>
    </message>
    <message>
        <location filename="../src/libtiled/mapwriter.cpp" line="+113"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location filename="../src/libtiled/varianttomapconverter.cpp" line="-170"/>
        <source>Invalid (negative) tile id: %1</source>
        <translation>タイルIDが不正です (負の値): %1</translation>
    </message>
</context>
<context>
    <name>NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.ui" line="+14"/>
        <source>New Map</source>
        <translation>新しいマップ</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Map size</source>
        <translation>マップの大きさ</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+68"/>
        <source>Width:</source>
        <translation>幅:</translation>
    </message>
    <message>
        <location line="-58"/>
        <location line="+26"/>
        <source> tiles</source>
        <extracomment>Remember starting with a space.</extracomment>
        <translation> タイル</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+68"/>
        <source>Height:</source>
        <translation>高さ:</translation>
    </message>
    <message>
        <location line="-32"/>
        <source>Tile size</source>
        <translation>タイルの大きさ</translation>
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
        <translation>マップ</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Orientation:</source>
        <translation>種類:</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Tile layer format:</source>
        <translation>タイルの出力形式:</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tile render order:</source>
        <translation>タイルの描画順序:</translation>
    </message>
</context>
<context>
    <name>NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.ui" line="+14"/>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="+235"/>
        <source>New Tileset</source>
        <translation>新しいタイルセット</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tileset</source>
        <translation>タイルセット</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Based on Tileset Image</source>
        <translation>均等にタイルが並んだ画像を使用</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Collection of Images</source>
        <translation>バラバラの大きさの画像をタイルとして使用</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Type:</source>
        <translation>種類:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Name:</source>
        <translation>名前(&amp;N):</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>&amp;Browse...</source>
        <translation>参照(&amp;B)...</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Use transparent color:</source>
        <translation>透過色を設定する:</translation>
    </message>
    <message>
        <location line="+129"/>
        <source>Tile width:</source>
        <translation>タイルの幅:</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Pick color from image</source>
        <translation type="unfinished"></translation>
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
        <translation>画像</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Source:</source>
        <translation>パス:</translation>
    </message>
    <message>
        <location line="+94"/>
        <source>The space at the edges of the tileset.</source>
        <translation>タイルセット画像の一番端（枠）の余白.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Margin:</source>
        <translation>上左の余白:</translation>
    </message>
    <message>
        <location line="-45"/>
        <source>Tile height:</source>
        <translation>タイルの高さ:</translation>
    </message>
    <message>
        <location line="+91"/>
        <source>The space between the tiles.</source>
        <translation>タイル間のスペース.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Spacing:</source>
        <translation>タイル間の余白:</translation>
    </message>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-2"/>
        <source>Edit Tileset</source>
        <translation>タイルセットを編集</translation>
    </message>
</context>
<context>
    <name>ObjectTypes</name>
    <message>
        <location filename="../src/tiled/objecttypes.cpp" line="+43"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Could not open file.</source>
        <translation>ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>File doesn&apos;t contain object types.</source>
        <translation>オブジェクト・タイプがファイルに含まれていない.</translation>
    </message>
    <message>
        <location line="+27"/>
        <source>%3

Line %1, column %2</source>
        <translation>%3

行 %1, 位置 %2</translation>
    </message>
</context>
<context>
    <name>ObjectTypesEditor</name>
    <message>
        <location filename="../src/tiled/objecttypeseditor.ui" line="+14"/>
        <source>Object Types Editor</source>
        <translation>オブジェクト・タイプ・エディター</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>File</source>
        <translation>ファイル</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Export Object Types...</source>
        <translation>オブジェクト・タイプをエクスポート...</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Import Object Types...</source>
        <translation>オブジェクト・タイプをインポート...</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Choose Object Types File...</source>
        <translation>オブジェクト・タイプ・ファイルを選択...</translation>
    </message>
</context>
<context>
    <name>OffsetMapDialog</name>
    <message>
        <location filename="../src/tiled/offsetmapdialog.ui" line="+17"/>
        <source>Offset Map</source>
        <translation>マップのオフセット</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Offset Contents of Map</source>
        <translation>マップのずらし方</translation>
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
        <translation type="unfinished"> タイル</translation>
    </message>
    <message>
        <location line="-30"/>
        <location line="+46"/>
        <source>Wrap</source>
        <translation>右端を左へ回転</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Layers:</source>
        <translation>対象レイヤー:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>All Visible Layers</source>
        <translation>表示中のすべてのレイヤー</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>All Layers</source>
        <translation>すべてのレイヤー</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Selected Layer</source>
        <translation>選択されたレイヤー</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bounds:</source>
        <translation>範囲:</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Whole Map</source>
        <translation>マップ全体</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Current Selection</source>
        <translation>選択部分のみ</translation>
    </message>
</context>
<context>
    <name>PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.ui" line="+14"/>
        <source>Become a Patron</source>
        <translation>パトロンになろう</translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Visit https://www.patreon.com/bjorn</source>
        <translation>https://www.patreon.com/bjorn へアクセス</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>I&apos;m already a patron!</source>
        <translation>私は既にパトロンです!</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Maybe later</source>
        <translation>また今度</translation>
    </message>
</context>
<context>
    <name>PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+14"/>
        <source>Preferences</source>
        <translation>設定</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>General</source>
        <translation>一般</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Saving and Loading</source>
        <translation>保存と読み込み</translation>
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
        <translation>Base64 (未圧縮)</translation>
    </message>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (gzip compressed)</source>
        <translation>Base64 (gzip圧縮)</translation>
    </message>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Base64 (zlib compressed)</source>
        <translation>Base64 (zlib圧縮)</translation>
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
        <translation>左から右、上から下</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Right Up</source>
        <translation>左から右、下から上</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Down</source>
        <translation>右から左、上から下</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Left Up</source>
        <translation>右から左、下から上</translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+6"/>
        <source>&amp;Reload tileset images when they change</source>
        <translation>タイルセットの画像が変更された時、再読み込みする(&amp;R)</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Not enabled by default since a reference to an external DTD is known to cause problems with some XML parsers.</source>
        <translation>エクスターナルDTDはXMLパーサーで問題が起こる可能性があるため、デフォルトではオフになっています。</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Include &amp;DTD reference in saved maps</source>
        <translation>マップを保存するときDTDリファレンスを入れる(&amp;D)</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Interface</source>
        <translation>インターフェース</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>&amp;Language:</source>
        <translation>表示言語(&amp;L):</translation>
    </message>
    <message>
        <location line="-7"/>
        <source>Hardware &amp;accelerated drawing (OpenGL)</source>
        <translation>OpenGLで高速描画 (&amp;A)</translation>
    </message>
    <message>
        <location line="-19"/>
        <source>Open last files on startup</source>
        <translation>最後に使ったファイルを起動時に開く</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Grid color:</source>
        <translation>グリッド色:</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Fine grid divisions:</source>
        <translation>Fine Gridの分割数
 (1だとグリッド毎、4だとグリッドを４分割した部分に吸着):</translation>
    </message>
    <message>
        <location line="+20"/>
        <source> pixels</source>
        <translation>ピクセル</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Object line width:</source>
        <translation>オブジェクトを描画する際の線幅:</translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Theme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="+67"/>
        <location line="+122"/>
        <source>Native</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Fusion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-121"/>
        <location line="+122"/>
        <source>Tiled Fusion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/tiled/preferencesdialog.ui" line="+22"/>
        <source>Selection color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Style:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Base color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+27"/>
        <location line="+6"/>
        <source>Updates</source>
        <translation>ソフトの更新</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Check Now</source>
        <translation>今すぐ更新</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Automatically check for updates</source>
        <translation>自動的に新しいバージョンへ更新する</translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Plugins</source>
        <translation>プラグイン</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Enabled Plugins</source>
        <translation>有効なプラグイン</translation>
    </message>
</context>
<context>
    <name>Python::PythonMapFormat</name>
    <message>
        <location filename="../src/plugins/python/pythonplugin.cpp" line="+268"/>
        <source>-- Using script %1 to read %2</source>
        <translation>-- スクリプト %1 で %2 を読み込み</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>-- Using script %1 to write %2</source>
        <translation>-- スクリプト %1 で %2 を書き込み</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Uncaught exception in script. Please check console.</source>
        <translation>不明な例外がスクリプトで発生しました。コンソールをチェックしてください.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Script returned false. Please check console.</source>
        <translation>スクリプトがfalseを返しました。コンソールをチェックしてください.</translation>
    </message>
</context>
<context>
    <name>Python::PythonPlugin</name>
    <message>
        <location line="-164"/>
        <source>Reloading Python scripts</source>
        <translation>Pythonスクリプトを再読込中</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/automappingconverter/convertercontrol.h" line="+33"/>
        <source>v0.8 and before</source>
        <translation>v0.8以前</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>v0.9 and later</source>
        <translation>v0.9以降</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>unknown</source>
        <translation>不明</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>not a map</source>
        <translation>マップでは無い</translation>
    </message>
</context>
<context>
    <name>QtBoolEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+237"/>
        <location line="+10"/>
        <location line="+25"/>
        <source>True</source>
        <translation>True</translation>
    </message>
    <message>
        <location line="-25"/>
        <location line="+25"/>
        <source>False</source>
        <translation>False</translation>
    </message>
</context>
<context>
    <name>QtBoolPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="+1703"/>
        <source>True</source>
        <translation>True</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>False</source>
        <translation>False</translation>
    </message>
</context>
<context>
    <name>QtCharEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qteditorfactory.cpp" line="+1712"/>
        <source>Clear Char</source>
        <translation>文字をクリア</translation>
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
        <translation>赤</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Green</source>
        <translation>緑</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Blue</source>
        <translation>青</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Alpha</source>
        <translation>透明度</translation>
    </message>
</context>
<context>
    <name>QtCursorDatabase</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="-214"/>
        <source>Arrow</source>
        <translation>矢印</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Up Arrow</source>
        <translation>上向き矢印</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Cross</source>
        <translation>十字架</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Wait</source>
        <translation>待機</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>IBeam</source>
        <translation>I ビーム</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Vertical</source>
        <translation>サイズ変更(縦方向)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Horizontal</source>
        <translation>サイズ変更(横方向)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Backslash</source>
        <translation>サイズ変更(バックスラッシュ)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size Slash</source>
        <translation>サイズ変更(スラッシュ)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size All</source>
        <translation>サイズ変更(4方向)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Blank</source>
        <translation>なし</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Vertical</source>
        <translation>分割(縦方向)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Split Horizontal</source>
        <translation>分割(横方向)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Pointing Hand</source>
        <translation>ハンド(指差し)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Forbidden</source>
        <translation>禁止</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Open Hand</source>
        <translation>ハンド(オープン)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Closed Hand</source>
        <translation>ハンド(クローズ)</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>What&apos;s This</source>
        <translation>ヒント</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Busy</source>
        <translation>ビジー</translation>
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
        <translation>フォントを選択</translation>
    </message>
</context>
<context>
    <name>QtFontPropertyManager</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertymanager.cpp" line="-362"/>
        <source>Family</source>
        <translation>ファミリー</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Point Size</source>
        <translation>ポイント数</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Bold</source>
        <translation>太字</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Italic</source>
        <translation>斜体</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Underline</source>
        <translation>下線</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Strikeout</source>
        <translation>打ち消し線</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Kerning</source>
        <translation>文字間隔</translation>
    </message>
</context>
<context>
    <name>QtKeySequenceEdit</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qtpropertybrowserutils.cpp" line="+238"/>
        <source>Clear Shortcut</source>
        <translation>ショートカットをクリア</translation>
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
        <translation>言語</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Country</source>
        <translation>国</translation>
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
        <translation>未設定</translation>
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
        <translation>幅</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>高さ</translation>
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
        <translation>幅</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>高さ</translation>
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
        <translation>幅</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Height</source>
        <translation>高さ</translation>
    </message>
</context>
<context>
    <name>QtSizePolicyPropertyManager</name>
    <message>
        <location line="+1704"/>
        <location line="+1"/>
        <source>&lt;Invalid&gt;</source>
        <translation>&lt;無効&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>[%1, %2, %3, %4]</source>
        <translation>[%1, %2, %3, %4]</translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Horizontal Policy</source>
        <translation>横方向のポリシー</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Vertical Policy</source>
        <translation>縦方向のポリシー</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Horizontal Stretch</source>
        <translation>横方向のストレッチ</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Vertical Stretch</source>
        <translation>縦方向のストレッチ</translation>
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
        <translation>幅</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Height</source>
        <translation>高さ</translation>
    </message>
</context>
<context>
    <name>QtTreePropertyBrowser</name>
    <message>
        <location filename="../src/qtpropertybrowser/src/qttreepropertybrowser.cpp" line="+478"/>
        <source>Property</source>
        <translation>プロパティ</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Value</source>
        <translation>値</translation>
    </message>
</context>
<context>
    <name>ReplicaIsland::ReplicaIslandPlugin</name>
    <message>
        <location filename="../src/plugins/replicaisland/replicaislandplugin.cpp" line="+58"/>
        <source>Cannot open Replica Island map file!</source>
        <translation>Replica Islandマップファイルが開けませんでした!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Can&apos;t parse file header!</source>
        <translation>ファイルヘッダを読み込めませんでした!</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Can&apos;t parse layer header!</source>
        <translation>レイヤーヘッダが解析できませんでした!</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Inconsistent layer sizes!</source>
        <translation>レイヤーの大きさが矛盾しています!</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File ended in middle of layer!</source>
        <translation>レイヤーの途中でファイルが終わっています!</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Unexpected data at end of file!</source>
        <translation>想定外のファイルの終わりです!</translation>
    </message>
    <message>
        <location line="+64"/>
        <source>Replica Island map files (*.bin)</source>
        <translation>Replica Islandマップファイル (*.bin)</translation>
    </message>
    <message>
        <location line="+32"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>You must define a background_index property on the map!</source>
        <translation>background_indexプロパティをマップの設定する必要があります!</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Can&apos;t save non-tile layer!</source>
        <translation>非タイルレイヤーは保存できません!</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>You must define a type property on each layer!</source>
        <translation>typeプロパティをレイヤーごとに設定する必要があります!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a tile_index property on each layer!</source>
        <translation>tile_indexプロパティをレイヤーごとに設定する必要があります!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You must define a scroll_speed property on each layer!</source>
        <translation>scroll_speedプロパティをレイヤーごとに設定する必要があります!</translation>
    </message>
</context>
<context>
    <name>ResizeDialog</name>
    <message>
        <location filename="../src/tiled/resizedialog.ui" line="+14"/>
        <source>Resize</source>
        <translation>リサイズ</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Size</source>
        <translation>大きさ</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+33"/>
        <location line="+32"/>
        <location line="+23"/>
        <source> tiles</source>
        <translation type="unfinished"> タイル</translation>
    </message>
    <message>
        <location line="-75"/>
        <source>Width:</source>
        <translation>幅:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Height:</source>
        <translation>高さ:</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>Offset</source>
        <translation>オフセット（ずらす）</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>X:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+23"/>
        <source>Y:</source>
        <translation></translation>
    </message>
    <message>
        <location line="+47"/>
        <source>Remove objects outside of the map</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tengine::TenginePlugin</name>
    <message>
        <location filename="../src/plugins/tengine/tengineplugin.cpp" line="+49"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+246"/>
        <source>T-Engine4 map files (*.lua)</source>
        <translation>Tエンジン4 マップファイル (*.lue)</translation>
    </message>
</context>
<context>
    <name>TextEditorDialog</name>
    <message>
        <location filename="../src/tiled/texteditordialog.ui" line="+14"/>
        <source>Edit Text</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.ui" line="+14"/>
        <source>Tile Animation Editor</source>
        <translation>タイル・アニメーション エディター</translation>
    </message>
    <message>
        <location line="+99"/>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="+523"/>
        <source>Preview</source>
        <translation>プレビュー</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractObjectTool</name>
    <message>
        <location filename="../src/tiled/abstractobjecttool.cpp" line="+167"/>
        <location line="+70"/>
        <source>Reset Tile Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location line="-13"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを複製</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+2"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを削除</numerusform>
        </translation>
    </message>
    <message>
        <location line="+18"/>
        <source>Flip Horizontally</source>
        <translation>横反転</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Flip Vertically</source>
        <translation>縦反転</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Raise Object</source>
        <translation>オブジェクトを１つ前面へ</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgUp</source>
        <translation>PgUp</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object</source>
        <translation>オブジェクトを１つ背面へ</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>PgDown</source>
        <translation>PgDown</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Object to Top</source>
        <translation>オブジェクトを最前面へ</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Home</source>
        <translation>Home</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Lower Object to Bottom</source>
        <translation>オブジェクトを最背面へ</translation>
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
            <numerusform>%nつのオブジェクトを別レイヤーへ移動</numerusform>
        </translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Object &amp;Properties...</source>
        <translation>オブジェクトのプロパティ(&amp;P)...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AbstractTileTool</name>
    <message>
        <location filename="../src/tiled/abstracttiletool.cpp" line="+124"/>
        <source>empty</source>
        <translation>空</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutoMapper</name>
    <message>
        <location filename="../src/tiled/automapper.cpp" line="+115"/>
        <source>&apos;%1&apos;: Property &apos;%2&apos; = &apos;%3&apos; does not make sense. Ignoring this property.</source>
        <translation>&apos;%1&apos;: プロパティ &apos;%2&apos; = &apos;%3&apos; は無効です。このプロパティは無視されました.</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Did you forget an underscore in layer &apos;%1&apos;?</source>
        <translation>レイヤー &apos;%1&apos; のアンダースコアを忘れていませんか?</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>Layer &apos;%1&apos; is not recognized as a valid layer for Automapping.</source>
        <translation>レイヤー &apos;%1&apos;は自動マップ用のレイヤーとして認識できません.</translation>
    </message>
    <message>
        <location line="-105"/>
        <source>&apos;regions_input&apos; layer must not occur more than once.</source>
        <translation>&apos;regions_input&apos;レイヤーは１つも存在してはいけません.</translation>
    </message>
    <message>
        <location line="+6"/>
        <location line="+13"/>
        <source>&apos;regions_*&apos; layers must be tile layers.</source>
        <translation>&apos;regions_*&apos;レイヤーはタイル・レイヤーである必要があります.</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>&apos;regions_output&apos; layer must not occur more than once.</source>
        <translation>&apos;regions_output&apos;レイヤーは１つも存在してはいけません.</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>&apos;input_*&apos; and &apos;inputnot_*&apos; layers must be tile layers.</source>
        <translation>&apos;input_*&apos;と&apos;inputnot_*&apos;レイヤーはタイル・レイヤーである必要があります.</translation>
    </message>
    <message>
        <location line="+56"/>
        <source>No &apos;regions&apos; or &apos;regions_input&apos; layer found.</source>
        <translation>&apos;regions&apos;、または&apos;regions_input&apos;レイヤーが見つかりません.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No &apos;regions&apos; or &apos;regions_output&apos; layer found.</source>
        <translation>&apos;regions&apos;、または&apos;regions_output&apos;レイヤーが見つかりません.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>No input_&lt;name&gt; layer found!</source>
        <translation>input_&lt;name&gt;レイヤーが見つかりません!</translation>
    </message>
    <message>
        <location line="+165"/>
        <source>Tile</source>
        <translation>タイル</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::AutomappingManager</name>
    <message>
        <location filename="../src/tiled/automappingmanager.cpp" line="+103"/>
        <source>Apply AutoMap rules</source>
        <translation>AutoMap rulesを適用</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>No rules file found at:
%1</source>
        <translation>rulesファイルが見つかりません:
%1</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Error opening rules file:
%1</source>
        <translation>rulesファイルが開けませんでした:
%1</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>File not found:
%1</source>
        <translation>ファイルが見つかりません:
%1</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Opening rules map failed:
%1</source>
        <translation>rules mapを開くことができませんでした:
%1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksModel</name>
    <message>
        <location filename="../src/tiled/brokenlinks.cpp" line="+144"/>
        <source>Tileset image</source>
        <translation>タイルセットの画像</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tileset</source>
        <translation>タイルセット</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Tile image</source>
        <translation>タイル画像</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>File name</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Location</source>
        <translation>場所</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>種類</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BrokenLinksWidget</name>
    <message>
        <location line="+66"/>
        <source>Some files could not be found</source>
        <translation>いくつかのファイルが見つかりませんでした</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>One or more files referenced by the map could not be found. You can help locate them below.</source>
        <translation>マップで参照されるいくつかのファイルが見つかりませんでした。場所を指定してください.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Locate File...</source>
        <translation>ファイルの場所...</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Locate File</source>
        <translation>ファイルの場所</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Loading Image</source>
        <translation>画像を読込中にエラー</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>All Files (*)</source>
        <translation>すべてのファイル (*)</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Locate External Tileset</source>
        <translation>タイルセットの場所</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Error Reading Tileset</source>
        <translation>タイルセットを読み込み中にエラー</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::BucketFillTool</name>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="+40"/>
        <location line="+194"/>
        <source>Bucket Fill Tool</source>
        <translation>塗りつぶし</translation>
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
        <translation>オブジェクトを貼りつけ</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandButton</name>
    <message>
        <location filename="../src/tiled/commandbutton.cpp" line="+130"/>
        <source>Execute Command</source>
        <translation>コマンドを実行</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>F5</source>
        <translation>F5</translation>
    </message>
    <message>
        <location line="-67"/>
        <source>Error Executing Command</source>
        <translation>コマンド実行中にエラー発生</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>You do not have any commands setup.</source>
        <translation>コマンドが１つも登録されていません.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Edit commands...</source>
        <translation>コマンドを編集...</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Edit Commands...</source>
        <translation>コマンドを編集...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDataModel</name>
    <message>
        <location filename="../src/tiled/commanddatamodel.cpp" line="+60"/>
        <source>Open in text editor</source>
        <translation>テキストエディタで開く</translation>
    </message>
    <message>
        <location line="+91"/>
        <location line="+69"/>
        <source>&lt;new command&gt;</source>
        <translation>&lt;コマンドを追加&gt;</translation>
    </message>
    <message>
        <location line="-61"/>
        <source>Set a name for this command</source>
        <translation>このコマンドの名前を設定</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Set the shell command to execute</source>
        <translation>実行するシェルコマンドを設定</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Show or hide this command in the command list</source>
        <translation>コマンドを有効、無効化します</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Add a new command</source>
        <translation>コマンドを追加</translation>
    </message>
    <message>
        <location line="+107"/>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Command</source>
        <translation>コマンド</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Enable</source>
        <translation>有効化</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Move Up</source>
        <translation>上に移動</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Move Down</source>
        <translation>下に移動</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Execute</source>
        <translation>実行</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Execute in Terminal</source>
        <translation>ターミナルで実行</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Delete</source>
        <translation>削除</translation>
    </message>
    <message>
        <location line="+84"/>
        <source>%1 (copy)</source>
        <translation>%1 (コピー)</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>New command</source>
        <translation>新しいコマンド</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandDialog</name>
    <message>
        <location filename="../src/tiled/commanddialog.cpp" line="+44"/>
        <source>Edit Commands</source>
        <translation>コマンドの編集</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CommandProcess</name>
    <message>
        <location filename="../src/tiled/command.cpp" line="+144"/>
        <source>Unable to create/open %1</source>
        <translation>%1の作成/オープンに失敗</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Unable to add executable permissions to %1</source>
        <translation>%1に実行権限を付与できなかった</translation>
    </message>
    <message>
        <location line="+26"/>
        <source>The command failed to start.</source>
        <translation>コマンドのスタートに失敗しました.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command crashed.</source>
        <translation>コマンドはクラッシュしました.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The command timed out.</source>
        <translation>コマンドはタイムアウトしました.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>An unknown error occurred.</source>
        <translation>不明なエラー.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Error Executing %1</source>
        <translation>%1 を実行中にエラー</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ConsoleDock</name>
    <message>
        <location filename="../src/tiled/consoledock.cpp" line="+36"/>
        <source>Debug Console</source>
        <translation>デバッグコンソール</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::CreateEllipseObjectTool</name>
    <message>
        <location filename="../src/tiled/createellipseobjecttool.cpp" line="+39"/>
        <source>Insert Ellipse</source>
        <translation>楕円形を追加</translation>
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
        <translation>ポリゴンを追加</translation>
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
        <translation>ポリラインを追加</translation>
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
        <translation>四角形を追加</translation>
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
        <translation>タイルを追加</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Open Containing Folder...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+141"/>
        <source>Tileset Columns Changed</source>
        <translation>タイルセットの列数が変更されました</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The number of tile columns in the tileset &apos;%1&apos; appears to have changed from %2 to %3. Do you want to adjust tile references?</source>
        <translation>タイルセット %1 の列数は %2 から %3 へ変更されました。タイルへの参照を変更しますか?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::EditPolygonTool</name>
    <message>
        <location filename="../src/tiled/editpolygontool.cpp" line="+129"/>
        <location line="+209"/>
        <source>Edit Polygons</source>
        <translation>ポリゴンを編集</translation>
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
            <numerusform>%n ポイントを移動</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+26"/>
        <location line="+45"/>
        <source>Delete %n Node(s)</source>
        <translation>
            <numerusform>%n ノードを削除</numerusform>
        </translation>
    </message>
    <message>
        <location line="-40"/>
        <location line="+215"/>
        <source>Join Nodes</source>
        <translation>ノードを結合</translation>
    </message>
    <message>
        <location line="-214"/>
        <location line="+250"/>
        <source>Split Segments</source>
        <translation>分割</translation>
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
        <translation>新しい地形</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::Eraser</name>
    <message>
        <location filename="../src/tiled/eraser.cpp" line="+35"/>
        <location line="+56"/>
        <source>Eraser</source>
        <translation>消しゴム</translation>
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
        <translation>エクスポート</translation>
    </message>
    <message>
        <location line="+73"/>
        <source>Export as Image</source>
        <translation>画像でエクスポート</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>%1 already exists.
Do you want to replace it?</source>
        <translation>%1 は既に存在します。
上書きしてよろしいですか?</translation>
    </message>
    <message>
        <location line="+46"/>
        <source>Out of Memory</source>
        <translation>メモリ不足</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not allocate sufficient memory for the image. Try reducing the zoom level or using a 64-bit version of Tiled.</source>
        <translation>画像用のメモリが確保に失敗しました。表示を縮小するか、64ビット版のTiledを使用してください。</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image too Big</source>
        <translation>画像が大きすぎます</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The resulting image would be %1 x %2 pixels and take %3 GB of memory. Tiled is unable to create such an image. Try reducing the zoom level.</source>
        <translation>%1 x %2 pxの生成画像が %3 GBのメモリを使用します。Tiledはこの画像を生成できません。表示を縮小してください。</translation>
    </message>
    <message>
        <location line="+100"/>
        <source>Image</source>
        <translation>画像</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileChangedWarning</name>
    <message>
        <location filename="../src/tiled/documentmanager.cpp" line="-600"/>
        <source>File change detected. Discard changes and reload the map?</source>
        <translation>現在開いているファイルへの変更を検知しました。このアプリでの変更をすべて破棄し、元ファイルを再読込しますか?</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::FileEdit</name>
    <message>
        <location filename="../src/tiled/fileedit.cpp" line="+113"/>
        <source>Choose a File</source>
        <translation>ファイルを選択</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerDock</name>
    <message>
        <location filename="../src/tiled/layerdock.cpp" line="+217"/>
        <source>Layers</source>
        <translation>レイヤー</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Opacity:</source>
        <translation>透過度:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerModel</name>
    <message>
        <location filename="../src/tiled/layermodel.cpp" line="+151"/>
        <source>Layer</source>
        <translation>レイヤー</translation>
    </message>
    <message>
        <location line="+130"/>
        <source>Show Other Layers</source>
        <translation>他レイヤーを表示</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Hide Other Layers</source>
        <translation>他レイヤーを隠す</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::LayerOffsetTool</name>
    <message>
        <location filename="../src/tiled/layeroffsettool.cpp" line="+38"/>
        <location line="+94"/>
        <source>Offset Layers</source>
        <translation>レイヤーをずらす</translation>
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
        <translation>自動選択ツール</translation>
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
        <translation>元に戻す</translation>
    </message>
    <message>
        <location line="-10"/>
        <location line="+9"/>
        <source>Redo</source>
        <translation>やり直す</translation>
    </message>
    <message>
        <location line="+607"/>
        <source>Open Map</source>
        <translation>マップを開く</translation>
    </message>
    <message>
        <location line="-513"/>
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
        <translation>ランダムモード</translation>
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
        <translation>レイヤー(&amp;L)</translation>
    </message>
    <message>
        <location line="-1396"/>
        <location line="+1397"/>
        <source>&amp;New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-1210"/>
        <source>Object Types Editor</source>
        <translation>オブジェクト・タイプ・エディター</translation>
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
        <translation>マップを開いている途中にエラー</translation>
    </message>
    <message>
        <location line="+83"/>
        <location line="+196"/>
        <location line="+296"/>
        <source>All Files (*)</source>
        <translation>すべてのファイル (*)</translation>
    </message>
    <message>
        <location line="-456"/>
        <location line="+88"/>
        <source>Error Saving Map</source>
        <translation>マップを保存中にエラー</translation>
    </message>
    <message>
        <location line="-46"/>
        <source>untitled.tmx</source>
        <translation>untitled.tmx</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Extension Mismatch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The file extension does not match the chosen file type.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Tiled may not automatically recognize your file when loading. Are you sure you want to save with this extension?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+45"/>
        <source>Unsaved Changes</source>
        <translation>変更が保存されていません</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>There are unsaved changes. Do you want to save now?</source>
        <translation>変更が保存されていません。保存しますか?</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Exported to %1</source>
        <translation>%1へエクスポートしました</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+117"/>
        <source>Error Exporting Map</source>
        <translation>マップをエクスポート中にエラー</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Export As...</source>
        <translation>名前をつけてエクスポート...</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Non-unique file extension</source>
        <translation>ファイルの拡張子が無い</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Non-unique file extension.
Please select specific format.</source>
        <translation>ファイルの拡張子が無いです
拡張子を指定してください.</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Unknown File Format</source>
        <translation>不明のファイルフォーマット</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The given filename does not have any known file extension.</source>
        <translation>入力されたファイル名にサポートしている拡張子が含まれていません。</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Some export files already exist:</source>
        <translation>エクスポート先ファイルが既に存在しています:</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Do you want to replace them?</source>
        <translation>上書きしてもよろしいですか?</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Overwrite Files</source>
        <translation>ファイルの上書き</translation>
    </message>
    <message>
        <location line="+621"/>
        <source>[*]%1</source>
        <translation>[*]%1</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Error Reloading Map</source>
        <translation>マップを再読込中にエラー</translation>
    </message>
    <message>
        <location line="-434"/>
        <source>Automatic Mapping Warning</source>
        <translation>自動マップの警告</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Automatic Mapping Error</source>
        <translation>自動マップのエラー</translation>
    </message>
    <message>
        <location line="-874"/>
        <location line="+1212"/>
        <source>Views and Toolbars</source>
        <translation>ビューとツールバー</translation>
    </message>
    <message>
        <location line="-1209"/>
        <location line="+1210"/>
        <source>Tile Animation Editor</source>
        <translation>タイル・アニメーション エディター</translation>
    </message>
    <message>
        <location line="-1208"/>
        <location line="+1209"/>
        <source>Tile Collision Editor</source>
        <translation>タイルの当たり判定 エディター</translation>
    </message>
    <message>
        <location line="-1175"/>
        <source>Alt+Left</source>
        <translation>Alt + ←</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Alt+Right</source>
        <translation>Alt + →</translation>
    </message>
    <message>
        <location line="+737"/>
        <source>Add External Tileset(s)</source>
        <translation>タイルセットを読み込み</translation>
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
            <numerusform>%nつのタイルセットを追加</numerusform>
        </translation>
    </message>
    <message>
        <location line="-16"/>
        <location line="+5"/>
        <source>Error Reading Tileset</source>
        <translation>タイルセットを読み込み中にエラー</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocument</name>
    <message>
        <location filename="../src/tiled/mapdocument.cpp" line="+246"/>
        <source>untitled.tmx</source>
        <translation>untitled.tmx</translation>
    </message>
    <message>
        <location line="+90"/>
        <source>Resize Map</source>
        <translation>マップのリサイズ</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Offset Map</source>
        <translation>マップをずらす</translation>
    </message>
    <message numerus="yes">
        <location line="+28"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを回転</numerusform>
        </translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer %1</source>
        <translation>タイル・レイヤー %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Object Layer %1</source>
        <translation>オブジェクト・レイヤー %1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Image Layer %1</source>
        <translation>画像・レイヤー %1</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Copy of %1</source>
        <translation>%1をコピー</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Duplicate Layer</source>
        <translation>レイヤーの複製</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Merge Layer Down</source>
        <translation>下レイヤーと結合</translation>
    </message>
    <message>
        <location line="+238"/>
        <source>Tile</source>
        <translation>タイル</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Tileset Changes</source>
        <translation>タイルセットの変更点</translation>
    </message>
    <message numerus="yes">
        <location line="+190"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを複製</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+21"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを削除</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+12"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>%nつのオブジェクトを別レイヤーへ移動</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+37"/>
        <source>Move %n Object(s) Up</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+36"/>
        <source>Move %n Object(s) Down</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapDocumentActionHandler</name>
    <message>
        <location filename="../src/tiled/mapdocumentactionhandler.cpp" line="+172"/>
        <source>Select &amp;All</source>
        <translation>すべて選択(&amp;A)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;None</source>
        <translation>すべて選択解除(&amp;N)</translation>
    </message>
    <message>
        <location line="-113"/>
        <source>Ctrl+Shift+A</source>
        <translation>Ctrl+Shift+A</translation>
    </message>
    <message>
        <location line="+120"/>
        <source>&amp;Duplicate Layer</source>
        <translation>レイヤーの複製(&amp;D)</translation>
    </message>
    <message>
        <location line="-101"/>
        <source>Ctrl+Shift+D</source>
        <translation>Ctrl+Shift+D</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Ctrl+Shift+H</source>
        <translation>Ctrl+Shift+H</translation>
    </message>
    <message>
        <location line="+62"/>
        <source>&amp;Crop to Selection</source>
        <translation>選択範囲を切り抜き(&amp;C)</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>&amp;Merge Layer Down</source>
        <translation>下レイヤーと結合(&amp;M)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Layer</source>
        <translation>レイヤーを削除(&amp;R)</translation>
    </message>
    <message>
        <location line="-80"/>
        <source>Ctrl+Shift+Up</source>
        <translation>Ctrl+Shift+Up</translation>
    </message>
    <message>
        <location line="-12"/>
        <source>Ctrl+J</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+Shift+J</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgUp</source>
        <translation>Ctrl+PgUp</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Ctrl+PgDown</source>
        <translation>Ctrl+PgDown</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Ctrl+Shift+Down</source>
        <translation>Ctrl+Shift+Down</translation>
    </message>
    <message>
        <location line="+68"/>
        <source>&amp;Tile Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Object Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Image Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+197"/>
        <source>Layer via Copy</source>
        <translation type="unfinished"></translation>
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
        <translation>前のレイヤーを選択(&amp;v)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Select &amp;Next Layer</source>
        <translation>次のレイヤーを選択(&amp;N)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>R&amp;aise Layer</source>
        <translation>レイヤーを上へ(&amp;a)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Lower Layer</source>
        <translation>レイヤーを下へ(&amp;L)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Show/&amp;Hide all Other Layers</source>
        <translation>すべてのレイヤーを表示/非表示(&amp;H)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Layer &amp;Properties...</source>
        <translation>レイヤーの設定(&amp;P)...</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Cut</source>
        <translation type="unfinished">カット</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Delete</source>
        <translation type="unfinished">削除</translation>
    </message>
    <message numerus="yes">
        <location line="+327"/>
        <source>Duplicate %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを複製</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+1"/>
        <source>Remove %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを削除</numerusform>
        </translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Duplicate Objects</source>
        <translation>オブジェクトを複製</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Objects</source>
        <translation>オブジェクトを削除</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapObjectModel</name>
    <message>
        <location filename="../src/tiled/mapobjectmodel.cpp" line="+150"/>
        <source>Change Object Name</source>
        <translation>オブジェクト名を変更</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Change Object Type</source>
        <translation>オブジェクト・タイプを変更</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Type</source>
        <translation>種類</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MapsDock</name>
    <message>
        <location filename="../src/tiled/mapsdock.cpp" line="+83"/>
        <source>Browse...</source>
        <translation>参照...</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Choose the Maps Folder</source>
        <translation>マップのフォルダを選択</translation>
    </message>
    <message>
        <location line="+34"/>
        <source>Maps</source>
        <translation>マップ</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::MiniMapDock</name>
    <message>
        <location filename="../src/tiled/minimapdock.cpp" line="+60"/>
        <source>Mini-map</source>
        <translation>ミニマップ</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewMapDialog</name>
    <message>
        <location filename="../src/tiled/newmapdialog.cpp" line="+2"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="-14"/>
        <source>Orthogonal</source>
        <translation>□型タイル(長方形マップ)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric</source>
        <translation>◇型タイル(◇型マップ)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Isometric (Staggered)</source>
        <translation>◇型タイル(長方形マップ)</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="../src/tiled/propertybrowser.cpp" line="+1"/>
        <source>Hexagonal (Staggered)</source>
        <translation>六角タイル(長方形マップ)</translation>
    </message>
    <message>
        <location line="+60"/>
        <source>Tile Layer 1</source>
        <translation>タイル・レイヤー1</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Memory Usage Warning</source>
        <translation>メモリ警告</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile layers for this map will consume %L1 GB of memory each. Not creating one by default.</source>
        <translation>このマップは%L1 GBものメモリを必要とします。標準設定では作れません.</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>%1 x %2 pixels</source>
        <translation>%1 x %2 ピクセル</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::NewTilesetDialog</name>
    <message>
        <location filename="../src/tiled/newtilesetdialog.cpp" line="-40"/>
        <location line="+7"/>
        <source>Error</source>
        <translation>エラー</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Failed to load tileset image &apos;%1&apos;.</source>
        <translation>タイルセットの画像 &apos;%1&apos; の読み込みに失敗.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No tiles found in the tileset image when using the given tile size, margin and spacing!</source>
        <translation>タイルセットの画像に１つもタイルがありません（画像サイズ、マージン、スペースを考慮した結果）!</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Tileset Image</source>
        <translation>タイルセットの画像</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectSelectionTool</name>
    <message>
        <location filename="../src/tiled/objectselectiontool.cpp" line="+309"/>
        <location line="+300"/>
        <source>Select Objects</source>
        <translation>オブジェクトを選択</translation>
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
            <numerusform>%nつのオブジェクトを移動</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+87"/>
        <source>Rotate %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを回転</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <location line="+266"/>
        <source>Resize %n Object(s)</source>
        <translation>
            <numerusform>%nつのオブジェクトを拡縮</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesEditor</name>
    <message>
        <location filename="../src/tiled/objecttypeseditor.cpp" line="+224"/>
        <source>Add Object Type</source>
        <translation>オブジェクト・タイプの追加</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Object Type</source>
        <translation>オブジェクト・タイプを削除</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>プロパティを追加</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove Property</source>
        <translation>プロパティを削除</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+323"/>
        <source>Rename Property</source>
        <translation>プロパティ名を変更</translation>
    </message>
    <message>
        <location line="-265"/>
        <location line="+129"/>
        <source>Error Writing Object Types</source>
        <translation>オブジェクト・タイプを書き込み中にエラー</translation>
    </message>
    <message>
        <location line="-128"/>
        <source>Error writing to %1:
%2</source>
        <translation>%1 へ書き込み中にエラー:
%2</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>Choose Object Types File</source>
        <translation>オブジェクト・タイプ・ファイルを選択</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+34"/>
        <location line="+44"/>
        <source>Object Types files (*.xml)</source>
        <translation>オブジェクト・タイプ ファイル (*.xml)</translation>
    </message>
    <message>
        <location line="-62"/>
        <location line="+44"/>
        <source>Error Reading Object Types</source>
        <translation>オブジェクト・タイプを読み込み中にエラー</translation>
    </message>
    <message>
        <location line="-28"/>
        <source>Import Object Types</source>
        <translation>オブジェクト・タイプをインポート</translation>
    </message>
    <message>
        <location line="+44"/>
        <source>Export Object Types</source>
        <translation>オブジェクト・タイプをエクスポート</translation>
    </message>
    <message>
        <location line="+144"/>
        <source>Name:</source>
        <translation>名前:</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectTypesModel</name>
    <message>
        <location filename="../src/tiled/objecttypesmodel.cpp" line="+59"/>
        <source>Type</source>
        <translation>種類</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Color</source>
        <translation>色</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::ObjectsDock</name>
    <message>
        <location filename="../src/tiled/objectsdock.cpp" line="+170"/>
        <source>Object Properties</source>
        <translation>オブジェクトのプロパティ</translation>
    </message>
    <message>
        <location line="-1"/>
        <source>Add Object Layer</source>
        <translation>オブジェクト・レイヤーの追加</translation>
    </message>
    <message>
        <location line="-2"/>
        <source>Objects</source>
        <translation>オブジェクト</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Move Objects Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Move Objects Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location line="+17"/>
        <source>Move %n Object(s) to Layer</source>
        <translation>
            <numerusform>%nつのオブジェクトを別レイヤーへ移動</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PatreonDialog</name>
    <message>
        <location filename="../src/tiled/patreondialog.cpp" line="+68"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;Thank you for support!&lt;/h3&gt;
&lt;p&gt;Your support as a patron makes a big difference to me as the main developer and maintainer of Tiled. It allows me to spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Keep an eye out for exclusive updates in the Activity feed on my Patreon page to find out what I&apos;ve been up to in the time I could spend on Tiled thanks to your support!&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;ご支援頂きありがとうございます！&lt;/h3&gt;
&lt;p&gt;あなたのパトロンとしてのご支援はTiledのメイン開発者やメンテナにとってとても重要です。それは私がより多くの時間をTiledの開発に費やすことを可能にします。&lt;/p&gt;
&lt;p&gt;PatreonのページのActivityボタンを押すとパトロンの方々のご支援で何ができたのかを受信できます。あなたのご支援ありがとうございました！&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m no longer a patron</source>
        <translation>しばらく支援していない</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;With your help I can continue to improve Tiled!&lt;/h3&gt;
&lt;p&gt;Please consider supporting me as a patron. Your support would make a big difference to me, the main developer and maintainer of Tiled. I could spend less time working for money elsewhere and spend more time working on Tiled instead.&lt;/p&gt;
&lt;p&gt;Every little bit helps. Tiled has a lot of users and if each would contribute a small donation each month I will have time to make sure Tiled keeps getting better.&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;
&lt;h3&gt;あなたのご支援で私はTiledを開発できます!&lt;/h3&gt;
&lt;p&gt;パトロンになって私をご支援していただければ幸いに思います。あなたのサポートはTiledの開発やメンテナンスにとても重要です。寄付金によってより多くの時間をTiledの開発に費やすことができます。&lt;/p&gt;
&lt;p&gt;塵も積もれば山となります。Tiledのたくさんのユーザーさんがそれぞれ少しずつ毎月寄付して頂けばTiledをより良くすることができます。&lt;/p&gt;
&lt;p&gt;&lt;i&gt;Thorbj&amp;oslash;rn Lindeijer&lt;/i&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>I&apos;m already a patron!</source>
        <translation>もう寄付したよ!</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PreferencesDialog</name>
    <message>
        <location filename="../src/tiled/preferencesdialog.cpp" line="-127"/>
        <location line="+123"/>
        <source>System default</source>
        <translation>システム標準</translation>
    </message>
    <message>
        <location line="-6"/>
        <source>Last checked: %1</source>
        <translation>最終チェック: %1</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertiesDock</name>
    <message>
        <location filename="../src/tiled/propertiesdock.cpp" line="+278"/>
        <source>Name:</source>
        <translation>名前:</translation>
    </message>
    <message>
        <location line="+104"/>
        <source>Add Property</source>
        <translation>プロパティを追加</translation>
    </message>
    <message>
        <location line="-102"/>
        <location line="+104"/>
        <source>Rename Property</source>
        <translation>プロパティ名を変更</translation>
    </message>
    <message>
        <location line="-71"/>
        <source>Convert To</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Rename...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Remove</source>
        <translation type="unfinished">削除</translation>
    </message>
    <message>
        <location line="+65"/>
        <source>Properties</source>
        <translation>プロパティ</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Remove Property</source>
        <translation>プロパティを削除</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::PropertyBrowser</name>
    <message>
        <location filename="../src/tiled/propertybrowser.cpp" line="+13"/>
        <source>Horizontal</source>
        <translation>横反転</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Vertical</source>
        <translation>縦反転</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Top Down</source>
        <translation>上から下</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Manual</source>
        <translation>個別指定</translation>
    </message>
    <message>
        <location line="+485"/>
        <source>Columns</source>
        <translation>列</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Source</source>
        <translation>パス</translation>
    </message>
    <message>
        <location line="+31"/>
        <source>Relative chance this tile will be picked</source>
        <translation>このタイルが選ばれる確率</translation>
    </message>
    <message>
        <location line="+286"/>
        <source>Error Reading Tileset</source>
        <translation>タイルセットを読み込み中にエラー</translation>
    </message>
    <message>
        <location line="+142"/>
        <source>Custom Properties</source>
        <translation>カスタムプロパティ</translation>
    </message>
    <message>
        <location line="-637"/>
        <source>Map</source>
        <translation>マップ</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Tile Layer Format</source>
        <translation>タイルの出力形式</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Tile Render Order</source>
        <translation>タイルの描画順序</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Background Color</source>
        <translation>背景色</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Object</source>
        <translation>オブジェクト</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+26"/>
        <location line="+74"/>
        <location line="+60"/>
        <source>Name</source>
        <translation>名前</translation>
    </message>
    <message>
        <location line="-157"/>
        <source>Type</source>
        <translation>種類</translation>
    </message>
    <message>
        <location line="+3"/>
        <location line="+21"/>
        <source>Visible</source>
        <translation>表示</translation>
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
        <translation>奇数</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Even</source>
        <translation>偶数</translation>
    </message>
    <message>
        <location line="+342"/>
        <source>Orientation</source>
        <translation>種類</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Width</source>
        <translation>幅</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+66"/>
        <location line="+125"/>
        <source>Height</source>
        <translation>高さ</translation>
    </message>
    <message>
        <location line="-190"/>
        <location line="+167"/>
        <source>Tile Width</source>
        <translation>タイルの幅</translation>
    </message>
    <message>
        <location line="-166"/>
        <location line="+167"/>
        <source>Tile Height</source>
        <translation>タイルの高さ</translation>
    </message>
    <message>
        <location line="-165"/>
        <source>Tile Side Length (Hex)</source>
        <translation>六角タイルの辺長</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Stagger Axis</source>
        <translation>ジグザグ方向</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Stagger Index</source>
        <translation>ジグザグ開始位置</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Rotation</source>
        <translation>回転</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Flipping</source>
        <translation>反転</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Opacity</source>
        <translation>透過度</translation>
    </message>
    <message>
        <location line="+10"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Horizontal Offset</source>
        <translation>横方向のオフセット</translation>
    </message>
    <message>
        <location line="-37"/>
        <location line="+9"/>
        <location line="+29"/>
        <source>Vertical Offset</source>
        <translation>縦方向のオフセット</translation>
    </message>
    <message>
        <location line="-41"/>
        <source>Tile Layer</source>
        <translation>タイル・レイヤー</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Object Layer</source>
        <translation>オブジェクト・レイヤー</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Color</source>
        <translation>色</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Drawing Order</source>
        <translation>描画順序</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Image Layer</source>
        <translation>画像・レイヤー</translation>
    </message>
    <message>
        <location line="+5"/>
        <location line="+37"/>
        <location line="+39"/>
        <source>Image</source>
        <translation>画像</translation>
    </message>
    <message>
        <location line="-71"/>
        <location line="+39"/>
        <source>Transparent Color</source>
        <translation>透過色</translation>
    </message>
    <message>
        <location line="-29"/>
        <source>Tileset</source>
        <translation>タイルセット</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Filename</source>
        <translation>ファイル名</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Drawing Offset</source>
        <translation>描画オフセット</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Margin</source>
        <translation>上左の余白</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Spacing</source>
        <translation>タイル間の余白</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Tile</source>
        <translation>タイル</translation>
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
        <translation>確率</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Terrain</source>
        <translation>地形</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::SelectSameTileTool</name>
    <message>
        <location filename="../src/tiled/selectsametiletool.cpp" line="+33"/>
        <location line="+62"/>
        <source>Select Same Tile</source>
        <translation>同じタイルを選択</translation>
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
        <translation>スタンプ</translation>
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
        <translation>地形ブラシ</translation>
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
        <translation>地形</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Erase Terrain</source>
        <translation>地形を削除</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TerrainView</name>
    <message>
        <location filename="../src/tiled/terrainview.cpp" line="+97"/>
        <source>Terrain &amp;Properties...</source>
        <translation>地形のプロパティ (&amp;P)...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TextPropertyEdit</name>
    <message>
        <location filename="../src/tiled/textpropertyedit.cpp" line="+121"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileAnimationEditor</name>
    <message>
        <location filename="../src/tiled/tileanimationeditor.cpp" line="-58"/>
        <source>Delete Frames</source>
        <translation>フレームを削除</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileCollisionEditor</name>
    <message>
        <location filename="../src/tiled/tilecollisioneditor.cpp" line="+263"/>
        <source>Delete</source>
        <translation>削除</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>Cut</source>
        <translation>カット</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Tile Collision Editor</source>
        <translation>タイルの当たり判定 エディター</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileSelectionTool</name>
    <message>
        <location filename="../src/tiled/tileselectiontool.cpp" line="+34"/>
        <location line="+96"/>
        <source>Rectangular Select</source>
        <translation>選択範囲</translation>
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
        <translation>%1, %2 - 四角形: (%3 x %4)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampModel</name>
    <message>
        <location filename="../src/tiled/tilestampmodel.cpp" line="+78"/>
        <source>Stamp</source>
        <translation>スタンプ</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Probability</source>
        <translation>確率</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TileStampsDock</name>
    <message>
        <location filename="../src/tiled/tilestampsdock.cpp" line="+194"/>
        <source>Delete Stamp</source>
        <translation>スタンプを削除</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Variation</source>
        <translation>バリエーションを削除</translation>
    </message>
    <message>
        <location line="+71"/>
        <source>Choose the Stamps Folder</source>
        <translation>スタンプフォルダを選択</translation>
    </message>
    <message>
        <location line="+15"/>
        <source>Tile Stamps</source>
        <translation>タイル・スタンプ</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add New Stamp</source>
        <translation>新しいスタンプを追加</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Add Variation</source>
        <translation>バリエーションを追加</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Duplicate Stamp</source>
        <translation>スタンプを複製</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Delete Selected</source>
        <translation>選択したものを削除</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Set Stamps Folder</source>
        <translation>スタンプフォルダを設定</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Filter</source>
        <translation>フィルタ</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetDock</name>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="+731"/>
        <source>Remove Tileset</source>
        <translation>タイルセットを削除</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>The tileset &quot;%1&quot; is still in use by the map!</source>
        <translation>タイルセット &quot;%1&quot;はまだマップで使われています!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove this tileset and all references to the tiles in this tileset?</source>
        <translation>このタイルセットを削除し、タイル使用部分をすべて取り除いてよろしいですか?</translation>
    </message>
    <message>
        <location line="+81"/>
        <source>Tilesets</source>
        <translation>タイルセット</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>New Tileset</source>
        <translation>新しいタイルセット</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Import Tileset</source>
        <translation>タイルセットをインポート(&amp;I)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Export Tileset As...</source>
        <translation>タイルセットを名前をつけてエクスポート(&amp;E)...</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Tile&amp;set Properties</source>
        <translation>タイルセットのプロパティ(&amp;s)</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&amp;Remove Tileset</source>
        <translation>タイルセットを削除(&amp;R)</translation>
    </message>
    <message>
        <location line="+2"/>
        <location line="+128"/>
        <location line="+15"/>
        <source>Add Tiles</source>
        <translation>タイルを追加</translation>
    </message>
    <message>
        <location line="-142"/>
        <location line="+199"/>
        <location line="+13"/>
        <source>Remove Tiles</source>
        <translation>タイルを削除</translation>
    </message>
    <message>
        <location line="-121"/>
        <source>Error saving tileset: %1</source>
        <translation>タイルセットを保存中にエラー: %1</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Could not load &quot;%1&quot;!</source>
        <translation>&quot;%1&quot;を読み込めません!</translation>
    </message>
    <message>
        <location line="+57"/>
        <source>One or more of the tiles to be removed are still in use by the map!</source>
        <translation>マップ内で削除されたタイルを使用しています!</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Remove all references to these tiles?</source>
        <translation>このタイルを使っているところをすべて削除しますか?</translation>
    </message>
    <message>
        <location line="-207"/>
        <source>Edit &amp;Terrain Information</source>
        <translation>地形の情報を変更 (&amp;T)</translation>
    </message>
    <message>
        <location line="+69"/>
        <location line="+23"/>
        <source>Export Tileset</source>
        <translation>タイルセットをエクスポート</translation>
    </message>
    <message>
        <location line="-39"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiledタイルセットファイル (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetParametersEdit</name>
    <message>
        <location filename="../src/tiled/tilesetparametersedit.cpp" line="+48"/>
        <source>Edit...</source>
        <translation>編集...</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TilesetView</name>
    <message>
        <location filename="../src/tiled/tilesetview.cpp" line="+639"/>
        <source>Tile &amp;Properties...</source>
        <translation>タイルの設定(&amp;P)...</translation>
    </message>
    <message>
        <location line="-11"/>
        <source>Add Terrain Type</source>
        <translation>地形の種類を追加</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Set Terrain Image</source>
        <translation>地形の画像を設定</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Show &amp;Grid</source>
        <translation>グリッドの表示(&amp;G)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TmxMapFormat</name>
    <message>
        <location filename="../src/tiled/tmxmapformat.h" line="+62"/>
        <source>Tiled map files (*.tmx)</source>
        <translation>Tiledマップファイル (*.tmx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::TsxTilesetFormat</name>
    <message>
        <location line="+24"/>
        <source>Tiled tileset files (*.tsx)</source>
        <translation>Tiledタイルセットファイル (*.tsx)</translation>
    </message>
</context>
<context>
    <name>Tiled::Internal::UndoDock</name>
    <message>
        <location filename="../src/tiled/undodock.cpp" line="+64"/>
        <source>History</source>
        <translation>履歴</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>&lt;empty&gt;</source>
        <translation>&lt;何も無し&gt;</translation>
    </message>
</context>
<context>
    <name>Tmw::TmwPlugin</name>
    <message>
        <location filename="../src/plugins/tmw/tmwplugin.cpp" line="+47"/>
        <source>Multiple collision layers found!</source>
        <translation>当たり判定・レイヤーが複数存在します!</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>No collision layer found!</source>
        <translation>当たり判定・レイヤーが見つかりません!</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Could not open file for writing.</source>
        <translation>書き込み用ファイルを開けませんでした.</translation>
    </message>
    <message>
        <location line="+30"/>
        <source>TMW-eAthena collision files (*.wlk)</source>
        <translation>TMW-eAthena collision files (*.wlk)</translation>
    </message>
</context>
<context>
    <name>TmxViewer</name>
    <message>
        <location filename="../src/tmxviewer/tmxviewer.cpp" line="+182"/>
        <source>TMX Viewer</source>
        <translation>TMXビューアー</translation>
    </message>
</context>
<context>
    <name>Undo Commands</name>
    <message>
        <location filename="../src/tiled/addremovelayer.h" line="+67"/>
        <source>Add Layer</source>
        <translation>レイヤーを追加</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Remove Layer</source>
        <translation>レイヤーを削除</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovemapobject.cpp" line="+76"/>
        <source>Add Object</source>
        <translation>オブジェクトを追加</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove Object</source>
        <translation>オブジェクトを削除</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+36"/>
        <source>Change Object</source>
        <translation>オブジェクトを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeobjectgroupproperties.cpp" line="+39"/>
        <source>Change Object Layer Properties</source>
        <translation>オブジェクト・レイヤーのプロパティを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeproperties.cpp" line="+40"/>
        <source>Change %1 Properties</source>
        <translation>%1 プロパティを変更</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Set Property</source>
        <translation>プロパティを設定</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Add Property</source>
        <translation>プロパティを追加</translation>
    </message>
    <message>
        <location line="+33"/>
        <source>Remove Property</source>
        <translation>プロパティを削除</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Rename Property</source>
        <translation>プロパティ名を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeselectedarea.cpp" line="+31"/>
        <source>Change Selection</source>
        <translation>選択を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/erasetiles.cpp" line="+39"/>
        <source>Erase</source>
        <translation>消去</translation>
    </message>
    <message>
        <location filename="../src/tiled/bucketfilltool.cpp" line="-30"/>
        <source>Fill Area</source>
        <translation>塗りつぶし</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Move Object</source>
        <translation>オブジェクトを移動</translation>
    </message>
    <message>
        <location filename="../src/tiled/offsetlayer.cpp" line="+42"/>
        <source>Offset Layer</source>
        <translation>レイヤーをずらす</translation>
    </message>
    <message>
        <location filename="../src/tiled/painttilelayer.cpp" line="+51"/>
        <location line="+22"/>
        <source>Paint</source>
        <translation>ペイント</translation>
    </message>
    <message>
        <location filename="../src/tiled/renamelayer.cpp" line="+40"/>
        <source>Rename Layer</source>
        <translation>レイヤー名を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizetilelayer.cpp" line="+37"/>
        <source>Resize Layer</source>
        <translation>レイヤーをリサイズ</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemap.cpp" line="+32"/>
        <source>Resize Map</source>
        <translation>マップをリサイズ</translation>
    </message>
    <message>
        <location filename="../src/tiled/resizemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Resize Object</source>
        <translation>オブジェクトをリサイズ</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetileset.cpp" line="+66"/>
        <source>Add Tileset</source>
        <translation>タイルセットを追加</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tileset</source>
        <translation>タイルセットを削除</translation>
    </message>
    <message>
        <location filename="../src/tiled/movetileset.cpp" line="+31"/>
        <source>Move Tileset</source>
        <translation>タイルセットを移動</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetdock.cpp" line="-788"/>
        <source>Import Tileset</source>
        <translation>タイルセットをインポート</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Export Tileset</source>
        <translation>タイルセットをエクスポート</translation>
    </message>
    <message>
        <location filename="../src/tiled/tilesetchanges.cpp" line="+36"/>
        <source>Change Tileset Name</source>
        <translation>タイルセット名を変更</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Change Drawing Offset</source>
        <translation>描画オフセットを変更</translation>
    </message>
    <message>
        <location line="+50"/>
        <source>Edit Tileset</source>
        <translation>タイルセットを編集</translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Change Columns</source>
        <translation>列を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/movemapobjecttogroup.cpp" line="+41"/>
        <source>Move Object to Layer</source>
        <translation>オブジェクトをレイヤーに移動</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerproperties.cpp" line="+39"/>
        <source>Change Image Layer Properties</source>
        <translation>画像・レイヤーのプロパティを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/movelayer.cpp" line="+37"/>
        <source>Lower Layer</source>
        <translation>レイヤーを下へ</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Raise Layer</source>
        <translation>レイヤーを上へ</translation>
    </message>
    <message>
        <location filename="../src/tiled/changepolygon.cpp" line="+40"/>
        <location line="+12"/>
        <source>Change Polygon</source>
        <translation>ポリゴンを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremoveterrain.cpp" line="+69"/>
        <source>Add Terrain</source>
        <translation>地形を追加</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Remove Terrain</source>
        <translation>地形を削除</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileterrain.cpp" line="+133"/>
        <source>Change Tile Terrain</source>
        <translation>タイルの地形を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/editterraindialog.cpp" line="-135"/>
        <source>Change Terrain Image</source>
        <translation>地形の画像を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changelayer.cpp" line="+41"/>
        <source>Show Layer</source>
        <translation>レイヤーを表示</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Layer</source>
        <translation>レイヤーを非表示</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Change Layer Opacity</source>
        <translation>レイヤーの透明度を変更</translation>
    </message>
    <message>
        <location line="+29"/>
        <source>Change Layer Offset</source>
        <translation>レイヤーのオフセットを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobject.cpp" line="+31"/>
        <source>Show Object</source>
        <translation>オブジェクトを表示</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Hide Object</source>
        <translation>オブジェクトを非表示</translation>
    </message>
    <message>
        <location filename="../src/tiled/renameterrain.cpp" line="+37"/>
        <source>Change Terrain Name</source>
        <translation>地形名を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/addremovetiles.cpp" line="+69"/>
        <source>Add Tiles</source>
        <translation>タイルを追加</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Remove Tiles</source>
        <translation>タイルを削除</translation>
    </message>
    <message>
        <location filename="../src/tiled/changeimagelayerposition.cpp" line="+36"/>
        <source>Change Image Layer Position</source>
        <translation>画像・レイヤーの位置を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapobjectsorder.cpp" line="+46"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+67"/>
        <source>Raise Object</source>
        <translation>オブジェクトを前面へ</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+29"/>
        <source>Lower Object</source>
        <translation>オブジェクトを背面へ</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileanimation.cpp" line="+35"/>
        <source>Change Tile Animation</source>
        <translation>タイル・アニメーションを変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileobjectgroup.cpp" line="+16"/>
        <source>Change Tile Collision</source>
        <translation>タイルの当たり判定を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/raiselowerhelper.cpp" line="+43"/>
        <source>Raise Object To Top</source>
        <translation>オブジェクトを最前面へ</translation>
    </message>
    <message>
        <location line="+37"/>
        <source>Lower Object To Bottom</source>
        <translation>オブジェクトを最背面へ</translation>
    </message>
    <message>
        <location filename="../src/tiled/rotatemapobject.cpp" line="+40"/>
        <location line="+12"/>
        <source>Rotate Object</source>
        <translation>オブジェクトを回転</translation>
    </message>
    <message>
        <location filename="../src/tiled/changemapproperty.cpp" line="+41"/>
        <source>Change Tile Width</source>
        <translation>タイルの幅を変更</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Tile Height</source>
        <translation>タイルの高さを変更</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Change Hex Side Length</source>
        <translation>六角タイルの辺長を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Background Color</source>
        <translation>背景色を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Axis</source>
        <translation>ジグザグ方向を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Stagger Index</source>
        <translation>ジグザグ開始位置を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Orientation</source>
        <translation>種類を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Render Order</source>
        <translation>描画順序を変更</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Change Layer Data Format</source>
        <translation>レイヤーのデータ形式を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileprobability.cpp" line="+41"/>
        <location line="+14"/>
        <source>Change Tile Probability</source>
        <translation>タイルの確率を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/adjusttileindexes.cpp" line="-134"/>
        <location line="+89"/>
        <source>Adjust Tile Indexes</source>
        <translation>タイルのインデックスを調整</translation>
    </message>
    <message>
        <location filename="../src/tiled/changetileimagesource.cpp" line="+39"/>
        <source>Change Tile Image</source>
        <translation>タイル画像を変更</translation>
    </message>
    <message>
        <location filename="../src/tiled/replacetileset.cpp" line="+33"/>
        <source>Replace Tileset</source>
        <translation>タイルセットを差し替え</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/tiled/flipmapobjects.cpp" line="+39"/>
        <source>Flip %n Object(s)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Utils</name>
    <message>
        <location filename="../src/tiled/utils.cpp" line="+37"/>
        <source>Image files</source>
        <translation>画像ファイル</translation>
    </message>
</context>
</TS>
