/*
 * automapperwrapper.h
 * Copyright 2010-2011, Stefan Beller, stefanbeller@googlemail.com
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

#ifndef AUTOMAPPERWRAPPER_H
#define AUTOMAPPERWRAPPER_H

#include "automapper.h"

#include <QUndoCommand>
#include <QVector>

namespace Tiled {

namespace Internal {

class MapDocument;

/**
 * This is a wrapper class for the AutoMapper class.
 * Here in this class only undo/redo functionality all rulemaps
 * is provided.
 * This class will take a snapshot of the layers before and after the
 * automapping is done. In between instances of AutoMapper are doing the work.
 */
class AutoMapperWrapper : public QUndoCommand
{
public:
    AutoMapperWrapper(MapDocument *mapDocument, QVector<AutoMapper*> autoMapper,
                      QRegion *where);
    ~AutoMapperWrapper();

    void undo();
    void redo();

private:
    void patchLayer(int layerIndex, TileLayer *layer);

    MapDocument *mMapDocument;
    QVector<TileLayer*> mLayersAfter;
    QVector<TileLayer*> mLayersBefore;
};

} // namespace Internal
} // namespace Tiled

#endif // AUTOMAPPERWRAPPER_H
