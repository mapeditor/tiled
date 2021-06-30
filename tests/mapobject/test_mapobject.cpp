#include "mapobject.h"
#include "objecttemplate.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_MapObject : public QObject
{
    Q_OBJECT

private slots:
    void test_syncWithTemplate();
};


void test_MapObject::test_syncWithTemplate()
{
    MapObject obj;
    Properties props;
    props["hp"] = 50;
    obj.addComponent("component1", props);
    ObjectTemplate templateObj;
    templateObj.setObject(&obj);

    MapObject mapObject;
    mapObject.setObjectTemplate(&templateObj);

    mapObject.syncWithTemplate();
    QCOMPARE(mapObject.componentProperties("component1")["hp"], 50);

    Properties props2;
    props2["volume"] = 60;
    obj.addComponent("component2", props2);
    templateObj.setObject(&obj);

    mapObject.syncWithTemplate();
    QCOMPARE(mapObject.componentProperties("component2")["volume"], 60);
}

QTEST_MAIN(test_MapObject)
#include "test_mapobject.moc"
