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

#include "changeevents.h"
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

ScriptImage *EditableImageLayer::image() const
{
    return new ScriptImage(imageLayer()->image().toImage());
}

void EditableImageLayer::setTransparentColor(const QColor &transparentColor)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerTransparentColor(doc,
                                                           { imageLayer() },
                                                           transparentColor));
    } else if (!checkReadOnly()) {
        imageLayer()->setTransparentColor(transparentColor);
        if (!imageSource().isEmpty())
            imageLayer()->loadFromImage(imageSource());
    }
}

void EditableImageLayer::setImageSource(const QUrl &imageSource)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerImageSource(doc,
                                                      { imageLayer() },
                                                      imageSource));
    } else if (!checkReadOnly()) {
        if (imageSource.isEmpty())
            imageLayer()->resetImage();
        else
            imageLayer()->loadFromImage(imageSource);
    }
}

void EditableImageLayer::setImageFileName(const QString &fileName)
{
    setImageSource(QUrl::fromLocalFile(fileName));
}

void EditableImageLayer::setImage(ScriptImage *image, const QUrl &source)
{
    if (checkReadOnly())
        return;

    // WARNING: This function has no undo!
    imageLayer()->loadFromImage(QPixmap::fromImage(image->image()), source);

    if (auto doc = document())
        emit doc->changed(ImageLayerChangeEvent(imageLayer(), ImageLayerChangeEvent::ImageSourceProperty));
}

void EditableImageLayer::setRepeatX(bool repeatX)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerRepeatX(doc,
                                                  { imageLayer() },
                                                  repeatX));
    } else if (!checkReadOnly()) {
        imageLayer()->setRepeatX(repeatX);
    }
}

void EditableImageLayer::setRepeatY(bool repeatY)
{
    if (auto doc = mapDocument()) {
        asset()->push(new ChangeImageLayerRepeatY(doc,
                                                  { imageLayer() },
                                                  repeatY));
    } else if (!checkReadOnly()) {
        imageLayer()->setRepeatY(repeatY);
    }
}
} // namespace Tiled

#include "moc_editableimagelayer.cpp"
