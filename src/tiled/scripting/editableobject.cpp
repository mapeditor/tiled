/*
 * editableobject.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableobject.h"

#include "changeproperties.h"
#include "editableasset.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

EditableObject::EditableObject(EditableAsset *asset,
                               Object *object,
                               QObject *parent)
    : QObject(parent)
    , mAsset(asset)
    , mObject(object)
{
}

bool EditableObject::isReadOnly() const
{
    return asset() && asset()->isReadOnly();
}

void EditableObject::setProperty(const QString &name, const QVariant &value)
{
    if (asset())
        asset()->push(new SetProperty(asset()->document(), { mObject }, name, value));
    else
        mObject->setProperty(name, value);
}

void EditableObject::setProperties(const QVariantMap &properties)
{
    if (asset())
        asset()->push(new ChangeProperties(asset()->document(), QString(), mObject, properties));
    else
        mObject->setProperties(properties);
}

void EditableObject::removeProperty(const QString &name)
{
    if (asset())
        asset()->push(new RemoveProperty(asset()->document(), { mObject }, name));
    else
        mObject->removeProperty(name);
}

Document *EditableObject::document() const
{
    return asset() ? asset()->document() : nullptr;
}

bool EditableObject::checkReadOnly() const
{
    if (isReadOnly()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Asset is read-only"));
        return true;
    }
    return false;
}

} // namespace Tiled
