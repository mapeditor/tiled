/*
 * tidmapper.cpp
 * Copyright 2017, Your Name <your.name@domain>
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

#include "tidmapper.h"

#include "templategroup.h"

using namespace Tiled;

TidMapper::TidMapper()
{
}

void TidMapper::insert(unsigned firstTid, Tiled::TemplateGroup *templateGroup)
{
    mFirstTidToTemplateGroup.insert(firstTid, templateGroup);
}

TemplateRef TidMapper::tidToTemplateRef(unsigned tid, bool &ok) const
{
    QMap<unsigned, TemplateGroup*>::const_iterator i = mFirstTidToTemplateGroup.upperBound(tid);

    // This works exactly like GidMapper
    if (isEmpty() || i == mFirstTidToTemplateGroup.begin()) {
        ok = false;
        return {nullptr, 0};
    } else {
        --i;

        unsigned id = tid - i.key();
        TemplateGroup *templateGroup = i.value();

        if (templateGroup->loaded()) {
            ok = id < templateGroup->nextTemplateId();
        } else { // We can't be sure if the id is invalid if the template group is not loaded
            ok = true;
            templateGroup->updateMaxId(id);
        }

        return {templateGroup, id};
    }
}

unsigned TidMapper::templateGroupToFirstTid(TemplateGroup *templateGroup) const
{
    return mFirstTidToTemplateGroup.key(templateGroup);
}

unsigned TidMapper::templateRefToTid(TemplateRef templateRef) const
{
    QMapIterator<unsigned, TemplateGroup*> it(mFirstTidToTemplateGroup);

    while (it.hasNext()) {
        it.next();
        if (it.value() == templateRef.templateGroup)
            return templateRef.templateId + it.key();
    }

    return 0;
}
