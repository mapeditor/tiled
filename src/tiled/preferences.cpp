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
#include "tilesetmanager.h"

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

#include <QFileInfo>
#include <QSettings>

using namespace Tiled;
using namespace Tiled::Internal;

Preferences *Preferences::mInstance = 0;

Preferences *Preferences::instance()
{
    if (!mInstance)
        mInstance = new Preferences;
    return mInstance;
}

void Preferences::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

Preferences::Preferences()
    : mSettings(new QSettings(this))
{
    // Retrieve storage settings
    mSettings->beginGroup(QLatin1String("Storage"));
    mLayerDataFormat = (Map::LayerDataFormat)
                       mSettings->value(QLatin1String("LayerDataFormat"),
                                        Map::Base64Zlib).toInt();
    mDtdEnabled = boolValue("DtdEnabled");
    mReloadTilesetsOnChange = boolValue("ReloadTilesets", true);
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
}

Preferences::~Preferences()
{
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
#if QT_VERSION >= 0x050000
        path = QStandardPaths::writableLocation(
                    QStandardPaths::DocumentsLocation);
#else
        path = QDesktopServices::storageLocation(
                    QDesktopServices::DocumentsLocation);
#endif
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
