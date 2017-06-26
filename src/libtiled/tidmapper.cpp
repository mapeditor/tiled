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

ObjectTemplate *TidMapper::tidToTemplate(unsigned tid, bool &ok) const
{
    ObjectTemplate *objectTemplate;

    QMap<unsigned, TemplateGroup*>::const_iterator i = mFirstTidToTemplateGroup.upperBound(tid);

    // This works exactly like GidMapper
    if (i == mFirstTidToTemplateGroup.begin()) {
        ok = false;
    } else if (isEmpty()) {
        ok = false;
    } else {
        --i;

        int index = tid - i.key();
        TemplateGroup *templateGroup = i.value();

        if (!templateGroup->loaded()) { // Create blank placeholder object
            ok = true;
            objectTemplate = blankObjectTemplate();
        } else if (index < templateGroup->templateCount()) { // Make sure that the index is valid
            ok = true;
            objectTemplate = templateGroup->templateAt(index);
        } else {
            ok = false;
            objectTemplate = nullptr;
        }
    }

    return objectTemplate;
}
