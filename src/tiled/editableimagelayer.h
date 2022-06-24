/*
 * editableimagelayer.h
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

#pragma once

#include "editablelayer.h"
#include "imagelayer.h"

namespace Tiled {

class ScriptImage;

class EditableImageLayer : public EditableLayer
{
    Q_OBJECT

    Q_PROPERTY(QColor transparentColor READ transparentColor WRITE setTransparentColor)
    Q_PROPERTY(QUrl imageSource READ imageSource WRITE setImageSource)
    Q_PROPERTY(Tiled::ScriptImage *image READ image WRITE setImage)
    Q_PROPERTY(bool repeatX READ repeatX WRITE setRepeatX)
    Q_PROPERTY(bool repeatY READ repeatY WRITE setRepeatY)

public:
    Q_INVOKABLE explicit EditableImageLayer(const QString &name = QString(),
                                            QObject *parent = nullptr);

    EditableImageLayer(EditableMap *map,
                       ImageLayer *imageLayer,
                       QObject *parent = nullptr);

    const QColor &transparentColor() const;
    const QUrl &imageSource() const;
    ScriptImage *image() const;
    bool repeatX() const;
    bool repeatY() const;

    void setTransparentColor(const QColor &transparentColor);
    void setImageSource(const QUrl &imageSource);
    void setRepeatX(bool repeatX);
    void setRepeatY(bool repeatY);

    Q_INVOKABLE void setImage(Tiled::ScriptImage *image, const QUrl &source = QUrl());

private:
    ImageLayer *imageLayer() const;
};

inline const QColor &EditableImageLayer::transparentColor() const
{
    return imageLayer()->transparentColor();
}

inline const QUrl &EditableImageLayer::imageSource() const
{
    return imageLayer()->imageSource();
}

inline bool EditableImageLayer::repeatX() const
{
    return imageLayer()->repeatX();
}

inline bool EditableImageLayer::repeatY() const
{
    return imageLayer()->repeatY();
}

inline ImageLayer *EditableImageLayer::imageLayer() const
{
    return static_cast<ImageLayer*>(layer());
}

} // namespace Tiled
