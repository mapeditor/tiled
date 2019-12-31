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
    explicit ScriptTilesetFormatWrapper(TilesetFormat *format, QObject *parent = nullptr);

    QString name() const;
    QString extension() const;

    Q_INVOKABLE Tiled::EditableTileset *read(const QString &filename);
    Q_INVOKABLE QString write(const EditableTileset *tileset, const QString &filename);
    Q_INVOKABLE bool supportsFile(const QString &filename) const;

private:
    TilesetFormat *mFormat;
};

class ScriptMapFormatWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString extension READ extension)

public:
    explicit ScriptMapFormatWrapper(MapFormat *format, QObject *parent = nullptr);

    QString name() const;
    QString extension() const;

    Q_INVOKABLE Tiled::EditableMap *read(const QString &filename);
    Q_INVOKABLE QString write(const EditableMap *map, const QString &filename);
    Q_INVOKABLE bool supportsFile(const QString &filename) const;

private:
    MapFormat *mFormat;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptTilesetFormatWrapper*)
Q_DECLARE_METATYPE(Tiled::ScriptMapFormatWrapper*)
