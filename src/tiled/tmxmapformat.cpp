/*
 * tmxmapformat.cpp
 * Copyright 2008-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tmxmapformat.h"

#include "map.h"
#include "mapreader.h"
#include "mapwriter.h"
#include "preferences.h"
#include "tilesetmanager.h"

#include <QBuffer>
#include <QDir>
#include <QXmlStreamReader>

using namespace Tiled;
using namespace Tiled::Internal;

TmxMapFormat::TmxMapFormat(QObject *parent)
    : MapFormat(parent)
{
}

Map *TmxMapFormat::read(const QString &fileName)
{
    mError.clear();

    MapReader reader;
    Map *map = reader.readMap(fileName);
    if (!map)
        mError = reader.errorString();

    return map;
}

bool TmxMapFormat::write(const Map *map, const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result = writer.writeMap(map, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

QByteArray TmxMapFormat::toByteArray(const Map *map)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    MapWriter writer;
    writer.writeMap(map, &buffer);

    return buffer.data();
}

Map *TmxMapFormat::fromByteArray(const QByteArray &data)
{
    mError.clear();

    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QBuffer::ReadOnly);

    MapReader reader;
    Map *map = reader.readMap(&buffer);
    if (!map)
        mError = reader.errorString();

    return map;
}

bool TmxMapFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive))
        return true;

    if (fileName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xml;
            xml.setDevice(&file);

            if (xml.readNextStartElement() && xml.name() == QLatin1String("map"))
                return true;
        }
    }

    return false;
}


TsxTilesetFormat::TsxTilesetFormat(QObject *parent)
    : TilesetFormat(parent)
{
}

SharedTileset TsxTilesetFormat::read(const QString &fileName)
{
    mError.clear();

    MapReader reader;
    SharedTileset tileset = reader.readTileset(fileName);
    if (!tileset)
        mError = reader.errorString();

    return tileset;
}

bool TsxTilesetFormat::write(const Tileset &tileset, const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result = writer.writeTileset(tileset, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

bool TsxTilesetFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".tsx"), Qt::CaseInsensitive))
        return true;

    if (fileName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xml;
            xml.setDevice(&file);

            if (xml.readNextStartElement() && xml.name() == QLatin1String("tileset"))
                return true;
        }
    }

    return false;
}

TtxTemplateGroupFormat::TtxTemplateGroupFormat(QObject *parent)
    : TemplateGroupFormat(parent)
{
}

TemplateGroup *TtxTemplateGroupFormat::read(const QString &fileName)
{
    mError.clear();

    MapReader reader;
    TemplateGroup *templateGroup =  reader.readTemplateGroup(fileName);
    if (!templateGroup)
        mError = reader.errorString();

    return templateGroup;
}

bool TtxTemplateGroupFormat::write(const TemplateGroup *templateGroup, const QString &fileName)
{
    Preferences *prefs = Preferences::instance();

    MapWriter writer;
    writer.setDtdEnabled(prefs->dtdEnabled());

    bool result =  writer.writeTemplateGroup(templateGroup, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

bool TtxTemplateGroupFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".ttx"), Qt::CaseInsensitive))
        return true;

    if (fileName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xml;
            xml.setDevice(&file);

            if (xml.readNextStartElement() && xml.name() == QLatin1String("templategroup"))
                return true;
        }
    }

    return false;
}
