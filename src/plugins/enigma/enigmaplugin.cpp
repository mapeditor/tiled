/*
 * Enigma Tiled Plugin
 * Copyright 2022, Kartik Shrivastava <shrivastavakartik19@gmail.com>
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

#include "enigmaplugin.h"
#include "egm.h"
#include "tmx.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace egm;

namespace Enigma {

EnigmaPlugin::EnigmaPlugin()
{
  std::cout << "Constructor called" << std::endl;
}

std::unique_ptr<Tiled::Map> EnigmaPlugin::read(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open (QIODevice::ReadOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    // default to values of the original Flare alpha game.
    Map::Parameters mapParameters;
    mapParameters.orientation = Map::Isometric;
    mapParameters.width = 256;
    mapParameters.height = 256;
    mapParameters.tileWidth = 64;
    mapParameters.tileHeight = 32;

    auto map = std::make_unique<Map>(mapParameters);

    std::cout << "Trying to read egm file" << std::endl;

    EGMFileFormat egmFileFormat(NULL);

    std::filesystem::path fPath(fileName.toStdString());
    std::unique_ptr<egm::Project> project = egmFileFormat.LoadProject(fPath);
    if(project)
      std::cout << "Object group size " << project->game().root().room().objectgroups_size() << std::endl;
    else
      std::cout << "Project is null" << std::endl;

    return map;
}

bool EnigmaPlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".egm"), Qt::CaseInsensitive);
}

QString EnigmaPlugin::nameFilter() const
{
    return tr("EGM project files (*.egm)");
}

QString EnigmaPlugin::shortName() const
{
    return QStringLiteral("egm");
}

QString EnigmaPlugin::errorString() const
{
    return mError;
}

bool EnigmaPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    return true;
}

}
