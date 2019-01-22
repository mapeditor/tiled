/*
 * JSON Tiled Plugin
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "json_global.h"

#include "mapformat.h"
#include "objecttemplateformat.h"
#include "plugin.h"
#include "tilesetformat.h"

#include <QObject>

namespace Tiled {
class Map;
}

namespace Json {

class JSONSHARED_EXPORT JsonPlugin : public Tiled::Plugin
{
    Q_OBJECT
    Q_INTERFACES(Tiled::Plugin)
    Q_PLUGIN_METADATA(IID "org.mapeditor.Plugin" FILE "plugin.json")

public:
    void initialize() override;
};


class JSONSHARED_EXPORT JsonMapFormat : public Tiled::MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)

public:
    enum SubFormat {
        Json,
        JavaScript,
    };

    JsonMapFormat(SubFormat subFormat, QObject *parent = nullptr);

    Tiled::Map *read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Map *map, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;
    SubFormat mSubFormat;
};


class JSONSHARED_EXPORT JsonTilesetFormat : public Tiled::TilesetFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::TilesetFormat)

public:
    JsonTilesetFormat(QObject *parent = nullptr);

    Tiled::SharedTileset read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::Tileset &tileset, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;
};

class JSONSHARED_EXPORT JsonObjectTemplateFormat : public Tiled::ObjectTemplateFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::ObjectTemplateFormat)

public:
    JsonObjectTemplateFormat(QObject *parent = nullptr);

    Tiled::ObjectTemplate *read(const QString &fileName) override;
    bool supportsFile(const QString &fileName) const override;

    bool write(const Tiled::ObjectTemplate *objectTemplate, const QString &fileName) override;

    QString nameFilter() const override;
    QString shortName() const override;
    QString errorString() const override;

protected:
    QString mError;
};

} // namespace Json
