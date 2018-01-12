/*
 * replacetemplate.h
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

#pragma once

#include <QUndoCommand>

namespace Tiled {

class ObjectTemplate;

namespace Internal {

class MapDocument;

class ReplaceTemplate : public QUndoCommand
{
public:
    ReplaceTemplate(MapDocument *mapDocument,
                    const ObjectTemplate *oldObjectTemplate,
                    const ObjectTemplate *newObjectTemplate);

    ~ReplaceTemplate();

    void undo() { swap(); }
    void redo() { swap(); }

private:
    void swap();

    MapDocument *mMapDocument;
    const ObjectTemplate *mOldObjectTemplate;
    const ObjectTemplate *mNewObjectTemplate;
};

} // namespace Internal
} // namespace Tiled
