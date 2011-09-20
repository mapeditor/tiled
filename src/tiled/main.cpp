/*
 * main.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mainwindow.h"
#include "languagemanager.h"
#include "tiledapplication.h"

#include <QDebug>
#include <QtPlugin>

#ifdef STATIC_BUILD
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qtiff)
#endif

using namespace Tiled::Internal;

namespace {

// a structure containing the resulted of parsing the command-line
// instead of taking action all at once, so we can catch errors early
struct ParsedCommandLine {
    ParsedCommandLine()
        : quit(false)
        , showedHelp(false)
        , showedVersion(false)
        , filesToOpen()
    {}

    bool quit;
    bool showedHelp;
    bool showedVersion;
    QStringList filesToOpen;
};

typedef void (*ArgCallback)(ParsedCommandLine& later, QStringList& rest);

struct PossibleArgument {
    char shortName;
    const char *longName;
    ArgCallback func;
    const char *help;
};

void showHelp(ParsedCommandLine& cl, QStringList&);

void showVersion(ParsedCommandLine& cl, QStringList&)
{
    if (cl.showedVersion)
        return;
    cl.showedVersion = true;
    qWarning() << "Tiled (Qt) Map Editor"
            << qPrintable(QApplication::applicationVersion());
    cl.quit = true;
}

void justQuit(ParsedCommandLine& cl, QStringList&)
{
    cl.quit = true;
}
/// A list of callback functions be be invoked immediately.
// These shouldn't do anything major, until ALL arguments are done.
PossibleArgument possibleArguments[] =
{
    { 'h',      "--help",       showHelp,       "Display this help" },
    { 'v',      "--version",    showVersion,    "Display the version" },
    { '\0',     "--quit",       justQuit,       "Only check validity of arguments, don't actually load any files" },
};

const size_t possibleArgumentCount = sizeof(possibleArguments) / sizeof(possibleArguments[0]);
// must be of type int
const int longestArgument = strlen("--version");

void showHelp(ParsedCommandLine& cl, QStringList&)
{
    if (cl.showedHelp)
        return;
    cl.showedHelp = true;
    // TODO: Make translatable
    qWarning() <<
            "Usage: tiled [option] [files...]\n\n"
            "Options:";
    for (size_t i = 0; i < possibleArgumentCount; i++) {
        char c = possibleArguments[i].shortName;
        const char *l = possibleArguments[i].longName;
        const char *h = possibleArguments[i].help;
        if (c)
            qWarning("  -%c %-*s : %s", c, longestArgument, l, h);
        else
            qWarning("     %-*s : %s", longestArgument, l, h);
    }
    cl.quit = true;
}

ParsedCommandLine parseCommandLineArguments()
{
    ParsedCommandLine out;
    QStringList arguments = QCoreApplication::arguments();
    //std::cout << "invoked as " << arguments.first();
    arguments.removeFirst();

    size_t idx = 0;
    bool noMoreArguments = false;
    while (!arguments.empty()) {
        idx++;
        const QString arg = arguments.takeFirst();
        if (arg.isEmpty())
            continue;
        if (noMoreArguments || arg.at(0) != QLatin1Char('-')) {
            out.filesToOpen.append(arg);
            continue;
        }
        if (arg.length() == 1) {
            // traditionally this means:
            // read file from stdin
            // write file to stdout

            // I'm not sure whether that's applicable
            // and in any case it would be hard.
            qWarning() << "Bad argument " << idx << ": lonely hyphen";
            showHelp(out, arguments);
            // showHelp sets quit for the caller
            return out;
        }
        // long options
        if (arg.at(1) == QLatin1Char('-')) {
            if (arg.length() == 2) {
                noMoreArguments = true;
                continue;
            }
            // yay, lookup! It's linear, but that's as fast as a series of if
            for (size_t i = 0; i < possibleArgumentCount; i++) {
                if (arg == QLatin1String(possibleArguments[i].longName)) {
                    possibleArguments[i].func(out, arguments);
                    goto continue_outer;
                }
            }
            qWarning() << "Unknown long argument " << idx << ": " << arg;
            showHelp(out, arguments);
            // showHelp sets quit for the caller
            return out;
        }
        // short options

        // can't use foreach because we have to skip the first character
        for (int ci = 1; ci < arg.length(); ++ci) {
            QChar c = arg.at(ci);
            for (size_t i = 0; i < possibleArgumentCount; i++) {
                if (c == QLatin1Char(possibleArguments[i].shortName)) {
                    possibleArguments[i].func(out, arguments);
                    goto continue_middle;
                }
            }
            qWarning() << "Unknown short argument " << idx << '.' << ci << ": " << c;
            showHelp(out, arguments);
            //showHelp sets quit for the caller
            return out;
        continue_middle:
            // label can't be empty
            ;
        }
    continue_outer:
        // label can't be empty
        ;
    }
    return out;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
    /*
     * On X11, Tiled uses the 'raster' graphics system by default, because the
     * X11 native graphics system has performance problems with drawing the
     * tile grid.
     */
#ifdef Q_WS_X11
    QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

    TiledApplication a(argc, argv);

    a.setOrganizationDomain(QLatin1String("mapeditor.org"));
    a.setApplicationName(QLatin1String("Tiled"));
    a.setApplicationVersion(QLatin1String("0.7.0"));
#ifdef Q_WS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    LanguageManager *languageManager = LanguageManager::instance();
    languageManager->installTranslators();

    ParsedCommandLine whatNow = parseCommandLineArguments();
    if (whatNow.quit)
        return 0;

    MainWindow w;
    w.show();

    QObject::connect(&a, SIGNAL(fileOpenRequest(QString)),
                     &w, SLOT(openFile(QString)));

    if (!whatNow.filesToOpen.empty()) {
        foreach (const QString &fileName, whatNow.filesToOpen)
            w.openFile(fileName);
    } else {
        w.openLastFiles();
    }

    return a.exec();
}
