/*
 * scriptfileformatwrappers.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>, Phlosioneer
 * <mattmdrr2@gmail.com>
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

#include "scriptfileformatwrappers.h"

#include "editablemap.h"
#include "editabletileset.h"
#include "mapformat.h"
#include "scriptedfileformat.h"
#include "scriptmanager.h"
#include "tilesetformat.h"

#include <QCoreApplication>
#include <QQmlEngine>

namespace Tiled {

ScriptTilesetFormatWrapper::ScriptTilesetFormatWrapper(TilesetFormat* format, QObject *parent)
    : QObject(parent)
    , mFormat(format)
{}

QString ScriptTilesetFormatWrapper::name() const
{
    if (auto scriptedFormat = qobject_cast<ScriptedTilesetFormat*>(mFormat))
        return scriptedFormat->name();
    else
        return mFormat->shortName();
}

QString ScriptTilesetFormatWrapper::extension() const
{
    if (auto scriptedFormat = qobject_cast<ScriptedTilesetFormat*>(mFormat))
        return scriptedFormat->extension();
    else
        return QString();
}

EditableTileset *ScriptTilesetFormatWrapper::read(const QString &filename)
{
    auto tileset = mFormat->read(filename);
    if (tileset) {
        auto editable = new EditableTileset(tileset.data());
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
        return editable;
    } else {
        auto message = QCoreApplication::translate("Script Errors", "Error reading tileset");
        ScriptManager::instance().throwError(message);
        return nullptr;
    }
}

QString ScriptTilesetFormatWrapper::write(const EditableTileset *editable, const QString &filename)
{
    auto tileset = editable->tileset();
    if (mFormat->write(*tileset, filename))
        return QString();
    else
        return mFormat->errorString();
}

bool ScriptTilesetFormatWrapper::supportsFile(const QString &filename) const
{
    return mFormat->supportsFile(filename);
}

ScriptMapFormatWrapper::ScriptMapFormatWrapper(MapFormat *format, QObject *parent)
    : QObject(parent)
    , mFormat(format)
{}

QString ScriptMapFormatWrapper::name() const
{
    if (auto scriptedFormat = qobject_cast<ScriptedMapFormat*>(mFormat))
        return scriptedFormat->name();
    else
        return mFormat->shortName();
}

QString ScriptMapFormatWrapper::extension() const
{
    if (auto scriptedFormat = qobject_cast<ScriptedMapFormat*>(mFormat))
        return scriptedFormat->extension();
    else
        return QString();
}

EditableMap *ScriptMapFormatWrapper::read(const QString &filename)
{
    auto map = mFormat->read(filename);
    if (map) {
        auto editable = new EditableMap(std::move(map));
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::JavaScriptOwnership);
        return editable;
    } else {
        auto message = QCoreApplication::translate("Script Errors", "Error reading map");
        ScriptManager::instance().throwError(message);
        return nullptr;
    }
}

QString ScriptMapFormatWrapper::write(const EditableMap *editable, const QString &filename)
{
    auto map = editable->map();
    if (mFormat->write(map, filename))
        return QString();
    else
        return mFormat->errorString();
}

bool ScriptMapFormatWrapper::supportsFile(const QString &filename) const
{
    return mFormat->supportsFile(filename);
}

}
