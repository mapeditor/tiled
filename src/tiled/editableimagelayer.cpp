/*
 * editableimagelayer.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableimagelayer.h"

#include "changeimagelayerproperty.h"
#include "editablemap.h"
#include "scriptimage.h"

namespace Tiled {

EditableImageLayer::EditableImageLayer(const QString &name, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new ImageLayer(name, 0, 0)), parent)
{
}

EditableImageLayer::EditableImageLayer(EditableMap *map, ImageLayer *imageLayer, QObject *parent)
    : EditableLayer(map, imageLayer, parent)
{
}

void EditableImageLayer::setTransparentColor(const QColor &transparentColor)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerProperty(doc,
                                                   imageLayer(),
                                                   transparentColor));
    } else {
        imageLayer()->setTransparentColor(transparentColor);
        if (!imageSource().isEmpty())
            imageLayer()->loadFromImage(imageSource());
    }
}

void EditableImageLayer::setImageSource(const QUrl &imageSource)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerProperty(doc,
                                                   imageLayer(),
                                                   imageSource));
    } else {
        if (imageSource.isEmpty())
            imageLayer()->resetImage();
        else
            imageLayer()->loadFromImage(imageSource);
    }
}

void EditableImageLayer::setImage(ScriptImage *image, const QUrl &source)
{
    // WARNING: This function has no undo!
    imageLayer()->loadFromImage(QPixmap::fromImage(image->image()), source);
}

void EditableImageLayer::setRepeatX(bool repeatX)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerProperty(doc,
                                                   imageLayer(),
                                                   ChangeImageLayerProperty::RepeatXProperty,
                                                   repeatX));
    } else {
        imageLayer()->setRepeatX(repeatX);
    }
}

void EditableImageLayer::setRepeatY(bool repeatY)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerProperty(doc,
                                                   imageLayer(),
                                                   ChangeImageLayerProperty::RepeatYProperty,
                                                   repeatY));
    } else {
        imageLayer()->setRepeatY(repeatY);
    }
}
} // namespace Tiled

#include "moc_editableimagelayer.cpp"
