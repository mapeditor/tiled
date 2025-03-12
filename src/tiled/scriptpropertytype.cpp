/*
 * scriptpropertytype.cpp
 * Copyright 2024, chris <dogboydog@users.noreply.github.com>
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

#include "scriptpropertytype.h"

#include "editableobject.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

const QString &ScriptPropertyType::name() const
{
    return mType->name;
}

void ScriptPropertyType::setName(const QString &name)
{
    if (this->name() == name)
        return;

    Project &project = ProjectManager::instance()->project();
    if (project.propertyTypes()->findTypeByName(name)) {
        throwDuplicateNameError(name);
        return;
    }

    mType->name = name;
    applyPropertyChanges();
}

void ScriptPropertyType::throwDuplicateNameError(const QString &name)
{
    ScriptManager::instance().throwError(
        QCoreApplication::translate("Script Errors", "The name '%1' is already in use.").arg(name));
}

/*
 * Called when we make a change to a property type -
 * this will reflect any property changes in the UI.
 *
 * TODO: Scheduling project save rather than every time
 */
void ScriptPropertyType::applyPropertyChanges()
{
    emit Preferences::instance()->propertyTypesChanged();

    Project &project = ProjectManager::instance()->project();
    project.save();
}

void ScriptEnumPropertyType::setStorageType(StorageType value)
{
    enumType().storageType = static_cast<EnumPropertyType::StorageType>(value);
    applyPropertyChanges();
}

void ScriptEnumPropertyType::setValues(const QStringList &values)
{
    enumType().values = values;
    applyPropertyChanges();
}

void ScriptEnumPropertyType::addValue(const QString &name)
{
    enumType().values.append(name);
    applyPropertyChanges();
}

QString ScriptEnumPropertyType::nameOf(int value) const
{
    if (value >= 0 && value < enumType().values.size())
        return enumType().values.at(value);

    return QString();
}

QString ScriptEnumPropertyType::nameOf(const QVariant &value) const
{
    const auto propertyValue = value.value<PropertyValue>();
    if (propertyValue.typeId != enumType().id) {
        ScriptManager::instance().throwError(
            QCoreApplication::translate("Script Errors",
                                        "The specified value is not of the correct type"));
    }

    return nameOf(propertyValue.value.toInt());
}


void ScriptClassPropertyType::setMember(const QString &name, const QVariant &value)
{
    QVariant v;

    if (auto scriptPropertyType = value.value<ScriptPropertyType *>())
        v = scriptPropertyType->defaultValue();
    else
        v = EditableObject::propertyValueFromScript(value);

    classType().members.insert(name, v);
    applyPropertyChanges();
}

void ScriptClassPropertyType::removeMember(const QString &name)
{
    if (!classType().members.contains(name))
        return;

    classType().members.remove(name);
    applyPropertyChanges();
}

void registerPropertyTypes(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("EnumPropertyType"),
                                         jsEngine->newQMetaObject<ScriptEnumPropertyType>());
    jsEngine->globalObject().setProperty(QStringLiteral("ClassPropertyType"),
                                         jsEngine->newQMetaObject<ScriptClassPropertyType>());
}

} // namespace Tiled

#include "moc_scriptpropertytype.cpp"
