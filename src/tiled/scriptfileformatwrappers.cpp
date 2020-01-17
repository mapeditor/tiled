/*
 * scriptfileformatwrappers.h
 * Copyright 2019, Phlosioneer <mattmdrr2@gmail.com>
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

ScriptFileFormatWrapper::ScriptFileFormatWrapper(FileFormat *format, QObject *parent)
    : QObject(parent),
      mFormat(format)
{}

bool ScriptFileFormatWrapper::supportsFile(const QString &filename) const
{
    return mFormat->supportsFile(filename);
}

bool ScriptFileFormatWrapper::canRead() const
{
    return mFormat->capabilities() & FileFormat::Read;
}

bool ScriptFileFormatWrapper::canWrite() const
{
    return mFormat->capabilities() & FileFormat::Write;
}

bool ScriptFileFormatWrapper::assertCanRead() const
{
    if (canRead())
        return true;
    auto message = QCoreApplication::translate("Script Errors", "File format doesn't support `read`");
    ScriptManager::instance().throwError(message);
    return false;
}

bool ScriptFileFormatWrapper::assertCanWrite() const
{
    if (canWrite())
        return true;
    auto message = QCoreApplication::translate("Script Errors", "File format doesn't support `write`");
    ScriptManager::instance().throwError(message);
    return false;
}


ScriptTilesetFormatWrapper::ScriptTilesetFormatWrapper(TilesetFormat* format, QObject *parent)
    : ScriptFileFormatWrapper(format, parent)
{}

EditableTileset *ScriptTilesetFormatWrapper::read(const QString &filename)
{
    if (!assertCanRead())
        return nullptr;

    auto tileset = static_cast<TilesetFormat*>(mFormat)->read(filename);
    if (!tileset) {
        auto message = QCoreApplication::translate("Script Errors", "Error reading tileset");
        ScriptManager::instance().throwError(message);
        return nullptr;
    }

    return new EditableTileset(tileset.data());
}

void ScriptTilesetFormatWrapper::write(EditableTileset *editable, const QString &filename)
{
    if (!editable) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    if (!assertCanWrite())
        return;

    auto tileset = editable->tileset();
    auto success = static_cast<TilesetFormat*>(mFormat)->write(*tileset, filename);
    if (!success)
        ScriptManager::instance().throwError(mFormat->errorString());
}


ScriptMapFormatWrapper::ScriptMapFormatWrapper(MapFormat *format, QObject *parent)
    : ScriptFileFormatWrapper(format, parent)
{}

EditableMap *ScriptMapFormatWrapper::read(const QString &filename)
{
    if (!assertCanRead())
        return nullptr;

    auto map = static_cast<MapFormat*>(mFormat)->read(filename);
    if (!map) {
        auto message = QCoreApplication::translate("Script Errors", "Error reading map");
        ScriptManager::instance().throwError(message);
        return nullptr;
    }

    return new EditableMap(std::move(map));
}

void ScriptMapFormatWrapper::write(EditableMap *editable, const QString &filename)
{
    if (!editable) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    if (!assertCanWrite())
        return;

    auto map = editable->map();
    auto success = static_cast<MapFormat*>(mFormat)->write(map, filename);
    if (!success)
        ScriptManager::instance().throwError(mFormat->errorString());
}

} // namespace Tiled
