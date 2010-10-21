/*
 * main.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
