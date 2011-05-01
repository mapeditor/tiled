/*
 * changetileselection.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CHANGETILESELECTION_H
#define CHANGETILESELECTION_H

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

class ChangeTileSelection : public QUndoCommand
{
public:
    /**
     * Creates an undo command that sets the selection of \a mapDocument to
     * the given \a selection.
     */
    ChangeTileSelection(MapDocument *mapDocument,
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

#endif // CHANGETILESELECTION_H
