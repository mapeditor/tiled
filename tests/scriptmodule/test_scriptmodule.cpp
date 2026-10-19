#include "pluginmanager.h"
#include "projectmanager.h"
#include "scriptmanager.h"
#include "tmxmapformat.h"
#include "worldmanager.h"

#include <QJSEngine>
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

QTEST_MAIN(test_ScriptModule)

#include "test_scriptmodule.moc"
