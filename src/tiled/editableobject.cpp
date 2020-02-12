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
    if (Document *doc = document())
        asset()->push(new SetProperty(doc, { mObject }, name, value));
    else if (!checkReadOnly())
        mObject->setProperty(name, value);
}

void EditableObject::setProperties(const QVariantMap &properties)
{
    if (Document *doc = document())
        asset()->push(new ChangeProperties(doc, QString(), mObject, properties));
    else if (!checkReadOnly())
        mObject->setProperties(properties);
}

void EditableObject::removeProperty(const QString &name)
{
    if (Document *doc = document())
        asset()->push(new RemoveProperty(doc, { mObject }, name));
    else if (!checkReadOnly())
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
