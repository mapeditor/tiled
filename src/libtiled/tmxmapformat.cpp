/*
 * tmxmapformat.cpp
 * Copyright 2008-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tmxmapformat.h"

#include "map.h"
#include "mapreader.h"
#include "mapwriter.h"

#include <QBuffer>
#include <QDir>
#include <QXmlStreamReader>

using namespace Tiled;

TmxMapFormat::TmxMapFormat(QObject *parent)
    : MapFormat(parent)
{
}

std::unique_ptr<Map> TmxMapFormat::read(const QString &fileName)
{
    mError.clear();

    MapReader reader;
    std::unique_ptr<Map> map(reader.readMap(fileName));
    if (!map)
        mError = reader.errorString();

    return map;
}

bool TmxMapFormat::write(const Map *map, const QString &fileName, Options options)
{
    MapWriter writer;
    writer.setMinimizeOutput(options.testFlag(WriteMinimized));

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

std::unique_ptr<Map> TmxMapFormat::fromByteArray(const QByteArray &data)
{
    mError.clear();

    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QBuffer::ReadOnly);

    MapReader reader;
    std::unique_ptr<Map> map(reader.readMap(&buffer));
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

bool TsxTilesetFormat::write(const Tileset &tileset, const QString &fileName, Options options)
{
    MapWriter writer;
    writer.setMinimizeOutput(options.testFlag(WriteMinimized));

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

XmlObjectTemplateFormat::XmlObjectTemplateFormat(QObject *parent)
    : ObjectTemplateFormat(parent)
{
}

std::unique_ptr<ObjectTemplate> XmlObjectTemplateFormat::read(const QString &fileName)
{
    mError.clear();

    MapReader reader;
    auto objectTemplate = reader.readObjectTemplate(fileName);
    if (!objectTemplate)
        mError = reader.errorString();

    return objectTemplate;
}

bool XmlObjectTemplateFormat::write(const ObjectTemplate *objectTemplate, const QString &fileName)
{
    MapWriter writer;

    bool result = writer.writeObjectTemplate(objectTemplate, fileName);
    if (!result)
        mError = writer.errorString();
    else
        mError.clear();

    return result;
}

bool XmlObjectTemplateFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".tx"), Qt::CaseInsensitive))
        return true;

    if (fileName.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive)) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xml;
            xml.setDevice(&file);

            if (xml.readNextStartElement() && xml.name() == QLatin1String("template"))
                return true;
        }
    }

    return false;
}

#include "moc_tmxmapformat.cpp"
