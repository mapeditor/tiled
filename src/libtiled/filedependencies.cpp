/*
 * filedependencies.cpp
 * Copyright 2024, File Dependencies Contributor
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

#include "filedependencies.h"

#include "imagelayer.h"
#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "tile.h"
#include "tileset.h"

#include <QCoreApplication>
#include <QSet>
#include <QUrl>

namespace Tiled {

static QString urlToPath(const QUrl &url)
{
    return url.isLocalFile() ? url.toLocalFile() : url.toString();
}

static bool isMappable(const QString &filePath)
{
    const QString lowerPath = filePath.toLower();
    return lowerPath.endsWith(QLatin1String(".tmx")) ||
           lowerPath.endsWith(QLatin1String(".tsx")) ||
           lowerPath.endsWith(QLatin1String(".tx"));
}

static void collectFromTilesetInternal(const Tileset *tileset, QList<FileReference> &references, QSet<QString> &seenPaths)
{
    if (!tileset)
        return;

    // External tileset file
    if (!tileset->fileName().isEmpty()) {
        const QString path = tileset->fileName();
        if (!seenPaths.contains(path)) {
            seenPaths.insert(path);
            references.append({path, QCoreApplication::translate("FileDependencies", "Tileset"), isMappable(path)});
        }
    }

    // Tileset image
    if (!tileset->imageSource().isEmpty()) {
        const QString path = urlToPath(tileset->imageSource());
        if (!seenPaths.contains(path)) {
            seenPaths.insert(path);
            references.append({path, QCoreApplication::translate("FileDependencies", "Tileset Image"), isMappable(path)});
        }
    }

    // Per-tile images (for image collection tilesets)
    for (const Tile *tile : tileset->tiles()) {
        if (!tile->imageSource().isEmpty()) {
            const QString path = urlToPath(tile->imageSource());
            if (!seenPaths.contains(path)) {
                seenPaths.insert(path);
                references.append({path, QCoreApplication::translate("FileDependencies", "Tile Image"), isMappable(path)});
            }
        }
    }
}

QList<FileReference> FileDependencyCollector::collectFromTileset(const Tileset *tileset)
{
    QList<FileReference> references;
    QSet<QString> seenPaths;
    collectFromTilesetInternal(tileset, references, seenPaths);
    return references;
}

QList<FileReference> FileDependencyCollector::collectFromMap(const Map *map)
{
    QList<FileReference> references;
    QSet<QString> seenPaths;

    if (!map)
        return references;

    // Collect from all map tilesets
    for (const SharedTileset &tileset : map->tilesets()) {
        collectFromTilesetInternal(tileset.data(), references, seenPaths);
    }

    // Collect from layers (image layers, object templates)
    for (Layer *layer : map->allLayers()) {
        if (ImageLayer *imageLayer = layer->asImageLayer()) {
            if (!imageLayer->imageSource().isEmpty()) {
                const QString path = urlToPath(imageLayer->imageSource());
                if (!seenPaths.contains(path)) {
                    seenPaths.insert(path);
                    references.append({path, QCoreApplication::translate("FileDependencies", "Image Layer"), isMappable(path)});
                }
            }
        } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            for (const MapObject *mapObject : *objectGroup) {
                if (const ObjectTemplate *objectTemplate = mapObject->objectTemplate()) {
                    // Template file itself
                    if (!objectTemplate->fileName().isEmpty()) {
                        const QString path = objectTemplate->fileName();
                        if (!seenPaths.contains(path)) {
                            seenPaths.insert(path);
                            references.append({path, QCoreApplication::translate("FileDependencies", "Object Template"), isMappable(path)});
                        }
                    }

                    // Template's tileset
                    if (const MapObject *templateObject = objectTemplate->object()) {
                        if (const Tileset *templateTileset = templateObject->cell().tileset()) {
                            // Recursively collect from the template's tileset.
                            // We use a different translated type name here if it's the main file,
                            // but internal collection labels it as "Tileset".
                            if (!templateTileset->fileName().isEmpty()) {
                                const QString path = templateTileset->fileName();
                                if (!seenPaths.contains(path)) {
                                    seenPaths.insert(path);
                                    references.append({path, QCoreApplication::translate("FileDependencies", "Template Tileset"), isMappable(path)});
                                }
                            }
                            // Also need to get its images if not seen yet
                            collectFromTilesetInternal(templateTileset, references, seenPaths);
                        }
                    }
                }
            }
        }
    }

    return references;
}

} // namespace Tiled
