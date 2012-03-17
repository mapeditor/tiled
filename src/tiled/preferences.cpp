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

#include <QDesktopServices>
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
    : mSettings(new QSettings)
{
    // Retrieve storage settings
    mSettings->beginGroup(QLatin1String("Storage"));
    mLayerDataFormat = (MapWriter::LayerDataFormat)
                       mSettings->value(QLatin1String("LayerDataFormat"),
                                        MapWriter::Base64Zlib).toInt();
    mDtdEnabled = mSettings->value(QLatin1String("DtdEnabled")).toBool();
    mReloadTilesetsOnChange =
            mSettings->value(QLatin1String("ReloadTilesets"), true).toBool();
    mSettings->endGroup();

    // Retrieve interface settings
    mSettings->beginGroup(QLatin1String("Interface"));
    mShowGrid = mSettings->value(QLatin1String("ShowGrid"), false).toBool();
    mSnapToGrid = mSettings->value(QLatin1String("SnapToGrid"),
                                   false).toBool();
    mGridColor = QColor(mSettings->value(QLatin1String("GridColor"),
                                  QColor(Qt::black).name()).toString());
    mHighlightCurrentLayer = mSettings->value(QLatin1String("HighlightCurrentLayer"),
                                              false).toBool();
    mShowTilesetGrid = mSettings->value(QLatin1String("ShowTilesetGrid"),
                                        true).toBool();
    mLanguage = mSettings->value(QLatin1String("Language"),
                                 QString()).toString();
    mUseOpenGL = mSettings->value(QLatin1String("OpenGL"), false).toBool();
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
    mAutoMapDrawing = mSettings->value(QLatin1String("WhileDrawing"),
                                       false).toBool();
    mSettings->endGroup();

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(mReloadTilesetsOnChange);
}

Preferences::~Preferences()
{
    delete mSettings;
}

void Preferences::setShowGrid(bool showGrid)
{
    if (mShowGrid == showGrid)
        return;

    mShowGrid = showGrid;
    mSettings->setValue(QLatin1String("Interface/ShowGrid"), mShowGrid);
    emit showGridChanged(mShowGrid);
}

void Preferences::setSnapToGrid(bool snapToGrid)
{
    if (mSnapToGrid == snapToGrid)
        return;

    mSnapToGrid = snapToGrid;
    mSettings->setValue(QLatin1String("Interface/SnapToGrid"), mSnapToGrid);
    emit snapToGridChanged(mSnapToGrid);
}

void Preferences::setGridColor(QColor gridColor)
{
    if (mGridColor == gridColor)
        return;

    mGridColor = gridColor;
    mSettings->setValue(QLatin1String("Interface/GridColor"), mGridColor.name());
    emit gridColorChanged(mGridColor);
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

MapWriter::LayerDataFormat Preferences::layerDataFormat() const
{
    return mLayerDataFormat;
}

void Preferences::setLayerDataFormat(MapWriter::LayerDataFormat
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

    if (path.isEmpty())
        path = QDesktopServices::storageLocation(
                    QDesktopServices::DocumentsLocation);

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
