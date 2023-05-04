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

#pragma once

#include <QObject>

namespace Tiled {

class EditableTileset;
class EditableMap;
class MapFormat;
class TilesetFormat;
class FileFormat;

class ScriptFileFormatWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool canRead READ canRead)
    Q_PROPERTY(bool canWrite READ canWrite)

public:
    explicit ScriptFileFormatWrapper(FileFormat *format, QObject *parent = nullptr);

    Q_INVOKABLE bool supportsFile(const QString &filename) const;

    bool canRead() const;
    bool canWrite() const;

protected:
    bool assertCanRead() const;
    bool assertCanWrite() const;

    FileFormat *mFormat;
};

class ScriptTilesetFormatWrapper : public ScriptFileFormatWrapper
{
    Q_OBJECT

public:
    explicit ScriptTilesetFormatWrapper(TilesetFormat *format, QObject *parent = nullptr);

    Q_INVOKABLE Tiled::EditableTileset *read(const QString &filename);
    Q_INVOKABLE void write(Tiled::EditableTileset *tileset, const QString &filename);
};

class ScriptMapFormatWrapper : public ScriptFileFormatWrapper
{
    Q_OBJECT

public:
    explicit ScriptMapFormatWrapper(MapFormat *format, QObject *parent = nullptr);

    Q_INVOKABLE Tiled::EditableMap *read(const QString &filename);
    Q_INVOKABLE void write(Tiled::EditableMap *map, const QString &filename);
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptTilesetFormatWrapper*)
Q_DECLARE_METATYPE(Tiled::ScriptMapFormatWrapper*)
