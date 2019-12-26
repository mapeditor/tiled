/*
 * scriptedfileformat.h
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

#include "mapformat.h"
#include "tilesetformat.h"

#include <QJSValue>

namespace Tiled {

class EditableAsset;

class ScriptedFileFormat
{
public:
    explicit ScriptedFileFormat(const QJSValue &object);

    FileFormat::Capabilities capabilities() const;
    QString nameFilter() const;
    bool supportsFile(const QString &fileName) const;

    QJSValue read(const QString &fileName);
    bool write(EditableAsset *asset, const QString &fileName,
               FileFormat::Options options, QString &error);

    QStringList outputFiles(EditableAsset *asset, const QString &fileName) const;

    static bool validateFileFormatObject(const QJSValue &value);

private:
    QJSValue mObject;
};

class ScriptedMapFormat final : public MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    ScriptedMapFormat(const QString &shortName, const QJSValue &object,
                      QObject *parent = nullptr);
    ~ScriptedMapFormat() override;

    // FileFormat interface
    Capabilities capabilities() const override { return mFormat.capabilities(); }
    QString nameFilter() const override { return mFormat.nameFilter(); }
    QString shortName() const override { return mShortName; }
    bool supportsFile(const QString &fileName) const override { return mFormat.supportsFile(fileName); }
    QString errorString() const override { return mError; }

    // MapFormat interface
    QStringList outputFiles(const Map *map, const QString &fileName) const override;
    std::unique_ptr<Map> read(const QString &fileName) override;
    bool write(const Map *map, const QString &fileName, Options options) override;

private:
    QString mShortName;
    QString mError;
    ScriptedFileFormat mFormat;
};

class ScriptedTilesetFormat final : public TilesetFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat)

public:
    ScriptedTilesetFormat(const QString &shortName, const QJSValue &object,
                          QObject *parent = nullptr);
    ~ScriptedTilesetFormat() override;

    // FileFormat interface
    Capabilities capabilities() const override { return mFormat.capabilities(); }
    QString nameFilter() const override { return mFormat.nameFilter(); }
    QString shortName() const override { return mShortName; }
    bool supportsFile(const QString &fileName) const override { return mFormat.supportsFile(fileName); }
    QString errorString() const override { return mError; }

    // TilesetFormat interface
    SharedTileset read(const QString &fileName) override;
    bool write(const Tileset &tileset, const QString &fileName, Options options = Options()) override;

private:
    QString mShortName;
    QString mError;
    ScriptedFileFormat mFormat;
};

} // namespace Tiled
