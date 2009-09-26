/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tsxtilesetreader.h"

#include "tile.h"
#include "tileset.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QXmlStreamReader>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

QDir tilesetDir;     // The directory from which the tileset is being loaded

/**
 * A stream based tileset reader for the TSX format.
 */
class TsxReader : public QXmlStreamReader
{
public:
    TsxReader();

    Tileset *read(QIODevice *device);

private:
    bool readNextStartElement();
    void skipCurrentElement();
    void readTileset();
    void readProperties();
    void readProperty();
    void readTile();
    void readTilesetImage();

    QMap<QString, QString> mProperties;
    Tileset *mTileset;
};

} // anonymous namespace

TsxReader::TsxReader():
    mTileset(0)
{
}

Tileset *TsxReader::read(QIODevice *device)
{
    setDevice(device);

    mTileset = 0;

    if (readNextStartElement() && name() == "tileset")
        readTileset();
    else
        raiseError(QObject::tr("Not a tileset file."));

    return mTileset;
}

void TsxReader::skipCurrentElement()
{
    while (readNextStartElement())
        skipCurrentElement();
}

bool TsxReader::readNextStartElement()
{
    while (readNext() != Invalid) {
        if (isEndElement())
            return false;
        else if (isStartElement())
            return true;
    }
    return false;
}

void TsxReader::readTileset()
{
    Q_ASSERT(isStartElement() && name() == "tileset");

    const QXmlStreamAttributes attr = attributes();
    QString tilesetName = attr.value(QLatin1String("name")).toString();
    const int tileWidth =
        attr.value(QLatin1String("tilewidth")).toString().toInt();
    const int tileHeight =
        attr.value(QLatin1String("tileheight")).toString().toInt();
    const int tileSpacing =
        attr.value(QLatin1String("spacing")).toString().toInt();
    const int margin =
        attr.value(QLatin1String("margin")).toString().toInt();

    mTileset = new Tileset(tilesetName, tileWidth, tileHeight,
                           tileSpacing, margin);

    while (readNextStartElement()) {
        if (name() == "tile")
            readTile();
        else if (name() == "image")
            readTilesetImage();
        else
            skipCurrentElement();
    }
}

void TsxReader::readProperties()
{
    Q_ASSERT(isStartElement() && name() == "properties");

    mProperties.clear();

    while (readNextStartElement()) {
        if (name() == "property")
            readProperty();
        else
            skipCurrentElement();
    }
}

void TsxReader::readProperty()
{
    Q_ASSERT(isStartElement() && name() == "property");

    const QXmlStreamAttributes attr = attributes();
    QString propertyName = attr.value(QLatin1String("name")).toString();
    QString propertyValue = attr.value(QLatin1String("value")).toString();

    while (readNext() != Invalid) {
        if (isEndElement())
            break;
        else if (isCharacters() && !isWhitespace() && propertyValue.isEmpty())
            propertyValue = text().toString();
        else if (isStartElement())
            skipCurrentElement();
    }

    mProperties.insert(propertyName, propertyValue);
}

void TsxReader::readTile()
{
    Q_ASSERT(isStartElement() && name() == "tile");

    const QXmlStreamAttributes attr = attributes();
    const int id = attr.value(QLatin1String("id")).toString().toInt();

    if (id < 0 || id >= mTileset->tileCount()) {
        raiseError(QObject::tr("Invalid tile ID: %1").arg(id));
        return;
    }

    Tile *tile = mTileset->tileAt(id);

    // TODO: Add support for individual tiles (then it needs to be added here)

    while (readNextStartElement()) {
        if (name() == "properties") {
            readProperties();
            tile->properties()->unite(mProperties);
        } else {
            skipCurrentElement();
        }
    }
}

void TsxReader::readTilesetImage()
{
    Q_ASSERT(isStartElement() && name() == "image");

    const QXmlStreamAttributes attr = attributes();
    QString source = attr.value(QLatin1String("source")).toString();
    QString trans = attr.value(QLatin1String("trans")).toString();

    if (!trans.isEmpty()) {
        if (!trans.startsWith(QLatin1Char('#')))
            trans.prepend(QLatin1Char('#'));
        mTileset->setTransparentColor(QColor(trans));
    }

    if (QDir::isRelativePath(source))
        source = tilesetDir.absolutePath() + QDir::separator() + source;

    mTileset->loadFromImage(source);

    skipCurrentElement();
}

TsxTilesetReader::TsxTilesetReader()
{
}

Tileset *TsxTilesetReader::readTileset(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
        mError = QObject::tr("File not found: %1.").arg(fileName);
        return 0;
    }
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        mError = QObject::tr("Cannot read file %1:\n%2.")
            .arg(fileName)
            .arg(file.errorString());
        return 0;
    }

    tilesetDir = QFileInfo(file).dir();

    TsxReader reader;
    Tileset *tileset = reader.read(&file);

    if (reader.hasError()) {
        //const int lineNumber = reader.lineNumber();
        //const int columnNumber = reader.columnNumber();
        mError = reader.errorString();
    }

    if (tileset)
        tileset->setFileName(fileName);

    return tileset;
}

QString TsxTilesetReader::errorString() const
{
    return mError;
}
