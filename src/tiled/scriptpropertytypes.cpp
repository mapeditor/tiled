/*
 * scriptimage.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptpropertytypes.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"

namespace Tiled {

QString ScriptPropertyType::name() const
{
    return mType->name;
}

size_t ScriptPropertyTypes::count()
{
    return mTypes->count();
}

ScriptPropertyType *ScriptPropertyTypes::toScriptType(const PropertyType *type) const
{
    if (!type)
        return nullptr;

    if (type->isEnum())
        return new ScriptEnumPropertyType(static_cast<const EnumPropertyType *>(type));

    if (type->isClass())
        return new ScriptClassPropertyType(static_cast<const ClassPropertyType *>(type));

    return new ScriptPropertyType(type);
}

ScriptPropertyType *ScriptPropertyTypes::findByName(const QString &name)
{
    const PropertyType *type = mTypes->findTypeByName(name);
    return toScriptType(type);
}

void ScriptPropertyTypes::removeByName(const QString &name)
{
    int index = mTypes->findIndexByName(name);
    if (index < 0 )
        return

    mTypes->removeAt(index);
    applyPropertyChanges();
}

// TODO remove if we can implement an iterator
QVector<ScriptPropertyType *>ScriptPropertyTypes::all() const
{
    QVector<ScriptPropertyType*> scriptTypes;
    for (const PropertyType *type : *mTypes)
        scriptTypes.append(toScriptType(type));
    return scriptTypes;
}

void ScriptPropertyTypes::applyPropertyChanges()
{
    emit Preferences::instance()->propertyTypesChanged();

    Project &project = ProjectManager::instance()->project();
    project.save();
}
void registerPropertyTypes(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("EnumPropertyType"),
                                         jsEngine->newQMetaObject<ScriptEnumPropertyType>());
}

} // namespace Tiled

#include "moc_scriptpropertytypes.cpp"
