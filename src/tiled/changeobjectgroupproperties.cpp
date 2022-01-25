/*
 * changeobjectgroupproperties.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeobjectgroupproperties.h"

#include "changeevents.h"
#include "document.h"

#include <QCoreApplication>

namespace Tiled {

ChangeObjectGroupColor::ChangeObjectGroupColor(Document *document,
                                               QList<ObjectGroup *> objectGroups,
                                               const QColor &newColor)
    : ChangeValue<ObjectGroup, QColor>(document, std::move(objectGroups), newColor)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Object Layer Color"));
}

QColor ChangeObjectGroupColor::getValue(const ObjectGroup *objectGroup) const
{
    return objectGroup->color();
}

void ChangeObjectGroupColor::setValue(ObjectGroup *objectGroup, const QColor &value) const
{
    objectGroup->setColor(value);
    emit document()->changed(ObjectGroupChangeEvent(objectGroup, ObjectGroupChangeEvent::ColorProperty));
}


ChangeObjectGroupDrawOrder::ChangeObjectGroupDrawOrder(Document *document,
                                                       QList<ObjectGroup *> objectGroups,
                                                       ObjectGroup::DrawOrder newDrawOrder)
    : ChangeValue<ObjectGroup, ObjectGroup::DrawOrder>(document, std::move(objectGroups), newDrawOrder)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Object Layer Draw Order"));
}

ObjectGroup::DrawOrder ChangeObjectGroupDrawOrder::getValue(const ObjectGroup *objectGroup) const
{
    return objectGroup->drawOrder();
}

void ChangeObjectGroupDrawOrder::setValue(ObjectGroup *objectGroup, const ObjectGroup::DrawOrder &value) const
{
    objectGroup->setDrawOrder(value);
    emit document()->changed(ObjectGroupChangeEvent(objectGroup, ObjectGroupChangeEvent::DrawOrderProperty));
}

} // namespace Tiled
