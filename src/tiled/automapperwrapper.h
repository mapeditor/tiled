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

#pragma once

#include "automapper.h"

#include <QUndoCommand>
#include <QVector>

#include <vector>

namespace Tiled {

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
    AutoMapperWrapper(MapDocument *mapDocument,
                      QVector<AutoMapper*> autoMappers,
                      QRegion *where);
    ~AutoMapperWrapper() override;

    void undo() override;
    void redo() override;

private:
    void patchLayer(int layerIndex, const TileLayer &layer);

    MapDocument *mMapDocument;
    std::vector<std::unique_ptr<TileLayer>> mLayersAfter;
    std::vector<std::unique_ptr<TileLayer>> mLayersBefore;
};

} // namespace Tiled
