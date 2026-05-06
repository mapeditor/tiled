#include "map.h"
#include "mapobject.h"
#include "mapwriter.h"
#include "objectgroup.h"
#include "propertytype.h"
#include "tilelayer.h"
#include "mapreader.h"

#include <QBuffer>
#include <QtTest/QtTest>

using namespace Tiled;

class test_MapReader : public QObject
{
    Q_OBJECT

private slots:
    void loadMap();
    void roundTripListProperties();
};

void test_MapReader::loadMap()
{
    MapReader reader;
    auto map = reader.readMap("../data/mapobject.tmx");

    // TODO: Also test tilesets (internal and external), properties and tile
    // layer data.

    QVERIFY(map.get());
    QCOMPARE(map->layerCount(), 2);
    QCOMPARE(map->width(), 100);
    QCOMPARE(map->height(), 80);
    QCOMPARE(map->tileWidth(), 32);
    QCOMPARE(map->tileHeight(), 32);

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(map->layerAt(0));

    QVERIFY(tileLayer);
    QCOMPARE(tileLayer->width(), 100);
    QCOMPARE(tileLayer->height(), 80);

    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(map->layerAt(1));

    QVERIFY(objectGroup);
    QCOMPARE(objectGroup->name(), QLatin1String("Objects"));
    QCOMPARE(objectGroup->objects().count(), 1);

    MapObject *mapObject = objectGroup->objects().at(0);

    QCOMPARE(mapObject->name(), QLatin1String("Some object"));
    QCOMPARE(mapObject->className(), QLatin1String("WARP"));
    QCOMPARE(mapObject->x(), qreal(200));
    QCOMPARE(mapObject->y(), qreal(200));
    QCOMPARE(mapObject->width(), qreal(128));
    QCOMPARE(mapObject->height(), qreal(64));
}

void test_MapReader::roundTripListProperties()
{
    auto classWithList = SharedPropertyType(new ClassPropertyType(QStringLiteral("ClassWithList")));
    classWithList->id = 1;
    static_cast<ClassPropertyType&>(*classWithList).members
        .insert(QStringLiteral("items"), QVariantList());

    SharedPropertyTypes types(new PropertyTypes());
    types->add(classWithList);
    Object::setPropertyTypes(types);

    Map map(Map::Parameters{});
    Properties properties;
    properties.insert(QStringLiteral("ParallaxLayers"),
                      QVariantList { QStringLiteral("layer1"), QStringLiteral("layer2") });

    QVariantMap classValue;
    classValue.insert(QStringLiteral("items"),
                      QVariantList { QStringLiteral("x"), QStringLiteral("y") });
    properties.insert(QStringLiteral("Container"), classWithList->wrap(classValue));

    map.setProperties(properties);

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    MapWriter writer;
    writer.writeMap(&map, &buffer);

    buffer.seek(0);
    MapReader reader;
    auto loaded = reader.readMap(&buffer, QString());
    QVERIFY(loaded.get());
    QCOMPARE(loaded->properties(), properties);

    Object::setPropertyTypes(SharedPropertyTypes(new PropertyTypes()));
}

QTEST_MAIN(test_MapReader)
#include "test_mapreader.moc"
