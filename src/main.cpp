/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
#include "languagemanager.h"

#include <QApplication>
#include <QDebug>

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
    // TODO: Make translatable
    qWarning() <<
            "Usage: tiled [option] [file]\n\n"
            "Options:\n"
            "  -h --help    : Display this help\n"
            "  -v --version : Display the version";
}

void showVersion()
{
    qWarning() << "Tiled (Qt) Map Editor"
            << qPrintable(QApplication::applicationVersion());
}

void parseCommandLineArguments(CommandLineOptions &options)
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

} // anonymous namespace

int main(int argc, char *argv[])
{
    /*
     * On X11, Tiled uses the 'raster' graphics system, because the X11 native
     * graphics system has performance problems with drawing the tile grid. We
     * still want to allow people to override this with 'native', though.
     */
#ifdef Q_WS_X11
    {
        bool graphicsSystemSpecified = false;
        for (int i = 1; i < argc - 1; ++i) {
            if (strcmp(argv[i], "-graphicssystem") == 0)
                graphicsSystemSpecified = true;
        }
        if (!graphicsSystemSpecified)
            QApplication::setGraphicsSystem(QLatin1String("raster"));
    }
#endif

    QApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("Tiled"));
    a.setApplicationVersion(QLatin1String("0.3.1"));

    LanguageManager *languageManager = LanguageManager::instance();
    languageManager->installTranslators();

    CommandLineOptions options;
    parseCommandLineArguments(options);

    if (options.showVersion)
        showVersion();
    if (options.showHelp)
        showHelp();
    if (options.showVersion || options.showHelp)
        return 0;

    MainWindow w;
    w.show();

    if (!options.fileToOpen.isEmpty())
        w.openFile(options.fileToOpen);
    else
        w.openLastFile();

    return a.exec();
}
