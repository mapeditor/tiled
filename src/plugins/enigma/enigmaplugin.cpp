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
#include "layer.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "tilesetmanager.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace egm;

using TypeCase = buffers::TreeNode::TypeCase;

namespace Enigma {

EnigmaPlugin::EnigmaPlugin()
{
}

Cell EnigmaPlugin::cellForGid(unsigned gid, GidMapper &mGidMapper)
{
    bool ok;
    const Cell result = mGidMapper.gidToCell(gid, ok);

    if (!ok) {
        if (mGidMapper.isEmpty())
            std::cout << "Tile used but no tilesets specified" << std::endl;
        else
            std::cout << "Invalid tile: " << gid << std::endl;
    }

    return result;
}

void EnigmaPlugin::addObject(const buffers::resources::EGMRoom_ObjectGroup_Object &obj, Layer *objectGroupLayer, GidMapper &mGidMapper)
{
  const QString name = obj.name().c_str();
  const QString classname = obj.class_().c_str();

  const qreal x = obj.x();
  // correct the origin which was adjusted in RGM
  const qreal y = obj.y() + obj.height();
  const qreal width = obj.width();
  const qreal height = obj.height();
  const QPointF pos(x, y);
  const QSizeF size(width, height);

  std::unique_ptr<Tiled::MapObject> mapObject = std::make_unique<Tiled::MapObject>(name, classname, pos, size);
  mapObject->setId(obj.id());

  if (obj.has_rotation()) {
      mapObject->setRotation(obj.rotation());
      mapObject->setPropertyChanged(MapObject::RotationProperty);
  }

  if (obj.has_gid()) {
      mapObject->setCell(cellForGid(obj.gid(), mGidMapper));
      mapObject->setPropertyChanged(MapObject::CellProperty);
  }

  objectGroupLayer->asObjectGroup()->addObject(std::move(mapObject));
}

void EnigmaPlugin::addObjectGroup(const buffers::resources::EGMRoom_ObjectGroup &objGrp, Map *map, GidMapper &mGidMapper)
{
  const QString name = objGrp.name().c_str();
  std::unique_ptr<Layer> objectGroupLayer = std::make_unique<ObjectGroup>(name, 0, 0);

  objectGroupLayer->setId(objGrp.id());

  QPointF parallaxFactor(1.0, 1.0);
  if(objGrp.has_parallaxx())
    parallaxFactor.setX(objGrp.parallaxx());
  if(objGrp.has_parallaxy())
    parallaxFactor.setY(objGrp.parallaxy());
  objectGroupLayer->setParallaxFactor(parallaxFactor);

  if(objGrp.has_opacity())
    objectGroupLayer->setOpacity(objGrp.opacity());

  if(objGrp.has_visible())
    objectGroupLayer->setVisible(objGrp.visible());

  for(int i=0; i<objGrp.objects_size(); ++i) {
    addObject(objGrp.objects(i), objectGroupLayer.get(), mGidMapper);
  }

  map->addLayer(std::move(objectGroupLayer));
}

SharedTileset EnigmaPlugin::readExternalTileset(const QString &source, QString *error)
{
    return TilesetManager::instance()->loadTileset(source, error);
}

void EnigmaPlugin::addTileset(const buffers::resources::EGMRoom_Tileset &tilesetProto, Map *map, GidMapper &mGidMapper)
{
  const QString absoluteSource = tilesetProto.source().c_str();
  QString error;
  SharedTileset tileset = readExternalTileset(absoluteSource, &error);

  if (!tileset) {
      // Insert a placeholder to allow the map to load
      tileset = Tileset::create(QFileInfo(absoluteSource).completeBaseName(), 32, 32);
      tileset->setFileName(absoluteSource);
      tileset->setStatus(LoadingError);
  }

  if (tileset)// && !mReadingExternalTileset)
      mGidMapper.insert(tilesetProto.firstgid(), tileset);

  map->addTileset(tileset);
}

int EnigmaPlugin::flattenTree(const buffers::TreeNode &root, buffers::resources::EGMRoom &egmRoom)
{
  if(root.type_case() == TypeCase::kRoom) {
      egmRoom = root.room();
      return 0;
  }

  for (int i=0; i < root.folder().children_size(); ++i) {
    int res = flattenTree(root.folder().children(i), egmRoom);
    if (res) return res;
  }

  return 0; // success
}

std::unique_ptr<Tiled::Map> EnigmaPlugin::read(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open (QIODevice::ReadOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    std::unique_ptr<Map> map;

    EGMFileFormat egmFileFormat(NULL);

    std::filesystem::path fPath(fileName.toStdString());
    std::unique_ptr<egm::Project> project = egmFileFormat.LoadProject(fPath);

    if(project) {
      buffers::resources::EGMRoom egmRoom = project->game().root().room();
      flattenTree(project->game().root(), egmRoom);

      Map::Parameters mapParameters;

      mapParameters.orientation = Tiled::orientationFromString(egmRoom.orientation().c_str());
      mapParameters.renderOrder = Tiled::renderOrderFromString(egmRoom.renderorder().c_str());
      mapParameters.width = egmRoom.width() / egmRoom.tilewidth();
      mapParameters.height = egmRoom.height() / egmRoom.tileheight();
      mapParameters.tileWidth = egmRoom.tilewidth();
      mapParameters.tileHeight = egmRoom.tileheight();
      mapParameters.infinite = egmRoom.infinite();

      QPointF parallaxOrigin(1.0, 1.0);
      if(egmRoom.has_parallaxoriginx())
        parallaxOrigin.setX(egmRoom.parallaxoriginx());
      if(egmRoom.has_parallaxoriginy())
        parallaxOrigin.setY(egmRoom.parallaxoriginy());
      mapParameters.parallaxOrigin = parallaxOrigin;

      // convert int color to hex string
      int intColor = egmRoom.color();
      std::string hexColorStr;
      std::stringstream strStream;
      strStream << std::hex << intColor;
      hexColorStr = strStream.str();
      hexColorStr = "#" + hexColorStr;
      const QString backgroundColor = hexColorStr.c_str();
      if (QColor::isValidColor(backgroundColor))
          mapParameters.backgroundColor = QColor(backgroundColor);

      map = std::make_unique<Map>(mapParameters);

      map->setNextLayerId(egmRoom.nextlayerid());
      map->setNextObjectId(egmRoom.nextobjectid());

      GidMapper mGidMapper;
      for(int i=0; i<egmRoom.tilesets_size(); ++i) {
        addTileset(egmRoom.tilesets(i), map.get(), mGidMapper);
      }

      for(int i=0; i<egmRoom.objectgroups_size(); ++i) {
        addObjectGroup(egmRoom.objectgroups(i), map.get(), mGidMapper);
      }
    }
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
