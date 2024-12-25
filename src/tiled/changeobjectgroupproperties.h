/*
 * changeobjectgroupproperties.h
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

#pragma once

#include "changevalue.h"
#include "objectgroup.h"

#include <QColor>

namespace Tiled {

class ChangeObjectGroupColor : public ChangeValue<ObjectGroup, QColor>
{
public:
    /**
     * Constructs a new 'Change Object Layer Color' command.
     *
     * @param document        the document of the object groups
     * @param objectGroups    the object groups to modify
     * @param newColor        the new color to apply
     */
    ChangeObjectGroupColor(Document *document,
                           QList<ObjectGroup *> objectGroups,
                           const QColor &newColor);

private:
    QColor getValue(const ObjectGroup *objectGroup) const override;
    void setValue(ObjectGroup *objectGroup, const QColor &value) const override;
};

class ChangeObjectGroupDrawOrder : public ChangeValue<ObjectGroup, ObjectGroup::DrawOrder>
{
public:
    /**
     * Constructs a new 'Change Object Layer Draw Order' command.
     *
     * @param document        the document of the object groups
     * @param objectGroups    the object groups to modify
     * @param newDrawOrder    the new drawing order
     */
    ChangeObjectGroupDrawOrder(Document *document,
                               QList<ObjectGroup *> objectGroups,
                               ObjectGroup::DrawOrder newDrawOrder);

private:
    ObjectGroup::DrawOrder getValue(const ObjectGroup *objectGroup) const override;
    void setValue(ObjectGroup *objectGroup, const ObjectGroup::DrawOrder &value) const override;
};

} // namespace Tiled
