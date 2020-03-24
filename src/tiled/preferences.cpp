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

#include "documentmanager.h"
#include "languagemanager.h"
#include "mapdocument.h"
#include "pluginmanager.h"
#include "savefile.h"
#include "tilesetmanager.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QVariantMap>

using namespace Tiled;

Preferences *Preferences::mInstance;

Preferences *Preferences::instance()
{
    if (!mInstance)
        mInstance = new Preferences;
    return mInstance;
}

void Preferences::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

Preferences::Preferences()
    : mSettings { new QSettings(this) }
    , mSession { restoreSessionOnStartup() ? lastSession() : Session::defaultFileName() }
{
    // Make sure the data directory exists
    const QDir dataDir { dataLocation() };
    if (!dataDir.exists())
        dataDir.mkpath(QStringLiteral("."));

    connect(&mWatcher, &FileSystemWatcher::fileChanged,
            this, &Preferences::objectTypesFileChangedOnDisk);

    SaveFile::setSafeSavingEnabled(safeSavingEnabled());

    // Backwards compatibility check since 'FusionStyle' was removed from the
    // preferences dialog.
    if (applicationStyle() == FusionStyle)
        setApplicationStyle(TiledStyle);

    // Retrieve defined object types
    ObjectTypesSerializer objectTypesSerializer;
    ObjectTypes objectTypes;
    bool success = objectTypesSerializer.readObjectTypes(objectTypesFile(), objectTypes);

    // For backwards compatibilty, read in object types from settings
    if (!success) {
        mSettings->beginGroup(QLatin1String("ObjectTypes"));
        const QStringList names = mSettings->value(QLatin1String("Names")).toStringList();
        const QStringList colors = mSettings->value(QLatin1String("Colors")).toStringList();
        mSettings->endGroup();

        if (!names.isEmpty()) {
            const int count = qMin(names.size(), colors.size());
            for (int i = 0; i < count; ++i)
                objectTypes.append(ObjectType(names.at(i), QColor(colors.at(i))));
        }
    } else {
        mSettings->remove(QLatin1String("ObjectTypes"));

        mWatcher.addPath(objectTypesFile());
    }

    Object::setObjectTypes(objectTypes);

    mSaveSessionTimer.setInterval(1000);
    mSaveSessionTimer.setSingleShot(true);
    connect(&mSaveSessionTimer, &QTimer::timeout, this, [this] { saveSessionNow(); });

    // For backwards compatibility, import default session from settings
    if (mSettings->contains(QLatin1String("recentFiles/fileNames")) || mSettings->contains(QLatin1String("MapEditor/MapStates"))) {
        Session session(Session::defaultFileName());

        mSettings->beginGroup(QLatin1String("recentFiles"));
        session.recentFiles = mSettings->value(QLatin1String("fileNames")).toStringList();
        session.openFiles = mSettings->value(QLatin1String("lastOpenFiles")).toStringList();
        session.activeFile = mSettings->value(QLatin1String("lastActive")).toString();
        mSettings->endGroup();

        const QVariantMap mapStates = mSettings->value(QLatin1String("MapEditor/MapStates")).toMap();

        for (auto it = mapStates.begin(); it != mapStates.end(); ++it) {
            const QString &fileName = it.key();
            auto mapState = it.value().toMap();

            const QPointF viewCenter = mapState.value(QLatin1String("viewCenter")).toPointF();

            QVariantMap viewCenterVariant;
            viewCenterVariant.insert(QLatin1String("x"), viewCenter.x());
            viewCenterVariant.insert(QLatin1String("y"), viewCenter.y());
            mapState.insert(QLatin1String("viewCenter"), viewCenterVariant);

            session.setFileState(fileName, mapState);
        }

        if (session.save()) {
            mSettings->remove(QLatin1String("recentFiles"));
            mSettings->remove(QLatin1String("MapEditor/MapStates"));
        }
    }

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(reloadTilesetsOnChange());
    tilesetManager->setAnimateTiles(showTileAnimations());

    // Read the lists of enabled and disabled plugins
    const QStringList disabledPlugins = mSettings->value(QLatin1String("Plugins/Disabled")).toStringList();
    const QStringList enabledPlugins = mSettings->value(QLatin1String("Plugins/Enabled")).toStringList();

    PluginManager *pluginManager = PluginManager::instance();
    for (const QString &fileName : disabledPlugins)
        pluginManager->setPluginState(fileName, PluginDisabled);
    for (const QString &fileName : enabledPlugins)
        pluginManager->setPluginState(fileName, PluginEnabled);

    // Keeping track of some usage information
    if (mSettings->contains(QLatin1String("Install/PatreonDialogTime"))) {
        mSettings->setValue(QLatin1String("Install/DonationDialogTime"), mSettings->value(QLatin1String("Install/PatreonDialogTime")));
        mSettings->remove(QLatin1String("Install/PatreonDialogTime"));
    }

    if (!firstRun().isValid())
        mSettings->setValue(QLatin1String("Install/FirstRun"), QDate::currentDate().toString(Qt::ISODate));

    if (!mSettings->contains(QLatin1String("Install/DonationDialogTime"))) {
        QDate donationDialogTime = firstRun().addMonths(1);
        const QDate today(QDate::currentDate());
        if (donationDialogTime.daysTo(today) >= 0)
            donationDialogTime = today.addDays(2);
        mSettings->setValue(QLatin1String("Install/DonationDialogTime"), donationDialogTime.toString(Qt::ISODate));
    }
    mSettings->setValue(QLatin1String("Install/RunCount"), runCount() + 1);
}

Preferences::~Preferences()
{
}

bool Preferences::showGrid() const
{
    return boolValue("Interface/ShowGrid", true);
}

bool Preferences::showTileObjectOutlines() const
{
    return boolValue("Interface/ShowTileObjectOutlines");
}

bool Preferences::showTileAnimations() const
{
    return boolValue("Interface/ShowTileAnimations", true);
}

bool Preferences::showTileCollisionShapes() const
{
    return boolValue("Interface/ShowTileCollisionShapes");
}

bool Preferences::showObjectReferences() const
{
    return boolValue("Interface/ShowObjectReferences", true);
}

bool Preferences::snapToGrid() const
{
    return boolValue("Interface/SnapToGrid");
}

bool Preferences::snapToFineGrid() const
{
    return boolValue("Interface/SnapToFineGrid");
}

bool Preferences::snapToPixels() const
{
    return boolValue("Interface/SnapToPixels");
}

QColor Preferences::gridColor() const
{
    return colorValue("Interface/GridColor", Qt::black);
}

int Preferences::gridFine() const
{
    return intValue("Interface/GridFine", 4);
}

qreal Preferences::objectLineWidth() const
{
    return realValue("Interface/ObjectLineWidth", 2);
}

bool Preferences::highlightCurrentLayer() const
{
    return boolValue("Interface/HighlightCurrentLayer");
}

bool Preferences::highlightHoveredObject() const
{
    return boolValue("Interface/HighlightHoveredObject", true);
}

bool Preferences::showTilesetGrid() const
{
    return boolValue("Interface/ShowTilesetGrid", true);
}

Preferences::ObjectLabelVisiblity Preferences::objectLabelVisibility() const
{
    return static_cast<ObjectLabelVisiblity>(intValue("Interface/ObjectLabelVisibility", AllObjectLabels));
}

void Preferences::setObjectLabelVisibility(ObjectLabelVisiblity visibility)
{
    mSettings->setValue(QLatin1String("Interface/ObjectLabelVisibility"), visibility);
    emit objectLabelVisibilityChanged(visibility);
}

bool Preferences::labelForHoveredObject() const
{
    return boolValue("Interface/LabelForHoveredObject", false);
}

void Preferences::setLabelForHoveredObject(bool enabled)
{
    mSettings->setValue(QLatin1String("Interface/LabelForHoveredObject"), enabled);
    emit labelForHoveredObjectChanged(enabled);
}

Preferences::ApplicationStyle Preferences::applicationStyle() const
{
#if defined(Q_OS_MAC)
    return static_cast<ApplicationStyle>(intValue("Interface/ApplicationStyle", SystemDefaultStyle));
#else
    return static_cast<ApplicationStyle>(intValue("Interface/ApplicationStyle", TiledStyle));
#endif
}

void Preferences::setApplicationStyle(ApplicationStyle style)
{
    mSettings->setValue(QLatin1String("Interface/ApplicationStyle"), style);
    emit applicationStyleChanged(style);
}

QColor Preferences::baseColor() const
{
    return colorValue("Interface/BaseColor", Qt::lightGray);
}

void Preferences::setBaseColor(const QColor &color)
{
    mSettings->setValue(QLatin1String("Interface/BaseColor"), color.name());
    emit baseColorChanged(color);
}

QColor Preferences::selectionColor() const
{
    return colorValue("Interface/SelectionColor", QColor(48, 140, 198));
}

void Preferences::setSelectionColor(const QColor &color)
{
    mSettings->setValue(QLatin1String("Interface/SelectionColor"), color.name());
    emit selectionColorChanged(color);
}

Map::LayerDataFormat Preferences::layerDataFormat() const
{
    return static_cast<Map::LayerDataFormat>(intValue("Storage/LayerDataFormat", Map::CSV));
}

void Preferences::setShowGrid(bool showGrid)
{
    mSettings->setValue(QLatin1String("Interface/ShowGrid"), showGrid);
    emit showGridChanged(showGrid);
}

void Preferences::setShowTileObjectOutlines(bool enabled)
{
    mSettings->setValue(QLatin1String("Interface/ShowTileObjectOutlines"), enabled);
    emit showTileObjectOutlinesChanged(enabled);
}

void Preferences::setShowTileAnimations(bool enabled)
{
    mSettings->setValue(QLatin1String("Interface/ShowTileAnimations"), enabled);
    TilesetManager::instance()->setAnimateTiles(enabled);
    emit showTileAnimationsChanged(enabled);
}

void Preferences::setShowTileCollisionShapes(bool enabled)
{
    mSettings->setValue(QLatin1String("Interface/ShowTileCollisionShapes"), enabled);
    emit showTileCollisionShapesChanged(enabled);
}

void Preferences::setShowObjectReferences(bool enabled)
{
    mSettings->setValue(QLatin1String("Interface/ShowObjectReferences"), enabled);
    emit showObjectReferencesChanged(enabled);
}

void Preferences::setSnapToGrid(bool snapToGrid)
{
    mSettings->setValue(QLatin1String("Interface/SnapToGrid"), snapToGrid);
    emit snapToGridChanged(snapToGrid);
}

void Preferences::setSnapToFineGrid(bool snapToFineGrid)
{
    mSettings->setValue(QLatin1String("Interface/SnapToFineGrid"), snapToFineGrid);
    emit snapToFineGridChanged(snapToFineGrid);
}

void Preferences::setSnapToPixels(bool snapToPixels)
{
    mSettings->setValue(QLatin1String("Interface/SnapToPixels"), snapToPixels);
    emit snapToPixelsChanged(snapToPixels);
}

void Preferences::setGridColor(QColor gridColor)
{
    mSettings->setValue(QLatin1String("Interface/GridColor"), gridColor.name());
    emit gridColorChanged(gridColor);
}

void Preferences::setGridFine(int gridFine)
{
    mSettings->setValue(QLatin1String("Interface/GridFine"), gridFine);
    emit gridFineChanged(gridFine);
}

void Preferences::setObjectLineWidth(qreal lineWidth)
{
    mSettings->setValue(QLatin1String("Interface/ObjectLineWidth"), lineWidth);
    emit objectLineWidthChanged(lineWidth);
}

void Preferences::setHighlightCurrentLayer(bool highlight)
{
    mSettings->setValue(QLatin1String("Interface/HighlightCurrentLayer"), highlight);
    emit highlightCurrentLayerChanged(highlight);
}

void Preferences::setHighlightHoveredObject(bool highlight)
{
    mSettings->setValue(QLatin1String("Interface/HighlightHoveredObject"), highlight);
    emit highlightHoveredObjectChanged(highlight);
}

void Preferences::setShowTilesetGrid(bool showTilesetGrid)
{
    mSettings->setValue(QLatin1String("Interface/ShowTilesetGrid"), showTilesetGrid);
    emit showTilesetGridChanged(showTilesetGrid);
}

void Preferences::setLayerDataFormat(Map::LayerDataFormat layerDataFormat)
{
    mSettings->setValue(QLatin1String("Storage/LayerDataFormat"), layerDataFormat);
}

Map::RenderOrder Preferences::mapRenderOrder() const
{
    return static_cast<Map::RenderOrder>(intValue("Storage/MapRenderOrder", Map::RightDown));
}

void Preferences::setMapRenderOrder(Map::RenderOrder mapRenderOrder)
{
    mSettings->setValue(QLatin1String("Storage/MapRenderOrder"), mapRenderOrder);
}

bool Preferences::safeSavingEnabled() const
{
    return boolValue("SafeSavingEnabled", true);
}

void Preferences::setSafeSavingEnabled(bool enabled)
{
    mSettings->setValue(QLatin1String("Storage/SafeSavingEnabled"), enabled);
    SaveFile::setSafeSavingEnabled(enabled);
}

bool Preferences::exportOnSave() const
{
    return boolValue("Storage/ExportOnSave", false);
}

void Preferences::setExportOnSave(bool enabled)
{
    mSettings->setValue(QLatin1String("Storage/ExportOnSave"), enabled);
}

Preferences::ExportOptions Preferences::exportOptions() const
{
    ExportOptions options;

    if (boolValue("Export/EmbedTilesets", false))
        options |= EmbedTilesets;
    if (boolValue("Export/DetachTemplateInstances", false))
        options |= DetachTemplateInstances;
    if (boolValue("Export/ResolveObjectTypesAndProperties", false))
        options |= ResolveObjectTypesAndProperties;
    if (boolValue("Export/Minimized", false))
        options |= ExportMinimized;

    return options;
}

void Preferences::setExportOption(Preferences::ExportOption option, bool value)
{
    switch (option) {
    case EmbedTilesets:
        mSettings->setValue(QLatin1String("Export/EmbedTilesets"), value);
        break;
    case DetachTemplateInstances:
        mSettings->setValue(QLatin1String("Export/DetachTemplateInstances"), value);
        break;
    case ResolveObjectTypesAndProperties:
        mSettings->setValue(QLatin1String("Export/ResolveObjectTypesAndProperties"), value);
        break;
    case ExportMinimized:
        mSettings->setValue(QLatin1String("Export/Minimized"), value);
        break;
    }
}

bool Preferences::exportOption(Preferences::ExportOption option) const
{
    switch (option) {
    case EmbedTilesets:
        return boolValue("Export/EmbedTilesets", false);
    case DetachTemplateInstances:
        return boolValue("Export/DetachTemplateInstances", false);
    case ResolveObjectTypesAndProperties:
        return boolValue("Export/ResolveObjectTypesAndProperties", false);
    case ExportMinimized:
        return boolValue("Export/Minimized", false);
    }
    return false;
}

QString Preferences::language() const
{
    return stringValue("Interface/Language");
}

void Preferences::setLanguage(const QString &language)
{
    mSettings->setValue(QLatin1String("Interface/Language"), language);
    LanguageManager::instance()->installTranslators();
    emit languageChanged();
}

bool Preferences::reloadTilesetsOnChange() const
{
    return boolValue("Storage/ReloadTilesets", true);
}

void Preferences::setReloadTilesetsOnChanged(bool reloadOnChanged)
{
    mSettings->setValue(QLatin1String("Storage/ReloadTilesets"), reloadOnChanged);
    TilesetManager::instance()->setReloadTilesetsOnChange(reloadOnChanged);
}

bool Preferences::useOpenGL() const
{
    return boolValue("Interface/OpenGL");
}

void Preferences::setUseOpenGL(bool useOpenGL)
{
    mSettings->setValue(QLatin1String("Interface/OpenGL"), useOpenGL);
    emit useOpenGLChanged(useOpenGL);
}

void Preferences::setObjectTypes(const ObjectTypes &objectTypes)
{
    Object::setObjectTypes(objectTypes);
    emit objectTypesChanged();
}

static QString lastPathKey(Preferences::FileType fileType)
{
    QString key = QLatin1String("LastPaths/");

    switch (fileType) {
    case Preferences::ExportedFile:
        key.append(QLatin1String("ExportedFile"));
        break;
    case Preferences::ExternalTileset:
        key.append(QLatin1String("ExternalTileset"));
        break;
    case Preferences::ImageFile:
        key.append(QLatin1String("Images"));
        break;
    case Preferences::ObjectTemplateFile:
        key.append(QLatin1String("ObjectTemplates"));
        break;
    case Preferences::ObjectTypesFile:
        key.append(QLatin1String("ObjectTypes"));
        break;
    case Preferences::ProjectFile:
        key.append(QLatin1String("Project"));
        break;
    case Preferences::WorldFile:
        key.append(QLatin1String("WorldFile"));
        break;
    }

    return key;
}

/**
 * Returns the last location of a file chooser for the given file type. As long
 * as it was set using setLastPath().
 *
 * When no last path for this file type exists yet, the path of the currently
 * selected map is returned.
 *
 * When no map is open, the user's 'Documents' folder is returned.
 */
QString Preferences::lastPath(FileType fileType) const
{
    QString path = mSettings->value(lastPathKey(fileType)).toString();

    if (path.isEmpty()) {
        DocumentManager *documentManager = DocumentManager::instance();
        Document *document = documentManager->currentDocument();
        if (document)
            path = QFileInfo(document->fileName()).path();
    }

    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(
                    QStandardPaths::DocumentsLocation);
    }

    return path;
}

/**
 * \see lastPath()
 */
void Preferences::setLastPath(FileType fileType, const QString &path)
{
    if (path.isEmpty())
        return;

    mSettings->setValue(lastPathKey(fileType), path);
}

bool Preferences::automappingDrawing() const
{
    return boolValue("Automapping/WhileDrawing");
}

void Preferences::setAutomappingDrawing(bool enabled)
{
    mSettings->setValue(QLatin1String("Automapping/WhileDrawing"), enabled);
}

QDate Preferences::firstRun() const
{
    return mSettings->value(QLatin1String("Install/FirstRun")).toDate();
}

int Preferences::runCount() const
{
    return intValue("Install/RunCount", 0);
}

bool Preferences::isPatron() const
{
    return boolValue("Install/IsPatron");
}

void Preferences::setPatron(bool isPatron)
{
    mSettings->setValue(QLatin1String("Install/IsPatron"), isPatron);
    emit isPatronChanged();
}

bool Preferences::shouldShowDonationDialog() const
{
    if (isPatron())
        return false;
    if (runCount() < 7)
        return false;

    const QDate dialogTime = donationDialogTime();
    if (!dialogTime.isValid())
        return false;

    return dialogTime.daysTo(QDate::currentDate()) >= 0;
}

QDate Preferences::donationDialogTime() const
{
    return mSettings->value(QLatin1String("Install/DonationDialogTime")).toDate();
}

void Preferences::setDonationDialogReminder(const QDate &date)
{
    if (date.isValid())
        setPatron(false);
    mSettings->setValue(QLatin1String("Install/DonationDialogTime"), date.toString(Qt::ISODate));
}

QString Preferences::fileDialogStartLocation() const
{
    if (!mSession.activeFile.isEmpty())
        return QFileInfo(mSession.activeFile).path();

    if (!mSession.recentFiles.isEmpty())
        return QFileInfo(mSession.recentFiles.first()).path();

    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
}

/**
 * Adds the given file to the recent files list.
 */
void Preferences::addRecentFile(const QString &fileName)
{
    mSession.addRecentFile(fileName);
    saveSession();
    emit recentFilesChanged();
}

QStringList Preferences::recentProjects() const
{
    QVariant v = mSettings->value(QLatin1String("Project/RecentProjects"));
    return v.toStringList();
}

void Preferences::addRecentProject(const QString &fileName)
{
    setLastPath(ProjectFile, fileName);

    QStringList files = mSettings->value(QLatin1String("Project/RecentProjects")).toStringList();
    addToRecentFileList(fileName, files);
    mSettings->setValue(QLatin1String("Project/RecentProjects"), files);
    emit recentProjectsChanged();
}

QString Preferences::lastSession() const
{
    const QString session = mSettings->value(QLatin1String("Project/LastSession")).toString();
    return session.isEmpty() ? Session::defaultFileName() : session;
}

void Preferences::setLastSession(const QString &fileName)
{
    mSettings->setValue(QLatin1String("Project/LastSession"), fileName);
}

bool Preferences::restoreSessionOnStartup() const
{
    return boolValue("Startup/RestorePreviousSession", true);
}

void Preferences::switchSession(Session session)
{
    mSession = std::move(session);
    setLastSession(mSession.fileName());

    emit recentFilesChanged();
}

void Preferences::saveSession()
{
    if (!mSaveSessionTimer.isActive())
        mSaveSessionTimer.start();
}

void Preferences::saveSessionNow()
{
    emit aboutToSaveSession();
    mSaveSessionTimer.stop();
    mSession.save();
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
    mSession.recentFiles.clear();
    emit recentFilesChanged();
}

void Preferences::clearRecentProjects()
{
    mSettings->remove(QLatin1String("Project/RecentProjects"));
    emit recentProjectsChanged();
}

bool Preferences::checkForUpdates() const
{
    return boolValue("Install/CheckForUpdates", true);
}

void Preferences::setCheckForUpdates(bool on)
{
    mSettings->setValue(QLatin1String("Install/CheckForUpdates"), on);
    emit checkForUpdatesChanged(on);
}

bool Preferences::displayNews() const
{
    return boolValue("Install/DisplayNews", true);
}

void Preferences::setDisplayNews(bool on)
{
    mSettings->setValue(QLatin1String("Install/DisplayNews"), on);
    emit displayNewsChanged(on);
}

bool Preferences::wheelZoomsByDefault() const
{
    return boolValue("Interface/WheelZoomsByDefault");
}

void Preferences::setRestoreSessionOnStartup(bool enabled)
{
    mSettings->setValue(QLatin1String("Startup/RestorePreviousSession"), enabled);
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

    mSettings->setValue(QLatin1String("Plugins/Disabled"), disabledPlugins);
    mSettings->setValue(QLatin1String("Plugins/Enabled"), enabledPlugins);
}

void Preferences::setWheelZoomsByDefault(bool mode)
{
    mSettings->setValue(QLatin1String("Interface/WheelZoomsByDefault"), mode);
}

bool Preferences::boolValue(const char *key, bool defaultValue) const
{
    return mSettings->value(QLatin1String(key), defaultValue).toBool();
}

QColor Preferences::colorValue(const char *key, const QColor &def) const
{
    const QString name = mSettings->value(QLatin1String(key),
                                          def.name()).toString();
    if (!QColor::isValidColor(name))
        return QColor();

    return QColor(name);
}

QString Preferences::stringValue(const char *key, const QString &def) const
{
    return mSettings->value(QLatin1String(key), def).toString();
}

int Preferences::intValue(const char *key, int defaultValue) const
{
    return mSettings->value(QLatin1String(key), defaultValue).toInt();
}

qreal Preferences::realValue(const char *key, qreal defaultValue) const
{
    return mSettings->value(QLatin1String(key), defaultValue).toReal();
}

QString Preferences::dataLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString Preferences::stampsDirectory() const
{
    QString directory = stringValue("Storage/StampsDirectory");
    if (directory.isEmpty())
        return dataLocation() + QLatin1String("/stamps");

    return directory;
}

void Preferences::setStampsDirectory(const QString &stampsDirectory)
{
    mSettings->setValue(QLatin1String("Storage/StampsDirectory"), stampsDirectory);
    emit stampsDirectoryChanged(stampsDirectory);
}

QString Preferences::templatesDirectory() const
{
    QString directory = stringValue("Storage/TemplatesDirectory");
    if (directory.isEmpty())
        return dataLocation() + QLatin1String("/templates");

    return directory;
}

void Preferences::setTemplatesDirectory(const QString &templatesDirectory)
{
    mSettings->setValue(QLatin1String("Storage/TemplatesDirectory"), templatesDirectory);
    emit templatesDirectoryChanged(templatesDirectory);
}

QString Preferences::objectTypesFile() const
{
    QString file = stringValue("Storage/ObjectTypesFile");
    if (file.isEmpty())
        return dataLocation() + QLatin1String("/objecttypes.xml");

    return file;
}

void Preferences::setObjectTypesFile(const QString &fileName)
{
    QString previousObjectTypesFile = objectTypesFile();
    if (previousObjectTypesFile == fileName)
        return;

    if (!previousObjectTypesFile.isEmpty())
        mWatcher.removePath(previousObjectTypesFile);

    mSettings->setValue(QLatin1String("Storage/ObjectTypesFile"), fileName);
    mWatcher.addPath(fileName);
}

void Preferences::setObjectTypesFileLastSaved(const QDateTime &time)
{
    mObjectTypesFileLastSaved = time;
}

void Preferences::objectTypesFileChangedOnDisk()
{
    const QFileInfo fileInfo { objectTypesFile() };
    if (fileInfo.lastModified() == mObjectTypesFileLastSaved)
        return;

    ObjectTypesSerializer objectTypesSerializer;
    ObjectTypes objectTypes;

    if (objectTypesSerializer.readObjectTypes(fileInfo.filePath(), objectTypes))
        setObjectTypes(objectTypes);
}
