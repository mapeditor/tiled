#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "mapreader.h"

#include "mapdocument.h"
#include "automapper.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_AutoMapping : public QObject
{
    Q_OBJECT

private slots:
    void autoMap_data();
    void autoMap();
};

void test_AutoMapping::autoMap_data()
{
    QTest::addColumn<QString>("directory");

    QTest::newRow("simple-replace") << QStringLiteral("simple-replace");
    QTest::newRow("simple-2x2-rule") << QStringLiteral("simple-2x2-rule");
    QTest::newRow("option-no-overlapping-rules") << QStringLiteral("option-no-overlapping-rules");
    QTest::newRow("option-overflow-border") << QStringLiteral("option-overflow-border");
    QTest::newRow("option-wrap-border") << QStringLiteral("option-wrap-border");
    QTest::newRow("terrain-corner") << QStringLiteral("terrain-corner");
}

void test_AutoMapping::autoMap()
{
    QFETCH(QString, directory);

    MapReader reader;
    auto inputMap = reader.readMap(directory + QStringLiteral("/map.tmx"));
    auto rulesMap = reader.readMap(directory + QStringLiteral("/rules.tmx"));
    auto resultMap = reader.readMap(directory + QStringLiteral("/map-result.tmx"));

    QVERIFY(inputMap.get());
    QVERIFY(resultMap.get());
    QVERIFY(rulesMap.get());

    MapDocument mapDocument(std::move(inputMap));
    AutoMapper autoMapper(&mapDocument, std::move(rulesMap), QStringLiteral("rules.tmx"));

    const QSize mapSize = mapDocument.map()->size();
    autoMapper.prepareAutoMap();
    autoMapper.autoMap(QRect(QPoint(), mapSize), nullptr);  // todo: test appliedRegion as well
    autoMapper.finalizeAutoMap();

    QCOMPARE(mapDocument.map()->layerCount(), resultMap->layerCount());

    for (int i = 0; i < resultMap->layerCount(); ++i) {
        const Layer *resultLayer = resultMap->layerAt(i);
        const Layer *mapLayer = mapDocument.map()->layerAt(i);
        QCOMPARE(resultLayer->layerType(), mapLayer->layerType());
        QCOMPARE(resultLayer->name(), mapLayer->name());
        if (resultLayer->layerType() != Layer::TileLayerType)
            continue;

        const TileLayer *resultTileLayer = static_cast<const TileLayer*>(resultLayer);
        const TileLayer *mapTileLayer = static_cast<const TileLayer*>(mapLayer);

        QCOMPARE(mapTileLayer->region(), resultTileLayer->region());

        for (auto it = resultTileLayer->begin(); it != resultTileLayer->end(); ++it)
            QCOMPARE(it.value(), mapTileLayer->cellAt(it.key()));
    }
}

QTEST_MAIN(test_AutoMapping)
#include "test_automapping.moc"
