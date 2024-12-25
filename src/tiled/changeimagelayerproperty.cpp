/*
 * changeimagelayerproperty.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>
 * Copyright 2010-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeevents.h"
#include "document.h"
#include "imagelayer.h"

#include <QCoreApplication>

namespace Tiled {

ChangeImageLayerTransparentColor::ChangeImageLayerTransparentColor(Document *document,
                                                                   QList<ImageLayer *> imageLayers,
                                                                   const QColor &newColor)
    : ChangeValue<ImageLayer, QColor>(document, std::move(imageLayers), newColor)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Transparent Color"));
}

QColor ChangeImageLayerTransparentColor::getValue(const ImageLayer *imageLayer) const
{
    return imageLayer->transparentColor();
}

void ChangeImageLayerTransparentColor::setValue(ImageLayer *imageLayer, const QColor &value) const
{
    imageLayer->setTransparentColor(value);

    if (imageLayer->imageSource().isEmpty())
        imageLayer->resetImage();
    else
        imageLayer->loadFromImage(imageLayer->imageSource());

    emit document()->changed(ImageLayerChangeEvent(imageLayer, ImageLayerChangeEvent::TransparentColorProperty));
}


ChangeImageLayerImageSource::ChangeImageLayerImageSource(Document *document, QList<ImageLayer *> imageLayers, const QUrl &imageSource)
    : ChangeValue<ImageLayer, QUrl>(document, std::move(imageLayers), imageSource)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Image Source"));
}

QUrl ChangeImageLayerImageSource::getValue(const ImageLayer *imageLayer) const
{
    return imageLayer->imageSource();
}

void ChangeImageLayerImageSource::setValue(ImageLayer *imageLayer, const QUrl &value) const
{
    if (value.isEmpty())
        imageLayer->resetImage();
    else
        imageLayer->loadFromImage(value);

    emit document()->changed(ImageLayerChangeEvent(imageLayer, ImageLayerChangeEvent::ImageSourceProperty));
}


ChangeImageLayerRepeatX::ChangeImageLayerRepeatX(Document *document,
                                                 QList<ImageLayer *> imageLayers,
                                                 bool repeatX)
    : ChangeValue<ImageLayer, bool>(document, std::move(imageLayers), repeatX)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Image Layer Repeat"));
}

bool ChangeImageLayerRepeatX::getValue(const ImageLayer *imageLayer) const
{
    return imageLayer->repeatX();
}

void ChangeImageLayerRepeatX::setValue(ImageLayer *imageLayer, const bool &value) const
{
    imageLayer->setRepeatX(value);
    emit document()->changed(ImageLayerChangeEvent(imageLayer, ImageLayerChangeEvent::RepeatProperty));
}


ChangeImageLayerRepeatY::ChangeImageLayerRepeatY(Document *document,
                                                 QList<ImageLayer *> imageLayers,
                                                 bool repeatY)
    : ChangeValue<ImageLayer, bool>(document, std::move(imageLayers), repeatY)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Image Layer Repeat"));
}

bool ChangeImageLayerRepeatY::getValue(const ImageLayer *imageLayer) const
{
    return imageLayer->repeatY();
}

void ChangeImageLayerRepeatY::setValue(ImageLayer *imageLayer, const bool &value) const
{
    imageLayer->setRepeatY(value);
    emit document()->changed(ImageLayerChangeEvent(imageLayer, ImageLayerChangeEvent::RepeatProperty));
}

} // namespace Tiled
