/*
 * main.cpp
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

#include "pluginmanager.h"
#include "tmxrasterizer.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include <QStringList>
#include <QUrl>

static QString localFile(const QString &fileNameOrUrl)
{
    const QUrl url(fileNameOrUrl);
    return url.isLocalFile() ? url.toLocalFile() : fileNameOrUrl;
}

int main(int argc, char *argv[])
{
    Tiled::increaseImageAllocationLimit();

    QGuiApplication app(argc, argv);

    app.setOrganizationDomain(QLatin1String("mapeditor.org"));
    app.setApplicationName(QLatin1String("TmxRasterizer"));
    app.setApplicationVersion(QLatin1String("1.0"));

    PluginManager::instance()->loadPlugins();

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Renders a Tiled map or world to an image."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
                          { { QStringLiteral("s"), QStringLiteral("scale") },
                            QCoreApplication::translate("main", "The scale of the output image (default: 1)."),
                            QCoreApplication::translate("main", "scale") },
                          { { QStringLiteral("t"), QStringLiteral("tilesize") },
                            QCoreApplication::translate("main", "The requested size in pixels at which a tile is rendered (overrides the --scale option)."),
                            QCoreApplication::translate("main", "size") },
                          { QStringLiteral("size"),
                            QCoreApplication::translate("main", "The output image fits within a SIZE x SIZE square (overrides the --scale and --tilesize options)."),
                            QCoreApplication::translate("main", "size") },
                          { { QStringLiteral("a"), QStringLiteral("anti-aliasing") },
                            QCoreApplication::translate("main", "Antialias edges of primitives.") },
                          { QStringLiteral("no-smoothing"),
                            QCoreApplication::translate("main", "Use nearest neighbour instead of smooth blending of pixels.") },
                          { QStringLiteral("ignore-visibility"),
                            QCoreApplication::translate("main", "Ignore all layer visibility flags in the map file, and render all layers in the output (default is to omit invisible layers).") },
                          { QStringLiteral("hide-layer"),
                            QCoreApplication::translate("main", "Specifies a layer to omit from the output image. Can be repeated to hide multiple layers."),
                            QCoreApplication::translate("main", "name") },
                          { QStringLiteral("show-layer"),
                            QCoreApplication::translate("main", "If used only specified layers are shown. Can be repeated to show multiple specified layers only."),
                            QCoreApplication::translate("main", "name") },
                          { QStringLiteral("hide-tile-layers"),
                            QCoreApplication::translate("main", "Don't render tile layers.") },
                          { QStringLiteral("hide-object-layers"),
                            QCoreApplication::translate("main", "Don't render object layers.") },
                          { QStringLiteral("hide-image-layers"),
                            QCoreApplication::translate("main", "Don't render image layers.") },
                          { QStringLiteral("hide-object"),
                            QCoreApplication::translate("main", "Specifies an object to omit from the output image. Can be repeated to hide multiple objects. If multiple objects share the specified name they all will be hidden."),
                            QCoreApplication::translate("main", "name") },
                          { QStringLiteral("show-object"),
                            QCoreApplication::translate("main", "If used only specified objects are shown. Can be repeated to show multiple specified objects only. If multiple objects share the specified name they all will be shown."),
                            QCoreApplication::translate("main", "name") },
                          { QStringLiteral("advance-animations"),
                            QCoreApplication::translate("main", "If used, tile animations are advanced by the specified duration in milliseconds."),
                            QCoreApplication::translate("main", "duration") },
                          { QStringLiteral("frames"),
                            QCoreApplication::translate("main", "Number of frames to export. This will add a frame number suffix to the image names. Animations are advanced by <frame-duration> for each frame."),
                            QCoreApplication::translate("main", "number") },
                          { QStringLiteral("frame-duration"),
                            QCoreApplication::translate("main", "Duration of each frame in milliseconds, defaults to 100."),
                            QCoreApplication::translate("main", "number") },
                      });
    parser.addPositionalArgument(QStringLiteral("map|world"), QCoreApplication::translate("main", "Map or world file to render."));
    parser.addPositionalArgument(QStringLiteral("image"), QCoreApplication::translate("main", "Image file to output."));
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.size() != 2)
        parser.showHelp(1);

    const QString &fileToOpen = localFile(args.at(0));
    const QString &fileToSave = args.at(1);

    if (fileToOpen.isEmpty() || fileToSave.isEmpty())
        parser.showHelp(1);

    TmxRasterizer w;
    w.setAntiAliasing(parser.isSet(QLatin1String("anti-aliasing")));
    w.setSmoothImages(!parser.isSet(QLatin1String("no-smoothing")));
    w.setIgnoreVisibility(parser.isSet(QLatin1String("ignore-visibility")));
    w.setLayersToHide(parser.values(QLatin1String("hide-layer")));
    w.setLayersToShow(parser.values(QLatin1String("show-layer")));
    w.setLayerTypeVisible(Layer::TileLayerType, !parser.isSet(QLatin1String("hide-tile-layers")));
    w.setLayerTypeVisible(Layer::ObjectGroupType, !parser.isSet(QLatin1String("hide-object-layers")));
    w.setLayerTypeVisible(Layer::ImageLayerType, !parser.isSet(QLatin1String("hide-image-layers")));
    w.setObjectsToHide(parser.values(QLatin1String("hide-object")));
    w.setObjectsToShow(parser.values(QLatin1String("show-object")));

    if (parser.isSet(QLatin1String("size"))) {
        bool ok;
        w.setSize(parser.value(QLatin1String("size")).toInt(&ok));
        if (!ok || w.size() <= 0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid size specified: \"%1\"").arg(parser.value(QLatin1String("size")));
            exit(1);
        }
    }

    if (parser.isSet(QLatin1String("tilesize"))) {
        bool ok;
        w.setTileSize(parser.value(QLatin1String("tilesize")).toInt(&ok));
        if (!ok || w.tileSize() <= 0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid tile size specified: \"%1\"").arg(parser.value(QLatin1String("tilesize")));
            exit(1);
        }
    }

    if (parser.isSet(QLatin1String("scale"))) {
        bool ok;
        w.setScale(parser.value(QLatin1String("scale")).toDouble(&ok));
        if (!ok || w.scale() <= 0.0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid scale specified: \"%1\"").arg(parser.value(QLatin1String("scale")));
            exit(1);
        }
    }

    if (parser.isSet(QLatin1String("advance-animations"))) {
        bool ok;
        w.setAdvanceAnimations(parser.value(QLatin1String("advance-animations")).toInt(&ok));
        if (!ok || w.advanceAnimations() < 0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid advance-animations specified: \"%1\"").arg(parser.value(QLatin1String("advance-animations")));
            exit(1);
        }
    }

    if (parser.isSet(QLatin1String("frames"))) {
        bool ok;
        w.setFrameCount(parser.value(QLatin1String("frames")).toInt(&ok));
        if (!ok || w.frameCount() < 0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid number of frames specified: \"%1\"").arg(parser.value(QLatin1String("frames")));
            exit(1);
        }
    }

    if (parser.isSet(QLatin1String("frame-duration"))) {
        bool ok;
        w.setFrameDuration(parser.value(QLatin1String("frame-duration")).toInt(&ok));
        if (!ok || w.frameDuration() < 0) {
            qWarning().noquote() << QCoreApplication::translate("main", "Invalid frame-duration specified: \"%1\"").arg(parser.value(QLatin1String("frame-duration")));
            exit(1);
        }
    }

    return w.render(fileToOpen, fileToSave);
}
