/*
 * automapperwrapper.h
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2018-2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "painttilelayer.h"

#include <QVector>

namespace Tiled {

class AutoMapper;

/**
 * This is a wrapper class for applying the changes by one or more AutoMapper
 * instances, providing undo/redo functionality.
 *
 * It derives from PaintTileLayer so that the changes can be merged with the
 * previous paint operation.
 */
class AutoMapperWrapper : public PaintTileLayer
{
public:
    AutoMapperWrapper(MapDocument *mapDocument,
                      const QVector<AutoMapper*> &autoMappers,
                      const QRegion &where,
                      const TileLayer *touchedLayer = nullptr);
};

} // namespace Tiled
