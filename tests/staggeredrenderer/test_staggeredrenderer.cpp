#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "staggeredrenderer.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_StaggeredRenderer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void mapSize();

    void boundingRect_data();
    void boundingRect();

    void pixelToTileCoords_data();
    void pixelToTileCoords();

    void tileToPixelCoords_data();
    void tileToPixelCoords();

    void relativeCoordinates();

private:
    Map *mMap;
};

void test_StaggeredRenderer::initTestCase()
{
    mMap = new Map(Map::Staggered, 10, 10, 64, 32);
    TileLayer *tileLayer = new TileLayer(QString(),
                                         0, 0,
                                         mMap->width(), mMap->height());
    mMap->addLayer(tileLayer);
}

void test_StaggeredRenderer::cleanupTestCase()
{
    delete mMap;
    mMap = 0;
}

void test_StaggeredRenderer::mapSize()
{
    StaggeredRenderer renderer(mMap);

    TileLayer *tileLayer = static_cast<TileLayer*>(mMap->layerAt(0));
    QSize mapSize = renderer.mapSize();

    QCOMPARE(mapSize, QSize(10 * 64 + 32, 10 * 16 + 16));
    QCOMPARE(renderer.boundingRect(tileLayer->bounds()),
             QRect(QPoint(), mapSize));
}

void test_StaggeredRenderer::boundingRect_data()
{
    QTest::addColumn<QRect>("tileRect");
    QTest::addColumn<QRect>("boundingRect");

    QTest::newRow("0,0 1x1") << QRect(0, 0, 1, 1)
                             << QRect(0, 0, 64, 32);

    QTest::newRow("0,1 2x2") << QRect(0, 1, 2, 2)
                             << QRect(0, 16, 64 * 2 + 32, 32 + 16);

    QTest::newRow("1,1 3x3") << QRect(1, 1, 3, 3)
                             << QRect(64, 16, 64 * 3 + 32, 32 * 2);
}

void test_StaggeredRenderer::boundingRect()
{
    QFETCH(QRect, tileRect);
    QFETCH(QRect, boundingRect);

    StaggeredRenderer renderer(mMap);
    QCOMPARE(renderer.boundingRect(tileRect), boundingRect);
}

void test_StaggeredRenderer::pixelToTileCoords_data()
{
    QTest::addColumn<QPointF>("pixelCoords");
    QTest::addColumn<QPointF>("tileCoords");

    QTest::newRow("10,16") << QPointF(10, 16) << QPointF(0, 0);
    QTest::newRow("5,5")   << QPointF(5, 5)   << QPointF(-1, -1);
    QTest::newRow("1,20")  << QPointF(1, 20)  << QPointF(-1, 1);
    QTest::newRow("64,32") << QPointF(64, 32) << QPointF(0, 1);
    QTest::newRow("32,-16") << QPointF(32, -16) << QPointF(0, -2);
}

void test_StaggeredRenderer::pixelToTileCoords()
{
    QFETCH(QPointF, pixelCoords);
    QFETCH(QPointF, tileCoords);

    StaggeredRenderer renderer(mMap);
    QCOMPARE(renderer.pixelToTileCoords(pixelCoords), tileCoords);
}

void test_StaggeredRenderer::tileToPixelCoords_data()
{
    QTest::addColumn<QPointF>("tileCoords");
    QTest::addColumn<QPointF>("pixelCoords");

    QTest::newRow("0,0") << QPointF(0, 0) << QPointF(0, 0);
    QTest::newRow("1,0") << QPointF(1, 0) << QPointF(64, 0);
    QTest::newRow("0,1") << QPointF(0, 1) << QPointF(32, 16);
    QTest::newRow("-1,-1") << QPointF(-1, -1) << QPointF(-32, -16);
}

void test_StaggeredRenderer::tileToPixelCoords()
{
    QFETCH(QPointF, tileCoords);
    QFETCH(QPointF, pixelCoords);

    StaggeredRenderer renderer(mMap);
    QCOMPARE(renderer.tileToPixelCoords(tileCoords), pixelCoords);
}

void test_StaggeredRenderer::relativeCoordinates()
{
    StaggeredRenderer renderer(mMap);

    QCOMPARE(renderer.topLeft(0, 0), QPoint(-1, -1));
    QCOMPARE(renderer.topRight(0, 0), QPoint(0, -1));
    QCOMPARE(renderer.bottomLeft(0, 0), QPoint(-1, 1));
    QCOMPARE(renderer.bottomRight(0, 0), QPoint(0, 1));

    QCOMPARE(renderer.topLeft(1, 1), QPoint(1, 0));
    QCOMPARE(renderer.topRight(1, 1), QPoint(2, 0));
    QCOMPARE(renderer.bottomLeft(1, 1), QPoint(1, 2));
    QCOMPARE(renderer.bottomRight(1, 1), QPoint(2, 2));
}

QTEST_MAIN(test_StaggeredRenderer)
#include "test_staggeredrenderer.moc"
