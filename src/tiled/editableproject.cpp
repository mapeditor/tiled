/*
 * editableproject.cpp
 * Copyright 2023, dogboydog
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableproject.h"
#include "projectdocument.h"
#include "projectmanager.h"

namespace Tiled {

EditableProject::EditableProject(ProjectDocument *projectDocument, QObject *parent)
    : EditableAsset(&projectDocument->project(), parent)
{
    setDocument(projectDocument);
}

QString EditableProject::extensionsPath() const
{
    return project()->mExtensionsPath;
}

QString EditableProject::automappingRulesFile() const
{
    return project()->mAutomappingRulesFile;
}

QString EditableProject::fileName() const
{
    return project()->fileName();
}

QStringList EditableProject::folders() const
{
    return project()->folders();
}


bool EditableProject::isReadOnly() const
{
    return false;
}

QSharedPointer<Document> EditableProject::createDocument()
{
    // We don't currently support opening a project in a tab, which this
    // function is meant for.
    return nullptr;
}


ScriptPropertyType *EditableProject::toScriptType(const PropertyType *type) const
{
    if (!type)
        return nullptr;

    if (type->isEnum())
        return new ScriptEnumPropertyType(static_cast<const EnumPropertyType *>(type));

    if (type->isClass())
        return new ScriptClassPropertyType(static_cast<const ClassPropertyType *>(type));

    return new ScriptPropertyType(type);
}

ScriptPropertyType *EditableProject::findTypeByName(const QString &name)
{
    const PropertyType *type = project()->propertyTypes()->findTypeByName(name);
    return toScriptType(type);
}

void EditableProject::removeTypeByName(const QString &name)
{
    int index = project()->propertyTypes()->findIndexByName(name);
    if (index < 0 )
        return

    // TODO the type isn't actually being deleted even when index >= 0
    project()->propertyTypes()->removeAt(index);
    applyPropertyChanges();
}

QVector<ScriptPropertyType *>EditableProject::propertyTypes() const
{
    QVector<ScriptPropertyType*> scriptTypes;
    for (const PropertyType *type : *project()->propertyTypes())
        scriptTypes.append(toScriptType(type));
    return scriptTypes;
}

void EditableProject::applyPropertyChanges()
{
    emit Preferences::instance()->propertyTypesChanged();

    Project &project = ProjectManager::instance()->project();
    project.save();
}

} // namespace Tiled

#include "moc_editableproject.cpp"
