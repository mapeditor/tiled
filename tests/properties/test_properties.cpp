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
    EnumPropertyType &addEnum(const QString &name);
    ClassPropertyType &addClass(const QString &name);

    int mNextId = 0;
    PropertyTypes mTypes;
};

void test_Properties::initTestCase()
{
    auto &enumStringType = addEnum(QStringLiteral("EnumString"));
    enumStringType.storageType = EnumPropertyType::StringValue;
    enumStringType.values = QStringList { "A", "B", "C" };
    auto stringValue = enumStringType.wrap(0);

    auto &enumIntType = addEnum(QStringLiteral("EnumInt"));
    enumIntType.storageType = EnumPropertyType::IntValue;
    enumIntType.values = QStringList { "A", "B", "C" };
    auto intValue = enumIntType.wrap(0);

    auto &enumFlagsStringType = addEnum(QStringLiteral("EnumFlagsString"));
    enumFlagsStringType.storageType = EnumPropertyType::StringValue;
    enumFlagsStringType.values = QStringList { "A", "B", "C" };
    enumFlagsStringType.valuesAsFlags = true;
    auto flagsStringValue = enumFlagsStringType.wrap(0);

    auto &enumFlagsIntType = addEnum(QStringLiteral("EnumFlagsInt"));
    enumFlagsIntType.storageType = EnumPropertyType::IntValue;
    enumFlagsIntType.values = QStringList { "A", "B", "C" };
    enumFlagsIntType.valuesAsFlags = true;
    auto flagsIntValue = enumFlagsIntType.wrap(0);

    auto &classType = addClass(QStringLiteral("ClassWithEnums"));
    classType.members.insert(QStringLiteral("enumString"), stringValue);
    classType.members.insert(QStringLiteral("enumInt"), intValue);
    classType.members.insert(QStringLiteral("enumFlagsString"), flagsStringValue);
    classType.members.insert(QStringLiteral("enumFlagsInt"), flagsIntValue);
}

void test_Properties::loadProperties()
{
    ExportContext context(mTypes, QString());

    const auto enumStringType = mTypes.findTypeByName(QStringLiteral("EnumString"));
    const auto classType = mTypes.findTypeByName(QStringLiteral("ClassWithEnums"));
    QVERIFY(enumStringType);
    QVERIFY(classType);

    ExportValue enumValue;
    enumValue.propertyTypeName = QStringLiteral("EnumString");
    enumValue.typeName = QStringLiteral("string");
    enumValue.value = QStringLiteral("B");

    auto eProp = context.toPropertyValue(enumValue);
    QCOMPARE(eProp.value<PropertyValue>().value, QVariant::fromValue(1));
    QCOMPARE(eProp.value<PropertyValue>().typeId, enumStringType->id);

    ExportValue classValue;
    classValue.propertyTypeName = QStringLiteral("ClassWithEnums");
    classValue.typeName = QStringLiteral("class");
    classValue.value = QVariantMap {
        { QStringLiteral("enumString"), QVariant::fromValue(QStringLiteral("B")) },
        { QStringLiteral("enumInt"), QVariant::fromValue(1) },
        { QStringLiteral("enumFlagsString"), QVariant::fromValue(QStringLiteral("B,C")) },
        { QStringLiteral("enumFlagsInt"), QVariant::fromValue(2 | 4) },
    };

    auto cProp = context.toPropertyValue(classValue);
    QCOMPARE(cProp.value<PropertyValue>().typeId, classType->id);

    auto cValue = cProp.value<PropertyValue>().value.toMap();
    auto cValueEnumString = cValue.value(QStringLiteral("enumString")).value<PropertyValue>();
    auto cValueEnumInt = cValue.value(QStringLiteral("enumInt")).value<PropertyValue>();
    auto cValueEnumFlagsString = cValue.value(QStringLiteral("enumFlagsString")).value<PropertyValue>();
    auto cValueEnumFlagsInt = cValue.value(QStringLiteral("enumFlagsInt")).value<PropertyValue>();
    QCOMPARE(cValueEnumString.typeId, enumStringType->id);
    QCOMPARE(cValueEnumString.value, QVariant::fromValue(1));
    QCOMPARE(cValueEnumInt.value, QVariant::fromValue(1));
    QCOMPARE(cValueEnumFlagsString.value, QVariant::fromValue(2 | 4));
    QCOMPARE(cValueEnumFlagsInt.value, QVariant::fromValue(2 | 4));

    // todo: test loading a class with nested class
}

void test_Properties::saveProperties()
{
    ExportContext context(mTypes, QString());

    const auto enumStringType = mTypes.findTypeByName(QStringLiteral("EnumString"));
    const auto classType = mTypes.findTypeByName(QStringLiteral("ClassWithEnums"));
    QVERIFY(enumStringType);
    QVERIFY(classType);

    auto objectRefValue = QVariant::fromValue(ObjectRef { 10 });

    auto objectRefExportValue = context.toExportValue(objectRefValue);
    QCOMPARE(objectRefExportValue.value, QVariant::fromValue(10));
    QCOMPARE(objectRefExportValue.typeName, QStringLiteral("object"));
    QCOMPARE(objectRefExportValue.propertyTypeName, QString());

    auto enumStringValue = enumStringType->wrap(1);
    auto enumStringExportValue = context.toExportValue(enumStringValue);
    QCOMPARE(enumStringExportValue.value, QVariant::fromValue(QStringLiteral("B")));
    QCOMPARE(enumStringExportValue.typeName, QStringLiteral("string"));
    QCOMPARE(enumStringExportValue.propertyTypeName, enumStringType->name);

    // todo: test converting other enum property types to export values

    auto classValue = classType->wrap(classType->defaultValue());
    auto classExportValue = context.toExportValue(classValue);
    QCOMPARE(classExportValue.value, QVariant::fromValue(QVariantMap()));
    QCOMPARE(classExportValue.typeName, QStringLiteral("class"));
    QCOMPARE(classExportValue.propertyTypeName, classType->name);

    // todo: test saving a class with nested class
}

void test_Properties::cleanupTestCase()
{
    mTypes.clear();
    mNextId = 0;
}

EnumPropertyType &test_Properties::addEnum(const QString &name)
{
    auto &type = mTypes.add(std::make_unique<EnumPropertyType>(name));
    type.id = ++mNextId;
    return static_cast<EnumPropertyType&>(type);
}

ClassPropertyType &test_Properties::addClass(const QString &name)
{
    auto &type = mTypes.add(std::make_unique<ClassPropertyType>(name));
    type.id = ++mNextId;
    return static_cast<ClassPropertyType&>(type);
}

QTEST_MAIN(test_Properties)
#include "test_properties.moc"
