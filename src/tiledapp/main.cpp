/*
 * main.cpp
 * Copyright 2008-2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Ben Longbons <b.r.longbons@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "commandlineparser.h"
#include "exporthelper.h"
#include "logginginterface.h"
#include "mainwindow.h"
#include "mapformat.h"
#include "pluginmanager.h"
#include "preferences.h"
#include "scriptmanager.h"
#include "sentryhelper.h"
#include "stylehelper.h"
#include "tiledapplication.h"
#include "tileset.h"
#include "tmxmapformat.h"

#include <QDebug>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtPlugin>

#include "qtcompat_p.h"

#include <memory>

#ifdef Q_OS_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>
#else
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif

#ifdef ERROR
#undef ERROR
#endif

#endif // Q_OS_WIN

using namespace Tiled;

static QTextStream& stdOut()
{
    static QTextStream ts(stdout);
    return ts;
}

namespace {

class CommandLineHandler : public CommandLineParser
{
    Q_DECLARE_TR_FUNCTIONS(CommandLineHandler)

public:
    CommandLineHandler();

    bool quit = false;
    bool showedVersion = false;
    bool disableOpenGL = false;
    bool exportMap = false;
    bool exportTileset = false;
    bool newInstance = false;
    Preferences::ExportOptions exportOptions;

private:
    void showVersion();
    void justQuit();
    void setDisableOpenGL();
    void setProject();
    void setExportMap();
    void setExportTileset();
    void setExportEmbedTilesets();
    void setExportDetachTemplateInstances();
    void setExportResolveObjectTypesAndProperties();
    void setExportMinimized();
    void showExportFormats();
    void setCompatibilityVersion();
    void evaluateScript();
    void startNewInstance();

    // Convenience wrapper around registerOption
    template <void (CommandLineHandler::*memberFunction)()>
    void option(QChar shortName,
                const QString &longName,
                const QString &help)
    {
        registerOption<CommandLineHandler, memberFunction>(this,
                                                           shortName,
                                                           longName,
                                                           help);
    }
};

static void initializePluginsAndExtensions()
{
    // Load the project without restoring the session
    if (!Preferences::startupProject().isEmpty()) {
        if (auto project = Project::load(Preferences::startupProject())) {
            ProjectManager::instance()->setProject(std::move(project));
        } else {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Failed to load project '%1'.")
                                    .arg(Preferences::startupProject());
        }
    }

    PluginManager::instance()->loadPlugins();
    ScriptManager::instance().ensureInitialized();
}

/**
 * Used during file export, attempt to determine the output file format
 * from the command line parameters.
 * Query errorMsg if result is null.
 */
template <typename T>
inline T *findExportFormat(const QString *filter,
                           const QString &targetFile,
                           QString &errorMsg)
{
    T *outputFormat = nullptr;
    const auto formats = PluginManager::objects<T>();

    if (filter) {
        // Find the format supporting the given filter
        for (T *format : formats) {
            if (!format->hasCapabilities(T::Write))
                continue;
            if (format->shortName().compare(*filter, Qt::CaseInsensitive) == 0) {
                outputFormat = format;
                break;
            }
        }
        if (!outputFormat) {
            errorMsg = QCoreApplication::translate("Command line", "Format not recognized (see --export-formats)");
            return nullptr;
        }
    } else {
        // Find the format based on target file extension
        QString suffix = QFileInfo(targetFile).completeSuffix();
        for (T *format : formats) {
            if (!format->hasCapabilities(T::Write))
                continue;
            if (format->nameFilter().contains(suffix, Qt::CaseInsensitive)) {
                if (outputFormat) {
                    errorMsg = QCoreApplication::translate("Command line", "Non-unique file extension. Can't determine correct export format.");
                    return nullptr;
                }
                outputFormat = format;
            }
        }
        if (!outputFormat) {
            errorMsg = QCoreApplication::translate("Command line", "No exporter found for target file.");
            return nullptr;
        }
    }

    return outputFormat;
}


} // anonymous namespace


CommandLineHandler::CommandLineHandler()
{
    option<&CommandLineHandler::showVersion>(
                QLatin1Char('v'),
                QLatin1String("--version"),
                tr("Display the version"));

    option<&CommandLineHandler::justQuit>(
                QChar(),
                QLatin1String("--quit"),
                tr("Only check validity of arguments"));

    option<&CommandLineHandler::setDisableOpenGL>(
                QChar(),
                QLatin1String("--disable-opengl"),
                tr("Disable hardware accelerated rendering"));

    option<&CommandLineHandler::setProject>(
                QChar(),
                QLatin1String("--project"),
                tr("Project file to load"));

    option<&CommandLineHandler::setExportMap>(
                QChar(),
                QLatin1String("--export-map"),
                tr("Export the specified map file to target"));

    option<&CommandLineHandler::setExportTileset>(
                QChar(),
                QLatin1String("--export-tileset"),
                tr("Export the specified tileset file to target"));

    option<&CommandLineHandler::showExportFormats>(
                QChar(),
                QLatin1String("--export-formats"),
                tr("Print a list of supported export formats"));

    option<&CommandLineHandler::setCompatibilityVersion>(
                QChar(),
                QLatin1String("--export-version"),
                tr("Set the compatibility version used when exporting"));

    option<&CommandLineHandler::setExportEmbedTilesets>(
                QChar(),
                QLatin1String("--embed-tilesets"),
                tr("Export the map with tilesets embedded"));

    option<&CommandLineHandler::setExportDetachTemplateInstances>(
                QChar(),
                QLatin1String("--detach-templates"),
                tr("Export the map or tileset with template instances detached"));

    option<&CommandLineHandler::setExportResolveObjectTypesAndProperties>(
                QChar(),
                QLatin1String("--resolve-types-and-properties"),
                tr("Export the map or tileset with types and properties resolved"));

    option<&CommandLineHandler::setExportMinimized>(
                QChar(),
                QLatin1String("--minimize"),
                tr("Minimize the exported file by omitting unnecessary whitespace"));

    option<&CommandLineHandler::startNewInstance>(
                QChar(),
                QLatin1String("--new-instance"),
                tr("Start a new instance, even if an instance is already running"));

    option<&CommandLineHandler::evaluateScript>(
                QLatin1Char('e'),
                QLatin1String("--evaluate"),
                tr("Evaluate a script file and quit"));
}

void CommandLineHandler::showVersion()
{
    if (!showedVersion) {
        showedVersion = true;
        stdOut() << QApplication::applicationDisplayName() << " "
                 << QApplication::applicationVersion() << Qt::endl;
        quit = true;
    }
}

void CommandLineHandler::justQuit()
{
    quit = true;
}

void CommandLineHandler::setDisableOpenGL()
{
    disableOpenGL = true;
}

void CommandLineHandler::setProject()
{
    const QString projectFile = nextArgument();
    const QFileInfo fileInfo(projectFile);

    if (fileInfo.suffix() != QLatin1String("tiled-project")) {
        qWarning().noquote() << QCoreApplication::translate("Command line", "Project file expected: --project <.tiled-project file>");
        justQuit();
    } else if (!fileInfo.exists()) {
        qWarning().noquote() << QCoreApplication::translate("Command line", "Project file '%1' not found.").arg(projectFile);
        justQuit();
    } else {
        Preferences::setStartupProject(QDir::cleanPath(fileInfo.absoluteFilePath()));
    }
}

void CommandLineHandler::setExportMap()
{
    exportMap = true;
}

void CommandLineHandler::setExportTileset()
{
    exportTileset = true;
}

void CommandLineHandler::setExportEmbedTilesets()
{
    exportOptions |= Preferences::EmbedTilesets;
}

void CommandLineHandler::setExportDetachTemplateInstances()
{
    exportOptions |= Preferences::DetachTemplateInstances;
}

void CommandLineHandler::setExportResolveObjectTypesAndProperties()
{
    exportOptions |= Preferences::ResolveObjectTypesAndProperties;
}

void CommandLineHandler::setExportMinimized()
{
    exportOptions |= Preferences::ExportMinimized;
}

void CommandLineHandler::showExportFormats()
{
    initializePluginsAndExtensions();

    QStringList formats;
    const auto mapFormats = PluginManager::objects<MapFormat>();
    for (MapFormat *format : mapFormats) {
        if (format->hasCapabilities(MapFormat::Write))
            formats.append(format->shortName());
    }
    formats.sort(Qt::CaseSensitive);

    stdOut() << tr("Map export formats:") << Qt::endl;
    for (const QString &name : formats)
        stdOut() << " " << name << Qt::endl;

    formats.clear();
    const auto tilesetFormats = PluginManager::objects<TilesetFormat>();
    for (TilesetFormat *format : tilesetFormats) {
        if (format->hasCapabilities(TilesetFormat::Write))
            formats.append(format->shortName());
    }
    formats.sort(Qt::CaseSensitive);

    stdOut() << tr("Tileset export formats:") << Qt::endl;
    for (const QString &name : formats)
        stdOut() << " " << name << Qt::endl;

    quit = true;
}

void CommandLineHandler::setCompatibilityVersion()
{
    const QString versionString = nextArgument();
    if (versionString.isNull()) {
        qWarning().noquote() << QCoreApplication::translate("Command line", "Missing argument, set version using: --export-version <version>");
        justQuit();
        return;
    }

    const auto version = versionFromString(versionString);
    if (version == UnknownVersion) {
        qWarning().noquote() << QCoreApplication::translate("Command line", "Unknown version: %1").arg(versionString);
        justQuit();
    }

    FileFormat::setCompatibilityVersion(version);
}

void CommandLineHandler::evaluateScript()
{
    justQuit(); // always quit after running the script

    const QString scriptFile = nextArgument();
    if (scriptFile.isEmpty()) {
        qWarning().noquote() << QCoreApplication::translate("Command line", "Missing argument, evaluate a script using: --evaluate <script-file> [args]");
        return;
    }

    QStringList arguments;
    for (QString argument = nextArgument(); !argument.isNull(); argument = nextArgument())
        arguments.append(argument);

    ScriptManager &scriptManager = ScriptManager::instance();

    static bool initialized = false;
    if (!initialized) {
        initialized = true;

        PluginManager::instance()->loadPlugins();

        // Output messages to command-line
        auto& logger = LoggingInterface::instance();
        QObject::connect(&logger, &LoggingInterface::info, [] (const QString &message) { stdOut() << message << Qt::endl; });
        QObject::connect(&logger, &LoggingInterface::warning, [] (const QString &message) { qWarning() << message; });
        QObject::connect(&logger, &LoggingInterface::error, [] (const QString &message) { qWarning() << message; });

        scriptManager.ensureInitialized();
    }

    scriptManager.setScriptArguments(arguments);
    scriptManager.evaluateFileOrLoadModule(scriptFile);
}

void CommandLineHandler::startNewInstance()
{
    newInstance = true;
}


int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && (!defined(Q_CC_MINGW) || __GNUC__ >= 5)
    // Make console output work on Windows, if running in a console.
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *dummy = nullptr;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
    }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);

    // High-DPI scaling is always enabled in Qt 6
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Enable support for highres images (added in Qt 5.1, but off by default, always enabled in Qt 6)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // Fallback session management was removed from Qt 6
    QGuiApplication::setFallbackSessionManagementEnabled(false);

    // Window context help buttons are disabled by default in Qt 6
    QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

#ifdef Q_OS_MAC
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    Tiled::increaseImageAllocationLimit();

    TiledApplication a(argc, argv);

#ifdef TILED_SENTRY
    Sentry sentry;
#endif

    initializeMetatypes();

    // Add the built-in file formats
    TmxMapFormat tmxMapFormat;
    PluginManager::addObject(&tmxMapFormat);

    TsxTilesetFormat tsxTilesetFormat;
    PluginManager::addObject(&tsxTilesetFormat);

    XmlObjectTemplateFormat xmlObjectTemplateFormat;
    PluginManager::addObject(&xmlObjectTemplateFormat);

    CommandLineHandler commandLine;

    if (!commandLine.parse(QCoreApplication::arguments()))
        return 0;
    if (commandLine.quit)
        return 0;
    if (commandLine.disableOpenGL)
        Preferences::instance()->setUseOpenGL(false);

    if (commandLine.exportMap) {
        // Get the path to the source file and target file
        if (commandLine.exportTileset || commandLine.filesToOpen().length() < 2) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Export syntax is --export-map [format] <source> <target>");
            return 1;
        }

        initializePluginsAndExtensions();

        int index = 0;
        const QString *filter = commandLine.filesToOpen().length() > 2 ? &commandLine.filesToOpen().at(index++) : nullptr;
        const QString &sourceFile = commandLine.filesToOpen().at(index++);
        const QString &targetFile = commandLine.filesToOpen().at(index++);

        QString errorMsg;
        MapFormat *outputFormat = findExportFormat<MapFormat>(filter, targetFile, errorMsg);
        if (!outputFormat) {
            Q_ASSERT(!errorMsg.isEmpty());
            qWarning().noquote() << errorMsg;
            return 1;
        }

        // Load the source file
        const std::unique_ptr<Map> sourceMap(readMap(sourceFile, &errorMsg));
        if (!sourceMap) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Failed to load source map.");
            if (!errorMsg.isEmpty())
                qWarning().noquote() << errorMsg;
            return 1;
        }

        // Apply export options
        std::unique_ptr<Map> exportMap;
        ExportHelper exportHelper(commandLine.exportOptions);
        const Map *map = exportHelper.prepareExportMap(sourceMap.get(), exportMap);

        // Write out the file
        bool success = outputFormat->write(map, targetFile, exportHelper.formatOptions());

        if (!success) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Failed to export map to target file.");
            return 1;
        }
        return 0;
    }

    if (commandLine.exportTileset) {
        // Get the path to the source file and target file
        if (commandLine.filesToOpen().length() < 2) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Export syntax is --export-tileset [format] <source> <target>");
            return 1;
        }

        initializePluginsAndExtensions();

        int index = 0;
        const QString *filter = commandLine.filesToOpen().length() > 2 ? &commandLine.filesToOpen().at(index++) : nullptr;
        const QString &sourceFile = commandLine.filesToOpen().at(index++);
        const QString &targetFile = commandLine.filesToOpen().at(index++);

        QString errorMsg;
        TilesetFormat *outputFormat = findExportFormat<TilesetFormat>(filter, targetFile, errorMsg);
        if (!outputFormat) {
            Q_ASSERT(!errorMsg.isEmpty());
            qWarning().noquote() << errorMsg;
            return 1;
        }

        // Load the source file
        SharedTileset sourceTileset(readTileset(sourceFile, &errorMsg));
        if (!sourceTileset) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Failed to load source tileset.");
            if (!errorMsg.isEmpty())
                qWarning().noquote() << errorMsg;
            return 1;
        }

        // Apply export options
        ExportHelper exportHelper(commandLine.exportOptions);
        SharedTileset exportTileset = exportHelper.prepareExportTileset(sourceTileset);

        // Write out the file
        bool success = outputFormat->write(*exportTileset, targetFile, exportHelper.formatOptions());

        if (!success) {
            qWarning().noquote() << QCoreApplication::translate("Command line", "Failed to export tileset to target file.");
            return 1;
        }
        return 0;
    }

    QStringList filesToOpen;

    for (const QString &fileName : commandLine.filesToOpen()) {
        const QFileInfo fileInfo(fileName);
        const QString filePath = QDir::cleanPath(fileInfo.absoluteFilePath());

        if (fileInfo.suffix() == QLatin1String("tiled-project")) {
            if (!fileInfo.exists()) {
                qWarning().noquote() << QCoreApplication::translate("Command line", "Project file '%1' not found.").arg(fileName);
                return 1;
            }
            Preferences::setStartupProject(filePath);
        } if (fileInfo.suffix() == QLatin1String("tiled-session")) {
            if (!fileInfo.exists()) {
                qWarning().noquote() << QCoreApplication::translate("Command line", "Session file '%1' not found.").arg(fileName);
                return 1;
            }
            Preferences::setStartupSession(filePath);
        } else {
            filesToOpen.append(filePath);
        }
    }

    if (a.isRunning() && !filesToOpen.isEmpty() && !commandLine.newInstance) {
        // Files need to be absolute paths because the already running Tiled
        // instance likely does not have the same working directory.
        QJsonDocument doc(QJsonArray::fromStringList(filesToOpen));
        if (a.sendMessage(QLatin1String(doc.toJson())))
            return 0;
    }

    Session::initialize();
    StyleHelper::initialize();

    MainWindow w;
    w.show();

    a.setActivationWindow(&w);
#ifdef Q_OS_WIN
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    using QWindowsApplication = QNativeInterface::Private::QWindowsApplication;
    if (auto nativeWindowsApp = dynamic_cast<QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration()))
        nativeWindowsApp->setWindowActivationBehavior(QWindowsApplication::AlwaysActivateWindow);
#else
    QWindowsWindowFunctions::setWindowActivationBehavior(QWindowsWindowFunctions::AlwaysActivateWindow);
#endif
#endif

    QObject::connect(&a, &TiledApplication::fileOpenRequest,
                     &w, [&] (const QString &file) { w.openFile(file); });

    PluginManager::instance()->loadPlugins();

    w.initializeSession();

    for (const QString &fileName : std::as_const(filesToOpen))
        w.openFile(fileName);

    return a.exec();
}
