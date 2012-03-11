#include "properties.h"

#include <QtTest/QtTest>
#include <iostream>
#include <QString>
#include <QMap>


using namespace Tiled;

class test_Properties : public QObject
{
    Q_OBJECT

private slots:
    void test1();
};

void test_Properties::test1()
{
    Properties properties1, properties2, properties3, properties4;

    properties1.insert("a","b");
    properties2.merge(properties1);
	QCOMPARE(properties1.keys(), properties2.keys());
	QCOMPARE(properties1.values(), properties2.values());
}

QTEST_MAIN(test_Properties)
#include "test_properties.moc"
