/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QTranslator>

using namespace Tiled::Internal;

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

void showHelp()
{
    qWarning() <<
            "Usage: tiled [option] [file]\n\n"
            "Options:\n"
            "  -h --help    : Display this help\n"
            "  -v --version : Display the version";
}

void showVersion()
{
    qWarning() << "Tiled (Qt) Map Editor 0.1";
}

void parseCommandLineArguments(CommandLineOptions &options)
{
    QStringList arguments = QCoreApplication::arguments();

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

} // anonymous namespace

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CommandLineOptions options;
    parseCommandLineArguments(options);

    if (options.showVersion)
        showVersion();
    if (options.showHelp)
        showHelp();
    if (options.showVersion || options.showHelp)
        return 0;

    QTranslator translator;
    translator.load(QLatin1String("tiled_") + QLocale::system().name());
    a.installTranslator(&translator);

    a.setOrganizationName(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("Tiled"));

    MainWindow w;
    w.show();

    if (!options.fileToOpen.isEmpty())
        w.openFile(options.fileToOpen);

    return a.exec();
}
