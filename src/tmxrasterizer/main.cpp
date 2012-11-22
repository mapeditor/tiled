/*
 * main.cpp
 * Copyright 2012, Vincent Petithory <vincent.petithory@gmail.com>
 * Mostly inspired from tmxviewer by Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QApplication>
#include <QDebug>
#include <QStringList>

namespace {

struct CommandLineOptions {
    CommandLineOptions()
        : showHelp(false)
        , showVersion(false)
    {}

    bool showHelp;
    bool showVersion;
    QString fileToOpen;
    QString fileToSave;
    qreal scale;
    int tileSize;
    bool useAntiAliasing;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qWarning() <<
            "Usage: tmxrasterizer [option] [file] [outfile]\n\n"
            "Options:\n"
            "  -h --help           : Display this help\n"
            "  -v --version        : Display the version\n"
            "  -s --scale SCALE    : The scale of the output image\n"
            "  -t --tilesize SIZE  : The requested size in pixels at which a tile is rendered. Superseeds --scale.";
}

static void showVersion()
{
    qWarning() << "TMX Map Rasterizer"
            << qPrintable(QApplication::applicationVersion());
}

static void parseCommandLineArguments(CommandLineOptions &options)
{
    const QStringList arguments = QCoreApplication::arguments();
    options.scale = 0.0;
    options.tileSize = 0;
    options.useAntiAliasing = false;
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
                options.scale = arguments.at(i).toDouble();
            }
        } else if (arg == QLatin1String("--tilesize")
                || arg == QLatin1String("-t")) {
            i++;
            if (i >= arguments.size()) {
                options.showHelp = true;
            } else {
                options.tileSize = arguments.at(i).toInt();
            }
        } else if (arg == QLatin1String("--anti-aliasing")
                || arg == QLatin1String("-a")) {
            options.useAntiAliasing = true;
        } else if (arg.at(0) == QLatin1Char('-')) {
            qWarning() << "Unknown option" << arg;
            options.showHelp = true;
        } else if (options.fileToOpen.isEmpty()) {
            options.fileToOpen = arg;
        } else if (options.fileToSave.isEmpty()) {
            options.fileToSave = arg;
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("TmxRasterizer"));
    a.setApplicationVersion(QLatin1String("1.0"));

    CommandLineOptions options;
    parseCommandLineArguments(options);

    if (options.showVersion) {
        showVersion();
        return 0;
    }
    if (options.showHelp || (options.fileToOpen.isEmpty() || options.fileToOpen.isEmpty())) {
        showHelp();
        return 0;
    }
    if (options.scale <= 0.0 && options.tileSize <= 0) {
        showHelp();
        return 0;
    }

    TmxRasterizer w;
    w.setAntiAliasing(options.useAntiAliasing);

    if (options.tileSize > 0) {
        w.setTileSize(options.tileSize);
    } else if (options.scale > 0.0) {
        w.setScale(options.scale);
    }

    w.render(options.fileToOpen, options.fileToSave);
    a.quit();
    return 0;
}
