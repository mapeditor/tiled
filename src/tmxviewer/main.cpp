/*
 * main.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of the TMX Viewer example.
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

#include "tmxviewer.h"

#include <QApplication>
#include <QDebug>

namespace {

struct CommandLineOptions {
    CommandLineOptions()
        : showHelp(false)
        , showVersion(false)
    {}

    bool showHelp;
    bool showVersion;
    QString fileToOpen;
};

} // anonymous namespace

static void showHelp()
{
    // TODO: Make translatable
    qWarning() <<
            "Usage: tmxviewer [option] [file]\n\n"
            "Options:\n"
            "  -h --help    : Display this help\n"
            "  -v --version : Display the version";
}

static void showVersion()
{
    qWarning() << "TMX Map Viewer"
            << qPrintable(QApplication::applicationVersion());
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
        } else if (arg.at(0) == QLatin1Char('-')) {
            qWarning() << "Unknown option" << arg;
            options.showHelp = true;
        } else if (options.fileToOpen.isEmpty()) {
            options.fileToOpen = arg;
        }
    }
}

int main(int argc, char *argv[])
{
    // Avoid performance issues with X11 engine when rendering objects
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

    QApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("TmxViewer"));
    a.setApplicationVersion(QLatin1String("1.0"));

    CommandLineOptions options;
    parseCommandLineArguments(options);

    if (options.showVersion)
        showVersion();
    if (options.showHelp || (options.fileToOpen.isEmpty()
                             && !options.showVersion))
        showHelp();
    if (options.showVersion
            || options.showHelp
            || options.fileToOpen.isEmpty())
        return 0;

    TmxViewer w;
    w.viewMap(options.fileToOpen);
    w.show();

    return a.exec();
}
