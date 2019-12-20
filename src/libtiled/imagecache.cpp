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

#include "map.h"
#include "mapreader.h"
#include "orthogonalrenderer.h"
#include "logginginterface.h"
#include "objectgroup.h"
#include "imagelayer.h"
#include "qtcompat_p.h"

#include <QBitmap>
#include <QFileInfo>

namespace Tiled {

bool TilesheetParameters::operator==(const TilesheetParameters &other) const
{
    return fileName == other.fileName &&
            tileWidth == other.tileWidth &&
            tileHeight == other.tileHeight &&
            margin == other.margin &&
            spacing == other.spacing &&
            transparentColor == other.transparentColor;
}

uint qHash(const TilesheetParameters &key, uint seed) Q_DECL_NOTHROW
{
    uint h = ::qHash(key.fileName, seed);
    h = ::qHash(key.tileWidth, h);
    h = ::qHash(key.tileHeight, h);
    h = ::qHash(key.margin, h);
    h = ::qHash(key.spacing, h);
    h = ::qHash(key.transparentColor.rgba(), h);
    return h;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

struct LoadedMapRender {
    explicit LoadedMapRender(const Map *map, LoadedImage image);

    const Map *map;
    LoadedImage image;
};

LoadedImage::LoadedImage(const QString &fileName)
    : image(fileName)
    , lastModified(QFileInfo(fileName).lastModified())
{}

LoadedImage::LoadedImage(QImage &image, const QDateTime &lastModified)
    : image(std::move(image)),
      lastModified(lastModified)
{}

LoadedPixmap::LoadedPixmap(const LoadedImage &cachedImage)
    : pixmap(QPixmap::fromImage(cachedImage))
    , lastModified(cachedImage.lastModified)
{}


LoadedMapRender::LoadedMapRender(const Map *map, LoadedImage image)
    : map(map),
      image(image)
{}

QHash<QString, LoadedImage> ImageCache::sLoadedImages;
QHash<QString, LoadedPixmap> ImageCache::sLoadedPixmaps;
QHash<QString, LoadedMapRender> ImageCache::sLoadedMapRenders;
QHash<TilesheetParameters, CutTiles> ImageCache::sCutTiles;
std::vector<std::unique_ptr<Map>> ImageCache::sLoadedMaps;

LoadedImage ImageCache::loadImage(const QString &fileName)
{
    auto it = sLoadedImages.find(fileName);

    bool found = it != sLoadedImages.end();
    // Keep image loading fast by only doing a string compare if we failed to find
    // it. 99% of the time we're only loading images.
    QFileInfo info(fileName);
    if (!found && info.suffix().compare(QLatin1String("tmx"), Qt::CaseInsensitive) == 0)
        return loadMapRender(fileName);


    bool old = found && it.value().lastModified < info.lastModified();

    if (old)
        remove(fileName);
    if (old || !found) {
        it = sLoadedImages.insert(fileName, LoadedImage(fileName));
    }

    return it.value();
}

QPixmap ImageCache::loadPixmap(const QString &fileName)
{
    auto it = sLoadedPixmaps.find(fileName);

    bool found = it != sLoadedPixmaps.end();
    bool old = found && it.value().lastModified < QFileInfo(fileName).lastModified();

    if (old)
        remove(fileName);
    if (old || !found)
        it = sLoadedPixmaps.insert(fileName, LoadedPixmap(loadImage(fileName)));

    return it.value();
}

static CutTiles cutTilesImpl(const TilesheetParameters &p)
{
    Q_ASSERT(p.tileWidth > 0 && p.tileHeight > 0);

    const LoadedImage loadedImage = ImageCache::loadImage(p.fileName);
    const QImage &image = loadedImage.image;
    const int stopWidth = image.width() - p.tileWidth;
    const int stopHeight = image.height() - p.tileHeight;
    INFO(QString::number(image.width()) + QLatin1String(" + ") + QString::number(image.height()));

    CutTiles result;
    result.lastModified = loadedImage.lastModified;

    for (int y = p.margin; y <= stopHeight; y += p.tileHeight + p.spacing) {
        for (int x = p.margin; x <= stopWidth; x += p.tileWidth + p.spacing) {
            const QImage tileImage = image.copy(x, y, p.tileWidth, p.tileHeight);
            QPixmap tilePixmap = QPixmap::fromImage(tileImage);

            if (p.transparentColor.isValid()) {
                const QImage mask = tileImage.createMaskFromColor(p.transparentColor.rgb());
                tilePixmap.setMask(QBitmap::fromImage(mask));
            }

            result.tiles.append(tilePixmap);
        }
    }

    return result;
}

QVector<QPixmap> ImageCache::cutTiles(const TilesheetParameters &parameters)
{
    auto it = sCutTiles.find(parameters);

    bool found = it != sCutTiles.end();
    bool old = found && it.value().lastModified < QFileInfo(parameters.fileName).lastModified();

    if (old)
        remove(parameters.fileName);
    if (old || !found)
        it = sCutTiles.insert(parameters, cutTilesImpl(parameters));

    return it.value();
}

void ImageCache::remove(const QString &fileName)
{
    int imagesRemoved = sLoadedImages.remove(fileName);
    if (imagesRemoved == 0) {
        // It wasn't a normal image file. Check if it was a rendered
        // map.
        auto entry = sLoadedMapRenders.find(fileName);
        if (entry != sLoadedMapRenders.end()) {

            auto it = sLoadedMaps.cbegin();
            for (; it != sLoadedMaps.cend(); ++it) {
                if (it->get() == entry.value().map) {
                    break;
                }
            }

            Q_ASSERT(it != sLoadedMaps.cend());

            sLoadedMapRenders.erase(entry);
            sLoadedMaps.erase(it);
        }
    }
    sLoadedPixmaps.remove(fileName);

    // Also remove any previously cut tiles
    QMutableHashIterator<TilesheetParameters, CutTiles> it(sCutTiles);
    while (it.hasNext()) {
        if (it.next().key().fileName == fileName)
            it.remove();
    }
}

LoadedImage ImageCache::loadMapRender(const QString &fileName)
{
    auto it = sLoadedMapRenders.find(fileName);

    bool found = it != sLoadedMapRenders.end();
    bool old = found && it.value().image.lastModified < QFileInfo(fileName).lastModified();

    // TODO: Currently, there is no way to tie a map render to a pre-existing
    // Map instance. So even if the user has the map open in a separate tab,
    // the image will not update until it's saved to disk, and the saved
    // file would be re-parsed.
    if (old)
        remove(fileName);
    if (old || !found) {
        MapReader reader;
        sLoadedMaps.push_back(reader.readMap(fileName));
        auto map = sLoadedMaps.back().get();
        if (!map) {
            ERROR(tr("Failed to read metatile map %1: %2")
                  .arg(fileName).arg(reader.errorString()));
            // TODO: What to return for an error? This produces a loaded image that
            // is guaranteed to be invalid, in a hackey way.
            return LoadedImage(fileName);
        }

        auto rawImage = renderMapToImage(map);
        LoadedImage image(rawImage, QFileInfo(fileName).lastModified());



        it = sLoadedMapRenders.insert(fileName, LoadedMapRender(map, image));
    }

    return it->image;
}

QImage ImageCache::renderMapToImage(const Map *map)
{
    QSize mapSize(map->tileWidth() * map->width(),
                  map->tileHeight() * map->height());
    QImage image(mapSize, QImage::Format_ARGB32_Premultiplied);
    if (map->backgroundColor().isValid())
        image.fill(map->backgroundColor());
    else
        image.fill(Qt::transparent);

    QPainter painter(&image);
    OrthogonalRenderer renderer(map);
    LayerIterator iterator(map);
    while (const Layer *layer = iterator.next()) {
        if (layer->isHidden())
            continue;

        const auto offset = layer->totalOffset();

        painter.setOpacity(layer->effectiveOpacity());
        painter.translate(offset);

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
            renderer.drawTileLayer(&painter, tileLayer);
            break;
        }
        case Layer::ObjectGroupType: {
            const ObjectGroup *objectGroup = static_cast<const ObjectGroup*>(layer);
            QList<MapObject*> objects = objectGroup->objects();

            if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
                std::stable_sort(objects.begin(), objects.end(), objectLessThan);

            for (const MapObject *object : qAsConst(objects)) {
                if (!object->isVisible())
                    continue;

                if (object->rotation() != qreal(0)) {
                    WARNING(QLatin1String("Rotated objects are not supported in metatile maps. Skipping..."));
                    continue;
                }

                if (!object->isTileObject()) {
                    WARNING(QLatin1String("Only tile objects are supported in metatile maps. Skipping..."));
                    continue;
                }

                renderer.drawMapObject(&painter, object, QColor());
            }
            break;
        }
        case Layer::ImageLayerType: {
            const ImageLayer *imageLayer = static_cast<const ImageLayer*>(layer);
            renderer.drawImageLayer(&painter, imageLayer);
            break;
        }
        case Layer::GroupLayerType:
            // Recursion handled by LayerIterator
            break;
        }

        painter.translate(-offset);
    }

    return image;
}

void ImageCache::purgeSubMaps()
{
    // This is safe because we never return a copy of LoadedMapRender, so there are never
    // dangling map pointers.
    sLoadedMapRenders.clear();
    sLoadedMaps.clear();
}

} // namespace Tiled
