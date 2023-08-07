#include "properties.h"

#include <QtTest/QtTest>

using namespace Tiled;

constexpr auto propertyTypesJson = R"([
    {
        "id": 1,
        "name": "EnumString",
        "storageType": "string",
        "type": "enum",
        "values": [
            "A",
            "B",
            "C"
        ],
        "valuesAsFlags": false
    },
    {
        "id": 2,
        "name": "EnumInt",
        "storageType": "int",
        "type": "enum",
        "values": [
            "A",
            "B",
            "C"
        ],
        "valuesAsFlags": false
    },
    {
        "id": 3,
        "name": "EnumFlagsString",
        "storageType": "string",
        "type": "enum",
        "values": [
            "A",
            "B",
            "C"
        ],
        "valuesAsFlags": true
    },
    {
        "id": 4,
        "name": "EnumFlagsInt",
        "storageType": "int",
        "type": "enum",
        "values": [
            "A",
            "B",
            "C"
        ],
        "valuesAsFlags": true
    },
    {
        "color": "#ffa0a0a4",
        "drawFill": true,
        "id": 5,
        "members": [
            {
                "name": "enumFlagsInt",
                "propertyType": "EnumFlagsInt",
                "type": "int",
                "value": 6
            },
            {
                "name": "enumFlagsString",
                "propertyType": "EnumFlagsString",
                "type": "string",
                "value": "B,C"
            },
            {
                "name": "enumInt",
                "propertyType": "EnumInt",
                "type": "int",
                "value": 0
            },
            {
                "name": "enumString",
                "propertyType": "EnumString",
                "type": "string",
                "value": "A"
            }
        ],
        "name": "ClassWithEnums",
        "type": "class",
        "useAs": [
            "property"
        ]
    }
]
)";

constexpr auto propertyTypesWithCircularReference = R"([
    {
        "id": 1,
        "members": [
            {
                "name": "self",
                "propertyType": "ClassReferencingItself",
                "type": "class",
                "value": {}
            }
        ],
        "name": "ClassReferencingItself",
        "type": "class"
    }
]
)";

constexpr auto propertyTypesWithCircularReference2 = R"([
    {
        "id": 1,
        "members": [
            {
                "name": "b",
                "propertyType": "B",
                "type": "class",
                "value": {}
            }
        ],
        "name": "A",
        "type": "class"
    },
    {
        "id": 2,
        "members": [
            {
                "name": "a",
                "propertyType": "A",
                "type": "class",
                "value": {}
            }
        ],
        "name": "B",
        "type": "class"
    }
]
)";

constexpr auto propertyTypesToMerge = R"([
    {
        "id": 1,
        "name": "EnumString",
        "type": "enum",
        "values": [ "X", "Y", "Z" ]
    },
    {
        "id": 2,
        "name": "NewEnumString",
        "type": "enum",
        "values": [ "1", "2", "3" ]
    },
    {
        "id": 5,
        "members": [
            {
                "name": "enumString",
                "propertyType": "EnumString",
                "type": "string",
                "value": "Y"
            },
            {
                "name": "newEnumString",
                "propertyType": "NewEnumString",
                "type": "string",
                "value": "2"
            }
        ],
        "name": "NewClass",
        "type": "class"
    }
]
)";

constexpr auto propertyTypesWithEnumInNestedClass = R"([
    {
        "color": "#ffff0000",
        "id": 18,
        "members": [
            {
                "name": "00__ElementType",
                "propertyType": "ElementType",
                "type": "class",
                "value": {
                    "ElementType": "Panel_Inventory"
                }
            }
        ],
        "name": "!Prop_Special_Door",
        "type": "class",
        "useAs": [
            "object",
            "tile"
        ]
    },
    {
        "id": 28,
        "name": "*EN_ElementType",
        "storageType": "string",
        "type": "enum",
        "values": [
            "### Select ENUM value! ###",
            "Agent",
            "Panel_Container",
            "Panel_Inventory",
            "Panel_MasterPocket",
            "Panel_PlayerBackpack_temp",
            "Prop",
            "Blueprint_UseThis",
            "Tile"
        ],
        "valuesAsFlags": false
    },
    {
        "color": "#ffa0a0a4",
        "id": 50,
        "members": [
            {
                "name": "ElementType",
                "propertyType": "*EN_ElementType",
                "type": "string",
                "value": "Panel_Container"
            }
        ],
        "name": "ElementType",
        "type": "class",
        "useAs": [
            "property"
        ]
    }
]
)";


class test_Properties : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void toPropertyValue();
    void toPropertyValue_data();

    void enumWith31Flags();

    void loadAndSavePropertyTypes();
    void loadCircularReference();
    void loadEnumInNestedClass();

    void loadProperties();
    void saveProperties();
    void mergeProperties();

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

void test_Properties::toPropertyValue()
{
    QFETCH(QString, value);
    QFETCH(QString, type);
    QFETCH(QVariant, expected);

    ExportContext context;
    ExportValue exportValue;
    exportValue.value = value;
    exportValue.typeName = type;

    const auto actual = context.toPropertyValue(exportValue);
    QCOMPARE(actual, expected);
}

void test_Properties::toPropertyValue_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<QString>("type");
    QTest::addColumn<QVariant>("expected");

    QTest::newRow("color") << QStringLiteral("#ff0000ff") << QStringLiteral("color") << QVariant::fromValue(QColor(Qt::blue));
    QTest::newRow("invalid-color") << QString() << QStringLiteral("color") << QVariant::fromValue(QColor());
    QTest::newRow("int") << QStringLiteral("42") << QStringLiteral("int") << QVariant::fromValue(42);
    QTest::newRow("float") << QStringLiteral("42.0") << QStringLiteral("float") << QVariant::fromValue(42.0);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QTest::newRow("invalid-int") << QStringLiteral("foo") << QStringLiteral("int") << QVariant(QMetaType(QMetaType::Int));
    QTest::newRow("invalid-float") << QStringLiteral("foo") << QStringLiteral("float") << QVariant(QMetaType(QMetaType::Double));
#else
    QTest::newRow("invalid-int") << QStringLiteral("foo") << QStringLiteral("int") << QVariant(QVariant::Int);
    QTest::newRow("invalid-float") << QStringLiteral("foo") << QStringLiteral("float") << QVariant(QVariant::Double);
#endif
    QTest::newRow("bool-true") << QStringLiteral("true") << QStringLiteral("bool") << QVariant::fromValue(true);
    QTest::newRow("bool-false") << QStringLiteral("false") << QStringLiteral("bool") << QVariant::fromValue(false);
    QTest::newRow("string") << QStringLiteral("foo") << QStringLiteral("string") << QVariant::fromValue(QStringLiteral("foo"));
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QTest::newRow("file") << QStringLiteral("/foo") << QStringLiteral("file") << QVariant::fromValue(FilePath { QUrl::fromLocalFile(QStringLiteral("/foo")) });
#endif
    QTest::newRow("object") << QStringLiteral("1") << QStringLiteral("object") << QVariant::fromValue(ObjectRef { 1 });

    // todo: test enums and classes, and also add a test for toExportValue
}

void test_Properties::enumWith31Flags()
{
    EnumPropertyType flagsAsString("flagsAsString");
    flagsAsString.storageType = EnumPropertyType::StringValue;
    flagsAsString.valuesAsFlags = true;

    EnumPropertyType flagsAsInt("flagsAsInt");
    flagsAsInt.storageType = EnumPropertyType::IntValue;
    flagsAsInt.valuesAsFlags = true;

    QString allFlagsString;
    int allFlagsInt = 0;

    for (int i = 1; i <= 31; ++i) {
        flagsAsString.values.append(QString::number(i));
        flagsAsInt.values.append(QString::number(i));

        if (!allFlagsString.isEmpty())
            allFlagsString.append(QLatin1Char(','));
        allFlagsString.append(QString::number(i));
        allFlagsInt |= (1 << (i - 1));
    }

    ExportContext context;

    QVariant property1 = flagsAsString.toPropertyValue(QStringLiteral("1"), context);
    QVariant propertyAll = flagsAsString.toPropertyValue(allFlagsString, context);

    QCOMPARE(property1, QVariant::fromValue(PropertyValue(1, flagsAsString.id)));
    QCOMPARE(propertyAll, QVariant::fromValue(PropertyValue(allFlagsInt, flagsAsString.id)));

    ExportValue exportString1 = flagsAsString.toExportValue(property1.value<PropertyValue>().value, context);
    ExportValue exportStringAll = flagsAsString.toExportValue(propertyAll.value<PropertyValue>().value, context);
    ExportValue exportInt1 = flagsAsInt.toExportValue(property1.value<PropertyValue>().value, context);
    ExportValue exportIntAll = flagsAsInt.toExportValue(propertyAll.value<PropertyValue>().value, context);

    QCOMPARE(exportString1.value, QStringLiteral("1"));
    QCOMPARE(exportStringAll.value, allFlagsString);
    QCOMPARE(exportInt1.value, 1);
    QCOMPARE(exportIntAll.value, allFlagsInt);
}

void test_Properties::loadAndSavePropertyTypes()
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(propertyTypesJson, &error);
    QVERIFY(error.error == QJsonParseError::NoError);

    PropertyTypes types;
    types.loadFromJson(doc.array(), QString());
    QCOMPARE(types.count(), mTypes.count());

    const auto json = QJsonDocument(types.toJson()).toJson();
    QCOMPARE(json, propertyTypesJson);
}

void test_Properties::loadCircularReference()
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(propertyTypesWithCircularReference, &error);
    QVERIFY(error.error == QJsonParseError::NoError);

    PropertyTypes types;
    types.loadFromJson(doc.array(), QString());

    const auto type = types.findPropertyValueType(QStringLiteral("ClassReferencingItself"));
    QVERIFY(type);
    QCOMPARE(type->type, PropertyType::PT_Class);

    // Verify the circular reference is not present
    const auto &members = static_cast<const ClassPropertyType*>(type)->members;
    QVERIFY(!members.contains(QStringLiteral("self")));

    doc = QJsonDocument::fromJson(propertyTypesWithCircularReference2, &error);
    QVERIFY(error.error == QJsonParseError::NoError);

    types.loadFromJson(doc.array(), QString());

    // Verify the back reference is not present
    const auto a = types.findPropertyValueType(QStringLiteral("A"));
    QVERIFY(a);
    QCOMPARE(a->type, PropertyType::PT_Class);
    const auto &membersA = static_cast<const ClassPropertyType*>(a)->members;
    QVERIFY(!membersA.contains(QStringLiteral("b")));
}

void test_Properties::loadEnumInNestedClass()
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(propertyTypesWithEnumInNestedClass, &error);
    QVERIFY(error.error == QJsonParseError::NoError);

    PropertyTypes types;
    types.loadFromJson(doc.array(), QString());

    const auto type = types.findTypeByName(QStringLiteral("!Prop_Special_Door"));
    const auto elementType = types.findPropertyValueType(QStringLiteral("ElementType"));
    const auto enumType = types.findPropertyValueType(QStringLiteral("*EN_ElementType"));
    QVERIFY(type);
    QVERIFY(elementType);
    QVERIFY(enumType);

    const auto &members = static_cast<const ClassPropertyType*>(type)->members;
    const auto classMember = members.value(QStringLiteral("00__ElementType")).value<PropertyValue>();
    QCOMPARE(classMember.typeId, elementType->id);

    const auto nestedEnumValue = classMember.value.toMap().value(QStringLiteral("ElementType"));
    QCOMPARE(nestedEnumValue.value<PropertyValue>().typeId, enumType->id);
}

void test_Properties::loadProperties()
{
    ExportContext context(mTypes, QString());

    const auto enumStringType = mTypes.findPropertyValueType(QStringLiteral("EnumString"));
    const auto classType = mTypes.findPropertyValueType(QStringLiteral("ClassWithEnums"));
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

    const auto enumStringType = mTypes.findPropertyValueType(QStringLiteral("EnumString"));
    const auto classType = mTypes.findPropertyValueType(QStringLiteral("ClassWithEnums"));
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

void test_Properties::mergeProperties()
{
    PropertyTypes types;

    {
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(propertyTypesJson, &error);
        QVERIFY(error.error == QJsonParseError::NoError);

        types.loadFromJson(doc.array(), QString());
        QCOMPARE(types.count(), mTypes.count());
    }

    {
        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(propertyTypesToMerge, &error);
        QVERIFY(error.error == QJsonParseError::NoError);

        PropertyTypes typesToMerge;
        typesToMerge.loadFromJson(doc.array(), QString());
        QCOMPARE(typesToMerge.count(), size_t(3));

        types.merge(std::move(typesToMerge));
        QCOMPARE(types.count(), mTypes.count() + 2);
    }

    // Verify EnumString was replaced
    auto enumStringType = static_cast<const EnumPropertyType*>(types.findPropertyValueType(QStringLiteral("EnumString")));
    QVERIFY(enumStringType && enumStringType->isEnum());
    const auto expectedValues = QStringList { QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z") };
    QCOMPARE(enumStringType->values, expectedValues);

    // Verify NewEnumString was added and got a new ID
    auto newEnumStringType = types.findPropertyValueType(QStringLiteral("NewEnumString"));
    QVERIFY(newEnumStringType);
    QCOMPARE(newEnumStringType->id, 6);

    // Verify NewClass was added, and that its member newEnumString has the right type ID
    auto classType = static_cast<const ClassPropertyType*>(types.findPropertyValueType(QStringLiteral("NewClass")));
    QVERIFY(classType && classType->isClass());
    const auto classMember = classType->members.value(QStringLiteral("newEnumString")).value<PropertyValue>();
    QCOMPARE(classMember.typeId, newEnumStringType->id);
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
