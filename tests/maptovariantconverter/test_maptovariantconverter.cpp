#include "map.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "imagelayer.h"
#include "grouplayer.h"
#include "maptovariantconverter.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_MapToVariantConverter : public QObject
{
    Q_OBJECT

private slots:
    void mapToVariant();
    void tilesetToVariant();

private:
    void assignComponent(Object *obj);
    void assertComponent(const QVariantMap &output);
};

void test_MapToVariantConverter::mapToVariant()
{
    MapToVariantConverter converter;

    Map *map = new Map();
    assignComponent(map);

    map->addLayer(new ObjectGroup());
    map->addLayer(new TileLayer());
    map->addLayer(new ImageLayer("", 0, 0));
    map->addLayer(new GroupLayer("", 0, 0));

    for (Layer *layer : map->layers())
        assignComponent(layer);

    const QVariant out = converter.toVariant(*map, QDir());
    QVERIFY(out.isValid());

    const QVariantMap outmap = out.toMap();
    assertComponent(outmap);

    const QVariantList layers = outmap["layers"].toList();
    QVERIFY(layers.size() > 0);

    for (const QVariant &layer : layers) {
        const QVariantMap layermap = layer.toMap();
        assertComponent(layermap);
    }
}

void test_MapToVariantConverter::tilesetToVariant()
{
    MapToVariantConverter converter;

    SharedTileset tileset = Tileset::create("", 32, 32);
    assignComponent(tileset.data());

    QVariant out = converter.toVariant(*tileset, QDir());
    QVariantMap outts = out.toMap();

    QVERIFY(out.isValid());

    assertComponent(outts);
}

void test_MapToVariantConverter::assignComponent(Object *obj)
{
    Properties properties;
    properties["test_prop"] = 1;
    obj->addComponent(QStringLiteral("ComponentName"), properties);
}

void test_MapToVariantConverter::assertComponent(const QVariantMap &output)
{
    auto const &component = output["components"].toList()[0].toMap();
    QCOMPARE(component["name"], "ComponentName");
    QCOMPARE(component["properties"].toList()[0].toMap()["name"].toString(), "test_prop");
    QCOMPARE(component["properties"].toList()[0].toMap()["value"].toInt(), 1);
}

QTEST_MAIN(test_MapToVariantConverter)
#include "test_maptovariantconverter.moc"
