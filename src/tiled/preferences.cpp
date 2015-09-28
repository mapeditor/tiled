/*
 * preferences.cpp
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "tilesetmanager.h"

#include <QStandardPaths>

#include <QFileInfo>
#include <QSettings>

using namespace Tiled;
using namespace Tiled::Internal;

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
    : mSettings(new QSettings(this))
{
    // Retrieve storage settings
    mSettings->beginGroup(QLatin1String("Storage"));
    mLayerDataFormat = static_cast<Map::LayerDataFormat>
            (intValue("LayerDataFormat", Map::CSV));
    mMapRenderOrder = static_cast<Map::RenderOrder>
            (intValue("MapRenderOrder", Map::RightDown));
    mDtdEnabled = boolValue("DtdEnabled");
    mReloadTilesetsOnChange = boolValue("ReloadTilesets", true);
    mStampsDirectory = stringValue("StampsDirectory");
    mSettings->endGroup();

    // Retrieve interface settings
    mSettings->beginGroup(QLatin1String("Interface"));
    mShowGrid = boolValue("ShowGrid");
    mShowTileObjectOutlines = boolValue("ShowTileObjectOutlines");
    mShowTileAnimations = boolValue("ShowTileAnimations", true);
    mSnapToGrid = boolValue("SnapToGrid");
    mSnapToFineGrid = boolValue("SnapToFineGrid");
    mGridColor = colorValue("GridColor", Qt::black);
    mGridFine = intValue("GridFine", 4);
    mObjectLineWidth = realValue("ObjectLineWidth", 2);
    mHighlightCurrentLayer = boolValue("HighlightCurrentLayer");
    mShowTilesetGrid = boolValue("ShowTilesetGrid", true);
    mLanguage = stringValue("Language");
    mUseOpenGL = boolValue("OpenGL");
    mObjectLabelVisibility = static_cast<ObjectLabelVisiblity>
            (intValue("ObjectLabelVisibility", AllObjectLabels));
    mSettings->endGroup();

    // Retrieve defined object types
    mSettings->beginGroup(QLatin1String("ObjectTypes"));
    const QStringList names =
            mSettings->value(QLatin1String("Names")).toStringList();
    const QStringList colors =
            mSettings->value(QLatin1String("Colors")).toStringList();
    mSettings->endGroup();

    const int count = qMin(names.size(), colors.size());
    for (int i = 0; i < count; ++i)
        mObjectTypes.append(ObjectType(names.at(i), QColor(colors.at(i))));

    mSettings->beginGroup(QLatin1String("Automapping"));
    mAutoMapDrawing = boolValue("WhileDrawing");
    mSettings->endGroup();

    mSettings->beginGroup(QLatin1String("MapsDirectory"));
    mMapsDirectory = stringValue("Current");
    mSettings->endGroup();

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(mReloadTilesetsOnChange);
    tilesetManager->setAnimateTiles(mShowTileAnimations);

    // Keeping track of some usage information
    mSettings->beginGroup(QLatin1String("Install"));
    mFirstRun = mSettings->value(QLatin1String("FirstRun")).toDate();
    mRunCount = intValue("RunCount", 0) + 1;
    mIsPatron = boolValue("IsPatron");
    if (!mFirstRun.isValid()) {
        mFirstRun = QDate::currentDate();
        mSettings->setValue(QLatin1String("FirstRun"), mFirstRun.toString(Qt::ISODate));
    }
    mSettings->setValue(QLatin1String("RunCount"), mRunCount);
    mSettings->endGroup();

    // Retrieve startup settings
    mSettings->beginGroup(QLatin1String("Startup"));
    mOpenLastFilesOnStartup = boolValue("OpenLastFiles", true);
    mSettings->endGroup();
}

Preferences::~Preferences()
{
}

void Preferences::setObjectLabelVisibility(ObjectLabelVisiblity visibility)
{
    if (mObjectLabelVisibility == visibility)
        return;

    mObjectLabelVisibility = visibility;
    mSettings->setValue(QLatin1String("Interface/ObjectLabelVisibility"), visibility);
    emit objectLabelVisibilityChanged(visibility);
}

void Preferences::setShowGrid(bool showGrid)
{
    if (mShowGrid == showGrid)
        return;

    mShowGrid = showGrid;
    mSettings->setValue(QLatin1String("Interface/ShowGrid"), mShowGrid);
    emit showGridChanged(mShowGrid);
}

void Preferences::setShowTileObjectOutlines(bool enabled)
{
    if (mShowTileObjectOutlines == enabled)
        return;

    mShowTileObjectOutlines = enabled;
    mSettings->setValue(QLatin1String("Interface/ShowTileObjectOutlines"),
                        mShowTileObjectOutlines);
    emit showTileObjectOutlinesChanged(mShowTileObjectOutlines);
}

void Preferences::setShowTileAnimations(bool enabled)
{
    if (mShowTileAnimations == enabled)
        return;

    mShowTileAnimations = enabled;
    mSettings->setValue(QLatin1String("Interface/ShowTileAnimations"),
                        mShowTileAnimations);

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setAnimateTiles(mShowTileAnimations);

    emit showTileAnimationsChanged(mShowTileAnimations);
}

void Preferences::setSnapToGrid(bool snapToGrid)
{
    if (mSnapToGrid == snapToGrid)
        return;

    mSnapToGrid = snapToGrid;
    mSettings->setValue(QLatin1String("Interface/SnapToGrid"), mSnapToGrid);
    emit snapToGridChanged(mSnapToGrid);
}

void Preferences::setSnapToFineGrid(bool snapToFineGrid)
{
    if (mSnapToFineGrid == snapToFineGrid)
        return;

    mSnapToFineGrid = snapToFineGrid;
    mSettings->setValue(QLatin1String("Interface/SnapToFineGrid"), mSnapToFineGrid);
    emit snapToFineGridChanged(mSnapToFineGrid);
}

void Preferences::setGridColor(QColor gridColor)
{
    if (mGridColor == gridColor)
        return;

    mGridColor = gridColor;
    mSettings->setValue(QLatin1String("Interface/GridColor"), mGridColor.name());
    emit gridColorChanged(mGridColor);
}

void Preferences::setGridFine(int gridFine)
{
    if (mGridFine == gridFine)
        return;
    mGridFine = gridFine;
    mSettings->setValue(QLatin1String("Interface/GridFine"), mGridFine);
    emit gridFineChanged(mGridFine);
}

void Preferences::setObjectLineWidth(qreal lineWidth)
{
    if (mObjectLineWidth == lineWidth)
        return;
    mObjectLineWidth = lineWidth;
    mSettings->setValue(QLatin1String("Interface/ObjectLineWidth"), mObjectLineWidth);
    emit objectLineWidthChanged(mObjectLineWidth);
}

void Preferences::setHighlightCurrentLayer(bool highlight)
{
    if (mHighlightCurrentLayer == highlight)
        return;

    mHighlightCurrentLayer = highlight;
    mSettings->setValue(QLatin1String("Interface/HighlightCurrentLayer"),
                        mHighlightCurrentLayer);
    emit highlightCurrentLayerChanged(mHighlightCurrentLayer);
}

void Preferences::setShowTilesetGrid(bool showTilesetGrid)
{
    if (mShowTilesetGrid == showTilesetGrid)
        return;

    mShowTilesetGrid = showTilesetGrid;
    mSettings->setValue(QLatin1String("Interface/ShowTilesetGrid"),
                        mShowTilesetGrid);
    emit showTilesetGridChanged(mShowTilesetGrid);
}

Map::LayerDataFormat Preferences::layerDataFormat() const
{
    return mLayerDataFormat;
}

void Preferences::setLayerDataFormat(Map::LayerDataFormat
                                     layerDataFormat)
{
    if (mLayerDataFormat == layerDataFormat)
        return;

    mLayerDataFormat = layerDataFormat;
    mSettings->setValue(QLatin1String("Storage/LayerDataFormat"),
                        mLayerDataFormat);
}

Map::RenderOrder Preferences::mapRenderOrder() const
{
    return mMapRenderOrder;
}

void Preferences::setMapRenderOrder(Map::RenderOrder mapRenderOrder)
{
    if (mMapRenderOrder == mapRenderOrder)
        return;

    mMapRenderOrder = mapRenderOrder;
    mSettings->setValue(QLatin1String("Storage/MapRenderOrder"),
                        mMapRenderOrder);
}


bool Preferences::dtdEnabled() const
{
    return mDtdEnabled;
}

void Preferences::setDtdEnabled(bool enabled)
{
    mDtdEnabled = enabled;
    mSettings->setValue(QLatin1String("Storage/DtdEnabled"), enabled);
}

QString Preferences::language() const
{
    return mLanguage;
}

void Preferences::setLanguage(const QString &language)
{
    if (mLanguage == language)
        return;

    mLanguage = language;
    mSettings->setValue(QLatin1String("Interface/Language"),
                        mLanguage);

    LanguageManager::instance()->installTranslators();
}

bool Preferences::reloadTilesetsOnChange() const
{
    return mReloadTilesetsOnChange;
}

void Preferences::setReloadTilesetsOnChanged(bool value)
{
    if (mReloadTilesetsOnChange == value)
        return;

    mReloadTilesetsOnChange = value;
    mSettings->setValue(QLatin1String("Storage/ReloadTilesets"),
                        mReloadTilesetsOnChange);

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(mReloadTilesetsOnChange);
}

void Preferences::setUseOpenGL(bool useOpenGL)
{
    if (mUseOpenGL == useOpenGL)
        return;

    mUseOpenGL = useOpenGL;
    mSettings->setValue(QLatin1String("Interface/OpenGL"), mUseOpenGL);

    emit useOpenGLChanged(mUseOpenGL);
}

void Preferences::setObjectTypes(const ObjectTypes &objectTypes)
{
    mObjectTypes = objectTypes;

    QStringList names;
    QStringList colors;
    foreach (const ObjectType &objectType, objectTypes) {
        names.append(objectType.name);
        colors.append(objectType.color.name());
    }

    mSettings->beginGroup(QLatin1String("ObjectTypes"));
    mSettings->setValue(QLatin1String("Names"), names);
    mSettings->setValue(QLatin1String("Colors"), colors);
    mSettings->endGroup();

    emit objectTypesChanged();
}

static QString lastPathKey(Preferences::FileType fileType)
{
    QString key = QLatin1String("LastPaths/");

    switch (fileType) {
    case Preferences::ObjectTypesFile:
        key.append(QLatin1String("ObjectTypes"));
        break;
    case Preferences::ImageFile:
        key.append(QLatin1String("Images"));
        break;
    case Preferences::ExportedFile:
        key.append(QLatin1String("ExportedFile"));
        break;
    case Preferences::ExternalTileset:
        key.append(QLatin1String("ExternalTileset"));
        break;
    default:
        Q_ASSERT(false); // Getting here means invalid file type
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
        MapDocument *mapDocument = documentManager->currentDocument();
        if (mapDocument)
            path = QFileInfo(mapDocument->fileName()).path();
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
    mSettings->setValue(lastPathKey(fileType), path);
}

void Preferences::setAutomappingDrawing(bool enabled)
{
    mAutoMapDrawing = enabled;
    mSettings->setValue(QLatin1String("Automapping/WhileDrawing"), enabled);
}

QString Preferences::mapsDirectory() const
{
    return mMapsDirectory;
}

void Preferences::setMapsDirectory(const QString &path)
{
    if (mMapsDirectory == path)
        return;
    mMapsDirectory = path;
    mSettings->setValue(QLatin1String("MapsDirectory/Current"), path);

    emit mapsDirectoryChanged();
}

void Preferences::setPatron(bool isPatron)
{
    if (mIsPatron == isPatron)
        return;

    mIsPatron = isPatron;
    mSettings->setValue(QLatin1String("Install/IsPatron"), isPatron);

    emit isPatronChanged();
}

void Preferences::setOpenLastFilesOnStartup(bool open)
{
    if (mOpenLastFilesOnStartup == open)
    	return;

    mOpenLastFilesOnStartup = open;
    mSettings->setValue(QLatin1String("Startup/OpenLastFiles"), open);
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

QString Preferences::stampsDirectory() const
{
    if (mStampsDirectory.isEmpty()) {
#if QT_VERSION >= 0x050400
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
        QString appData = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
        return appData + QLatin1String("/stamps");
    }
    return mStampsDirectory;
}

void Preferences::setStampsDirectory(const QString &stampsDirectory)
{
    if (mStampsDirectory == stampsDirectory)
        return;

    mStampsDirectory = stampsDirectory;
    mSettings->setValue(QLatin1String("Storage/StampsDirectory"), stampsDirectory);

    emit stampsDirectoryChanged(stampsDirectory);
}
