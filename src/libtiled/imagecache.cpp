/*
 * imagecache.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
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

#include "imagecache.h"

#include "logginginterface.h"
#include "map.h"
#include "mapformat.h"
#include "minimaprenderer.h"

#include <QBitmap>
#include <QCoreApplication>
#include <QFileInfo>

namespace Tiled {

struct LoadedPixmap
{
    explicit LoadedPixmap(const LoadedImage &cachedImage);

    operator const QPixmap &() const { return pixmap; }

    QPixmap pixmap;
    QDateTime lastModified;
};


LoadedImage::LoadedImage()
    : LoadedImage(QImage(), QDateTime())
{}

LoadedImage::LoadedImage(QImage image, const QDateTime &lastModified)
    : image(std::move(image))
    , lastModified(lastModified)
{}


LoadedPixmap::LoadedPixmap(const LoadedImage &cachedImage)
    : pixmap(QPixmap::fromImage(cachedImage))
    , lastModified(cachedImage.lastModified)
{}


QHash<QString, LoadedImage> ImageCache::sLoadedImages;
QHash<QString, LoadedPixmap> ImageCache::sLoadedPixmaps;

LoadedImage ImageCache::loadImage(const QString &fileName)
{
    if (fileName.isEmpty())
        return {};

    auto it = sLoadedImages.find(fileName);

    QFileInfo info(fileName);
    bool found = it != sLoadedImages.end();
    bool old = found && it.value().lastModified < info.lastModified();

    if (old)
        remove(fileName);

    if (old || !found) {
        QImage image(fileName);

        // If the image failed to load, try to load and render a map file
        if (image.isNull())
            image = renderMap(fileName);

        it = sLoadedImages.insert(fileName, LoadedImage(image, info.lastModified()));
    }

    return it.value();
}

QPixmap ImageCache::loadPixmap(const QString &fileName)
{
    if (fileName.isEmpty())
        return {};

    auto it = sLoadedPixmaps.find(fileName);

    bool found = it != sLoadedPixmaps.end();
    bool old = found && it.value().lastModified < QFileInfo(fileName).lastModified();

    if (old)
        remove(fileName);
    if (old || !found)
        it = sLoadedPixmaps.insert(fileName, LoadedPixmap(loadImage(fileName)));

    return it.value();
}

void ImageCache::remove(const QString &fileName)
{
    sLoadedImages.remove(fileName);
    sLoadedPixmaps.remove(fileName);
}

QImage ImageCache::renderMap(const QString &fileName)
{
    if (fileName.isEmpty())
        return {};

    static QSet<QString> loadingMaps;

    if (loadingMaps.contains(fileName)) {
        ERROR(QCoreApplication::translate("Tiled::ImageCache",
                                          "Recursive metatile map detected: %1")
              .arg(fileName), OpenFile { fileName });
        return {};
    }

    loadingMaps.insert(fileName);

    QString errorString;
    auto map = Tiled::readMap(fileName, &errorString);

    loadingMaps.remove(fileName);

    if (!map) {
        ERROR(QCoreApplication::translate("Tiled::ImageCache",
                                          "Failed to read metatile map %1: %2")
              .arg(fileName, errorString));

        return {};
    }

    MiniMapRenderer miniMapRenderer(map.get());

    const MiniMapRenderer::RenderFlags renderFlags(MiniMapRenderer::DrawTileLayers |
                                                   MiniMapRenderer::DrawMapObjects |
                                                   MiniMapRenderer::DrawImageLayers |
                                                   MiniMapRenderer::IgnoreInvisibleLayer |
                                                   MiniMapRenderer::DrawBackground);
    const QSize mapSize = miniMapRenderer.mapSize();
    return miniMapRenderer.render(mapSize, renderFlags);
}

} // namespace Tiled
