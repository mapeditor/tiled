/*
 * preferences.h
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

#pragma once

#include <QColor>
#include <QDateTime>
#include <QObject>
#include <QSettings>

#include "filesystemwatcher.h"
#include "map.h"
#include "objecttypes.h"

namespace Tiled {

/**
 * This class holds user preferences and provides a convenient interface to
 * access them.
 *
 * Since it derives from QSettings, you can also store/retrieve arbitrary
 * values. The naming style for groups and keys is CamelCase.
 */
class Preferences : public QSettings
{
    Q_OBJECT

public:
    static Preferences *instance();
    static void deleteInstance();

private:
    Preferences();
    ~Preferences() override;

public:
    bool showGrid() const;
    bool showTileObjectOutlines() const;
    bool showTileAnimations() const;
    bool showTileCollisionShapes() const;
    bool showObjectReferences() const;
    bool snapToGrid() const;
    bool snapToFineGrid() const;
    bool snapToPixels() const;
    QColor gridColor() const;
    int gridFine() const;
    qreal objectLineWidth() const;

    bool highlightCurrentLayer() const;
    bool highlightHoveredObject() const;
    bool showTilesetGrid() const;

    enum ObjectLabelVisiblity {
        NoObjectLabels,
        SelectedObjectLabels,
        AllObjectLabels
    };

    ObjectLabelVisiblity objectLabelVisibility() const;
    void setObjectLabelVisibility(ObjectLabelVisiblity visiblity);

    bool labelForHoveredObject() const;
    void setLabelForHoveredObject(bool enabled);

    enum ApplicationStyle {
        SystemDefaultStyle,
        FusionStyle,
        TiledStyle
    };

    ApplicationStyle applicationStyle() const;
    void setApplicationStyle(ApplicationStyle style);

    QColor baseColor() const;
    void setBaseColor(const QColor &color);

    QColor selectionColor() const;
    void setSelectionColor(const QColor &color);

    Map::LayerDataFormat layerDataFormat() const;
    void setLayerDataFormat(Map::LayerDataFormat layerDataFormat);

    Map::RenderOrder mapRenderOrder() const;
    void setMapRenderOrder(Map::RenderOrder mapRenderOrder);

    bool safeSavingEnabled() const;
    void setSafeSavingEnabled(bool enabled);

    bool exportOnSave() const;
    void setExportOnSave(bool enabled);

    enum ExportOption {
        EmbedTilesets                   = 0x1,
        DetachTemplateInstances         = 0x2,
        ResolveObjectTypesAndProperties = 0x4,
        ExportMinimized                 = 0x8,
    };
    Q_DECLARE_FLAGS(ExportOptions, ExportOption)

    ExportOptions exportOptions() const;
    void setExportOption(ExportOption option, bool value);
    bool exportOption(ExportOption option) const;

    QString language() const;
    void setLanguage(const QString &language);

    bool reloadTilesetsOnChange() const;
    void setReloadTilesetsOnChanged(bool reloadOnChanged);

    bool useOpenGL() const;
    void setUseOpenGL(bool useOpenGL);

    void setObjectTypes(const ObjectTypes &objectTypes);

    enum FileType {
        ExportedFile,
        ExternalTileset,
        ImageFile,
        ObjectTemplateFile,
        ObjectTypesFile,
        ProjectFile,
        WorldFile,
    };

    QString lastPath(FileType fileType) const;
    void setLastPath(FileType fileType, const QString &path);

    QString objectTypesFile() const;
    void setObjectTypesFile(const QString &filePath);
    void setObjectTypesFileLastSaved(const QDateTime &time);

    QDate firstRun() const;
    int runCount() const;

    bool isPatron() const;
    void setPatron(bool isPatron);

    bool shouldShowDonationDialog() const;
    QDate donationDialogTime() const;
    void setDonationDialogReminder(const QDate &date);

    enum { MaxRecentFiles = 12 };
    QString fileDialogStartLocation() const;
    void addRecentFile(const QString &fileName);

    QStringList recentProjects() const;
    void addRecentProject(const QString &fileName);

    QString startupSession() const;
    void setLastSession(const QString &fileName);
    bool restoreSessionOnStartup() const;

    bool checkForUpdates() const;
    void setCheckForUpdates(bool on);

    bool displayNews() const;
    void setDisplayNews(bool on);

    bool wheelZoomsByDefault() const;

    template <typename T>
    T get(const char *key, const T &defaultValue = T()) const
    { return value(QLatin1String(key), defaultValue).template value<T>(); }

    static QString dataLocation();
    static QString configLocation();

    static QString startupProject();
    static void setStartupProject(const QString &filePath);

public slots:
    void setShowGrid(bool showGrid);
    void setShowTileObjectOutlines(bool enabled);
    void setShowTileAnimations(bool enabled);
    void setShowTileCollisionShapes(bool enabled);
    void setShowObjectReferences(bool enabled);
    void setSnapToGrid(bool snapToGrid);
    void setSnapToFineGrid(bool snapToFineGrid);
    void setSnapToPixels(bool snapToPixels);
    void setGridColor(QColor gridColor);
    void setGridFine(int gridFine);
    void setObjectLineWidth(qreal lineWidth);
    void setHighlightCurrentLayer(bool highlight);
    void setHighlightHoveredObject(bool highlight);
    void setShowTilesetGrid(bool showTilesetGrid);
    void setRestoreSessionOnStartup(bool enabled);
    void setPluginEnabled(const QString &fileName, bool enabled);
    void setWheelZoomsByDefault(bool mode);

    void clearRecentFiles();
    void clearRecentProjects();

signals:
    void showGridChanged(bool showGrid);
    void showTileObjectOutlinesChanged(bool enabled);
    void showTileAnimationsChanged(bool enabled);
    void showTileCollisionShapesChanged(bool enabled);
    void showObjectReferencesChanged(bool enabled);
    void snapToGridChanged(bool snapToGrid);
    void snapToFineGridChanged(bool snapToFineGrid);
    void snapToPixelsChanged(bool snapToPixels);
    void gridColorChanged(QColor gridColor);
    void gridFineChanged(int gridFine);
    void objectLineWidthChanged(qreal lineWidth);
    void highlightCurrentLayerChanged(bool highlight);
    void highlightHoveredObjectChanged(bool highlight);
    void showTilesetGridChanged(bool showTilesetGrid);
    void objectLabelVisibilityChanged(ObjectLabelVisiblity);
    void labelForHoveredObjectChanged(bool enabled);

    void applicationStyleChanged(ApplicationStyle);
    void baseColorChanged(const QColor &baseColor);
    void selectionColorChanged(const QColor &selectionColor);

    void useOpenGLChanged(bool useOpenGL);

    void languageChanged();

    void objectTypesChanged();

    void isPatronChanged();

    void recentFilesChanged();
    void recentProjectsChanged();

    void checkForUpdatesChanged(bool on);
    void displayNewsChanged(bool on);

    void aboutToSwitchSession();

private:
    void addToRecentFileList(const QString &fileName, QStringList &files);

    void objectTypesFileChangedOnDisk();

    FileSystemWatcher mWatcher;

    QString mObjectTypesFile;
    QDateTime mObjectTypesFileLastSaved;

    static Preferences *mInstance;
    static QString mStartupProject;
};


template<typename T>
class Preference
{
public:
    Preference(const char * const key, T defaultValue = T())
        : mKey(key)
        , mDefault(defaultValue)
    {}

    inline T get() const;
    inline void set(const T &value);

    inline operator T() const { return get(); }
    inline Preference &operator =(const T &value) { set(value); return *this; }

private:
    const char * const mKey;
    const T mDefault;
};

template<typename T>
T Preference<T>::get() const
{
    return Preferences::instance()->get<T>(mKey, mDefault);
}

template<typename T>
void Preference<T>::set(const T &value)
{
    Preferences::instance()->setValue(QLatin1String(mKey), value);
}

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Preferences::ExportOptions)
