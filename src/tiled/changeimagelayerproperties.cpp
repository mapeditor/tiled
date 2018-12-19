/*
 * changeimagelayerproperties.cpp
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

#include "changeimagelayerproperties.h"

#include "mapdocument.h"
#include "imagelayer.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeImageLayerProperties::ChangeImageLayerProperties(
        MapDocument *mapDocument,
        ImageLayer *imageLayer,
        const QColor &color,
        const QUrl &newSource)
    : QUndoCommand(
          QCoreApplication::translate(
              "Undo Commands", "Change Image Layer Properties"))
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mUndoColor(imageLayer->transparentColor())
    , mRedoColor(color)
    , mUndoSource(imageLayer->imageSource())
    , mRedoSource(newSource)
{
}

void ChangeImageLayerProperties::redo()
{
    mImageLayer->setTransparentColor(mRedoColor);

    if (mRedoSource.isEmpty())
        mImageLayer->resetImage();
    else
        mImageLayer->loadFromImage(mRedoSource);

    emit mMapDocument->imageLayerChanged(mImageLayer);
}

void ChangeImageLayerProperties::undo()
{
    mImageLayer->setTransparentColor(mUndoColor);

    if (mUndoSource.isEmpty())
        mImageLayer->resetImage();
    else
        mImageLayer->loadFromImage(mUndoSource);

    emit mMapDocument->imageLayerChanged(mImageLayer);
}

