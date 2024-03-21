#include "map.h"
#include "tilelayer.h"
#include "mapreader.h"

#include "automapper.h"
#include "mapdocument.h"

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

    QTest::newRow("ignore-flip") << QStringLiteral("ignore-flip");
    QTest::newRow("infinite-target-map") << QStringLiteral("infinite-target-map");
    QTest::newRow("inputnot") << QStringLiteral("inputnot");
    QTest::newRow("match-type") << QStringLiteral("match-type");
    QTest::newRow("mod-and-offset") << QStringLiteral("mod-and-offset");
    QTest::newRow("option-ignore-lock") << QStringLiteral("option-ignore-lock");
    QTest::newRow("option-no-overlapping-output") << QStringLiteral("option-no-overlapping-output");
    QTest::newRow("option-overflow-border") << QStringLiteral("option-overflow-border");
    QTest::newRow("option-wrap-border") << QStringLiteral("option-wrap-border");
    QTest::newRow("simple-2x2-rule") << QStringLiteral("simple-2x2-rule");
    QTest::newRow("simple-replace") << QStringLiteral("simple-replace");
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
    AutoMapper autoMapper(std::move(rulesMap));
    AutoMappingContext context(&mapDocument);

    QRegion region;

    if (mapDocument.map()->infinite()) {
        LayerIterator iterator(mapDocument.map());

        QRect bounds;
        while (Layer *layer = iterator.next()) {
            if (auto *tileLayer = dynamic_cast<TileLayer*>(layer))
                bounds = bounds.united(tileLayer->bounds());
        }
        region = bounds;
    } else {
        region = QRect(QPoint(), mapDocument.map()->size());
    }

    autoMapper.prepareAutoMap(context);
    QBENCHMARK {
        autoMapper.autoMap(region, nullptr, context);  // todo: test appliedRegion as well
    }

    // Apply the changes done by AutoMapping (only checking tile layers for now)
    for (auto& [original, outputLayer] : context.originalToOutputLayerMapping) {
        const QRegion diffRegion = original->computeDiffRegion(*outputLayer);
        original->setCells(0, 0, outputLayer.get(), diffRegion);
    }
    for (auto &layer : context.newLayers)
        if (!layer->isEmpty())
            mapDocument.map()->addLayer(std::move(layer));

    QCOMPARE(mapDocument.map()->layerCount(), resultMap->layerCount());

    for (int i = 0; i < resultMap->layerCount(); ++i) {
        const Layer *resultLayer = resultMap->layerAt(i);
        const Layer *mapLayer = mapDocument.map()->layerAt(i);
        QCOMPARE(resultLayer->layerType(), mapLayer->layerType());
        QCOMPARE(resultLayer->name(), mapLayer->name());
        if (resultLayer->layerType() != Layer::TileLayerType)
            continue;

        const auto *resultTileLayer = static_cast<const TileLayer*>(resultLayer);
        const auto *mapTileLayer = static_cast<const TileLayer*>(mapLayer);

        QCOMPARE(mapTileLayer->region(), resultTileLayer->region());

        for (auto it = resultTileLayer->begin(); it != resultTileLayer->end(); ++it) {
            const QPoint pos = it.key();
            const Cell &expected = it.value();
            const Cell &seen = mapTileLayer->cellAt(pos);
            QCOMPARE(seen, expected);
        }
    }
}

QTEST_MAIN(test_AutoMapping)
#include "test_automapping.moc"
