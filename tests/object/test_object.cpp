#include "object.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_Object : public QObject
{
    Q_OBJECT

private slots:
    void test_commonComponents_empty();
    void test_commonComponents();
};



void test_Object::test_commonComponents_empty()
{
    QList<Object *> list;
    QSet<QString> set = Object::commonComponents(list, true);
    QVERIFY(set.empty());
}

void test_Object::test_commonComponents()
{
    ObjectType type1 = ObjectType("component1", QColor(Qt::red));
    ObjectType type2 = ObjectType("component2", QColor(Qt::red));
    ObjectType type3 = ObjectType("component3", QColor(Qt::red));
    ObjectType type4 = ObjectType("component4", QColor(Qt::red));

    ObjectTypes types;
    types << type1 << type2 << type3 << type4;

    Object::setObjectTypes(types);

    Object* o1 = new Object(Object::MapObjectType);
    Object* o2 = new Object(Object::MapObjectType);

    o1->addComponent("component1", Properties {});
    o1->addComponent("component2", Properties {});
    o2->addComponent("component2", Properties {});
    o2->addComponent("component3", Properties {});

    QList<Object *> objects;
    objects << o1 << o2;

    QSet<QString> common = Object::commonComponents(objects);
    QVERIFY(common.size() == 1);
    QVERIFY(common.contains("component2"));

    common = Object::commonComponents(objects, true);
    QVERIFY(common.size() == 1);
    QVERIFY(common.contains("component4"));
}

QTEST_MAIN(test_Object)
#include "test_object.moc"
