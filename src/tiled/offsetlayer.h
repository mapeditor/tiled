/*
 * offsetlayer.h
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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

#pragma once

#include <QRect>
#include <QPoint>
#include <QUndoCommand>

namespace Tiled {

class Layer;

class MapDocument;

/**
 * Undo command that offsets a map layer.
 */
class OffsetLayer : public QUndoCommand
{
public:
    OffsetLayer(MapDocument *mapDocument,
                Layer *layer,
                QPoint offset,
                const QRect &bounds,
                bool wholeMap,
                bool xWrap,
                bool yWrap);

    ~OffsetLayer() override;

    void undo() override;
    void redo() override;

private:
    MapDocument *mMapDocument;
    bool mDone = false;
    Layer *mOriginalLayer;
    Layer *mOffsetLayer = nullptr;
    QPointF mOldOffset;
    QPointF mNewOffset;
};

} // namespace Tiled
