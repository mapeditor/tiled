/*
 * TBIN Tiled Plugin
 * Copyright 2017, Casey Warrington <spacechase0.and.cat@gmail.com>
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

#include "tbinmapformat.h"

#include "tbinplugin.h"
#include "tbin/Map.hpp"

#include "logginginterface.h"
#include "map.h"

#include <QCoreApplication>
#include <QDir>
#include <QStringView>

#include <fstream>
#include <memory>

namespace Tbin {

TbinMapFormat::TbinMapFormat(QObject *parent)
    : Tiled::MapFormat(parent)
{
}

std::unique_ptr<Tiled::Map> TbinMapFormat::read(const QString &fileName)
{
    std::ifstream file( fileName.toStdString(), std::ios::in | std::ios::binary );
    if (!file) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    try
    {
        tbin::Map tmap;
        tmap.loadFromStream(file);

        const QDir fileDir(QFileInfo(fileName).dir());
        return TbinPlugin::fromTbin( tmap, fileDir );
    }
    catch (std::exception& e) {
        mError = tr((std::string("Exception: ") + e.what()).c_str());
        return nullptr;
    }
}

bool TbinMapFormat::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    try {
        const QDir fileDir(QFileInfo(fileName).dir());
        tbin::Map tmap = TbinPlugin::toTbin( map, fileDir );

        std::ofstream file(fileName.toStdString(), std::ios::trunc | std::ios::binary);
        if (!file) {
            mError = tr("Could not open file for writing");
            return false;
        }
        tmap.saveToStream(file);
        file.close();

        return true;
    }
    catch (std::exception& e)
    {
        mError = tr("Exception: %1").arg(tr(e.what()));
        return false;
    }
}

QString TbinMapFormat::nameFilter() const
{
    return tr("Tbin map files (*.tbin)");
}

QString TbinMapFormat::shortName() const
{
    return QStringLiteral("tbin");
}

bool TbinMapFormat::supportsFile(const QString &fileName) const
{
    std::ifstream file(fileName.toStdString(), std::ios::in | std::ios::binary);
    if (!file)
        return false;

    std::string magic(6, '\0');
    file.read(&magic[0], static_cast<std::streamsize>(magic.length()));

    return magic == "tBIN10";
}

QString TbinMapFormat::errorString() const
{
    return mError;
}

} // namespace Tbin
