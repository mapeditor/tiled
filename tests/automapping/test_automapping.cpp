#include "map.h"
#include "tilelayer.h"
#include "mapreader.h"

#include "automapper.h"
#include "mapdocument.h"

#include <QtTest/QtTest>

using namespace Tiled;

namespace {

// Helper to get a string representation of a cell (tile ID or '.' for empty)
QString cellToString(const Cell &cell)
{
    if (cell.isEmpty())
        return QStringLiteral(".");
    return QString::number(cell.tileId() + 1); // +1 to match TMX firstgid=1
}

// Print two tile layers side by side for comparison
void printLayerComparison(const TileLayer *actual, const TileLayer *expected, const QString &layerName)
{
    const QRect actualBounds = actual->bounds();
    const QRect expectedBounds = expected->bounds();
    const QRect combinedBounds = actualBounds.united(expectedBounds);

    // Find rows and columns that have any content or differences
    QVector<int> activeRows, activeCols;
    for (int y = combinedBounds.top(); y <= combinedBounds.bottom(); ++y) {
        for (int x = combinedBounds.left(); x <= combinedBounds.right(); ++x) {
            const bool hasActual = !actual->cellAt(x, y).isEmpty();
            const bool hasExpected = !expected->cellAt(x, y).isEmpty();
            if (hasActual || hasExpected) {
                if (activeRows.isEmpty() || activeRows.last() != y)
                    activeRows.append(y);
                if (!activeCols.contains(x))
                    activeCols.append(x);
            }
        }
    }
    std::sort(activeCols.begin(), activeCols.end());

    if (activeRows.isEmpty()) {
        qDebug() << "";
        qDebug() << "=== Layer:" << layerName << "=== (both empty)";
        return;
    }

    const int cellWidth = 4; // Width per cell for formatting

    qDebug() << "";
    qDebug() << "=== Layer:" << layerName << "===";
    qDebug() << "";

    // Header
    const int width = activeCols.size();
    QString header = QStringLiteral("     ACTUAL");
    header += QString(qMax(0, width * cellWidth - 6), ' ');
    header += QStringLiteral("| EXPECTED");
    qDebug().noquote() << header;

    // Column numbers
    QString colNums = QStringLiteral("  ");
    for (int x : activeCols)
        colNums += QString::number(x).rightJustified(cellWidth);
    colNums += QStringLiteral(" |");
    for (int x : activeCols)
        colNums += QString::number(x).rightJustified(cellWidth);
    qDebug().noquote() << colNums;

    QString separator = QStringLiteral("  ") + QString((width * cellWidth) + 1, '-') + QStringLiteral("+") + QString(width * cellWidth + 1, '-');
    qDebug().noquote() << separator;

    // Rows
    for (int y : activeRows) {
        QString row = QString::number(y).rightJustified(2);

        // Actual side
        for (int x : activeCols) {
            const Cell &cell = actual->cellAt(x, y);
            row += cellToString(cell).rightJustified(cellWidth);
        }

        row += QStringLiteral(" |");

        // Expected side
        for (int x : activeCols) {
            const Cell &cell = expected->cellAt(x, y);
            row += cellToString(cell).rightJustified(cellWidth);
        }

        qDebug().noquote() << row;
    }

    qDebug() << "";

    // Print differences
    qDebug() << "Differences:";
    bool hasDiff = false;
    for (int y : activeRows) {
        for (int x : activeCols) {
            const Cell &actualCell = actual->cellAt(x, y);
            const Cell &expectedCell = expected->cellAt(x, y);
            if (actualCell != expectedCell) {
                hasDiff = true;
                qDebug().noquote() << QString("  (%1,%2): actual=%3, expected=%4")
                    .arg(x).arg(y)
                    .arg(cellToString(actualCell))
                    .arg(cellToString(expectedCell));
            }
        }
    }
    if (!hasDiff)
        qDebug() << "  (none)";
    qDebug() << "";
}

} // anonymous namespace

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

    QTest::newRow("ignore-empty-region") << QStringLiteral("ignore-empty-region");
    QTest::newRow("ignore-flip") << QStringLiteral("ignore-flip");
    QTest::newRow("infinite-target-map") << QStringLiteral("infinite-target-map");
    QTest::newRow("inputnot") << QStringLiteral("inputnot");
    QTest::newRow("input-no-index-simple") << QStringLiteral("input-no-index-simple");
    QTest::newRow("match-type") << QStringLiteral("match-type");
    QTest::newRow("mod-and-offset") << QStringLiteral("mod-and-offset");
    QTest::newRow("option-ignore-lock") << QStringLiteral("option-ignore-lock");
    QTest::newRow("option-no-overlapping-output") << QStringLiteral("option-no-overlapping-output");
    QTest::newRow("option-overflow-border") << QStringLiteral("option-overflow-border");
    QTest::newRow("option-wrap-border") << QStringLiteral("option-wrap-border");
    QTest::newRow("simple-2x2-rule") << QStringLiteral("simple-2x2-rule");
    QTest::newRow("simple-replace") << QStringLiteral("simple-replace");
    QTest::newRow("terrain-corner") << QStringLiteral("terrain-corner");
    QTest::newRow("wildcard-single-layer") << QStringLiteral("wildcard-single-layer");
    QTest::newRow("wildcard-two-layers") << QStringLiteral("wildcard-two-layers");
    QTest::newRow("wildcard-missing-required-layer") << QStringLiteral("wildcard-missing-required-layer");
    QTest::newRow("wildcard-multiple-wildcards") << QStringLiteral("wildcard-multiple-wildcards");
    QTest::newRow("wildcard-cross-layer") << QStringLiteral("wildcard-cross-layer");
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

        // Check if layers differ and print comparison
        bool layersMatch = (mapTileLayer->region() == resultTileLayer->region());
        if (layersMatch) {
            for (auto it = resultTileLayer->begin(); it != resultTileLayer->end(); ++it) {
                const QPoint pos = it.key();
                if (mapTileLayer->cellAt(pos) != it.value()) {
                    layersMatch = false;
                    break;
                }
            }
        }

        if (!layersMatch) {
            printLayerComparison(mapTileLayer, resultTileLayer, mapLayer->name());
            QFAIL(qPrintable(QStringLiteral("Layer '%1' does not match expected result").arg(mapLayer->name())));
        }
    }
}

QTEST_MAIN(test_AutoMapping)
#include "test_automapping.moc"
