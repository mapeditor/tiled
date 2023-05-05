/*
 * preferences.cpp
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2016, Mamed Ibrahimov <ibramlab@gmail.com>
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

#include "preferences.h"

#include "languagemanager.h"
#include "pluginmanager.h"
#include "savefile.h"
#include "session.h"
#include "tilesetmanager.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVariantMap>

using namespace Tiled;

Preferences *Preferences::mInstance;
QString Preferences::mStartupProject;
QString Preferences::mStartupSession;

Preferences *Preferences::instance()
{
    if (!mInstance) {
        const auto iniFile = QDir(QApplication::applicationDirPath()).filePath(QStringLiteral("tiled.ini"));
        if (QFileInfo::exists(iniFile) && QFileInfo(iniFile).isFile())
            mInstance = new Preferences(iniFile);
        else
            mInstance = new Preferences;
    }
    return mInstance;
}

void Preferences::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

Preferences::Preferences()
    : QSettings()
{
    initialize();
}

/**
 * Uses the given settings file in INI format. This constructor is used for
 * portable installs.
 */
Preferences::Preferences(const QString &fileName)
    : QSettings(fileName, QSettings::IniFormat)
    , mPortable(true)
{
    initialize();
}

Preferences::~Preferences()
{
}

void Preferences::initialize()
{
    // Make sure the data directory exists
    const QDir dataDir { dataLocation() };
    if (!dataDir.exists())
        dataDir.mkpath(QStringLiteral("."));

    SaveFile::setSafeSavingEnabled(safeSavingEnabled());

    // Backwards compatibility check since 'FusionStyle' was removed from the
    // preferences dialog.
    if (applicationStyle() == FusionStyle)
        setApplicationStyle(TiledStyle);

    // Read object types from the default location (custom location moved to project)
    setObjectTypesFile(QString());

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(reloadTilesetsOnChange());
    tilesetManager->setAnimateTiles(showTileAnimations());

    // Read the lists of enabled and disabled plugins
    const auto disabledPlugins = get<QStringList>("Plugins/Disabled");
    const auto enabledPlugins = get<QStringList>("Plugins/Enabled");

    PluginManager *pluginManager = PluginManager::instance();
    for (const QString &fileName : disabledPlugins)
        pluginManager->setPluginState(fileName, PluginDisabled);
    for (const QString &fileName : enabledPlugins)
        pluginManager->setPluginState(fileName, PluginEnabled);

    // Keeping track of some usage information
    if (contains(QLatin1String("Install/PatreonDialogTime"))) {
        setValue(QLatin1String("Install/DonationDialogTime"), value(QLatin1String("Install/PatreonDialogTime")));
        remove(QLatin1String("Install/PatreonDialogTime"));
    }

    if (!firstRun().isValid())
        setValue(QLatin1String("Install/FirstRun"), QDate::currentDate().toString(Qt::ISODate));

    if (!contains(QLatin1String("Install/DonationDialogTime"))) {
        QDate donationDialogTime = firstRun().addMonths(1);
        const QDate today(QDate::currentDate());
        if (donationDialogTime.daysTo(today) >= 0)
            donationDialogTime = today.addDays(2);
        setValue(QLatin1String("Install/DonationDialogTime"), donationDialogTime.toString(Qt::ISODate));
    }
    setValue(QLatin1String("Install/RunCount"), runCount() + 1);

    const auto oldGridMajorKey = QStringLiteral("Interface/GridMajor");
    if (contains(oldGridMajorKey)) {
        const int gridMajor = value(oldGridMajorKey).toInt();
        setGridMajor(QSize(gridMajor, gridMajor));
        remove(oldGridMajorKey);
    }
}

bool Preferences::showGrid() const
{
    return get("Interface/ShowGrid", true);
}

bool Preferences::showTileObjectOutlines() const
{
    return get("Interface/ShowTileObjectOutlines", false);
}

bool Preferences::showTileAnimations() const
{
    return get("Interface/ShowTileAnimations", true);
}

bool Preferences::showTileCollisionShapes() const
{
    return get("Interface/ShowTileCollisionShapes", false);
}

bool Preferences::showObjectReferences() const
{
    return get("Interface/ShowObjectReferences", true);
}

bool Preferences::parallaxEnabled() const
{
    return get("Interface/ParallaxEnabled", true);
}

bool Preferences::snapToGrid() const
{
    return get("Interface/SnapToGrid", false);
}

bool Preferences::snapToFineGrid() const
{
    return get("Interface/SnapToFineGrid", false);
}

bool Preferences::snapToPixels() const
{
    return get("Interface/SnapToPixels", false);
}

QColor Preferences::gridColor() const
{
    return get<QColor>("Interface/GridColor", Qt::black);
}

QColor Preferences::backgroundFadeColor() const
{
    return get<QColor>("Interface/BackgroundFadeColor", Qt::black);
}

int Preferences::gridFine() const
{
    return get<int>("Interface/GridFine", 4);
}

QSize Preferences::gridMajor() const
{
    return get<QSize>("Interface/GridMajorSize", QSize(10, 10));
}

qreal Preferences::objectLineWidth() const
{
    return get<qreal>("Interface/ObjectLineWidth", 2.0);
}

bool Preferences::highlightCurrentLayer() const
{
    return get("Interface/HighlightCurrentLayer", false);
}

bool Preferences::highlightHoveredObject() const
{
    return get("Interface/HighlightHoveredObject", true);
}

bool Preferences::showTilesetGrid() const
{
    return get("Interface/ShowTilesetGrid", true);
}

Preferences::ObjectLabelVisiblity Preferences::objectLabelVisibility() const
{
    return static_cast<ObjectLabelVisiblity>(get<int>("Interface/ObjectLabelVisibility", AllObjectLabels));
}

void Preferences::setObjectLabelVisibility(ObjectLabelVisiblity visibility)
{
    setValue(QLatin1String("Interface/ObjectLabelVisibility"), visibility);
    emit objectLabelVisibilityChanged(visibility);
}

bool Preferences::labelForHoveredObject() const
{
    return get("Interface/LabelForHoveredObject", false);
}

void Preferences::setLabelForHoveredObject(bool enabled)
{
    setValue(QLatin1String("Interface/LabelForHoveredObject"), enabled);
    emit labelForHoveredObjectChanged(enabled);
}

Preferences::ApplicationStyle Preferences::applicationStyle() const
{
#if defined(Q_OS_MAC)
    return static_cast<ApplicationStyle>(get<int>("Interface/ApplicationStyle", SystemDefaultStyle));
#else
    return static_cast<ApplicationStyle>(get<int>("Interface/ApplicationStyle", TiledStyle));
#endif
}

void Preferences::setApplicationStyle(ApplicationStyle style)
{
    setValue(QLatin1String("Interface/ApplicationStyle"), style);
    emit applicationStyleChanged(style);
}

QColor Preferences::baseColor() const
{
    return get<QColor>("Interface/BaseColor", Qt::lightGray);
}

void Preferences::setBaseColor(const QColor &color)
{
    setValue(QLatin1String("Interface/BaseColor"), color.name());
    emit baseColorChanged(color);
}

QColor Preferences::selectionColor() const
{
    return get<QColor>("Interface/SelectionColor", QColor(48, 140, 198));
}

void Preferences::setSelectionColor(const QColor &color)
{
    setValue(QLatin1String("Interface/SelectionColor"), color.name());
    emit selectionColorChanged(color);
}

bool Preferences::useCustomFont() const
{
    return get<bool>("Interface/UseCustomFont", false);
}

void Preferences::setUseCustomFont(bool useCustomFont)
{
    setValue(QLatin1String("Interface/UseCustomFont"), useCustomFont);
    emit applicationFontChanged();
}

QFont Preferences::customFont() const
{
    return get<QFont>("Interface/CustomFont", QGuiApplication::font());
}

void Preferences::setCustomFont(const QFont &font)
{
    setValue(QLatin1String("Interface/CustomFont"), font);
    if (useCustomFont())
        emit applicationFontChanged();
}

Map::LayerDataFormat Preferences::layerDataFormat() const
{
    return static_cast<Map::LayerDataFormat>(get<int>("Storage/LayerDataFormat", Map::CSV));
}

void Preferences::setShowGrid(bool showGrid)
{
    setValue(QLatin1String("Interface/ShowGrid"), showGrid);
    emit showGridChanged(showGrid);
}

void Preferences::setShowTileObjectOutlines(bool enabled)
{
    setValue(QLatin1String("Interface/ShowTileObjectOutlines"), enabled);
    emit showTileObjectOutlinesChanged(enabled);
}

void Preferences::setShowTileAnimations(bool enabled)
{
    setValue(QLatin1String("Interface/ShowTileAnimations"), enabled);
    TilesetManager::instance()->setAnimateTiles(enabled);
    emit showTileAnimationsChanged(enabled);
}

void Preferences::setShowTileCollisionShapes(bool enabled)
{
    setValue(QLatin1String("Interface/ShowTileCollisionShapes"), enabled);
    emit showTileCollisionShapesChanged(enabled);
}

void Preferences::setShowObjectReferences(bool enabled)
{
    setValue(QLatin1String("Interface/ShowObjectReferences"), enabled);
    emit showObjectReferencesChanged(enabled);
}

void Preferences::setParallaxEnabled(bool enabled)
{
    setValue(QLatin1String("Interface/ParallaxEnabled"), enabled);
    emit parallaxEnabledChanged(enabled);
}

void Preferences::setSnapToGrid(bool snapToGrid)
{
    setValue(QLatin1String("Interface/SnapToGrid"), snapToGrid);
    emit snapToGridChanged(snapToGrid);
}

void Preferences::setSnapToFineGrid(bool snapToFineGrid)
{
    setValue(QLatin1String("Interface/SnapToFineGrid"), snapToFineGrid);
    emit snapToFineGridChanged(snapToFineGrid);
}

void Preferences::setSnapToPixels(bool snapToPixels)
{
    setValue(QLatin1String("Interface/SnapToPixels"), snapToPixels);
    emit snapToPixelsChanged(snapToPixels);
}

void Preferences::setGridColor(QColor gridColor)
{
    setValue(QLatin1String("Interface/GridColor"), gridColor.name());
    emit gridColorChanged(gridColor);
}

void Preferences::setBackgroundFadeColor(QColor backgroundFadeColor)
{
    setValue(QLatin1String("Interface/BackgroundFadeColor"), backgroundFadeColor.name());
    emit backgroundFadeColorChanged(backgroundFadeColor);
}

void Preferences::setGridFine(int gridFine)
{
    setValue(QLatin1String("Interface/GridFine"), gridFine);
    emit gridFineChanged(gridFine);
}

void Preferences::setGridMajor(QSize gridMajor)
{
    setValue(QLatin1String("Interface/GridMajorSize"), gridMajor);
    emit gridMajorChanged(gridMajor);
}

void Preferences::setGridMajorX(int gridMajorX)
{
    setGridMajor(QSize(gridMajorX, gridMajor().height()));
}

void Preferences::setGridMajorY(int gridMajorY)
{
    setGridMajor(QSize(gridMajor().width(), gridMajorY));
}

void Preferences::setObjectLineWidth(qreal lineWidth)
{
    setValue(QLatin1String("Interface/ObjectLineWidth"), lineWidth);
    emit objectLineWidthChanged(lineWidth);
}

void Preferences::setHighlightCurrentLayer(bool highlight)
{
    setValue(QLatin1String("Interface/HighlightCurrentLayer"), highlight);
    emit highlightCurrentLayerChanged(highlight);
}

void Preferences::setHighlightHoveredObject(bool highlight)
{
    setValue(QLatin1String("Interface/HighlightHoveredObject"), highlight);
    emit highlightHoveredObjectChanged(highlight);
}

void Preferences::setShowTilesetGrid(bool showTilesetGrid)
{
    setValue(QLatin1String("Interface/ShowTilesetGrid"), showTilesetGrid);
    emit showTilesetGridChanged(showTilesetGrid);
}

void Preferences::setLayerDataFormat(Map::LayerDataFormat layerDataFormat)
{
    setValue(QLatin1String("Storage/LayerDataFormat"), layerDataFormat);
}

Map::RenderOrder Preferences::mapRenderOrder() const
{
    return static_cast<Map::RenderOrder>(get<int>("Storage/MapRenderOrder", Map::RightDown));
}

void Preferences::setMapRenderOrder(Map::RenderOrder mapRenderOrder)
{
    setValue(QLatin1String("Storage/MapRenderOrder"), mapRenderOrder);
}

bool Preferences::safeSavingEnabled() const
{
    return get("Storage/SafeSavingEnabled", true);
}

void Preferences::setSafeSavingEnabled(bool enabled)
{
    setValue(QLatin1String("Storage/SafeSavingEnabled"), enabled);
    SaveFile::setSafeSavingEnabled(enabled);
}

bool Preferences::exportOnSave() const
{
    return get("Storage/ExportOnSave", false);
}

void Preferences::setExportOnSave(bool enabled)
{
    setValue(QLatin1String("Storage/ExportOnSave"), enabled);
}

Preferences::ExportOptions Preferences::exportOptions() const
{
    ExportOptions options;

    if (get("Export/EmbedTilesets", false))
        options |= EmbedTilesets;
    if (get("Export/DetachTemplateInstances", false))
        options |= DetachTemplateInstances;
    if (get("Export/ResolveObjectTypesAndProperties", false))
        options |= ResolveObjectTypesAndProperties;
    if (get("Export/Minimized", false))
        options |= ExportMinimized;

    return options;
}

void Preferences::setExportOption(Preferences::ExportOption option, bool value)
{
    switch (option) {
    case EmbedTilesets:
        setValue(QLatin1String("Export/EmbedTilesets"), value);
        break;
    case DetachTemplateInstances:
        setValue(QLatin1String("Export/DetachTemplateInstances"), value);
        break;
    case ResolveObjectTypesAndProperties:
        setValue(QLatin1String("Export/ResolveObjectTypesAndProperties"), value);
        break;
    case ExportMinimized:
        setValue(QLatin1String("Export/Minimized"), value);
        break;
    }
}

bool Preferences::exportOption(Preferences::ExportOption option) const
{
    switch (option) {
    case EmbedTilesets:
        return get("Export/EmbedTilesets", false);
    case DetachTemplateInstances:
        return get("Export/DetachTemplateInstances", false);
    case ResolveObjectTypesAndProperties:
        return get("Export/ResolveObjectTypesAndProperties", false);
    case ExportMinimized:
        return get("Export/Minimized", false);
    }
    return false;
}

QString Preferences::language() const
{
    return get<QString>("Interface/Language");
}

void Preferences::setLanguage(const QString &language)
{
    setValue(QLatin1String("Interface/Language"), language);
    LanguageManager::instance()->installTranslators();
    emit languageChanged();
}

bool Preferences::reloadTilesetsOnChange() const
{
    return get("Storage/ReloadTilesets", true);
}

void Preferences::setReloadTilesetsOnChanged(bool reloadOnChanged)
{
    setValue(QLatin1String("Storage/ReloadTilesets"), reloadOnChanged);
    TilesetManager::instance()->setReloadTilesetsOnChange(reloadOnChanged);
}

bool Preferences::useOpenGL() const
{
    return get("Interface/OpenGL", false);
}

void Preferences::setUseOpenGL(bool useOpenGL)
{
    setValue(QLatin1String("Interface/OpenGL"), useOpenGL);
    emit useOpenGLChanged(useOpenGL);
}

void Preferences::setPropertyTypes(const SharedPropertyTypes &propertyTypes)
{
    Object::setPropertyTypes(propertyTypes);
    emit propertyTypesChanged();
}

QDate Preferences::firstRun() const
{
    return get<QDate>("Install/FirstRun");
}

int Preferences::runCount() const
{
    return get<int>("Install/RunCount", 0);
}

bool Preferences::isPatron() const
{
    return get("Install/IsPatron", false);
}

void Preferences::setPatron(bool isPatron)
{
    setValue(QLatin1String("Install/IsPatron"), isPatron);
    emit isPatronChanged();
}

bool Preferences::shouldShowDonationReminder() const
{
    if (isPatron())
        return false;
    if (runCount() < 7)
        return false;

    const QDate dialogTime = donationReminderTime();
    if (!dialogTime.isValid())
        return false;

    return dialogTime.daysTo(QDate::currentDate()) >= 0;
}

QDate Preferences::donationReminderTime() const
{
    return get<QDate>("Install/DonationDialogTime");
}

void Preferences::setDonationReminder(const QDate &date)
{
    if (date.isValid())
        setPatron(false);
    setValue(QLatin1String("Install/DonationDialogTime"), date.toString(Qt::ISODate));
}

/**
 * Adds the given file to the recent files list.
 */
void Preferences::addRecentFile(const QString &fileName)
{
    Session::current().addRecentFile(fileName);
    emit recentFilesChanged();
}

QStringList Preferences::recentProjects() const
{
    return get<QStringList>("Project/RecentProjects");
}

QString Preferences::recentProjectPath() const
{
    QString path;

    const auto recents = recentProjects();
    if (!recents.isEmpty())
        path = QFileInfo(recents.first()).path();

    if (path.isEmpty())
        path = homeLocation();

    return path;
}

void Preferences::addRecentProject(const QString &fileName)
{
    auto files = get<QStringList>("Project/RecentProjects");
    addToRecentFileList(fileName, files);
    setValue(QLatin1String("Project/RecentProjects"), files);
    emit recentProjectsChanged();
}

QString Preferences::startupSession() const
{
    if (!mStartupSession.isEmpty())
        return mStartupSession;
    if (!startupProject().isEmpty())
        return Session::defaultFileNameForProject(startupProject());
    if (!restoreSessionOnStartup())
        return Session::defaultFileName();

    const auto session = get<QString>("Project/LastSession");
    if (session.isEmpty() || !QFileInfo::exists(session))
        return Session::defaultFileName();

    return session;
}

void Preferences::setLastSession(const QString &fileName)
{
    // Don't store the path to the default session, since this path may vary
    // between restarts. For example the snap release includes the build
    // number in the path and trying to save to a session file for a different
    // version doesn't work.
    if (fileName == Session::defaultFileName())
        setValue(QLatin1String("Project/LastSession"), QString());
    else
        setValue(QLatin1String("Project/LastSession"), fileName);
}

bool Preferences::restoreSessionOnStartup() const
{
    return get("Startup/RestorePreviousSession", true);
}

void Preferences::addToRecentFileList(const QString &fileName, QStringList& files)
{
    // Remember the file by its absolute file path (not the canonical one,
    // which avoids unexpected paths when symlinks are involved).
    const QString absoluteFilePath = QDir::cleanPath(QFileInfo(fileName).absoluteFilePath());
    if (absoluteFilePath.isEmpty())
        return;

    files.removeAll(absoluteFilePath);
    files.prepend(absoluteFilePath);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
}

void Preferences::clearRecentFiles()
{
    Session::current().clearRecentFiles();
    emit recentFilesChanged();
}

void Preferences::clearRecentProjects()
{
    remove(QLatin1String("Project/RecentProjects"));
    emit recentProjectsChanged();
}

bool Preferences::checkForUpdates() const
{
    return get("Install/CheckForUpdates", true);
}

void Preferences::setCheckForUpdates(bool on)
{
    setValue(QLatin1String("Install/CheckForUpdates"), on);
    emit checkForUpdatesChanged(on);
}

bool Preferences::displayNews() const
{
    return get("Install/DisplayNews", true);
}

void Preferences::setDisplayNews(bool on)
{
    setValue(QLatin1String("Install/DisplayNews"), on);
    emit displayNewsChanged(on);
}

bool Preferences::wheelZoomsByDefault() const
{
    return get("Interface/WheelZoomsByDefault", false);
}

void Preferences::setRestoreSessionOnStartup(bool enabled)
{
    setValue(QLatin1String("Startup/RestorePreviousSession"), enabled);
}

void Preferences::setPluginEnabled(const QString &fileName, bool enabled)
{
    PluginManager *pluginManager = PluginManager::instance();
    pluginManager->setPluginState(fileName, enabled ? PluginEnabled : PluginDisabled);

    QStringList disabledPlugins;
    QStringList enabledPlugins;

    auto &states = pluginManager->pluginStates();

    for (auto it = states.begin(), it_end = states.end(); it != it_end; ++it) {
        const QString &fileName = it.key();
        PluginState state = it.value();
        switch (state) {
        case PluginEnabled:
            enabledPlugins.append(fileName);
            break;
        case PluginDisabled:
            disabledPlugins.append(fileName);
            break;
        case PluginDefault:
        case PluginStatic:
            break;
        }
    }

    setValue(QLatin1String("Plugins/Disabled"), disabledPlugins);
    setValue(QLatin1String("Plugins/Enabled"), enabledPlugins);
}

void Preferences::setWheelZoomsByDefault(bool mode)
{
    setValue(QLatin1String("Interface/WheelZoomsByDefault"), mode);
}

QString Preferences::homeLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

QString Preferences::dataLocation() const
{
    if (mPortable) {
        const auto configDir = QFileInfo(fileName()).dir();
        return configDir.filePath(QStringLiteral("data"));
    }
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString Preferences::configLocation() const
{
    if (mPortable)
        return QFileInfo(fileName()).path();

    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString Preferences::startupProject()
{
    return mStartupProject;
}

/**
 * Sets the project to load on startup (passed on command-line).
 *
 * Needs to be set before the Session is initialized, because it determines
 * the location of the session file.
 */
void Preferences::setStartupProject(const QString &filePath)
{
    mStartupProject = filePath;
}

/**
 * Sets the session to load on startup.
 */
void Preferences::setStartupSession(const QString &filePath)
{
    mStartupSession = filePath;
}

void Preferences::setObjectTypesFile(const QString &fileName)
{
    QString newObjectTypesFile = fileName;
    if (newObjectTypesFile.isEmpty())
        newObjectTypesFile = dataLocation() + QLatin1String("/objecttypes.xml");

    if (mObjectTypesFile != newObjectTypesFile)
        mObjectTypesFile = newObjectTypesFile;
}

#include "moc_preferences.cpp"
