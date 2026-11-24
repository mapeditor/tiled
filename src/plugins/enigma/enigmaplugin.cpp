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

std::unique_ptr<Tiled::Map> EnigmaPlugin::read(const QString &fileName)
{
    QFile file(fileName);
    QDir dirPath;

    if (!file.open (QIODevice::ReadOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    dirPath.setPath(fileName);
    std::unique_ptr<Map> map;

    EGMFileFormat egmFileFormat(NULL);

    std::filesystem::path fPath(fileName.toStdString());
    std::unique_ptr<egm::Project> project = egmFileFormat.LoadProject(fPath);

    if(project) {
      std::unordered_map<std::string, buffers::resources::Background *> backgroundNamePtrMap;
      readBackgrounds(project->mutable_game()->mutable_root(), backgroundNamePtrMap);

      buffers::resources::EGMRoom *egmRoom = NULL;
      readRoom(project->mutable_game()->mutable_root(), egmRoom);

      if(egmRoom) {
        Map::Parameters mapParameters;

        mapParameters.orientation = Tiled::orientationFromString(egmRoom->orientation().c_str());
        mapParameters.renderOrder = Tiled::renderOrderFromString(egmRoom->renderorder().c_str());

        // corrent width height of map which was updated in egm to suit RGM rendering
        if(mapParameters.orientation == Map::Orientation::Orthogonal || mapParameters.orientation == Map::Orientation::Isometric) {
          mapParameters.width = egmRoom->width() / egmRoom->tilewidth();
          mapParameters.height = egmRoom->height() / egmRoom->tileheight();
        }
        else if(mapParameters.orientation == Map::Orientation::Hexagonal) {
          mapParameters.hexSideLength = egmRoom->hexsidelength();
          mapParameters.staggerAxis = Tiled::staggerAxisFromString(egmRoom->staggeraxis().c_str());
          mapParameters.staggerIndex = Tiled::staggerIndexFromString(egmRoom->staggerindex().c_str());

          if(mapParameters.staggerAxis == Map::StaggerAxis::StaggerX) {
            mapParameters.width = (2 * egmRoom->width() - egmRoom->hexsidelength()) / (3 * egmRoom->hexsidelength());
            mapParameters.height = (egmRoom->height() - egmRoom->hexsidelength()) / egmRoom->tileheight();
          }
          else if(mapParameters.staggerAxis == Map::StaggerAxis::StaggerY) {
            mapParameters.width = (egmRoom->width() - egmRoom->hexsidelength()) / egmRoom->tilewidth();
            mapParameters.height = (2 * egmRoom->height() - egmRoom->hexsidelength()) / (3 * egmRoom->hexsidelength());
          }
        }
        else if(mapParameters.orientation == Map::Orientation::Staggered){
          mapParameters.staggerAxis = Tiled::staggerAxisFromString(egmRoom->staggeraxis().c_str());
          mapParameters.staggerIndex = Tiled::staggerIndexFromString(egmRoom->staggerindex().c_str());

          if(mapParameters.staggerAxis == Map::StaggerAxis::StaggerX) {
            mapParameters.width = egmRoom->width() * 2 / egmRoom->tilewidth();
            mapParameters.height = egmRoom->height() / egmRoom->tileheight();
          }
          else if(mapParameters.staggerAxis == Map::StaggerAxis::StaggerY) {
            mapParameters.width = egmRoom->width() / egmRoom->tilewidth();
            mapParameters.height = egmRoom->height() * 2 / egmRoom->tileheight();
          }
        }

        mapParameters.tileWidth = egmRoom->tilewidth();
        mapParameters.tileHeight = egmRoom->tileheight();
        mapParameters.infinite = egmRoom->infinite();

        if(egmRoom->has_parallaxoriginx())
          mapParameters.parallaxOrigin.setX(egmRoom->parallaxoriginx());
        if(egmRoom->has_parallaxoriginy())
          mapParameters.parallaxOrigin.setY(egmRoom->parallaxoriginy());

        if(egmRoom->has_color()) {
          const QString backgroundColor = parseIntColor(egmRoom->color());
          if (QColor::isValidColor(backgroundColor))
              mapParameters.backgroundColor = QColor(backgroundColor);
        }

        map = std::make_unique<Map>(mapParameters);

        if(egmRoom->has_nextlayerid())
          map->setNextLayerId(egmRoom->nextlayerid());
        if(egmRoom->has_nextobjectid())
          map->setNextObjectId(egmRoom->nextobjectid());

        GidMapper gidMapper;
        for(int i=0; i<egmRoom->tilesets_size(); ++i) {
          addTileset(egmRoom->tilesets(i), map.get(), gidMapper, backgroundNamePtrMap);
        }

        for(int i=0; i<egmRoom->objectgroups_size(); ++i) {
          addObjectGroup(egmRoom->objectgroups(i), map.get(), gidMapper);
        }

        for(int i=0; i<egmRoom->tilelayers_size(); ++i) {
          addTileLayer(egmRoom->tilelayers(i), map.get(), gidMapper);
        }
      }
    }
    else
      std::cout << "Project is null" << std::endl;

    return map;
}

int EnigmaPlugin::readBackgrounds(buffers::TreeNode *root, std::unordered_map<std::string,
                                  buffers::resources::Background *> &backgroundNamePtrMap)
{
  if(root->type_case() == TypeCase::kBackground) {
      if(root->background().name().empty()) {
        std::cout << "Error background name not set properly" << std::endl;
        return -1;
      }
      backgroundNamePtrMap[root->background().name()] = root->mutable_background();
      return 0;
  }

  for (int i=0; i < root->folder().children_size(); ++i) {
    int res = readBackgrounds(root->mutable_folder()->mutable_children(i), backgroundNamePtrMap);
    if (res) return res;
  }

  return 0; // success
}

int EnigmaPlugin::readRoom(buffers::TreeNode *root, buffers::resources::EGMRoom *&egmRoom)
{
  if(root->type_case() == TypeCase::kRoom) {
      egmRoom = root->mutable_room();
      return 0;
  }

  for (int i=0; i < root->folder().children_size(); ++i) {
    int res = readRoom(root->mutable_folder()->mutable_children(i), egmRoom);
    if (res) return res;
  }

  return 0; // success
}

QString EnigmaPlugin::parseIntColor(int intColor)
{
  // convert int color to hex string
  std::string hexColorStr;
  std::stringstream strStream;
  strStream << std::hex << intColor;
  hexColorStr = strStream.str();
  hexColorStr = "#" + hexColorStr;
  QString backgroundColor = hexColorStr.c_str();

  return backgroundColor;
}

void EnigmaPlugin::addTileset(const buffers::resources::EGMRoom_Tileset &tilesetProto, Map *map, GidMapper &gidMapper,
                              std::unordered_map<std::string, buffers::resources::Background *> backgroundNamePtrMap)
{
  SharedTileset tileset;

  const QString absoluteSource = tilesetProto.source().c_str();
  const unsigned firstGid = tilesetProto.firstgid();

  // read internal/embedded tileset
  if(absoluteSource.isEmpty()) {
    const QString name = tilesetProto.name().c_str();

    // TODO: Add <tile> element support to internal tileset
    // if there is no direct match of name in backgroundNamePtrMap then, the tileset was a collection of individual
    // images (tile) prior to conversion to background, so iterate over all backgrounds starting with name
    if(backgroundNamePtrMap.find(tilesetProto.name()) == backgroundNamePtrMap.end()) {

    }
    // otherwise this internal tileset is made out of a single image so mostly straightforward thingy
    else {
      const int tileWidth = tilesetProto.tilewidth();
      const int tileHeight = tilesetProto.tileheight();

      if (tileWidth < 0 || tileHeight < 0 || firstGid == 0) {//== 0 && !mReadingExternalTileset)) {
        std::cout <<"Invalid tileset parameters for tileset " << name.toStdString();
        return;
      }

      buffers::resources::Background *bgProto = backgroundNamePtrMap[name.toStdString()];

      const int tileSpacing = bgProto->horizontal_spacing(); // also same as bgProto.vertical_spacing
      const int margin = bgProto->horizontal_offset(); // also same as bgProto.vertical_offset
      const int columns = bgProto->columns();

      tileset = Tileset::create(name, tileWidth, tileHeight, tileSpacing, margin);

      tileset->setColumnCount(columns);

      // TODO: Add properties as required
      // tileset->setClassName(className);
      // if (QColor::isValidColor(backgroundColor))
      //     tileset->setBackgroundColor(QColor(backgroundColor));
      // tileset->setObjectAlignment(alignmentFromString(alignment));
      // tileset->setTileRenderSize(Tileset::tileRenderSizeFromString(tileRenderSize));
      // tileset->setFillMode(Tileset::fillModeFromString(fillMode));

      ImageReference image;
      image.source = QUrl(bgProto->image().c_str());
      image.size = QSize(bgProto->width(), bgProto->height());

      tileset->setImageReference(image);
      tileset->setImageSource(bgProto->image().c_str());

      // TODO: Add other properties of internal tilesets
    }
  }
  // read external tileset
  else {
    QString error;
    tileset = TilesetManager::instance()->loadTileset(absoluteSource, &error);

    if (!tileset) {
        // Insert a placeholder to allow the map to load
        tileset = Tileset::create(QFileInfo(absoluteSource).completeBaseName(), 32, 32);
        tileset->setFileName(absoluteSource);
        tileset->setStatus(LoadingError);
    }
  }

  if(tileset) {
    gidMapper.insert(tilesetProto.firstgid(), tileset);
    map->addTileset(tileset);
    // Try to load the tileset images for embedded tilesets
    tileset->loadImage();
  }
}

void EnigmaPlugin::addObjectGroup(const buffers::resources::EGMRoom_ObjectGroup &objGrp, Map *map, GidMapper &gidMapper)
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
    addObject(objGrp.objects(i), objectGroupLayer.get(), gidMapper);
  }

  map->addLayer(std::move(objectGroupLayer));
}

void EnigmaPlugin::addObject(const buffers::resources::EGMRoom_ObjectGroup_Object &obj, Layer *objectGroupLayer,
                             GidMapper &gidMapper)
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
      mapObject->setCell(cellForGid(obj.gid(), gidMapper));
      mapObject->setPropertyChanged(MapObject::CellProperty);
  }

  objectGroupLayer->asObjectGroup()->addObject(std::move(mapObject));
}

Cell EnigmaPlugin::cellForGid(unsigned gid, GidMapper &gidMapper)
{
    bool ok;
    const Cell result = gidMapper.gidToCell(gid, ok);

    if (!ok) {
        if (gidMapper.isEmpty())
            std::cout << "Tile used but no tilesets specified" << std::endl;
        else
            std::cout << "Invalid tile: " << gid << std::endl;
    }

    return result;
}

void EnigmaPlugin::addTileLayer(const buffers::resources::EGMRoom_TileLayer &tileLayer, Map *map, GidMapper &gidMapper)
{
  const QString name = tileLayer.name().c_str();

  const int x = tileLayer.x();
  const int y = tileLayer.y();
  const int width = tileLayer.width();
  const int height = tileLayer.height();

  std::unique_ptr<Tiled::TileLayer> tileLayerTiled = std::make_unique<TileLayer>(name, x, y, width, height);

  tileLayerTiled->setClassName(tileLayer.class_().c_str());

  if(tileLayer.has_id())
    tileLayerTiled->setId(tileLayer.id());

  if(tileLayer.has_opacity())
    tileLayerTiled->setOpacity(tileLayer.opacity());

  if(tileLayer.has_tintcolor()) {
    const QString tintColor = tileLayer.tintcolor().c_str();
    if (!tintColor.isEmpty())
        tileLayerTiled->setTintColor(QColor(tintColor));
  }

  if(tileLayer.has_visible())
    tileLayerTiled->setVisible(tileLayer.visible());

  if(tileLayer.has_locked())
    tileLayerTiled->setLocked(tileLayer.locked());

  if(tileLayer.has_offsetx() && tileLayer.has_offsety()) {
    const QPointF offset(tileLayer.offsetx(), tileLayer.offsety());
    tileLayerTiled->setOffset(offset);
  }

  QPointF parallaxFactor(1.0, 1.0);
  if(tileLayer.has_parallaxx())
    parallaxFactor.setX(tileLayer.parallaxx());
  if(tileLayer.has_parallaxy())
    parallaxFactor.setX(tileLayer.parallaxy());

  tileLayerTiled->setParallaxFactor(parallaxFactor);

  if(tileLayer.has_data()) {
    const QString encoding = tileLayer.data().encoding().c_str();
    const QString compression = tileLayer.data().compression().c_str();

    Map::LayerDataFormat layerDataFormat;
    if (encoding.isEmpty()) {
        layerDataFormat = Map::XML;
    } else if (encoding == QLatin1String("csv")) {
        layerDataFormat = Map::CSV;
    } else if (encoding == QLatin1String("base64")) {
        if (compression.isEmpty()) {
            layerDataFormat = Map::Base64;
        } else if (compression == QLatin1String("gzip")) {
            layerDataFormat = Map::Base64Gzip;
        } else if (compression == QLatin1String("zlib")) {
            layerDataFormat = Map::Base64Zlib;
        } else if (compression == QLatin1String("zstd")) {
            layerDataFormat = Map::Base64Zstandard;
        } else {
            std::cout << "Compression method " << compression.toStdString() << " not supported" << std::endl;
            return;
        }
    } else {
        std::cout << "Unknown encoding: " << encoding.toStdString() << std::endl;
        return;
    }

    map->setLayerDataFormat(layerDataFormat);

    // cell x and y positions
    int x = 0;
    int y = 0;

    // iterate over all tiles present inside data node
    for(int i = 0; i < tileLayer.data().tiles_size(); ++i, ++x) {
      if(x == height) {
        x=0;
        y++;
      }

      if(!tileLayer.data().tiles(i).has_gid())
        continue;

      unsigned int gid = tileLayer.data().tiles(i).gid();

      tileLayerTiled->setCell(x, y, cellForGid(gid, gidMapper));
    }

    // iterate over all chunks and subsequent tiles present inside it
    for(int i = 0; i < tileLayer.data().chunks_size(); ++i) {
      int xStart = tileLayer.data().chunks(i).x();
      x = xStart;
      y = tileLayer.data().chunks(i).y();

      int chunkHeight = tileLayer.data().chunks(i).height();

      for(int j = 0; j < tileLayer.data().chunks(i).tiles_size(); ++j, ++x) {
        if(x == xStart + chunkHeight) {
          x = xStart;
          y++;
        }

        if(!tileLayer.data().chunks(i).tiles(j).has_gid())
          continue;

        unsigned int gid = tileLayer.data().chunks(i).tiles(j).gid();

        tileLayerTiled->setCell(x, y, cellForGid(gid, gidMapper));
      }
    }
  }
  map->addLayer(std::move(tileLayerTiled));
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

    return false;
}

}
