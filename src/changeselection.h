/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef CHANGESELECTION_H
#define CHANGESELECTION_H

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

class ChangeSelection : public QUndoCommand
{
public:
    /**
     * Creates an undo command that sets the selection of \a mapDocument to
     * the given \a selection.
     */
    ChangeSelection(MapDocument *mapDocument,
                    const QRegion &selection);

    void undo();
    void redo();

private:
    void swapSelection();

    MapDocument *mMapDocument;
    QRegion mSelection;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGESELECTION_H
