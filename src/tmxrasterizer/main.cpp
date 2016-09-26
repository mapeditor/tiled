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

#include "tmxrasterizer.h"

#include <QGuiApplication>
#include <QDebug>
#include <QStringList>
#include <QUrl>

namespace {

struct CommandLineOptions {
    CommandLineOptions()
        : showHelp(false)
        , showVersion(false)
        , scale(1.0)
        , tileSize(0)
        , size(0)
        , useAntiAliasing(false)
        , ignoreVisibility(false)
    {}

    bool showHelp;
    bool showVersion;
    QString fileToOpen;
    QString fileToSave;
    qreal scale;
    int tileSize;
    int size;
    bool useAntiAliasing;
    bool ignoreVisibility;
    QStringList layersToHide;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qWarning() <<
            "Usage:\n"
            "  tmxrasterizer [options] [input file] [output file]\n"
            "\n"
            "Options:\n"
            "  -h --help               : Display this help\n"
            "  -v --version            : Display the version\n"
            "  -s --scale SCALE        : The scale of the output image (default: 1)\n"
            "  -t --tilesize SIZE      : The requested size in pixels at which a tile is rendered\n"
            "                            Overrides the --scale option\n"
            "     --size SIZE          : The output image fits within a SIZE x SIZE square\n"
            "                            Overrides the --scale and --tilesize options\n"
            "  -a --anti-aliasing      : Smooth the output image using anti-aliasing\n"
            "     --ignore-visibility  : Ignore all layer visibility flags in the map file, and render all\n"
            "                            layers in the output (default is to omit invisible layers)\n"
            "     --hide-layer         : Specifies a layer to omit from the output image\n"
            "                            Can be repeated to hide multiple layers\n";
}

static void showVersion()
{
    qWarning() << "TMX Map Rasterizer"
            << qPrintable(QCoreApplication::applicationVersion());
}

static void parseCommandLineArguments(CommandLineOptions &options)
{
    const QStringList arguments = QCoreApplication::arguments();

    for (int i = 1; i < arguments.size(); ++i) {
        const QString &arg = arguments.at(i);
        if (arg == QLatin1String("--help") || arg == QLatin1String("-h")) {
            options.showHelp = true;
        } else if (arg == QLatin1String("--version")
                || arg == QLatin1String("-v")) {
            options.showVersion = true;
        } else if (arg == QLatin1String("--scale")
                || arg == QLatin1String("-s")) {
            i++;
            if (i >= arguments.size()) {
                options.showHelp = true;
            } else {
                bool scaleIsDouble;
                options.scale = arguments.at(i).toDouble(&scaleIsDouble);
                if (!scaleIsDouble) {
                    qWarning() << arguments.at(i) << ": the specified scale is not a number.";
                    options.showHelp = true;
                }
            }
        } else if (arg == QLatin1String("--tilesize")
                || arg == QLatin1String("-t")) {
            i++;
            if (i >= arguments.size()) {
                options.showHelp = true;
            } else {
                bool tileSizeIsInt;
                options.tileSize = arguments.at(i).toInt(&tileSizeIsInt);
                if (!tileSizeIsInt) {
                    qWarning() << arguments.at(i) << ": the specified tile size is not an integer.";
                    options.showHelp = true;
                }
            }
        } else if (arg == QLatin1String("--size")) {
            i++;
            if (i >= arguments.size()) {
                options.showHelp = true;
            } else {
                bool sizeIsInt;
                options.size = arguments.at(i).toInt(&sizeIsInt);
                if (!sizeIsInt) {
                    qWarning() << arguments.at(i) << ": the specified size is not an integer.";
                    options.showHelp = true;
                }
            }
        } else if (arg == QLatin1String("--hide-layer")) {
            i++;
            if (i >= arguments.size()) {
                options.showHelp = true;
            } else {
                options.layersToHide.append(arguments.at(i));
            }
        } else if (arg == QLatin1String("--anti-aliasing")
                || arg == QLatin1String("-a")) {
            options.useAntiAliasing = true;
        } else if (arg == QLatin1String("--ignore-visibility")) {
            options.ignoreVisibility = true;
        } else if (arg.isEmpty()) {
            options.showHelp = true;
        } else if (arg.at(0) == QLatin1Char('-')) {
            qWarning() << "Unknown option" << arg;
            options.showHelp = true;
        } else if (options.fileToOpen.isEmpty()) {
            const QUrl url(arg);
            if (url.isLocalFile())
                options.fileToOpen = url.toLocalFile();
            else
                options.fileToOpen = arg;
        } else if (options.fileToSave.isEmpty()) {
            options.fileToSave = arg;
        } else {
            // All args are already defined. Show help.
            options.showHelp = true;
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("TmxRasterizer"));
    a.setApplicationVersion(QLatin1String("1.0"));

    CommandLineOptions options;
    parseCommandLineArguments(options);

    if (options.showVersion) {
        showVersion();
        return 0;
    }
    if (options.showHelp || options.fileToOpen.isEmpty() || options.fileToSave.isEmpty()) {
        showHelp();
        return 0;
    }
    if (options.scale <= 0.0 && options.tileSize <= 0) {
        showHelp();
        return 0;
    }

    TmxRasterizer w;
    w.setAntiAliasing(options.useAntiAliasing);
    w.setIgnoreVisibility(options.ignoreVisibility);
    w.setLayersToHide(options.layersToHide);

    if (options.size > 0) {
        w.setSize(options.size);
    } else if (options.tileSize > 0) {
        w.setTileSize(options.tileSize);
    } else if (options.scale > 0.0) {
        w.setScale(options.scale);
    }

    return w.render(options.fileToOpen, options.fileToSave);
}

