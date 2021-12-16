#include "properties.h"

#include <QtTest/QtTest>

using namespace Tiled;

class test_Properties : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void loadProperties();
    void saveProperties();

    void cleanupTestCase();

private:
    PropertyTypes mTypes;
};

void test_Properties::initTestCase()
{
    int nextId = 0;

    auto customEnumType = std::make_unique<EnumPropertyType>(QStringLiteral("CustomEnum"));
    customEnumType->values = QStringList { "A", "B", "C" };
    customEnumType->id = ++nextId;
    auto aValue = customEnumType->wrap(0);
    auto bValue = customEnumType->wrap(1);
    mTypes.add(std::move(customEnumType));

    // todo: test enums saved as string
    // todo: test enum values used as flags

    auto customClassType = std::make_unique<ClassPropertyType>(QStringLiteral("CustomClass"));
    customClassType->id = ++nextId;
    customClassType->members.insert(QStringLiteral("enumA"), aValue);
    customClassType->members.insert(QStringLiteral("enumB"), bValue);
    mTypes.add(std::move(customClassType));
}

void test_Properties::loadProperties()
{
    ExportContext context(mTypes, QString());

    const auto customEnumType = mTypes.findTypeByName(QStringLiteral("CustomEnum"));
    QVERIFY(customEnumType);
    auto aValue = customEnumType->wrap(0);
    auto bValue = customEnumType->wrap(1);

    ExportValue enumValue;
    enumValue.propertyTypeName = QStringLiteral("CustomEnum");
    enumValue.typeName = QStringLiteral("string");
    enumValue.value = QStringLiteral("B");

    ExportValue classValue;
    classValue.propertyTypeName = QStringLiteral("CustomClass");
    classValue.typeName = QStringLiteral("class");
    classValue.value = QVariantMap {
        { QStringLiteral("enumA"), QVariant::fromValue(2) },
    };

    // todo: test a class with nested class

    auto eProp = context.toPropertyValue(enumValue);
    QCOMPARE(eProp.value<PropertyValue>().value, QVariant::fromValue(1));
    QCOMPARE(eProp.value<PropertyValue>().typeId, 1);

    auto cProp = context.toPropertyValue(classValue);
    QCOMPARE(cProp.value<PropertyValue>().typeId, 2);

    auto cValue = cProp.value<PropertyValue>().value.toMap();
    auto cValueEnumA = cValue.value(QStringLiteral("enumA")).value<PropertyValue>();
    QCOMPARE(cValueEnumA.typeId, 1);
    QCOMPARE(cValueEnumA.value, QVariant::fromValue(2));
}

void test_Properties::saveProperties()
{
    ExportContext context(mTypes, QString());

    auto value = QVariant::fromValue(ObjectRef { 10 });

    auto exportValue = context.toExportValue(value);
    QCOMPARE(exportValue.value, QVariant::fromValue(10));
    QCOMPARE(exportValue.typeName, QStringLiteral("object"));
    QCOMPARE(exportValue.propertyTypeName, QString());

    // todo: test saving a custom class
}

void test_Properties::cleanupTestCase()
{
    mTypes.clear();
}

QTEST_MAIN(test_Properties)
#include "test_properties.moc"
