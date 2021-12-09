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

#include "changeimagelayerproperty.h"

#include "mapdocument.h"
#include "imagelayer.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeImageLayerProperty::ChangeImageLayerProperty(
        MapDocument *mapDocument,
        ImageLayer *imageLayer,
        const QColor transparentColor)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Image Layer Transparent Color"))
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mProperty(TransparentColorProperty)
    , mTransparentColor(transparentColor)
{
}

ChangeImageLayerProperty::ChangeImageLayerProperty(
        MapDocument *mapDocument,
        ImageLayer *imageLayer,
        const QUrl imageSource)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Image Layer Image Source"))
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mProperty(ImageSourceProperty)
    , mImageSource(imageSource)
{
}

ChangeImageLayerProperty::ChangeImageLayerProperty(
        MapDocument *mapDocument,
        ImageLayer *imageLayer,
        ChangeImageLayerProperty::Property property,
        bool repeat)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Image Layer Repeat Property"))
    , mMapDocument(mapDocument)
    , mImageLayer(imageLayer)
    , mProperty(property)
    , mRepeat(repeat)
{
}

void ChangeImageLayerProperty::redo()
{
    swap();
}

void ChangeImageLayerProperty::undo()
{
    swap();
}

void ChangeImageLayerProperty::swap()
{
    switch (mProperty) {
    case TransparentColorProperty: {
        const QColor color = mImageLayer->transparentColor();
        mImageLayer->setTransparentColor(mTransparentColor);
        mTransparentColor = color;
        break;
    }
    case ImageSourceProperty: {
        const QUrl source = mImageLayer->imageSource();
        mImageLayer->setSource(mImageSource);
        mImageSource = source;

        if (mImageSource.isEmpty())
            mImageLayer->resetImage();
        else
            mImageLayer->loadFromImage(mImageSource);

        break;
    }
    case RepeatXProperty: {
        const bool repeatX = mImageLayer->repeatX();
        mImageLayer->setRepeatX(mRepeat);
        mRepeat = repeatX;
        break;
    }
    case RepeatYProperty: {
        const bool repeatY = mImageLayer->repeatY();
        mImageLayer->setRepeatY(mRepeat);
        mRepeat = repeatY;
        break;
    }
    }

    emit mMapDocument->imageLayerChanged(mImageLayer);
}
