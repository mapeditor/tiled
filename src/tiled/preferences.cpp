/*
 * preferences.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "tilesetmanager.h"

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
                                        MapWriter::Base64Gzip).toInt();
    mDtdEnabled = mSettings->value(QLatin1String("DtdEnabled")).toBool();
    mReloadTilesetsOnChange =
            mSettings->value(QLatin1String("ReloadTilesets"), true).toBool();
    mSettings->endGroup();

    // Retrieve interface settings
    mSettings->beginGroup(QLatin1String("Interface"));
    mLanguage = mSettings->value(QLatin1String("Language"),
                                 QString()).toString();
    mUseOpenGL = mSettings->value(QLatin1String("OpenGL"), false).toBool();
    mSettings->endGroup();

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->setReloadTilesetsOnChange(mReloadTilesetsOnChange);
}

Preferences::~Preferences()
{
    delete mSettings;
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
