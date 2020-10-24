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
#include <kzip.h>

#include <memory>

using namespace Tiled;

namespace RpMap {

RpMapPlugin::RpMapPlugin()
{
}

std::unique_ptr<Tiled::Map> RpMapPlugin::read(const QString &fileName)
{

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

bool RpMapPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)
    KZip archive(fileName);
    if (archive.open(QIODevice::WriteOnly))
    {
        QByteArray properties("Test");
        archive.writeFile(QStringLiteral("properties.xml"), properties, 0100644, QStringLiteral("owner"), QStringLiteral("group"));
        archive.close();
        return true;
    }
    return false;
}

} // namespace RpMap
