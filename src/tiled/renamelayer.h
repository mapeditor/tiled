/*
 * renamelayer.h
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

#ifndef RENAMELAYER_H
#define RENAMELAYER_H

#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

class RenameLayer : public QUndoCommand
{
public:
    RenameLayer(MapDocument *mapDocument,
                int layerIndex,
                const QString &name);

    void undo();
    void redo();

private:
    void swapName();

    MapDocument *mMapDocument;
    int mLayerIndex;
    QString mName;
};

} // namespace Internal
} // namespace Tiled

#endif // RENAMELAYER_H
