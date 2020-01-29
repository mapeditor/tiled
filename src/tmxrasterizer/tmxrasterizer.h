/*
 * tmxrasterizer.h
 * Copyright 2012, Vincent Petithory <vincent.petithory@gmail.com>
 *
 * This file is part of the TMX Rasterizer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "layer.h"

#include "map.h"
#include "mapreader.h"
#include <QString>
#include <QStringList>

using namespace Tiled;

class QImage;
class QPainter;

class TmxRasterizer
{

public:
    TmxRasterizer();

    qreal scale() const { return mScale; }
    int tileSize() const { return mTileSize; }
    int size() const { return mSize; }
    bool useAntiAliasing() const { return mUseAntiAliasing; }
    bool smoothImages() const { return mSmoothImages; }
    bool ignoreVisibility() const { return mIgnoreVisibility; }

    void setScale(qreal scale) { mScale = scale; }
    void setTileSize(int tileSize) { mTileSize = tileSize; }
    void setSize(int size) { mSize = size; }
    void setAntiAliasing(bool useAntiAliasing) { mUseAntiAliasing = useAntiAliasing; }
    void setSmoothImages(bool smoothImages) { mSmoothImages = smoothImages; }
    void setIgnoreVisibility(bool IgnoreVisibility) { mIgnoreVisibility = IgnoreVisibility; }

    void setLayersToHide(QStringList layersToHide) { mLayersToHide = layersToHide; }

    int render(const QString &fileName, const QString &imageFileName);

private:
    qreal mScale;
    int mTileSize;
    int mSize;
    bool mUseAntiAliasing;
    bool mSmoothImages;
    bool mIgnoreVisibility;
    QStringList mLayersToHide;

    void drawMapLayers(MapRenderer &renderer, QPainter &painter, Map &map, QPoint mapOffset = QPoint(0, 0)) const;
    int renderMap(const QString &mapFileName, const QString &imageFileName);
    int renderWorld(const QString &worldFileName, const QString &imageFileName);
    int saveImage(const QString &imageFileName, const QImage &image) const;
    bool shouldDrawLayer(const Layer *layer) const;
};
