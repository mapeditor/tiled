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

    void screenToTileCoords_data();
    void screenToTileCoords();

    void tileToScreenCoords_data();
    void tileToScreenCoords();

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
    mMap = nullptr;
}

void test_StaggeredRenderer::mapSize()
{
    StaggeredRenderer renderer(mMap);

    QRect mapBoundingRect = renderer.mapBoundingRect();

    TileLayer *tileLayer = static_cast<TileLayer*>(mMap->layerAt(0));

    QCOMPARE(mapBoundingRect, QRect(0, 0, 10 * 64 + 32, 10 * 16 + 16));
    QCOMPARE(renderer.boundingRect(tileLayer->rect()), mapBoundingRect);
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

void test_StaggeredRenderer::screenToTileCoords_data()
{
    QTest::addColumn<QPointF>("screenCoords");
    QTest::addColumn<QPointF>("tileCoords");

    QTest::newRow("10,16") << QPointF(10, 16) << QPointF(0, 0);
    QTest::newRow("5,5")   << QPointF(5, 5)   << QPointF(-1, -1);
    QTest::newRow("1,20")  << QPointF(1, 20)  << QPointF(-1, 1);
    QTest::newRow("64,32") << QPointF(64, 32) << QPointF(0, 1);
    QTest::newRow("32,-16") << QPointF(32, -16) << QPointF(0, -2);
}

void test_StaggeredRenderer::screenToTileCoords()
{
    QFETCH(QPointF, screenCoords);
    QFETCH(QPointF, tileCoords);

    StaggeredRenderer renderer(mMap);

    QPointF point = renderer.screenToTileCoords(screenCoords);

    QCOMPARE(QPoint(qFloor(point.x()), qFloor(point.y())), tileCoords.toPoint());
}

void test_StaggeredRenderer::tileToScreenCoords_data()
{
    QTest::addColumn<QPointF>("tileCoords");
    QTest::addColumn<QPointF>("screenCoords");

    QTest::newRow("0,0") << QPointF(0, 0) << QPointF(0, 0);
    QTest::newRow("1,0") << QPointF(1, 0) << QPointF(64, 0);
    QTest::newRow("0,1") << QPointF(0, 1) << QPointF(32, 16);
    QTest::newRow("-1,-1") << QPointF(-1, -1) << QPointF(-32, -16);
}

void test_StaggeredRenderer::tileToScreenCoords()
{
    QFETCH(QPointF, tileCoords);
    QFETCH(QPointF, screenCoords);

    StaggeredRenderer renderer(mMap);
    QCOMPARE(renderer.tileToScreenCoords(tileCoords), screenCoords);
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
