/*
 * Tiled Map Editor (Qt)
 * Copyright 2010 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef CHANGEOBJECTGROUPPROPERTIES_H
#define CHANGEOBJECTGROUPPROPERTIES_H

#include <QMap>
#include <QColor>
#include <QString>
#include <QUndoCommand>

namespace Tiled {

class ObjectGroup;

namespace Internal {

class ChangeObjectGroupProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Object Layer Properties' command.
     *
     * @param objectGroup     the object group in to modify
     * @param newProperties   the new properties to apply
     * @param newColor        the new color to apply
     */
    ChangeObjectGroupProperties(MapDocument *mapDocument,
                                ObjectGroup *objectGroup,
                                const QColor &newColor);

    void undo();
    void redo();

private:

    MapDocument *mMapDocument;
    ObjectGroup *mObjectGroup;
    const QColor mUndoColor;
    const QColor mRedoColor;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEOBJECTGROUPPROPERTIES_H
