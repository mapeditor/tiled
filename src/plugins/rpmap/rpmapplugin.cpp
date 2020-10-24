/*
 * Flare Tiled Plugin
 * Copyright 2010, Jaderamiso <jaderamiso@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2011, Clint Bellanger <clintbellanger@gmail.com>
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

#include "rpmapplugin.h"

#include "gidmapper.h"
#include "map.h"
#include "mapobject.h"
#include "savefile.h"
#include "tile.h"
#include "tiled.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QStringView>
#endif
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QUuid>
#include <kzip.h>

#include <memory>

using namespace Tiled;

namespace RpMap {

RpMapPlugin::RpMapPlugin()
{
}

std::unique_ptr<Tiled::Map> RpMapPlugin::read(const QString &fileName)
{
    KZip archive(fileName);
    if (archive.open(QIODevice::ReadOnly)) {
            const KArchiveDirectory *dir = archive.directory();

            const KArchiveEntry *e = dir->entry("content.xml");
            if (!e) {
                //qDebug() << "File not found!";
                return nullptr;
            }
            const KArchiveFile *f = static_cast<const KArchiveFile *>(e);
            QByteArray arr(f->data());
            //qDebug() << arr; // the file contents

            // To avoid reading everything into memory in one go, we can use createDevice() instead
#if 0
            QIODevice *dev = f->createDevice();
            while (!dev->atEnd()) {
                qDebug() << dev->readLine();
            }
            delete dev;
#endif
    }
    return nullptr;
}

bool RpMapPlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".rpmap"), Qt::CaseInsensitive);
}

QString RpMapPlugin::nameFilter() const
{
    return tr("RpTool MapTool files (*.rpmap)");
}

QString RpMapPlugin::shortName() const
{
    return QStringLiteral("rpmap");
}

QString RpMapPlugin::errorString() const
{
    return mError;
}

static void writeEntry(QXmlStreamWriter &writer, QString const &key, QString const& value) {
    writer.writeStartElement(QStringLiteral("entry"));
    writer.writeTextElement(QStringLiteral("string"), key);
    writer.writeTextElement(QStringLiteral("string"), value);
    writer.writeEndElement();
}

static void writeGUID(QXmlStreamWriter &writer, QString const &key, QUuid const& id) {
    writer.writeStartElement(key);
    writer.writeTextElement(QStringLiteral("baGUID"), id.toRfc4122().toBase64());
    writer.writeEndElement();
}

static void writeTile(QXmlStreamWriter &writer, int x, int y, QString const& name, int facing) {
    writer.writeStartElement(QStringLiteral("entry"));
    writeGUID(writer, QStringLiteral("net.rptools.maptool.model.GUID"), QUuid::createUuid());
    writer.writeStartElement(QStringLiteral("net.rptools.maptool.model.Token"));
    writeGUID(writer, QStringLiteral("exposedAreaGUID"), QUuid::createUuid());
    writer.writeStartElement(QStringLiteral("imageAssetMap"));
    writer.writeStartElement(QStringLiteral("entry"));
    writer.writeEmptyElement(QStringLiteral("null"));
    writer.writeStartElement(QStringLiteral("net.rptools.lib.MD5Key"));
    writer.writeTextElement(QStringLiteral("id"), QStringLiteral("5b55defab5ccba43e2fb54392064c377"));
    writer.writeEndElement(); // MD5Key
    writer.writeEndElement(); // entry
    writer.writeEndElement(); // imageAssetMap
    writer.writeTextElement(QStringLiteral("layer"), QStringLiteral("BACKGROUND"));
    writer.writeTextElement(QStringLiteral("facing"), QString::number(facing));
    writer.writeTextElement(QStringLiteral("x"), QString::number(x));
    writer.writeTextElement(QStringLiteral("y"), QString::number(y));
    writer.writeTextElement(QStringLiteral("name"), name);
    writer.writeEndElement(); // net.rptools.maptool.model.Token
    writer.writeEndElement(); // entry
}

static void writeMap(QXmlStreamWriter &writer, Tiled::Map const* map) {
    writer.writeStartElement(QStringLiteral("zone"));
    writer.writeStartElement(QStringLiteral("grid"));
    writer.writeAttribute(QStringLiteral("class"),QStringLiteral("net.rptools.maptool.model.SquareGrid"));
    writer.writeEndElement(); // grid
    writer.writeStartElement(QStringLiteral("tokenMap"));
    writeTile(writer, 400, 300, QStringLiteral("token"), 180);
    writer.writeEndElement(); // tokenMap   
    writer.writeEndElement(); // grid
    writer.writeEndElement(); // zone
}

bool RpMapPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)
    KZip archive(fileName);
    if (archive.open(QIODevice::WriteOnly))
    {
        {
        QByteArray properties;
        QXmlStreamWriter writer(&properties);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(1);
        writer.writeStartDocument();
        writer.writeStartElement(QStringLiteral("map"));
        writeEntry(writer, QStringLiteral("campaignVersion"), QStringLiteral("1.4.1"));
        writeEntry(writer, QStringLiteral("version"), QStringLiteral("1.7.0"));
        writer.writeEndElement();
        writer.writeEndDocument();
        archive.writeFile(QStringLiteral("properties.xml"), properties);
        }
        {
        QByteArray content;
        QXmlStreamWriter writer(&content);
        writer.setAutoFormatting(true);
        writer.setAutoFormattingIndent(1);
        writer.writeStartDocument();
        writer.writeStartElement(QStringLiteral("net.rptools.maptool.util.PersistenceUtil_-PersistedMap"));
        writeMap(writer, map);
        writer.writeEndElement();
        writer.writeEndDocument();
        archive.writeFile(QStringLiteral("content.xml"), content);
        }
        archive.close();
        return true;
    }
    return false;
}

} // namespace RpMap
