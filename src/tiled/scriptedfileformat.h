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
#include <QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

namespace Tiled {

class EditableAsset;

class ScriptedFileFormat
{
public:
    ScriptedFileFormat() = default;
    explicit ScriptedFileFormat(const QJSValue &object);

    void setObject(const QJSValue &object) { mObject = object; }

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

/**
 * A map format registered by an extension, either through
 * tiled.registerMapFormat or declared as a QML component. The 'read',
 * 'write' and 'outputFiles' functions are looked up on the script object,
 * which for QML declared formats is the format object itself.
 */
class ScriptedMapFormat : public MapFormat, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat QQmlParserStatus)
    QML_NAMED_ELEMENT(MapFormat)

    Q_PROPERTY(QString shortName READ shortName WRITE setShortName)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString extension READ extension WRITE setExtension)

public:
    explicit ScriptedMapFormat(QObject *parent = nullptr);
    ScriptedMapFormat(const QString &shortName, const QJSValue &object,
                      QObject *parent = nullptr);
    ~ScriptedMapFormat() override;

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString extension() const { return mExtension; }
    void setExtension(const QString &extension) { mExtension = extension; }

    void setShortName(const QString &shortName) { mShortName = shortName; }

    void classBegin() override {}
    void componentComplete() override;

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
    QString mName;
    QString mExtension;
    QString mError;
    ScriptedFileFormat mFormat;
    bool mAddedToPluginManager = false;
};

/**
 * A tileset format registered by an extension, either through
 * tiled.registerTilesetFormat or declared as a QML component.
 */
class ScriptedTilesetFormat : public TilesetFormat, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat QQmlParserStatus)
    QML_NAMED_ELEMENT(TilesetFormat)

    Q_PROPERTY(QString shortName READ shortName WRITE setShortName)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString extension READ extension WRITE setExtension)

public:
    explicit ScriptedTilesetFormat(QObject *parent = nullptr);
    ScriptedTilesetFormat(const QString &shortName, const QJSValue &object,
                          QObject *parent = nullptr);
    ~ScriptedTilesetFormat() override;

    QString name() const { return mName; }
    void setName(const QString &name) { mName = name; }

    QString extension() const { return mExtension; }
    void setExtension(const QString &extension) { mExtension = extension; }

    void setShortName(const QString &shortName) { mShortName = shortName; }

    void classBegin() override {}
    void componentComplete() override;

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
    QString mName;
    QString mExtension;
    QString mError;
    ScriptedFileFormat mFormat;
    bool mAddedToPluginManager = false;
};

} // namespace Tiled
