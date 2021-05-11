#include "map.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "imagelayer.h"
#include "grouplayer.h"
#include "varianttomapconverter.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_VariantToMapConverter : public QObject
{
    Q_OBJECT

private slots:
    void toMap();

private:
    void assignComponents(QVariantMap &map);
};

void test_VariantToMapConverter::toMap()
{
    VariantToMapConverter converter;

    QVariantMap mapVariant;
    mapVariant["orientation"] = "orthogonal";

    assignComponents(mapVariant);

    auto map = converter.toMap(mapVariant, QDir());

    QVERIFY(map.get());
    QVERIFY(!map->components().isEmpty());
}

void test_VariantToMapConverter::assignComponents(QVariantMap &map)
{
    QVariantMap property;
    property["name"] = "test_prop";
    property["value"] = 123;

    QVariantList properties;
    properties << property;

    QVariantMap component;
    component["name"] = "ComponentName";
    component["properties"] = properties;

    QVariantList components;
    components << component;

    map["components"] = components;
}

QTEST_MAIN(test_VariantToMapConverter)
#include "test_varianttomapconverter.moc"
