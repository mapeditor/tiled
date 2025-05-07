/*
 * variantmapproperty.cpp
 * Copyright 2024, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "variantmapproperty.h"

#include "listedit.h"
#include "mapdocument.h"
#include "objectrefedit.h"
#include "preferences.h"
#include "propertyeditorwidgets.h"
#include "propertytypesmodel.h"
#include "session.h"
#include "utils.h"

#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include <QMenu>
#include <QScopedValueRollback>

namespace Tiled {

namespace session {
static SessionOption<QString> propertyType { "property.type", QStringLiteral("string") };
} // namespace session


static void updateModifiedRecursively(Property *property, const QVariant &value)
{
    auto groupProperty = qobject_cast<GroupProperty*>(property);
    if (!groupProperty)
        return;

    const bool isDimmed = property->isDimmed();

    if (value.userType() == QMetaType::QVariantList) {
        const QVariantList listValue = value.toList();
        for (int i = 0; i < groupProperty->subProperties().size() && i < listValue.size(); ++i) {
            auto subProperty = groupProperty->subProperties().at(i);

            subProperty->setDimmed(isDimmed);
            subProperty->setModified(false);
            updateModifiedRecursively(subProperty, listValue.at(i));
        }
    } else {
        const QVariantMap classValue = value.value<PropertyValue>().value.toMap();
        for (auto subProperty : groupProperty->subProperties()) {
            const auto &name = subProperty->name();
            const bool isModified = classValue.contains(name);

            if (subProperty->isModified() != isModified || subProperty->isDimmed() != isDimmed || isModified) {
                subProperty->setDimmed(isDimmed);
                subProperty->setModified(isModified);
                updateModifiedRecursively(subProperty, classValue.value(name));
            }
        }
    }
}

static void emitValueChangedRecursively(Property *property)
{
    emit property->valueChanged();

    if (auto groupProperty = qobject_cast<GroupProperty*>(property))
        for (auto subProperty : groupProperty->subProperties())
            emitValueChangedRecursively(subProperty);
}


class ObjectRefProperty : public PropertyTemplate<DisplayObjectRef>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new ObjectRefEdit(parent);
        auto syncEditor = [this, editor] {
            const QSignalBlocker blocker(editor);
            editor->setValue(value());
        };
        syncEditor();
        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(editor, &ObjectRefEdit::valueChanged,
                this, [this](const DisplayObjectRef &value) {
            setValue(value);
        });
        return editor;
    }

    void addContextMenuActions(QMenu *menu) override
    {
        auto objectRef = value();
        menu->addAction(QCoreApplication::translate("Tiled::PropertiesDock", "Go to Object"), [=] {
            if (auto object = objectRef.object()) {
                objectRef.mapDocument->setSelectedObjects({object});
                emit objectRef.mapDocument->focusMapObjectRequested(object);
            }
        })->setEnabled(objectRef.object());
    }
};


VariantMapProperty::VariantMapProperty(const QString &name, QObject *parent)
    : GroupProperty(name, parent)
{
    connect(Preferences::instance(), &Preferences::propertyTypesChanged,
            this, &VariantMapProperty::propertyTypesChanged);
}

void VariantMapProperty::setValue(const QVariantMap &value,
                                  const QVariantMap &suggestions)
{
    QVariantMap oldProperties = mSuggestions;
    mergeProperties(oldProperties, mValue);

    mValue = value;
    mSuggestions = suggestions;

    QVariantMap newProperties = suggestions;
    mergeProperties(newProperties, value);

    // First, delete all properties that are not in allProperties
    for (auto it = mPropertyMap.begin(); it != mPropertyMap.end(); ) {
        if (newProperties.contains(it.key())) {
            ++it;
        } else {
            deleteProperty(it.value());
            it = mPropertyMap.erase(it);
        }
    }

    int index = 0;

    QMapIterator<QString, QVariant> it(newProperties);
    while (it.hasNext()) {
        it.next();
        const QString &name = it.key();
        const auto &newValue = it.value();
        const auto oldValue = oldProperties.value(name);

        if (createOrUpdateProperty(index, name, oldValue, newValue))
            ++index;
    }

    // Don't emit valueChanged when we're just recreating properties with custom types
    if (mPropertyTypesChanged)
        return;

    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit valueChanged();
}

static bool canReuseProperty(const QVariant &a,
                             const QVariant &b,
                             bool propertyTypesChanged)
{
    if (a.userType() != b.userType())
        return false;

    // Two PropertyValue values might still have different types
    if (a.userType() == propertyValueId()) {
        // Trigger re-creation of the property when the types have changed
        if (propertyTypesChanged)
            return false;

        auto aTypeId = a.value<PropertyValue>().typeId;
        auto bTypeId = b.value<PropertyValue>().typeId;
        if (aTypeId != bTypeId)
            return false;
    }

    return true;
}

static Property *createProperty(const QString &name,
                                std::function<QVariant()> get,
                                std::function<void(const QVariant &)> set)
{
    Property *property = nullptr;

    const auto value = get();
    const auto type = value.userType();
    QString typeName;

    if (type == QMetaType::QVariantList) {
        auto getList = [get = std::move(get)] { return get().toList(); };
        property = new VariantListProperty(name, std::move(getList), std::move(set));
    } else if (type == filePathTypeId()) {
        auto getUrl = [get = std::move(get)] { return get().value<FilePath>().url; };
        auto setUrl = [set = std::move(set)] (const QUrl &value) {
            set(QVariant::fromValue(FilePath { value }));
        };
        property = new UrlProperty(name, std::move(getUrl), std::move(setUrl));
    } else if (type == objectRefTypeId()) {
        auto getObjectRef = [get = std::move(get)/*, this*/] {
            return DisplayObjectRef(get().value<ObjectRef>()/*,
                                    qobject_cast<MapDocument *>(mDocument)*/);
        };
        auto setObjectRef = [set = std::move(set)](const DisplayObjectRef &value) {
            set(QVariant::fromValue(value.ref));
        };
        property = new ObjectRefProperty(name, std::move(getObjectRef), std::move(setObjectRef));
    } else if (type == propertyValueId()) {
        const auto propertyValue = value.value<PropertyValue>();
        if (auto propertyType = propertyValue.type()) {
            switch (propertyType->type) {
            case PropertyType::PT_Invalid:
                break;
            case PropertyType::PT_Class: {
                auto &classType = static_cast<const ClassPropertyType&>(*propertyType);

                // The ClassProperty does not use the set function, because we will instead handle
                // its memberValueChanged signal to apply changes.
                property = new ClassProperty(name, classType, [get = std::move(get)] {
                    return get().value<PropertyValue>().value.toMap();
                });
                break;
            }
            case PropertyType::PT_Enum: {
                auto enumProperty = new BaseEnumProperty(
                    name,
                    [get = std::move(get)] { return get().value<PropertyValue>().value.toInt(); },
                    [set = std::move(set), propertyType](int value) {
                        set(propertyType->wrap(value));
                    });

                auto enumType = static_cast<const EnumPropertyType&>(*propertyType);
                enumProperty->setEnumData(enumType.values);
                enumProperty->setFlags(enumType.valuesAsFlags);

                property = enumProperty;
                break;
            }
            }

            typeName = propertyType->name;
        } else {
            typeName = QCoreApplication::translate("VariantMapProperty", "Unknown type");
        }
    } else {
        property = createVariantProperty(name, std::move(get), std::move(set));
    }

    if (property) {
        if (typeName.isEmpty())
            typeName = typeToName(type);

        auto setToolTip = [=] {
            property->setToolTip(
                QStringLiteral("%1&nbsp;<span style=\"color: gray;\">:&nbsp;%2<span>")
                    .arg(property->name(), typeName));
        };

        setToolTip();
        QObject::connect(property, &Property::nameChanged, property, setToolTip);
    }

    return property;
}

bool VariantMapProperty::createOrUpdateProperty(int index,
                                                const QString &name,
                                                const QVariant &oldValue,
                                                const QVariant &newValue)
{
    auto property = mPropertyMap.value(name);

    // If it already exists, check whether we need to delete it
    if (property && !canReuseProperty(oldValue, newValue, mPropertyTypesChanged)) {
        deleteProperty(property);
        mPropertyMap.remove(name);
        property = nullptr;
    }

    // Try to create the property if necessary
    if (!property) {
        auto get = [=] {
            return mValue.value(name, mSuggestions.value(name));
        };
        auto set = [=] (const QVariant &value) {
            setMemberValue({ name }, value);
        };

        property = createProperty(name, std::move(get), std::move(set));

        // Changes to class properties are handled with a signal rather than
        // the set function, so that we can include the path.
        if (auto classProperty = qobject_cast<ClassProperty*>(property)) {
            connect(classProperty, &ClassProperty::memberValueChanged,
                    this, [=] (const PropertyPath &path, const QVariant &value) {
                PropertyPath fullPath = { name };
                fullPath.append(path);
                setMemberValue(fullPath, value);
            });
        }

        if (property) {
            connect(property, &Property::resetRequested, this, [=] {
                removeMember(name);
            });
            connect(property, &Property::removeRequested, this, [=] {
                removeMember(name);
            });

            connect(property, &Property::addRequested, this, [=] {
                addMember(name, mSuggestions.value(name));
            });

            insertProperty(index, property);
            mPropertyMap.insert(name, property);
        } else {
            qWarning() << "Failed to create property for" << name
                       << "with type" << newValue.typeName();
            return false;
        }
    }

    if (property) {
        const bool present = mValue.contains(name);
        const bool suggested = mSuggestions.contains(name);

        property->setModified(suggested && present);
        property->setDimmed(!present);
        property->setActions(Property::Action::Select |
                             (suggested ? Property::Action::Reset
                                        : Property::Action::Remove));

        updateModifiedRecursively(property, newValue);
        emitValueChangedRecursively(property);
    }

    return true;
}

void VariantMapProperty::removeMember(const QString &name)
{
    if (!mValue.contains(name))
        return;

    const auto oldValue = mValue.take(name);

    if (!mSuggestions.contains(name)) {
        if (auto property = mPropertyMap.take(name))
            deleteProperty(property);
    } else if (auto property = mPropertyMap.value(name)) {
        const auto newValue = mSuggestions.value(name);
        const int index = indexOfProperty(property);
        createOrUpdateProperty(index, name, oldValue, newValue);
    }

    emitMemberValueChanged({ name }, QVariant());
}

void VariantMapProperty::addMember(const QString &name, const QVariant &value)
{
    int index = 0;

    if (auto property = mPropertyMap.value(name)) {
        index = indexOfProperty(property);
    } else for (auto it = mValue.keyBegin(); it != mValue.keyEnd(); ++it) {
        if (*it < name)
            ++index;
        else
            break;
    }

    const auto oldValue = mValue.value(name, mSuggestions.value(name));
    mValue.insert(name, value);
    createOrUpdateProperty(index, name, oldValue, value);

    emitMemberValueChanged({ name }, value);
}

void VariantMapProperty::setMemberValue(const PropertyPath &path, const QVariant &value)
{
    Q_ASSERT(!path.isEmpty());

    const auto &topLevelEntry = path.first();
    Q_ASSERT(std::holds_alternative<QString>(topLevelEntry));

    auto &topLevelName = std::get<QString>(topLevelEntry);

    // If we're setting a member of a class property that doesn't exist yet,
    // we need to call addMember with the modified top-level value instead.
    if (path.size() > 1 && !mValue.contains(topLevelName)) {
        auto topLevelValue = mSuggestions.value(topLevelName);
        if (setNestedPropertyValue(topLevelValue, 1, path, value, false))
            addMember(topLevelName, topLevelValue);
        return;
    }

    if (!setPropertyMemberValue(mValue, path, value))
        return;

    auto property = mPropertyMap.value(topLevelName);
    const bool present = mValue.contains(topLevelName);
    const bool suggested = mSuggestions.contains(topLevelName);

    property->setModified(suggested && present);
    property->setDimmed(!present);
    updateModifiedRecursively(property, mValue.value(topLevelName));

    emitMemberValueChanged(path, value);
}

void VariantMapProperty::propertyTypesChanged()
{
    // When this happens in response to emitting value changed, it means we
    // have triggered a change in a class definition. In this case we should
    // not update ourselves.
    if (mEmittingValueChanged)
        return;

    QScopedValueRollback<bool> propertyTypesChanged(mPropertyTypesChanged, true);
    setValue(mValue, mSuggestions);
}

void VariantMapProperty::emitMemberValueChanged(const PropertyPath &path, const QVariant &value)
{
    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit memberValueChanged(path, value);
    emit valueChanged();
}


ClassProperty::ClassProperty(const QString &name,
                             const ClassPropertyType &classType,
                             std::function<QVariantMap()> get,
                             QObject *parent)
    : GroupProperty(name, parent)
    , mGet(std::move(get))
{
    setHeader(false);
    createMembers(classType);
}

void ClassProperty::setMemberValue(const QString &name, const QVariant &value)
{
    // TODO: More specific signal
    // if (auto childProperty = mPropertyMap.value(name))
    //     emitValueChangedRecursively(childProperty);

    // ClassProperty does not change its value directly, but instead sends this
    // signal, making its handler responsible for applying the change. This is
    // done because nested values need to be applied individually to each
    // selected object.

    // emit valueChanged();
}

void ClassProperty::createMembers(const ClassPropertyType &classType)
{
    // Create a sub-property for each member
    QMapIterator<QString, QVariant> it(classType.members);
    while (it.hasNext()) {
        it.next();
        const QString &name = it.key();

        auto getMember = [=] {
            return mGet().value(name, classType.members.value(name));
        };
        auto setMember = [=] (const QVariant &value) {
            emit memberValueChanged({ name }, value);
        };

        if (auto childProperty = createProperty(name, std::move(getMember), setMember)) {
            if (auto classProperty = qobject_cast<ClassProperty*>(childProperty)) {
                connect(classProperty,
                        &ClassProperty::memberValueChanged,
                        this,
                        [=](const PropertyPath &path, const QVariant &value) {
                            PropertyPath fullPath = { name };
                            fullPath.append(path);
                            emit memberValueChanged(fullPath, value);
                        });
            }

            childProperty->setActions(Property::Action::Reset);
            addProperty(childProperty);

            connect(childProperty, &Property::resetRequested, this, [=] {
                setMember(QVariant());
                emitValueChangedRecursively(childProperty);
            });
        }
    }
}

VariantListProperty::VariantListProperty(const QString &name,
                                         std::function<QVariantList()> get,
                                         std::function<void(const QVariantList &)> set,
                                         QObject *parent)
    : GroupProperty(name, parent)
    , mGet(std::move(get))
    , mSet(std::move(set))
{
    setHeader(false);
    setValue(mGet());

    connect(this, &Property::valueChanged, this, [this] {
        if (mEmittingValueChanged)
            return;

        setValue(mGet());
    });
}

void VariantListProperty::setValue(const QVariantList &value)
{
    QVariantList oldValue = mValue;
    mValue = value;

    // First, make sure we don't have too many properties
    while (mValue.size() < subProperties().size())
        deleteProperty(subProperties().last());

    for (int index = 0; index < mValue.size(); ++index)
        createOrUpdateProperty(index, oldValue.value(index), mValue.at(index));

    // // Don't emit valueChanged when we're just recreating properties with custom types
    // if (mPropertyTypesChanged)
    //     return;

    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit valueChanged();
}

void VariantListProperty::removeValueAt(int index)
{
    QVariantList value = mValue;
    value.removeAt(index);
    setValue(value);

    mSet(mValue);

    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit valueChanged();
}

void VariantListProperty::addValue(const QVariant &value)
{
    mValue.append(value);
    createOrUpdateProperty(mValue.size() - 1, QVariant(), value);

    mSet(mValue);

    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit valueChanged();
}

QWidget *VariantListProperty::createEditor(QWidget *parent)
{
    auto editor = new ListEdit(parent);
    auto syncEditor = [this, editor] {
        const QSignalBlocker blocker(editor);
        editor->setValue(value());
    };
    syncEditor();

    connect(this, &Property::valueChanged, editor, syncEditor);
    connect(editor, &ListEdit::valueChanged, this, [this, editor] {
        setValue(editor->value());
    });

    return editor;
}

void VariantListProperty::addContextMenuActions(QMenu *menu)
{
    auto addMenu = menu->addMenu(tr("Add Value"));

    const QVariantList values = possiblePropertyValues(nullptr);
    for (const auto &value : values) {
        const QIcon icon = PropertyTypesModel::iconForProperty(value);
        auto action = addMenu->addAction(icon, userTypeName(value));
        action->setData(value);
    }

    connect(addMenu, &QMenu::triggered, this, [this](QAction *action) {
        addValue(action->data());
    });
}

bool VariantListProperty::createOrUpdateProperty(int index,
                                                 const QVariant &oldValue,
                                                 const QVariant &newValue)
{
    auto property = subProperties().value(index);

    // If it already exists, check whether we need to delete it
    if (property && !canReuseProperty(oldValue, newValue, false/*mPropertyTypesChanged*/)) {
        deleteProperty(property);
        property = nullptr;
    }

    // Try to create the property if necessary
    if (!property) {
        auto get = [=] {
            return mValue.value(index);
        };
        auto set = [=] (const QVariant &value) {
            mValue[index] = value;
            mSet(mValue);

            updateModifiedRecursively(subProperties().at(index), mValue[index]);

            QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
            emit valueChanged();
        };

        property = createProperty(QString(), std::move(get), std::move(set));
        if (!property) {
            qWarning() << "Failed to create property at" << index
                       << "with type" << newValue.typeName();

            property = new StringProperty(QString(), [] { return QString(); }, {}, this);
        }

        if (auto classProperty = qobject_cast<ClassProperty*>(property)) {
            connect(classProperty,
                    &ClassProperty::memberValueChanged,
                    this,
                    [=](const PropertyPath &path, const QVariant &value) {
                        if (setNestedPropertyValue(mValue[index], 0, path, value, false)) {
                            mSet(mValue);

                            updateModifiedRecursively(subProperties().at(index), mValue[index]);

                            QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
                            emit valueChanged();
                        }
                    });
        }


        connect(property, &Property::removeRequested, this, [this, property] {
            int currentIndex = subProperties().indexOf(property);
            if (currentIndex >= 0)
                removeValueAt(currentIndex);
        });

        insertProperty(index, property);
    }

    if (property) {
        property->setName(QStringLiteral("[%1]").arg(index));
        property->setActions(Property::Action::Select | Property::Action::Remove);
        // updateModifiedRecursively(property, newValue);
        emitValueChangedRecursively(property);
    }

    return true;
}


AddValueProperty::AddValueProperty(QObject *parent)
    : Property(QString(), parent)
    , m_placeholderText(tr("Property name"))
{
    setActions(Action::AddDisabled);
}

void AddValueProperty::setPlaceholderText(const QString &text)
{
    if (m_placeholderText == text)
        return;

    m_placeholderText = text;
    emit placeholderTextChanged(text);
}

QWidget *AddValueProperty::createLabel(int level, QWidget *parent)
{
    constexpr int QLineEditPrivate_horizontalMargin = 2;
    const int spacing = Utils::dpiScaled(3);
    const int branchIndicatorWidth = Utils::dpiScaled(14);
    const int indent = branchIndicatorWidth * (level + 1);

    auto nameEdit = new LineEdit(parent);
    nameEdit->setText(name());
    nameEdit->setPlaceholderText(m_placeholderText);

    QStyleOptionFrame option;
    option.initFrom(nameEdit);
    const int frameWidth = nameEdit->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, nameEdit);
    const int nativeMargin = QLineEditPrivate_horizontalMargin + frameWidth;

    QMargins margins;

    if (parent->isLeftToRight())
        margins = QMargins(spacing + indent - nativeMargin, 0, spacing - nativeMargin, 0);
    else
        margins = QMargins(spacing - nativeMargin, 0, spacing + indent - nativeMargin, 0);

    nameEdit->setContentsMargins(margins);

    connect(nameEdit, &QLineEdit::textChanged, this, &Property::setName);
    connect(nameEdit, &QLineEdit::returnPressed, this, [this] {
        if (!name().isEmpty())
            emit addRequested();
    });
    connect(this, &Property::nameChanged, this, [=](const QString &name) {
        setActions(name.isEmpty() ? Action::AddDisabled : Action::Add);
    });
    connect(this, &AddValueProperty::placeholderTextChanged,
            nameEdit, &QLineEdit::setPlaceholderText);

    nameEdit->installEventFilter(this);

    connect(qApp, &QApplication::focusChanged, this, [=](QWidget *, QWidget *focusWidget) {
        // Ignore focus in different windows (popups, dialogs, etc.)
        if (!focusWidget || focusWidget->window() != parent->window())
            return;

        // Request removal if focus moved elsewhere
        if (!parent->isAncestorOf(focusWidget))
            emit removeRequested();
    });

    return nameEdit;
}

QWidget *AddValueProperty::createEditor(QWidget *parent)
{
    // Create combo box with property types
    auto typeBox = new ComboBox(parent);

    const QVariantList values = possiblePropertyValues(m_parentClassType);
    for (const auto &value : values) {
        const QIcon icon = PropertyTypesModel::iconForProperty(value);
        typeBox->addItem(icon, userTypeName(value), value);
    }

    // Restore previously used type
    typeBox->setCurrentText(session::propertyType);
    if (typeBox->currentIndex() == -1)
        typeBox->setCurrentIndex(typeBox->findData(QString()));

    m_value = typeBox->currentData();

    connect(typeBox, &QComboBox::currentIndexChanged, this, [=](int index) {
        m_value = typeBox->itemData(index);
        session::propertyType = typeBox->currentText();
    });
    connect(typeBox, &ComboBox::returnPressed, this, [this] {
        if (!name().isEmpty())
            emit addRequested();
    });

    typeBox->installEventFilter(this);

    return typeBox;
}

bool AddValueProperty::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress: {
        // When Escape is pressed while the name edit or the type combo has
        // focus, request the removal of this property.
        auto keyEvent = static_cast<QKeyEvent*>(event);
        bool isNameEdit = qobject_cast<QLineEdit*>(watched);
        bool isTypeCombo = qobject_cast<QComboBox*>(watched);
        if ((isNameEdit || isTypeCombo) && keyEvent->key() == Qt::Key_Escape) {
            emit removeRequested();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

} // namespace Tiled

#include "moc_variantmapproperty.cpp"
#include "variantmapproperty.moc"
