#include "document.h"
#include "editableasset.h"
#include "pluginmanager.h"
#include "projectmanager.h"
#include "scriptmanager.h"
#include "tileset.h"
#include "tmxmapformat.h"
#include "worldmanager.h"

#include <QJSEngine>
#include <QPointer>
#include <QStandardPaths>
#include <QtTest/QtTest>

#include <memory>

using namespace Tiled;

class test_ScriptModule : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void worldsWithoutDocumentManager();
    void worldsChangedWithoutDocumentManager();
    void loadMissingWorld();
    void objectPointerPropertyValues();
    void objectReferencesInListProperty();
    void gcOfTilesetAddedToMap_data();
    void gcOfTilesetAddedToMap();
    void addLayerToDocument_data();
    void addLayerToDocument();
    void gcOfTilesetAfterMap();
    void gcOfTilesetSharedByMaps();
    void gcOfUnusedTileset_data();
    void gcOfUnusedTileset();

private:
    std::unique_ptr<ProjectManager> mProjectManager;
};

void test_ScriptModule::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    mProjectManager = std::make_unique<ProjectManager>();
    PluginManager::addObject(new TmxMapFormat);
    ScriptManager::instance().ensureInitialized();
}

void test_ScriptModule::cleanupTestCase()
{
    ScriptManager::deleteInstance();
    mProjectManager.reset();
}

void test_ScriptModule::cleanup()
{
    WorldManager::instance().unloadAllWorlds();
}

void test_ScriptModule::worldsWithoutDocumentManager()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("{\"type\":\"world\",\"maps\":[]}");
    file.flush();

    auto &manager = ScriptManager::instance();
    manager.engine()->globalObject().setProperty(QStringLiteral("testWorldPath"), file.fileName());
    const auto result = manager.evaluate(QStringLiteral("tiled.loadWorld(testWorldPath); tiled.worlds.length"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(WorldManager::instance().worlds().size(), 1);
    QCOMPARE(result.toInt(), 1);

    const auto afterUnload = manager.evaluate(QStringLiteral("tiled.unloadWorld(testWorldPath); tiled.worlds.length"));
    QCOMPARE(afterUnload.toInt(), 0);
}

void test_ScriptModule::worldsChangedWithoutDocumentManager()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("{\"type\":\"world\",\"maps\":[]}");
    file.flush();

    auto &manager = ScriptManager::instance();
    manager.engine()->globalObject().setProperty(QStringLiteral("testWorldPath"), file.fileName());
    const auto result = manager.evaluate(QStringLiteral(
        "var changes = 0;"
        "tiled.worldsChanged.connect(function() { ++changes; });"
        "tiled.loadWorld(testWorldPath); changes"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toInt(), 1);

    const auto afterUnload = manager.evaluate(QStringLiteral("tiled.unloadAllWorlds(); changes"));
    QCOMPARE(afterUnload.toInt(), 2);
}

void test_ScriptModule::loadMissingWorld()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    auto &manager = ScriptManager::instance();
    manager.engine()->globalObject().setProperty(QStringLiteral("testWorldPath"),
                                                directory.filePath(QStringLiteral("missing.world")));
    const auto result = manager.evaluate(QStringLiteral("tiled.loadWorld(testWorldPath)"));
    QVERIFY(result.isError());
    QVERIFY(WorldManager::instance().worlds().isEmpty());
}

void test_ScriptModule::objectPointerPropertyValues()
{
    auto &manager = ScriptManager::instance();

    // Storing an object reference as a property value raises an error,
    // since object pointers can neither be persisted nor read back safely.
    const auto result = manager.evaluate(QStringLiteral(
        "var map = new TileMap();"
        "map.setProperty('ts', new Tileset('ts'));"));
    QVERIFY(result.isError());

    // The same applies to object references nested inside lists.
    const auto listResult = manager.evaluate(QStringLiteral(
        "var map = new TileMap();"
        "map.setProperty('ts', ['foo', new Tileset('ts')]);"));
    QVERIFY(listResult.isError());
}

void test_ScriptModule::objectReferencesInListProperty()
{
    // Values in list properties get the same conversion as plain values,
    // so an object reference in a list survives a file round-trip.
    QTemporaryFile file;
    QVERIFY(file.open());
    file.close();

    auto &manager = ScriptManager::instance();
    manager.engine()->globalObject().setProperty(QStringLiteral("testFilePath"), file.fileName());
    const auto result = manager.evaluate(QStringLiteral(
        "var map = new TileMap();"
        "var layer = new ObjectGroup('objects');"
        "map.addLayer(layer);"
        "var obj = new MapObject('hero');"
        "layer.addObject(obj);"
        "map.setProperty('refs', [obj]);"
        "tiled.mapFormat('tmx').write(map, testFilePath);"
        "var loaded = tiled.mapFormat('tmx').read(testFilePath);"
        "loaded.property('refs')[0].name"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toString(), QStringLiteral("hero"));
}

void test_ScriptModule::gcOfTilesetAddedToMap_data()
{
    QTest::addColumn<int>("insertionMode");
    QTest::newRow("explicit") << 0;
    QTest::newRow("layer") << 1;
    QTest::newRow("group-layer") << 2;
}

void test_ScriptModule::gcOfTilesetAddedToMap()
{
    QFETCH(int, insertionMode);
    auto &manager = ScriptManager::instance();
    manager.engine()->globalObject().setProperty(QStringLiteral("insertionMode"), insertionMode);
    const auto result = manager.evaluate(QStringLiteral(
        "var map = new TileMap();"
        "map.setSize(4, 4);"
        "map.setTileSize(32, 32);"
        "(function() {"
        "    var tileset = new Tileset(\"scripted\");"
        "    var tile = tileset.addTile();"
        "    var image = new Image(32, 32, Image.Format_ARGB32);"
        "    image.fill(\"#0000ff\");"
        "    tile.setImage(image);"
        "    if (insertionMode === 0) map.addTileset(tileset);"
        "    var layer = new ObjectGroup(\"objects\");"
        "    var object = new MapObject();"
        "    object.tile = tile;"
        "    object.width = 32;"
        "    object.height = 32;"
        "    layer.addObject(object);"
        "    if (insertionMode === 2) {"
        "        var group = new GroupLayer('group');"
        "        map.addLayer(group);"
        "        group.addLayer(layer);"
        "    } else {"
        "        map.addLayer(layer);"
        "    }"
        "})();"
        "gc();"
        "var tilesetOk = map.tilesets.length === 1 && map.tilesets[0] !== null;"
        "var layer = map.layerAt(0);"
        "if (insertionMode === 2) layer = layer.layerAt(0);"
        "var object = layer.objects[0];"
        "var tileOk = object.tile !== null && object.tile.tileset === map.tilesets[0];"
        "gc();"
        "tilesetOk && tileOk && map.tilesets[0] !== null;"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toBool(), true);
}

void test_ScriptModule::addLayerToDocument_data()
{
    QTest::addColumn<bool>("nested");
    QTest::newRow("object-layer") << false;
    QTest::newRow("group-layer") << true;
}

void test_ScriptModule::addLayerToDocument()
{
    QFETCH(bool, nested);
    auto &manager = ScriptManager::instance();
    auto mapValue = manager.evaluate(QStringLiteral("var testMap = new TileMap(); testMap;"));
    QVERIFY2(!mapValue.isError(), qPrintable(mapValue.toString()));
    auto document = static_cast<EditableAsset*>(mapValue.toQObject())->createDocument();
    auto result = manager.evaluate(QStringLiteral(
        "var testLayer = new ObjectGroup('objects');"
        "var testObject = new MapObject('hero');"
        "testLayer.addObject(testObject);"
        "var testGroup = new GroupLayer('group');"
        "testGroup.addLayer(testLayer);"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    result = manager.evaluate(nested
        ? QStringLiteral("testMap.addLayer(testGroup);")
        : QStringLiteral("testGroup.removeLayer(testLayer); testMap.addLayer(testLayer);"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    result = manager.evaluate(QStringLiteral(
        "testObject.asset === testMap && testLayer.asset === testMap"));
    QCOMPARE(result.toBool(), true);
    // Undo and redo also attach the layer through the document signals.
    result = manager.evaluate(QStringLiteral("testMap.undo(); testMap.redo(); testMap.layerCount"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toInt(), 1);
    manager.evaluate(QStringLiteral("testMap = testLayer = testObject = testGroup = null;"));
}

void test_ScriptModule::gcOfTilesetAfterMap()
{
    auto &manager = ScriptManager::instance();
    auto result = manager.evaluate(QStringLiteral(
        "(function () {"
        "    var map = new TileMap();"
        "    var tileset = new Tileset('temporary');"
        "    tileset.addTile();"
        "    map.addTileset(tileset);"
        "    return tileset;"
        "})()"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QPointer<QObject> editable = result.toQObject();
    QVERIFY(editable);
    const auto tileset = static_cast<Tileset*>(static_cast<EditableAsset*>(editable.data())->object());
    QWeakPointer<Tileset> weakTileset = tileset->sharedFromThis();
    result = QJSValue();
    for (int i = 0; i < 3; ++i) {
        manager.engine()->collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    QVERIFY(editable.isNull());
    QVERIFY(weakTileset.isNull());
}

void test_ScriptModule::gcOfTilesetSharedByMaps()
{
    auto &manager = ScriptManager::instance();
    auto result = manager.evaluate(QStringLiteral(
        "var firstMap = new TileMap();"
        "var secondMap = new TileMap();"
        "(function () {"
        "    var tileset = new Tileset('shared');"
        "    tileset.addTile();"
        "    firstMap.addTileset(tileset);"
        "    secondMap.addTileset(tileset);"
        "    return tileset;"
        "})()"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QPointer<QObject> editable = result.toQObject();
    result = QJSValue();
    manager.evaluate(QStringLiteral("firstMap = null; gc();"));
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(editable);
    result = manager.evaluate(QStringLiteral("secondMap.tilesets[0].tileCount"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toInt(), 1);
    manager.evaluate(QStringLiteral("secondMap = null;"));
    result = QJSValue();
    for (int i = 0; i < 3; ++i) {
        manager.engine()->collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    QVERIFY(editable.isNull());
}

void test_ScriptModule::gcOfUnusedTileset_data()
{
    QTest::addColumn<bool>("documentBacked");
    QTest::addColumn<bool>("replace");
    QTest::newRow("remove") << false << false;
    QTest::newRow("replace") << false << true;
    QTest::newRow("document-remove") << true << false;
    QTest::newRow("document-replace") << true << true;
}

void test_ScriptModule::gcOfUnusedTileset()
{
    QFETCH(bool, documentBacked);
    QFETCH(bool, replace);
    auto &manager = ScriptManager::instance();
    auto result = manager.evaluate(QStringLiteral("var testMap = new TileMap(); testMap;"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QSharedPointer<Document> document;
    if (documentBacked)
        document = static_cast<EditableAsset*>(result.toQObject())->createDocument();
    result = manager.evaluate(QStringLiteral(
        "var testTileset = new Tileset('unused');"
        "testMap.addTileset(testTileset); testTileset;"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QPointer<QObject> editable = result.toQObject();
    QVERIFY(editable);
    result = manager.evaluate(replace
        ? QStringLiteral("testMap.replaceTileset(testTileset, new Tileset('replacement'));")
        : QStringLiteral("testMap.removeTileset(testTileset);"));
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
    QCOMPARE(result.toBool(), true);
    manager.evaluate(QStringLiteral("testTileset = null;"));
    result = QJSValue();
    for (int i = 0; i < 3; ++i) {
        manager.engine()->collectGarbage();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }
    QVERIFY(editable.isNull());
    result = manager.evaluate(QStringLiteral("testMap.tilesets.length"));
    QCOMPARE(result.toInt(), replace ? 1 : 0);
    manager.evaluate(QStringLiteral("testMap = null;"));
}

QTEST_MAIN(test_ScriptModule)

#include "test_scriptmodule.moc"
