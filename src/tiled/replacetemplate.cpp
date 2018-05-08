/*
 * replacetemplate.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "replacetemplate.h"

#include "map.h"
#include "mapdocument.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ReplaceTemplate::ReplaceTemplate(MapDocument *mapDocument,
                                 const ObjectTemplate *oldObjectTemplate,
                                 const ObjectTemplate *newObjectTemplate)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Replace Template"))
    , mMapDocument(mapDocument)
    , mOldObjectTemplate(oldObjectTemplate)
    , mNewObjectTemplate(newObjectTemplate)
{
}

ReplaceTemplate::~ReplaceTemplate()
{
}

void ReplaceTemplate::swap()
{
    mMapDocument->replaceObjectTemplate(mOldObjectTemplate,
                                        mNewObjectTemplate);
    std::swap(mOldObjectTemplate, mNewObjectTemplate);
}

} // namespace Internal
} // namespace Tiled
