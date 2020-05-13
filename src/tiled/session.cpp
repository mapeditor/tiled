/*
 * session.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "session.h"

#include "preferences.h"
#include "utils.h"

#include <QFileInfo>

#include "qtcompat_p.h"

namespace Tiled {

FileHelper::FileHelper(const QString &fileName)
    : mDir { QFileInfo(fileName).dir() }
{}

void FileHelper::setFileName(const QString &fileName)
{
    mDir = QFileInfo(fileName).dir();
}

QStringList FileHelper::relative(const QStringList &fileNames) const
{
    QStringList result;
    for (const QString &fileName : fileNames)
        result.append(relative(fileName));
    return result;
}

QStringList FileHelper::resolve(const QStringList &fileNames) const
{
    QStringList result;
    for (const QString &fileName : fileNames)
        result.append(resolve(fileName));
    return result;
}

QHash<const char*, Session::Callbacks> Session::mChangedCallbacks;
std::unique_ptr<Session> Session::mCurrent;

Session::Session(const QString &fileName)
    : FileHelper            { fileName }
    , settings              { Utils::jsonSettings(fileName) }
    , project               { resolve(get<QString>("project")) }
    , recentFiles           { resolve(get<QStringList>("recentFiles")) }
    , openFiles             { resolve(get<QStringList>("openFiles")) }
    , expandedProjectPaths  { resolve(get<QStringList>("expandedProjectPaths")) }
    , activeFile            { resolve(get<QString>("activeFile")) }
{
    const auto states = get<QVariantMap>("fileStates");
    for (auto it = states.constBegin(); it != states.constEnd(); ++it)
        fileStates.insert(resolve(it.key()), it.value().toMap());

    mSyncSettingsTimer.setInterval(1000);
    mSyncSettingsTimer.setSingleShot(true);
    QObject::connect(&mSyncSettingsTimer, &QTimer::timeout, [this] { sync(); });
}

Session::~Session()
{
    if (mSyncSettingsTimer.isActive())
        sync();
}

void Session::sync()
{
    mSyncSettingsTimer.stop();

    set("project",              relative(project));
    set("recentFiles",          relative(recentFiles));
    set("openFiles",            relative(openFiles));
    set("expandedProjectPaths", relative(expandedProjectPaths));
    set("activeFile",           relative(activeFile));

    QVariantMap states;
    for (auto it = fileStates.constBegin(); it != fileStates.constEnd(); ++it)
        states.insert(relative(it.key()), it.value());
    set("fileStates", states);
}

bool Session::save()
{
    sync();
    settings->sync();
    return settings->status() == QSettings::NoError;
}

/**
 * This function "moves" the current session to a new location. It happens for
 * example when saving a project for the first time or saving it under a
 * different file name.
 */
void Session::setFileName(const QString &fileName)
{
    // Make sure we have no pending changes to save to our old location
    if (mSyncSettingsTimer.isActive())
        sync();

    auto newSettings = Utils::jsonSettings(fileName);

    // Copy over all settings
    const auto keys = settings->allKeys();
    for (const auto &key : keys)
        newSettings->setValue(key, settings->value(key));

    settings = std::move(newSettings);

    FileHelper::setFileName(fileName);

    scheduleSync();
}

void Session::setProject(const QString &fileName)
{
    project = fileName;
    scheduleSync();
}

void Session::addRecentFile(const QString &fileName)
{
    // Remember the file by its absolute file path (not the canonical one,
    // which avoids unexpected paths when symlinks are involved).
    const QString absoluteFilePath = QDir::cleanPath(QFileInfo(fileName).absoluteFilePath());
    if (absoluteFilePath.isEmpty())
        return;

    recentFiles.removeAll(absoluteFilePath);
    recentFiles.prepend(absoluteFilePath);
    while (recentFiles.size() > Preferences::MaxRecentFiles)
        recentFiles.removeLast();

    scheduleSync();
}

void Session::clearRecentFiles()
{
    recentFiles.clear();
    scheduleSync();
}

void Session::setOpenFiles(const QStringList &fileNames)
{
    openFiles = fileNames;
    scheduleSync();
}

void Session::setActiveFile(const QString &fileNames)
{
    activeFile = fileNames;
    scheduleSync();
}

QVariantMap Session::fileState(const QString &fileName) const
{
    return fileStates.value(fileName);
}

void Session::setFileState(const QString &fileName, const QVariantMap &fileState)
{
    fileStates.insert(fileName, fileState);
    scheduleSync();
}

void Session::setFileStateValue(const QString &fileName, const QString &name, const QVariant &value)
{
    auto &state = fileStates[fileName];
    auto &v = state[name];
    if (v != value) {
        v = value;
        scheduleSync();
    }
}

QString Session::defaultFileName()
{
    return Preferences::dataLocation() + QLatin1String("/default.tiled-session");
}

QString Session::defaultFileNameForProject(const QString &projectFile)
{
    if (projectFile.isEmpty())
        return defaultFileName();

    const QFileInfo fileInfo(projectFile);

    QString sessionFile = fileInfo.path();
    sessionFile += QLatin1Char('/');
    sessionFile += fileInfo.completeBaseName();
    sessionFile += QLatin1String(".tiled-session");

    return sessionFile;
}

Session &Session::initialize()
{
    Q_ASSERT(!mCurrent);
    return switchCurrent(Preferences::instance()->startupSession());
}

Session &Session::current()
{
    Q_ASSERT(mCurrent);
    return *mCurrent;
}

static void migratePreferences();

Session &Session::switchCurrent(const QString &fileName)
{
    const bool initialSession = !mCurrent;

    // Do nothing if this session is already current
    if (!initialSession && mCurrent->fileName() == fileName)
        return *mCurrent;

    mCurrent = std::make_unique<Session>(fileName);
    Preferences::instance()->setLastSession(mCurrent->fileName());

    if (initialSession)
        migratePreferences();

    // Call all registered callbacks because any value may have changed
    for (const auto &callbacks : qAsConst(mChangedCallbacks))
        for (const auto &callback : callbacks)
            callback();

    return *mCurrent;
}

template<typename T>
static void migrateToSession(const char *preferencesKey, const char *sessionKey)
{
    auto &session = Session::current();
    if (session.isSet(sessionKey))
        return;

    const auto value = Preferences::instance()->value(QLatin1String(preferencesKey));
    if (!value.isValid())
        return;

    session.set(sessionKey, value.value<T>());
}

static void migratePreferences()
{
    // Migrate some preferences to the session for compatibility
    migrateToSession<bool>("Automapping/WhileDrawing", "automapping.whileDrawing");

    migrateToSession<QStringList>("LoadedWorlds", "loadedWorlds");
    migrateToSession<QString>("Storage/StampsDirectory", "stampsFolder");

    migrateToSession<int>("Map/Orientation", "map.orientation");
    migrateToSession<int>("Storage/LayerDataFormat", "map.layerDataFormat");
    migrateToSession<int>("Storage/MapRenderOrder", "map.renderOrder");
    migrateToSession<bool>("Map/FixedSize", "map.fixedSize");
    migrateToSession<int>("Map/Width", "map.width");
    migrateToSession<int>("Map/Height", "map.height");
    migrateToSession<int>("Map/TileWidth", "map.tileWidth");
    migrateToSession<int>("Map/TileHeight", "map.tileHeight");

    migrateToSession<int>("Tileset/Type", "tileset.type");
    migrateToSession<bool>("Tileset/EmbedInMap", "tileset.embedInMap");
    migrateToSession<bool>("Tileset/UseTransparentColor", "tileset.useTransparentColor");
    migrateToSession<QColor>("Tileset/TransparentColor", "tileset.transparentColor");
    migrateToSession<QSize>("Tileset/TileSize", "tileset.tileSize");
    migrateToSession<int>("Tileset/Spacing", "tileset.spacing");
    migrateToSession<int>("Tileset/Margin", "tileset.margin");

    migrateToSession<QString>("AddPropertyDialog/PropertyType", "property.type");

    migrateToSession<QStringList>("Console/History", "console.history");

    migrateToSession<bool>("SaveAsImage/VisibleLayersOnly", "exportAsImage.visibleLayersOnly");
    migrateToSession<bool>("SaveAsImage/CurrentScale", "exportAsImage.useCurrentScale");
    migrateToSession<bool>("SaveAsImage/DrawGrid", "exportAsImage.drawTileGrid");
    migrateToSession<bool>("SaveAsImage/IncludeBackgroundColor", "exportAsImage.includeBackgroundColor");

    migrateToSession<bool>("ResizeMap/RemoveObjects", "resizeMap.removeObjects");

    migrateToSession<int>("Animation/FrameDuration", "frame.defaultDuration");

    migrateToSession<QString>("lastUsedExportFilter", "map.lastUsedExportFilter");
    migrateToSession<QString>("lastUsedMapFormat", "map.lastUsedFormat");
    migrateToSession<QString>("lastUsedOpenFilter", "file.lastUsedOpenFilter");
    migrateToSession<QString>("lastUsedTilesetExportFilter", "tileset.lastUsedExportFilter");
    migrateToSession<QString>("lastUsedTilesetFilter", "tileset.lastUsedFilter");
    migrateToSession<QString>("lastUsedTilesetFormat", "tileset.lastUsedFormat");

    auto &session = Session::current();
    auto prefs = Preferences::instance();

    // Migrate some preferences that need manual handling
    if (session.fileName() == Session::defaultFileName()) {
        if (prefs->contains(QLatin1String("recentFiles"))) {
            session.recentFiles = prefs->get<QStringList>("recentFiles/fileNames");
            session.setOpenFiles(prefs->get<QStringList>("recentFiles/lastOpenFiles"));
            session.setActiveFile(prefs->get<QString>("recentFiles/lastActive"));
        }

        if (prefs->contains(QLatin1String("MapEditor/MapStates"))) {
            const auto mapStates = prefs->get<QVariantMap>("MapEditor/MapStates");

            for (auto it = mapStates.begin(); it != mapStates.end(); ++it) {
                const QString &fileName = it.key();
                auto mapState = it.value().toMap();

                const QPointF viewCenter = mapState.value(QLatin1String("viewCenter")).toPointF();

                mapState.insert(QLatin1String("viewCenter"), toSettingsValue(viewCenter));

                session.setFileState(fileName, mapState);
            }
        }

        if (session.save()) {
            prefs->remove(QLatin1String("recentFiles"));
            prefs->remove(QLatin1String("MapEditor/MapStates"));
        }
    }
}

} // namespace Tiled
