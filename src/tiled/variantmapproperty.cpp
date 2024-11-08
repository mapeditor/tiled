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

#include "mapdocument.h"
#include "objectrefedit.h"
#include "preferences.h"
#include "utils.h"
#include "variantpropertymanager.h"

#include <QMenu>
#include <QScopedValueRollback>

namespace Tiled {

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
};


VariantMapProperty::VariantMapProperty(const QString &name, QObject *parent)
    : GroupProperty(name, parent)
    , m_resetIcon(QIcon(QStringLiteral(":/images/16/edit-clear.png")))
    , m_removeIcon(QIcon(QStringLiteral(":/images/16/remove.png")))
    , m_addIcon(QIcon(QStringLiteral(":/images/16/add.png")))
    , m_renameIcon(QIcon(QLatin1String(":/images/16/rename.png")))
{
    m_resetIcon.addFile(QStringLiteral(":/images/24/edit-clear.png"));
    m_removeIcon.addFile(QStringLiteral(":/images/22/remove.png"));
    m_addIcon.addFile(QStringLiteral(":/images/22/add.png"));

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
            mValue.insert(name, value);
            emitMemberValueChanged({ name }, value);
        };

        property = createProperty({ name }, std::move(get), std::move(set));
        if (property) {
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
        }
    }

    if (property) {
        if (mValue.contains(name)) {
            property->setEnabled(true);
            property->setActions(Property::Action::Remove);
        } else {
            property->setEnabled(false);
            property->setActions(Property::Action::Add);
        }

        updateModifiedRecursively(property, newValue);
        emitValueChangedRecursively(property);
    }

    return true;
}

Property *VariantMapProperty::createProperty(const QStringList &path,
                                             std::function<QVariant ()> get,
                                             std::function<void (const QVariant &)> set)
{
    Property *property = nullptr;

    const auto value = get();
    const auto type = value.userType();
    const auto &name = path.last();
    QString typeName;

    if (type == filePathTypeId()) {
        auto getUrl = [get = std::move(get)] { return get().value<FilePath>().url; };
        auto setUrl = [set = std::move(set)] (const QUrl &value) {
            set(QVariant::fromValue(FilePath { value }));
        };
        property = new UrlProperty(name, std::move(getUrl), std::move(setUrl));
    } else if (type == objectRefTypeId()) {
        auto getObjectRef = [get = std::move(get), this] {
            return DisplayObjectRef(get().value<ObjectRef>(),
                                    static_cast<MapDocument*>(mDocument));  // todo: shouldn't it be qobject_cast?
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
                auto classType = static_cast<const ClassPropertyType&>(*propertyType);

                auto groupProperty = new GroupProperty(name);
                groupProperty->setHeader(false);

                createClassMembers(path, groupProperty, classType, std::move(get));

                groupProperty->setExpanded(mExpandedProperties.contains(path));

                connect(groupProperty, &GroupProperty::expandedChanged, this, [=](bool expanded) {
                    if (expanded)
                        mExpandedProperties.insert(path);
                    else
                        mExpandedProperties.remove(path);
                });

                property = groupProperty;
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
            typeName = tr("Unknown type");
        }
    } else {
        property = createVariantProperty(name, std::move(get), std::move(set));
    }

    if (property) {
        if (typeName.isEmpty())
            typeName = typeToName(type);

        property->setToolTip(QStringLiteral("%1&nbsp;<span style=\"color: gray;\">:&nbsp;%2<span>")
                             .arg(property->name(), typeName));

        connect(property, &Property::contextMenuRequested, this, [=](const QPoint &globalPos) {
            memberContextMenuRequested(property, path, globalPos);
        });
    }

    return property;
}

void VariantMapProperty::createClassMembers(const QStringList &path,
                                            GroupProperty *groupProperty,
                                            const ClassPropertyType &classType,
                                            std::function<QVariant ()> get)
{
    // Create a sub-property for each member
    QMapIterator<QString, QVariant> it(classType.members);
    while (it.hasNext()) {
        it.next();
        const QString &name = it.key();

        auto childPath = path;
        childPath.append(name);

        auto getMember = [=] {
            auto def = classType.members.value(name);
            return get().value<PropertyValue>().value.toMap().value(name, def);
        };
        auto setMember = [=] (const QVariant &value) {
            setClassMember(childPath, value);
        };

        if (auto childProperty = createProperty(childPath, std::move(getMember), setMember)) {
            childProperty->setActions(Property::Action::Reset);
            groupProperty->addProperty(childProperty);

            connect(childProperty, &Property::resetRequested, this, [=] {
                setMember(QVariant());
                emitValueChangedRecursively(childProperty);
            });
        }
    }
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
    const auto oldValue = mValue.value(name, mSuggestions.value(name));

    mValue.insert(name, value);

    if (auto property = mPropertyMap.value(name)) {
        const int index = indexOfProperty(property);
        createOrUpdateProperty(index, name, oldValue, value);
    }

    emitMemberValueChanged({ name }, value);
}

void VariantMapProperty::setClassMember(const QStringList &path, const QVariant &value)
{
    if (!setPropertyMemberValue(mValue, path, value))
        return;

    const auto &topLevelName = path.first();
    updateModifiedRecursively(mPropertyMap.value(topLevelName),
                              mValue.value(topLevelName));

    emitMemberValueChanged(path, value);
}

void VariantMapProperty::updateModifiedRecursively(Property *property,
                                                   const QVariant &value)
{
    auto groupProperty = qobject_cast<GroupProperty*>(property);
    if (!groupProperty)
        return;

    const QVariantMap classValue = value.value<PropertyValue>().value.toMap();

    for (auto subProperty : groupProperty->subProperties()) {
        const auto &name = subProperty->name();
        const bool isModified = classValue.contains(name);

        if (subProperty->isModified() != isModified || subProperty->isModified()) {
            subProperty->setModified(isModified);
            updateModifiedRecursively(subProperty, classValue.value(name));
        }
    }
}

void VariantMapProperty::emitValueChangedRecursively(Property *property)
{
    emit property->valueChanged();

    if (auto groupProperty = qobject_cast<GroupProperty*>(property))
        for (auto subProperty : groupProperty->subProperties())
            emitValueChangedRecursively(subProperty);
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

void VariantMapProperty::emitMemberValueChanged(const QStringList &path, const QVariant &value)
{
    QScopedValueRollback<bool> emittingValueChanged(mEmittingValueChanged, true);
    emit memberValueChanged(path, value);
    emit valueChanged();
}

void VariantMapProperty::memberContextMenuRequested(Property *property, const QStringList &path, const QPoint &globalPos)
{
    QMenu menu;

    // Add Expand All and Collapse All actions to group properties
    if (auto groupProperty = qobject_cast<GroupProperty*>(property)) {
        menu.addAction(tr("Expand All"), groupProperty, &GroupProperty::expandAll);
        menu.addAction(tr("Collapse All"), groupProperty, &GroupProperty::collapseAll);
    }

    // Provide the Add, Remove and Reset actions also here
    if (property->actions()) {
        menu.addSeparator();

        if (property->actions() & Property::Action::Add) {
            QAction *add = menu.addAction(m_addIcon, tr("Add Property"), this, [this, name = path.first()] {
                addMember(name, mSuggestions.value(name));
            });
            Utils::setThemeIcon(add, "add");
        }
        if (property->actions() & Property::Action::Remove) {
            QAction *remove = menu.addAction(m_removeIcon, tr("Remove Property"), this, [this, name = path.first()] {
                removeMember(name);
            });
            Utils::setThemeIcon(remove, "remove");

            // If a property can be removed, it can also be renamed
            menu.addAction(m_renameIcon, tr("Rename Property..."), this, [this, name = path.first()] {
                emit renameRequested(name);
            });
        }
        if (property->actions() & Property::Action::Reset) {
            QAction *reset = menu.addAction(m_resetIcon, tr("Reset Member"), this, [=] {
                setClassMember(path, QVariant());
                emitValueChangedRecursively(property);
            });
            reset->setEnabled(property->isModified());
            Utils::setThemeIcon(reset, "edit-clear");
        }
    }

    // todo: Add "Convert" sub-menu
    // todo: Add "Copy" and "Paste" actions

    if (!menu.isEmpty())
        menu.exec(globalPos);
}

} // namespace Tiled

#include "moc_variantmapproperty.cpp"
#include "variantmapproperty.moc"
