/*
 * RpMap Tiled Plugin
 * Copyright 2020, Christof Petig <christof.petig@arcor.de>
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
#include <QCryptographicHash>
#include <kzip.h>

#include <memory>

using namespace Tiled;

namespace RpMap {

RpMapPlugin::RpMapPlugin()
{
}

#if 0 // not implemented for now
std::unique_ptr<Tiled::Map> RpMapPlugin::read(const QString &fileName)
{
    // the problem is to either reference an already loaded tileset (how) or to import the included image resources
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
#endif

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

static void writeEntry(QXmlStreamWriter &writer, QString const &key, QString const& value)
{
    writer.writeStartElement(QStringLiteral("entry"));
    writer.writeTextElement(QStringLiteral("string"), key);
    writer.writeTextElement(QStringLiteral("string"), value);
    writer.writeEndElement();
}

static void writeGUID(QXmlStreamWriter &writer, QString const &key, QUuid const& id)
{
    writer.writeStartElement(key);
    writer.writeTextElement(QStringLiteral("baGUID"), id.toRfc4122().toBase64());
    writer.writeEndElement();
}

static void writeTile(QXmlStreamWriter &writer, int x, int y, Tile const* tile, int facing, QString const& md5, bool flipx, bool flipy)
{
    const QSize size = tile->size();

    writer.writeStartElement(QStringLiteral("entry"));
    writeGUID(writer, QStringLiteral("net.rptools.maptool.model.GUID"), QUuid::createUuid());
    writer.writeStartElement(QStringLiteral("net.rptools.maptool.model.Token"));
    writer.writeStartElement(QStringLiteral("id")); writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../net.rptools.maptool.model.GUID")); writer.writeEndElement();
    //writeGUID(writer, QStringLiteral("exposedAreaGUID"), QUuid::createUuid());
    writer.writeStartElement(QStringLiteral("imageAssetMap"));
    writer.writeStartElement(QStringLiteral("entry"));
    writer.writeEmptyElement(QStringLiteral("null"));
    writer.writeStartElement(QStringLiteral("net.rptools.lib.MD5Key"));
    writer.writeTextElement(QStringLiteral("id"), md5);
    writer.writeEndElement(); // MD5Key
    writer.writeEndElement(); // entry
    writer.writeEndElement(); // imageAssetMap
    writer.writeTextElement(QStringLiteral("x"), QString::number(x));
    writer.writeTextElement(QStringLiteral("y"), QString::number(y));
    writer.writeTextElement(QStringLiteral("z"), QString::number(1));
    writer.writeTextElement(QStringLiteral("anchorX"), QString::number(0));
    writer.writeTextElement(QStringLiteral("anchorY"), QString::number(0));
    writer.writeTextElement(QStringLiteral("snapToScale"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("width"), QString::number(size.width()));
    writer.writeTextElement(QStringLiteral("height"), QString::number(size.height()));
    writer.writeTextElement(QStringLiteral("isoWidth"), QString::number(0));
    writer.writeTextElement(QStringLiteral("isoHeight"), QString::number(0));
    writer.writeTextElement(QStringLiteral("scaleX"), QString::number(1.0));
    writer.writeTextElement(QStringLiteral("scaleY"), QString::number(1.0));

    writer.writeStartElement(QStringLiteral("sizeMap"));
    writer.writeStartElement(QStringLiteral("entry"));
    writer.writeTextElement(QStringLiteral("java-class"), QStringLiteral("net.rptools.maptool.model.SquareGrid"));
    writeGUID(writer, QStringLiteral("net.rptools.maptool.model.GUID"), QUuid::createUuid());
    writer.writeEndElement(); // entry
    writer.writeEndElement(); // sizeMap

    writer.writeTextElement(QStringLiteral("snapToGrid"), QStringLiteral("true"));
    writer.writeTextElement(QStringLiteral("isVisible"), QStringLiteral("true"));
    writer.writeTextElement(QStringLiteral("visibleOnlyToOwner"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("vblColorSensitivity"), QString::number(-1));
    writer.writeTextElement(QStringLiteral("alwaysVisibleTolerance"), QString::number(2));
    writer.writeTextElement(QStringLiteral("isAlwaysVisible"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("name"), QStringLiteral("token"));
    writer.writeTextElement(QStringLiteral("ownerType"), QString::number(0));
    writer.writeTextElement(QStringLiteral("tokenShape"), QStringLiteral("TOP_DOWN"));
    writer.writeTextElement(QStringLiteral("tokenType"), QStringLiteral("NPC"));
    writer.writeTextElement(QStringLiteral("layer"), QStringLiteral("BACKGROUND"));
    writer.writeTextElement(QStringLiteral("propertyType"), QStringLiteral("Basic"));
    writer.writeTextElement(QStringLiteral("facing"), QString::number(facing));
    writer.writeTextElement(QStringLiteral("tokenOpacity"), QString::number(1.0));
    writer.writeTextElement(QStringLiteral("terrainModifier"), QString::number(0.0));
    writer.writeTextElement(QStringLiteral("terrainModifierOperation"), QStringLiteral("NONE"));
    writer.writeStartElement(QStringLiteral("terrainModifiersIgnored"));
    writer.writeTextElement(QStringLiteral("net.rptools.maptool.model.Token_-TerrainModifierOperation"), QStringLiteral("NONE"));
    writer.writeEndElement(); // terrainModifiersIgnored
    writer.writeTextElement(QStringLiteral("isFlippedX"), flipx ? QStringLiteral("true") : QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("isFlippedY"), flipy ? QStringLiteral("true") : QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("sightType"), QStringLiteral("Normal"));
    writer.writeTextElement(QStringLiteral("hasSight"), QStringLiteral("false"));
    writer.writeEmptyElement(QStringLiteral("state"));
    writer.writeStartElement(QStringLiteral("propertyMapCI"));
    writer.writeEmptyElement(QStringLiteral("store"));
    writer.writeEndElement(); // propertyMapCI
    writer.writeEmptyElement(QStringLiteral("macroPropertiesMap"));
    writer.writeEmptyElement(QStringLiteral("speechMap"));

    writer.writeEndElement(); // net.rptools.maptool.model.Token
    writer.writeEndElement(); // entry
}

static void writeCellShape(QXmlStreamWriter &writer, Tiled::Map const* map)
{
    writer.writeStartElement(QStringLiteral("cellShape"));
    writer.writeStartElement(QStringLiteral("curves"));
    writer.writeStartElement(QStringLiteral("sun.awt.geom.Order0"));
    writer.writeTextElement(QStringLiteral("direction"), QStringLiteral("1"));
    writer.writeTextElement(QStringLiteral("x"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("y"), QStringLiteral("0.0"));
    writer.writeEndElement(); // Order0
    writer.writeStartElement(QStringLiteral("sun.awt.geom.Order1"));
    writer.writeTextElement(QStringLiteral("direction"), QStringLiteral("1"));
    writer.writeTextElement(QStringLiteral("x0"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("y0"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("x1"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("y1"), QString::number(map->tileHeight()));
    writer.writeTextElement(QStringLiteral("xmin"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("xmax"), QStringLiteral("0.0"));
    writer.writeEndElement(); // Order1
    writer.writeStartElement(QStringLiteral("sun.awt.geom.Order1"));
    writer.writeTextElement(QStringLiteral("direction"), QStringLiteral("-1"));
    writer.writeTextElement(QStringLiteral("x0"), QString::number(map->tileWidth()));
    writer.writeTextElement(QStringLiteral("y0"), QStringLiteral("0.0"));
    writer.writeTextElement(QStringLiteral("x1"), QString::number(map->tileWidth()));
    writer.writeTextElement(QStringLiteral("y1"), QString::number(map->tileHeight()));
    writer.writeTextElement(QStringLiteral("xmin"), QString::number(map->tileWidth()));
    writer.writeTextElement(QStringLiteral("xmax"), QString::number(map->tileWidth()));
    writer.writeEndElement(); // Order1
    writer.writeEndElement(); // curves
    writer.writeEndElement(); // cellShape
}

static void writeGrid(QXmlStreamWriter &writer, Tiled::Map const* map)
{
    writer.writeStartElement(QStringLiteral("grid"));
    writer.writeAttribute(QStringLiteral("class"), QStringLiteral("net.rptools.maptool.model.SquareGrid"));
    writer.writeTextElement(QStringLiteral("offsetX"), QString::number(0));
    writer.writeTextElement(QStringLiteral("offsetY"), QString::number(0));
    writer.writeTextElement(QStringLiteral("size"), QString::number(std::max(map->tileWidth(),map->tileHeight())));
    writer.writeStartElement(QStringLiteral("zone")); writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../..")); writer.writeEndElement();
    writeCellShape(writer, map);
    writer.writeEndElement(); // grid
}

static void writeClass(QXmlStreamWriter &writer, QString const &name, QString const& type)
{
    writer.writeStartElement(name);
    writer.writeAttribute(QStringLiteral("class"), type);
    writer.writeEndElement();
}

void RpMapPlugin::writeTokenMap(QXmlStreamWriter &writer, Tiled::Map const* map)
{
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();
//    const QColor backgroundColor = map->backgroundColor();
    writer.writeStartElement(QStringLiteral("tokenMap"));

    for (Layer *layer : map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            for (int y = 0; y < tileLayer->height(); ++y) {
                for (int x = 0; x < tileLayer->width(); ++x) {
                    Cell cell = tileLayer->cellAt(x, y);
                    Tile const* tile = cell.tile();
                    if (!tile)
                        continue;

                    static constexpr uint16_t rotation[8] = { 270, 270, 270, 90, 0, 0, 180, 180 };
                    // in addition to rotation
                    static constexpr bool flip_horiz[8] = { false, false, true, false,  true, false, false, true };
                    static constexpr bool flip_vert[8] = { false, true, false, false,  false, false, false, false };

                    uint8_t rot_index = (cell.flippedVertically() ? 1 : 0) | (cell.flippedHorizontally() ? 2 : 0) | (cell.flippedAntiDiagonally() ? 4 : 0);
                    //int tileid= t.tileId();
                    QUrl tileurl = tile->imageSource();
                    if (tileurl.isLocalFile()) {
                        QString tilepath = tileurl.toLocalFile();
                        auto it = filename2md5.find(tilepath);
                        if (it == filename2md5.end()) {
                            QFile file(tilepath);
                            if (file.open(QIODevice::ReadOnly)) {
                                QByteArray image = file.readAll();
                                QByteArray md5bin = QCryptographicHash::hash(image, QCryptographicHash::Md5);
                                QString md5string = md5bin.toHex();
                                it = filename2md5.insert(tilepath, md5string);
                                // remember the first element (tile) referencing this file
                                first_used_md5.push_back(number_of_tiles);
                            }
                            else
                                continue;
                        }
                        assert(it != filename2md5.end());
                        QString md5 = it.value();
                        writeTile(writer, x*tileWidth, y*tileHeight, tile, rotation[rot_index], md5, flip_horiz[rot_index], flip_vert[rot_index]);
                        ++number_of_tiles;
                    }
                }
            }
            break; // only output first layer (for now)
        }
    }
    //writeTile(writer, 400, 300, QStringLiteral("token"), 180);
    //writeTile(writer, 400, 600, QStringLiteral("token2"), 0);

    writer.writeEndElement(); // tokenMap
}

void RpMapPlugin::writeTokenOrderedList(QXmlStreamWriter &writer)
{
    writer.writeStartElement(QStringLiteral("tokenOrderedList"));
    writer.writeAttribute(QStringLiteral("class"), QStringLiteral("linked-list"));

    writer.writeStartElement(QStringLiteral("net.rptools.maptool.model.Token"));
    writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../tokenMap/entry/net.rptools.maptool.model.Token"));
    writer.writeEndElement(); // Token
    for (uint32_t i = 1; i < number_of_tiles; ++i) {
        writer.writeStartElement(QStringLiteral("net.rptools.maptool.model.Token"));
        writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../tokenMap/entry[")
                              +QString::number(i+1)
                              +QStringLiteral("]/net.rptools.maptool.model.Token"));
        writer.writeEndElement(); // Token
    }

    writer.writeEndElement(); // tokenOrderedList
}

static void writeZone2(QXmlStreamWriter &writer)
{
    writer.writeStartElement(QStringLiteral("initiativeList"));
    writer.writeEmptyElement(QStringLiteral("tokens"));
    writer.writeTextElement(QStringLiteral("current"), QString::number(-1));
    writer.writeTextElement(QStringLiteral("round"), QString::number(-1));
    writer.writeStartElement(QStringLiteral("zoneId")); writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../id")); writer.writeEndElement();
    writer.writeTextElement(QStringLiteral("fullUpdate"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("hideNPC"), QStringLiteral("false"));
    writer.writeEndElement(); // initiativeList

    writer.writeStartElement(QStringLiteral("exposedArea"));
    writer.writeEmptyElement(QStringLiteral("curves"));
    writer.writeEndElement(); // exposedArea
    writer.writeTextElement(QStringLiteral("hasFog"), QStringLiteral("false"));

    writer.writeStartElement(QStringLiteral("fogPaint"));
    writer.writeAttribute(QStringLiteral("class"), QStringLiteral("net.rptools.maptool.model.drawing.DrawableColorPaint"));
    writer.writeTextElement(QStringLiteral("color"), QString::number(-16777216));
    writer.writeEndElement(); // fogPaint

    writer.writeStartElement(QStringLiteral("topology"));
    writer.writeStartElement(QStringLiteral("curves"));
    writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../exposedArea/curves"));
    writer.writeEndElement(); // curves
    writer.writeEndElement(); // topology
    writer.writeStartElement(QStringLiteral("topologyTerrain"));
    writer.writeStartElement(QStringLiteral("curves"));
    writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../exposedArea/curves"));
    writer.writeEndElement(); // curves
    writer.writeEndElement(); // topologyTerrain

    writer.writeStartElement(QStringLiteral("backgroundPaint"));
    writer.writeAttribute(QStringLiteral("class"), QStringLiteral("net.rptools.maptool.model.drawing.DrawableColorPaint"));
    writer.writeTextElement(QStringLiteral("color"), QString::number(-16777216));
    writer.writeEndElement(); // backgroundPaint

    writer.writeStartElement(QStringLiteral("boardPosition"));
    writer.writeTextElement(QStringLiteral("x"), QString::number(0));
    writer.writeTextElement(QStringLiteral("y"), QString::number(0));
    writer.writeEndElement(); // boardPosition

    writer.writeTextElement(QStringLiteral("drawBoard"), QStringLiteral("true"));
    writer.writeTextElement(QStringLiteral("boardChanged"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("name"), QStringLiteral("Tiled export"));
    writer.writeTextElement(QStringLiteral("isVisible"), QStringLiteral("true"));
    writer.writeTextElement(QStringLiteral("visionType"), QStringLiteral("OFF"));
    writer.writeTextElement(QStringLiteral("tokenSelection"), QStringLiteral("ALL"));
    writer.writeTextElement(QStringLiteral("height"), QString::number(0));
    writer.writeTextElement(QStringLiteral("width"), QString::number(0));
}

void RpMapPlugin::writeMap(QXmlStreamWriter &writer, Tiled::Map const* map)
{
    writer.writeStartElement(QStringLiteral("zone"));
    writer.writeTextElement(QStringLiteral("creationTime"), QString::number(QDateTime::currentMSecsSinceEpoch()));
    writeGUID(writer, QStringLiteral("id"), QUuid::createUuid());
    writeGrid(writer, map);
    writer.writeTextElement(QStringLiteral("gridColor"), QString::number(-16777216));
    writer.writeTextElement(QStringLiteral("imageScaleX"), QString::number(1.0));
    writer.writeTextElement(QStringLiteral("imageScaleY"), QString::number(1.0));
    writer.writeTextElement(QStringLiteral("tokenVisionDistance"), QString::number(1000));
    writer.writeTextElement(QStringLiteral("unitsPerCell"), QString::number(5.0));
    writer.writeTextElement(QStringLiteral("aStarRounding"), QStringLiteral("NONE"));
    writer.writeTextElement(QStringLiteral("topologyMode"), QStringLiteral("COMBINED"));
    writeClass(writer, QStringLiteral("drawables"), QStringLiteral("linked-list"));
    writeClass(writer, QStringLiteral("gmDrawables"), QStringLiteral("linked-list"));
    writeClass(writer, QStringLiteral("objectDrawables"), QStringLiteral("linked-list"));
    writeClass(writer, QStringLiteral("backgroundDrawables"), QStringLiteral("linked-list"));
    writeClass(writer, QStringLiteral("labels"), QStringLiteral("linked-hash-map"));
    writeTokenMap(writer, map);
    writer.writeStartElement(QStringLiteral("exposedAreaMeta"));
    writer.writeEndElement(); // exposedAreaMeta
    writeTokenOrderedList(writer);
    writeZone2(writer); // some arbitrary data
    writer.writeEndElement(); // zone

    writer.writeStartElement(QStringLiteral("assetMap"));
    for (auto i: first_used_md5) {
        writer.writeStartElement(QStringLiteral("entry"));
        writer.writeStartElement(QStringLiteral("net.rptools.lib.MD5Key"));
        QString item;
        if (i > 0)
            item = QStringLiteral("[") + QString::number(i+1) + QStringLiteral("]");
        writer.writeAttribute(QStringLiteral("reference"), QStringLiteral("../../../zone/tokenMap/entry")
                              +item
                              +QStringLiteral("/net.rptools.maptool.model.Token/imageAssetMap/entry/net.rptools.lib.MD5Key"));
        writer.writeEndElement(); // net.rptools.lib.MD5Key
        writer.writeEmptyElement(QStringLiteral("null"));
        writer.writeEndElement(); // entry
    }
    writer.writeEndElement(); // assetMap
}

bool RpMapPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    filename2md5.clear();
    first_used_md5.clear();
    number_of_tiles = 0;

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
            writer.writeEndElement(); // PersistedMap
            writer.writeEndDocument();
            archive.writeFile(QStringLiteral("content.xml"), content);
        }
        archive.close();
        return true;
    }
    return false;
}

} // namespace RpMap
