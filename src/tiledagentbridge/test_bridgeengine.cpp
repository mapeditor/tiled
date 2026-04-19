#include "bridgeengine.h"

#include <QtTest/QtTest>

class BridgeEngineTest : public QObject
{
    Q_OBJECT

private slots:
    void opensMapAndReturnsSnapshot();
    void appliesCommandsAndSupportsUndoRedo();
};

void BridgeEngineTest::opensMapAndReturnsSnapshot()
{
    tiledagent::BridgeEngine engine;

    const QString path = QStringLiteral(TEST_SOURCE_ROOT) + QStringLiteral("/tests/data/mapobject.tmx");
    const auto result = engine.openDocument(QStringLiteral("session-1"), path);

    QVERIFY2(result.success, qPrintable(result.errorMessage));
    QCOMPARE(result.documentType, QStringLiteral("map"));
    QCOMPARE(result.revision, 0);
    QCOMPARE(result.snapshot.value(QStringLiteral("documentType")).toString(), QStringLiteral("map"));

    const auto map = result.snapshot.value(QStringLiteral("map")).toObject();
    QCOMPARE(map.value(QStringLiteral("width")).toInt(), 100);
    QCOMPARE(map.value(QStringLiteral("height")).toInt(), 80);
}

void BridgeEngineTest::appliesCommandsAndSupportsUndoRedo()
{
    tiledagent::BridgeEngine engine;
    const QString sessionId = QStringLiteral("session-2");
    const QString path = QStringLiteral(TEST_SOURCE_ROOT) + QStringLiteral("/tests/automapping/simple-2x2-rule/map.tmx");

    const auto openResult = engine.openDocument(sessionId, path);
    QVERIFY2(openResult.success, qPrintable(openResult.errorMessage));

    const auto layerResult = engine.executeCommand(sessionId, QJsonObject{
        { QStringLiteral("type"), QStringLiteral("create_layer") },
        { QStringLiteral("payload"), QJsonObject{
            { QStringLiteral("layerType"), QStringLiteral("object") },
            { QStringLiteral("name"), QStringLiteral("Objects") },
        } }
    });
    QVERIFY2(layerResult.success, qPrintable(layerResult.errorMessage));
    QCOMPARE(layerResult.revisionAfter, 1);

    const auto objectResult = engine.executeCommand(sessionId, QJsonObject{
        { QStringLiteral("type"), QStringLiteral("create_object") },
        { QStringLiteral("payload"), QJsonObject{
            { QStringLiteral("layerId"), 6 },
            { QStringLiteral("name"), QStringLiteral("Spawn") },
            { QStringLiteral("className"), QStringLiteral("spawn") },
            { QStringLiteral("x"), 32.0 },
            { QStringLiteral("y"), 48.0 },
            { QStringLiteral("width"), 16.0 },
            { QStringLiteral("height"), 16.0 },
        } }
    });
    QVERIFY2(objectResult.success, qPrintable(objectResult.errorMessage));
    QCOMPARE(objectResult.revisionAfter, 2);

    const auto paintResult = engine.executeCommand(sessionId, QJsonObject{
        { QStringLiteral("type"), QStringLiteral("paint_tiles") },
        { QStringLiteral("payload"), QJsonObject{
            { QStringLiteral("layerId"), 1 },
            { QStringLiteral("cells"), QJsonArray{
                QJsonObject{
                    { QStringLiteral("x"), 0 },
                    { QStringLiteral("y"), 0 },
                    { QStringLiteral("gid"), 1 },
                }
            } }
        } }
    });
    QVERIFY2(paintResult.success, qPrintable(paintResult.errorMessage));
    QCOMPARE(paintResult.revisionAfter, 3);

    const auto undoResult = engine.undo(sessionId);
    QVERIFY2(undoResult.success, qPrintable(undoResult.errorMessage));
    QCOMPARE(undoResult.revisionAfter, 2);

    const auto redoResult = engine.redo(sessionId);
    QVERIFY2(redoResult.success, qPrintable(redoResult.errorMessage));
    QCOMPARE(redoResult.revisionAfter, 3);

    const auto snapshot = engine.getSnapshot(sessionId);
    QVERIFY2(snapshot.success, qPrintable(snapshot.errorMessage));
    const auto map = snapshot.snapshot.value(QStringLiteral("map")).toObject();
    const auto layers = map.value(QStringLiteral("layers")).toArray();

    QCOMPARE(layers.size(), 2);
}

QTEST_MAIN(BridgeEngineTest)
#include "test_bridgeengine.moc"
