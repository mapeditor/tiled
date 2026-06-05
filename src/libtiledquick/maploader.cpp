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

#include "mapreader.h"
#include "tiled.h"

using namespace TiledQuick;

MapLoader::MapLoader(QObject *parent)
    : QObject(parent)
{
}

MapLoader::~MapLoader() = default;

void MapLoader::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;

    Tiled::MapReader mapReader;

    auto map = mapReader.readMap(Tiled::urlToLocalFileOrQrc(source));
    auto status = map ? Ready : Error;
    auto error = map ? QString() : mapReader.errorString();
    auto editableMap = map ? std::make_unique<Tiled::EditableMap>(std::move(map)) : nullptr;

    const bool mapDiff = m_editableMap != editableMap;
    const bool statusDiff = m_status != status;
    const bool errorDiff = m_error != error;

    m_editableMap = std::move(editableMap);
    m_status = status;
    m_error = error;

    emit sourceChanged(source);

    if (mapDiff)
        emit mapChanged(m_editableMap.get());
    if (statusDiff)
        emit statusChanged(status);
    if (errorDiff)
        emit errorChanged(error);
}
