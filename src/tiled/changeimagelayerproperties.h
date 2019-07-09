/*
 * changeimagelayerproperties.h
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>
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

#include <QColor>
#include <QUndoCommand>
#include <QUrl>

namespace Tiled {

class ImageLayer;

class MapDocument;

class ChangeImageLayerProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Image Layer Properties' command.
     *
     * @param mapDocument   the map document of the layer's map
     * @param imageLayer    the image layer to modify
     * @param newColor      the new transparent color to apply
     * @param newSource     the new image source to apply
     */
    ChangeImageLayerProperties(MapDocument *mapDocument,
                               ImageLayer *imageLayer,
                               const QColor &newColor,
                               const QUrl &newSource);

    void undo() override;
    void redo() override;

private:
    MapDocument *mMapDocument;
    ImageLayer *mImageLayer;
    const QColor mUndoColor;
    const QColor mRedoColor;
    const QUrl mUndoSource;
    const QUrl mRedoSource;
};

} // namespace Tiled
