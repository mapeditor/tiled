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

#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "scriptpropertytype.h"

namespace Tiled {

const QString &ScriptPropertyType::name() const
{
    return mType->name;
}

void registerPropertyTypes(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("EnumPropertyType"),
                                         jsEngine->newQMetaObject<ScriptEnumPropertyType>());
    jsEngine->globalObject().setProperty(QStringLiteral("ClassPropertyType"),
                                         jsEngine->newQMetaObject<ScriptClassPropertyType>());
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
} // namespace Tiled

#include "moc_scriptpropertytype.cpp"
