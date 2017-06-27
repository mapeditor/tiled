/*
 * tidmapper.h
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

#pragma once

#include "tiled_global.h"

#include <QMap>

namespace Tiled {

class TemplateGroup;
class ObjectTemplate;

class TILEDSHARED_EXPORT TidMapper
{
public:
    TidMapper();

    QMap<unsigned, TemplateGroup*> mFirstTidToTemplateGroup;

    void insert(unsigned firstTid, TemplateGroup *templateGroup);
    void clear();
    bool isEmpty() const;

    ObjectTemplate *tidToTemplate(unsigned tid, bool &ok) const;
};

inline bool TidMapper::isEmpty() const
{ return mFirstTidToTemplateGroup.isEmpty(); }

inline void TidMapper::clear()
{ mFirstTidToTemplateGroup.clear(); }

} // namespace Tiled
