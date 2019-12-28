/*
 * scriptfileformatwrappers.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class ScriptTilesetFormatWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString extension READ extension)

public:
    ScriptTilesetFormatWrapper();
    explicit ScriptTilesetFormatWrapper(TilesetFormat *format, QObject *parent = nullptr);
    ~ScriptTilesetFormatWrapper() override {}

    QString name() const;
    QString extension() const;

    Q_INVOKABLE EditableTileset *read(QString &filename);
    Q_INVOKABLE QString write(EditableTileset *tileset, QString &filename);
    Q_INVOKABLE bool supportsFile(QString &filename) const;

private:
    TilesetFormat *mFormat;
};

class ScriptMapFormatWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString extension READ extension)

public:
    Q_INVOKABLE ScriptMapFormatWrapper();
    Q_INVOKABLE explicit ScriptMapFormatWrapper(MapFormat *format, QObject *parent = nullptr);
    ~ScriptMapFormatWrapper() override {}

    QString name() const;
    QString extension() const;

    Q_INVOKABLE EditableMap *read(QString &filename);
    Q_INVOKABLE QString write(EditableMap *map, QString &filename);
    Q_INVOKABLE bool supportsFile(QString &filename) const;

private:
    MapFormat *mFormat;
};

} // namespace Tiled

//Q_DECLARE_METATYPE(Tiled::ScriptTilesetFormatWrapper*)
//Q_DECLARE_METATYPE(Tiled::ScriptMapFormatWrapper*)
