/*
 * maploader.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled Quick.
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

#include "maploader.h"

#include "map.h"
#include "mapreader.h"
#include "tileset.h"

using namespace TiledQuick;

MapLoader::MapLoader(QObject *parent)
    : QObject(parent)
    , m_map(0)
    , m_status(Null)
{
}

MapLoader::~MapLoader()
{
}

/*
    If \a url is a local file returns a path suitable for passing to QFile.
    Otherwise returns an empty string.
*/
static QString urlToLocalFileOrQrc(const QUrl &url)
{
    if (url.scheme().compare(QLatin1String("qrc"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty())
            return QLatin1Char(':') + url.path();
        return QString();
    }

#if defined(Q_OS_ANDROID)
    else if (url.scheme().compare(QLatin1String("assets"), Qt::CaseInsensitive) == 0) {
        if (url.authority().isEmpty())
            return url.toString();
        return QString();
    }
#endif

    return url.toLocalFile();
}

void MapLoader::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;

    Tiled::MapReader mapReader;

    Tiled::Map *map = mapReader.readMap(urlToLocalFileOrQrc(source));
    Status status = map ? Ready : Error;
    QString error = map ? QString() : mapReader.errorString();

    const bool mapDiff = m_map != map;
    const bool statusDiff = m_status != status;
    const bool errorDiff = m_error != error;

    delete m_map;

    m_map = map;
    m_status = status;
    m_error = error;

    emit sourceChanged(source);

    if (mapDiff)
        emit mapChanged(map);
    if (statusDiff)
        emit statusChanged(status);
    if (errorDiff)
        emit errorChanged(error);
}
