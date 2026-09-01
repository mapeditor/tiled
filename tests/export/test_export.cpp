#include "exportasimagedialog.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_Export : public QObject
{
    Q_OBJECT

private slots:
    void selectedAreaImageRect_data();
    void selectedAreaImageRect();
};

void test_Export::selectedAreaImageRect_data()
{
    QTest::addColumn<QRect>("mapBounds");
    QTest::addColumn<QRect>("selectionBounds");
    QTest::addColumn<QSize>("imageSize");
    QTest::addColumn<QRect>("expected");

    QTest::newRow("regular-selection")
        << QRect(0, 0, 100, 80)
        << QRect(10, 20, 30, 40)
        << QSize(200, 160)
        << QRect(20, 40, 60, 80);

    QTest::newRow("non-zero-origin")
        << QRect(50, 40, 100, 80)
        << QRect(60, 50, 20, 30)
        << QSize(300, 240)
        << QRect(30, 30, 60, 90);

    QTest::newRow("empty-selection")
        << QRect(0, 0, 100, 80)
        << QRect()
        << QSize(200, 160)
        << QRect();
}

void test_Export::selectedAreaImageRect()
{
    QFETCH(QRect, mapBounds);
    QFETCH(QRect, selectionBounds);
    QFETCH(QSize, imageSize);
    QFETCH(QRect, expected);

    const QRect result = ExportAsImageDialog::selectedAreaImageRect(mapBounds, selectionBounds, imageSize);
    QCOMPARE(result, expected);
}

QTEST_MAIN(test_Export)
#include "test_export.moc"
