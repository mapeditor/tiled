/*
 * changeimagelayerproperty.h
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

#pragma once

#include "changevalue.h"
#include "undocommands.h"

#include <QColor>
#include <QUrl>

namespace Tiled {

class ImageLayer;

class ChangeImageLayerTransparentColor : public ChangeValue<ImageLayer, QColor>
{
public:
    ChangeImageLayerTransparentColor(Document *document,
                                     QList<ImageLayer *> imageLayers,
                                     const QColor &newColor);

private:
    QColor getValue(const ImageLayer *imageLayer) const override;
    void setValue(ImageLayer *imageLayer, const QColor &value) const override;
};

class ChangeImageLayerImageSource : public ChangeValue<ImageLayer, QUrl>
{
public:
    ChangeImageLayerImageSource(Document *document,
                                QList<ImageLayer *> imageLayers,
                                const QUrl &imageSource);

private:
    QUrl getValue(const ImageLayer *imageLayer) const override;
    void setValue(ImageLayer *imageLayer, const QUrl &value) const override;
};

class ChangeImageLayerRepeatX : public ChangeValue<ImageLayer, bool>
{
public:
    ChangeImageLayerRepeatX(Document *document,
                            QList<ImageLayer *> imageLayers,
                            bool repeatX);

    int id() const override { return Cmd_ChangeImageLayerRepeatX; }

private:
    bool getValue(const ImageLayer *imageLayer) const override;
    void setValue(ImageLayer *imageLayer, const bool &value) const override;
};

class ChangeImageLayerRepeatY : public ChangeValue<ImageLayer, bool>
{
public:
    ChangeImageLayerRepeatY(Document *document,
                            QList<ImageLayer *> imageLayers,
                            bool repeatY);

    int id() const override { return Cmd_ChangeImageLayerRepeatY; }

private:
    bool getValue(const ImageLayer *imageLayer) const override;
    void setValue(ImageLayer *imageLayer, const bool &value) const override;
};

} // namespace Tiled
