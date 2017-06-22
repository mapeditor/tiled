/*
 * JSON Tiled Plugin
 * Copyright 2017, Chase Warrington <spacechase0.and.cat@gmail.com>
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

#include "tbinplugin.h"

#include "tbin/Map.hpp"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <sstream>

namespace Tbin {

void JsonPlugin::initialize()
{
    addObject(new TbinMapFormat(this));
}


TbinMapFormat::TbinMapFormat(QObject *parent)
    : mSubFormat(subFormat)
{}

Tiled::Map *TbinMapFormat::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Binary)) {
        mError = tr("Could not open file for reading.");
        return nullptr;
    }

    QByteArray contents = file.readAll();
    QString data( contents );
    std::istringstream ss( data.toStdString() );

    tbin::Map tmap;
    Tiled::Map* map = new Tiled::Map();
    try
    {
        tmap.loadFromStream( ss );
    }
    catch ( std::exception& e )
    {
        mError = "Exception: " + e.what();
    }

    return map;
}

bool TbinMapFormat::write(const Tiled::Map *map, const QString &fileName)
{
    // ...

    return false;
}

QString TbinMapFormat::nameFilter() const
{
    return tr("Tbin map files (*.tbin)");
}

QString TbinMapFormat::shortName() const
{
    return QLatin1String("tbin");
}

bool TbinMapFormat::supportsFile(const QString &fileName) const
{
    // ...

    return false;
}

QString TbinMapFormat::errorString() const
{
    return mError;
}

} // namespace Tbin
